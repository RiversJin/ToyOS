#include "fs.h"
#include "log.h"
#include "../file/file.h"
#include "../include/stdint.h"
#include "../include/stat.h"
#include "../buffer/buf.h"
#include "../lib/string.h"
#include "../sync/spinlock.h"
#include "../sync/sleeplock.h"
#include "../include/param.h"
#include "../printf.h"
#include "../proc/proc.h"
#define min(a, b) ((a) < (b) ? (a) : (b))
// 每个磁盘都得有一个超级块 但是我们只运行一个磁盘 这样的话这一个唯一的结构体就够了
struct superblock sb[1];
/**
 * @brief 读取super block
 * 
 * @param dev 
 * @param sb 
 */
static void readsb(int dev, struct superblock *sb){
    struct buf* bp;
    bp = bread(dev, 1);
    memmove(sb, bp->data, sizeof(*sb));
    brelease(bp);
}

void fsinit(int dev){
    readsb(dev, &sb[0]);
    if(sb[0].magic != FSMAGIC){
        panic("fsinit: invalid file system.\n");
    }
    initlog(dev, &sb[0]);
}
/**
 * @brief zero a block
 * 
 */
static void bzero(int dev, int bno){
    struct buf* bp = bread(dev, bno);
    memset(bp->data,0,BSIZE);
    log_write(bp);
    brelease(bp);
}
/**
 * @brief 分配一个磁盘块 并且返回的磁盘块是已清零的
 * 
 * @param dev 
 * @return uint32_t 
 */
static uint32_t balloc(int dev){
    for(int b = 0; b < sb[0].size; b += BPB){
        // 获得b对应的bitmap块
        struct buf* bp = bread(dev,BBLOCK(b,sb[0]));
        for(int bi = 0; bi<BPB && b + bi < sb[0].size; bi++){
            int m = 1 << (bi % 8);
            // 如果bitmap中对应于bi的块空闲
            if((bp->data[bi/8] & m) == 0){
                
                bp->data[bi/8] |= m; // 标记此块已使用
                log_write(bp);
                brelease(bp);
                bzero(dev, b + bi);
                return b + bi;
            }
        }
        brelease(bp);
    }
    panic("balloc: out of blocks.\n");
    return -1;
}

static void bfree(int dev, uint32_t b){
    struct buf *bp = bread(dev, BBLOCK(b, sb[0]));
    int bi = b % BPB;
    int m = 1 << (bi % 8);
    if((bp->data[bi/8] & m) == 0){
        panic("bfree: freeing free block.\n");
    }
    bp->data[bi/8] &= ~m;
    log_write(bp);
    brelease(bp);
}

struct itable {
    struct spinlock lock;
    struct inode inode[NINODE];
} itable;
/**
 * @brief inode init
 * 
 */
void iinit(){
    init_spin_lock(&itable.lock,"itable");
    for(int i=0; i<NINODE; i++){
        init_sleep_lock(&itable.inode[i].lock,"inode");
    }
}

static struct inode* iget(uint32_t dev, uint32_t inum);
/**
 * @brief 在磁盘上分配一个inode 并标记给定的type 返回一个没有锁定但已增加引用计数的inode
 * 
 * @param dev 
 * @param type 
 * @return struct inode* 
 */
struct inode* ialloc(uint32_t dev, uint16_t type){
    for(int inum = 1; inum < sb->ninodes; ++inum){
        struct buf *bp = bread(dev, IBLOCK(inum,sb[0]));
        struct dinode *dip = (struct dinode*)bp->data + inum%IPB;
        if(dip->type == 0){
            memset(dip, 0, sizeof(*dip));
            dip->type = type;
            log_write(bp);
            brelease(bp);
            return iget(dev,inum);
        }
        brelease(bp); 
    }
    panic("balloc: no available inodes.\n");
    return NULL;
}
/**
 * @brief 将一个被修改后的在内存中的inode 复制到磁盘
 * 必须在每次更改磁盘上的ip->xxx字段后调用
 * 调用者必须持有对应inode的锁
 * @param ip 
 */
void iupdate(struct inode *ip){
    struct buf *bp = bread(ip->dev, IBLOCK(ip->inum,sb[0]));
    struct dinode *dip = (struct dinode *)bp->data + ip->inum%IPB;
    dip->type = ip->type;
    dip->major = ip->major;
    dip->minor = ip->minor;
    dip->nlink = ip->nlink;
    dip->size = ip->size;
    memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
    log_write(bp);
    brelease(bp);
}
/**
 * @brief 找到设备上id为inum的inode 返回其在内存中的副本
 * 没有锁定 也没有从磁盘中同步
 * 
 * @param dev 
 * @param inum 
 * @return struct inode* 
 */
static struct inode * iget(uint32_t dev, uint32_t inum){
    acquire_spin_lock(&itable.lock);
    struct inode *empty = NULL;
    struct inode *ip = NULL;
    for(ip = &itable.inode[0]; ip < &itable.inode[NINODE]; ++ip){
        // 如果这个块恰好在内存中
        if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){
            ip->ref++;
            release_spin_lock(&itable.lock);
            return ip;
        }
        // 或者找到一个没人用的块
        if(empty == NULL && ip->ref == 0){
            empty = ip;
        }
    }
    if(empty == NULL){ 
        panic("iget: no available inodes.\n");
    }
    ip = empty;
    ip -> dev = dev;
    ip -> inum = inum;
    ip -> ref = 1;
    ip -> valid = 0;
    release_spin_lock(&itable.lock);
    return ip;
}
/**
 * @brief 增加指定inode的引用计数
 * 
 * @param ip 
 * @return struct inode* 
 */
struct inode* idup(struct inode *ip){
  acquire_spin_lock(&itable.lock);
  ip->ref++;
  release_spin_lock(&itable.lock);
  return ip;
}
/**
 * @brief 锁定指定的inode 如果此inode还没有从磁盘中读取的话
 * 就同步一下
 * 
 * @param ip 
 */
void ilock(struct inode *ip){
  struct buf *bp;
  struct dinode *dip;

  if(ip == 0 || ip->ref < 1)
    panic("ilock");

  acquire_sleep_lock(&ip->lock);

  if(ip->valid == 0){
    bp = bread(ip->dev, IBLOCK(ip->inum, sb[0]));
    dip = (struct dinode*)bp->data + ip->inum%IPB;
    ip->type = dip->type;
    ip->major = dip->major;
    ip->minor = dip->minor;
    ip->nlink = dip->nlink;
    ip->size = dip->size;
    memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
    brelease(bp);
    ip->valid = 1;
    if(ip->type == 0)
      panic("ilock: no type");
  }
}
/**
 * @brief 解锁指定inode
 * 
 * @param ip 
 */
void iunlock(struct inode *ip){
    if(ip == NULL || !is_current_cpu_holing_sleep_lock(&ip->lock) || ip->ref < 1){
        panic("ilock.\n");
    }
    release_sleep_lock(&ip->lock);
}
/**
 * @brief 截断对应inode(即清空其中内容)
 * 调用方必须持有ip->lock
 * 
 * @param ip 
 */
void itrunc(struct inode *ip){
    for(int i=0;i<NDIRECT; ++i){
        if(ip->addrs[i] != 0){
            bfree(ip->dev, ip->addrs[i]);
        }
    }
    // 如果含有间接块
    if(ip->addrs[NDIRECT] != 0){
        struct buf *bp = bread(ip->dev, ip->addrs[NDIRECT]);
        uint32_t *a = (uint32_t *)bp->data;
        for(int j = 0; j < NINDIRECT; ++j){
            if(a[j] != 0){
                bfree(ip->dev, a[j]);
            }
        }
        brelease(bp);
        bfree(ip->dev, ip->addrs[NDIRECT]);
        ip->addrs[NDIRECT] = 0;
    }
    ip->size = 0;
    iupdate(ip);
}

/**
 * @brief 删除内存中的inode的引用
 * 如果此inode的引用是最后一个, 那么这个inode条目可以回收使用
 * 如果inode的nlink为0 那么在磁盘上释放此inode
 * 调用此函数时需确保在事务内
 * @param ip 
 */
void iput(struct inode *ip){
    acquire_spin_lock(&itable.lock);
    if(ip->ref == 1 && ip->valid && ip->nlink == 0){
        // 如果此inode 没有引用 没有nlink 清除内容,并释放
        acquire_sleep_lock(&ip->lock);
        release_spin_lock(&itable.lock);

        itrunc(ip);
        ip->type = 0;
        iupdate(ip);
        ip->valid = 0;
        release_sleep_lock(&ip->lock);
        acquire_spin_lock(&itable.lock);
    }
    ip->ref --;
    release_spin_lock(&itable.lock);
}

void iunlockandput(struct inode *ip) {
    iunlock(ip);
    iput(ip);
}
/**
 * @brief 获得指定inode中 第n块的 块地址
 * 如果没有这个块 那就分配一个
 * @param ip 
 * @param bn 
 * @return uint32_t 
 */
static uint32_t bmap(struct inode *ip, uint32_t bn){
    if( bn < NDIRECT){
        uint32_t addr = ip->addrs[bn];
        if(addr==0){
            ip -> addrs[bn] = addr = balloc(ip->dev);
            return addr;
        }
    }
    // 如果是间接块的话
    if(bn < NINDIRECT){
        // 如果间接块为空的话 先分配一个
        uint32_t addr = ip->addrs[NDIRECT];
        if(addr == 0){
            ip -> addrs[NDIRECT] = addr = balloc(ip->dev);
        }
        // 读取间接寻址块
        struct buf *bp = bread(ip->dev, addr);
        uint32_t *a = (uint32_t *)bp->data;
        if((addr = a[bn]) == 0){
            a[bn] = addr = balloc(ip->dev);
            log_write(bp);
        }
        brelease(bp);
        return addr;
    }
    panic("bmap bn: %d is out of range.\n", bn);
    return -1;
}
/**
 * @brief 将inode的信息复制到struct stat中
 * 调用者必须持有ip->lock
 * 
 * @param ip 
 * @param st 
 */
void stati(struct inode *ip, struct stat *st){
    st->dev = ip->dev;
    st->ino = ip->inum;
    st->type = ip->type;
    st->nlink = ip->nlink;
    st->size = ip->size;
}
/**
 * @brief 从inode中读取数据 调用者必须持有ip->lock
 * 
 * @param ip 
 * @param dst 
 * @param offset 
 * @param n 
 * @return int 读取的长度
 */
int readi(struct inode *ip, char* dst, uint32_t offset, uint32_t n){
    if(offset > ip->size || offset + n < offset){
        // 第二种情况 相当于 n是负数 
        return 0;
    }
    if(offset + n > ip->size){
        n = ip->size - offset;
    }
    uint32_t m = 0;
    uint32_t tot;
    for(tot = 0; tot < n; tot+=m, offset += m, dst += m){
        struct buf *b = bread(ip->dev, bmap(ip, offset/BSIZE));
        m = min(n - tot, BSIZE - offset%BSIZE);
        memmove(dst, b->data + (offset%BSIZE), m);
        brelease(b);
    }
    return tot;
}
/**
 * @brief 向inode中写入数据 调用者必须持有ip->lock
 * 
 * @param ip 
 * @param src 
 * @param offset 
 * @param n 
 * @return int 
 */
int writei(struct inode *ip, char* src, uint32_t offset, uint32_t n){
    if(offset > ip->size || offset + n < offset){
        return -1;
    }
    if(offset + n > MAXFILE*BSIZE) return -1;

    uint32_t tot = 0, m = 0;
    for(tot = 0; tot < n; tot+=m, offset += m, src += m){
        struct buf *bp = bread(ip->dev, bmap(ip, offset/BSIZE));
        m = min(n-tot,BSIZE - offset/BSIZE);
        memmove(bp->data + offset % BSIZE, src, m);
        log_write(bp);
        brelease(bp);
    }
    if(offset > ip->size){
        ip->size = offset;
    }
    iupdate(ip);
    return tot;
}

int namecmp(const char *s, const char *t){
    return strncmp(s,t, DIRSIZ);
}
/**
 * @brief 查找directory中名字为name的项目 如果找到后将poff设定为对应的偏移量
 * 
 * @param dp 
 * @param name 
 * @param poff 
 * @return struct inode* 
 */
struct inode* dirlookup(struct inode* dp, char *name, uint32_t *poff){
    if(dp->type != T_DIR){
        panic("dirlookup: not a dp");
    }
    for(uint32_t off = 0; off < dp->size; off+=sizeof(struct dirent)){
        struct dirent de;
        if(readi(dp, (char*)&de,off,sizeof(de))!=sizeof(de)){
            panic("dirlookup: readi failed.\n");
        }
        if(de.inum == 0) continue;
        if(namecmp(name, de.name) == 0){
            if(poff) *poff = off;
            return iget(dp->dev, de.inum);
        }
    }
    return NULL;
}

int dirlink(struct inode *dp, char *name, uint32_t inum){
    struct dirent de;
    struct inode *ip;
    if((ip = dirlookup(dp, name, NULL)) != NULL){
        iput(ip);
        return -1;
    }
    int off;
    // 找到一个空闲的条目
    for(off = 0; off < dp->size; off += sizeof(de)){
        if(readi(dp, (char*)&de, off,sizeof(de)) != sizeof(de)){
            panic("dirlink: read error.\n");
        }
        if(de.inum == 0)break;
    }
    strncpy(de.name, name, DIRSIZ);
    de.inum = inum;
    if(writei(dp,(char*)&de,off,sizeof(de)) != sizeof(de)){
        panic("dirlink: write error.\n");
    }
    return 0;
}
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
static char* skipelem(char *path, char *name){
    char *s;
    int len;    
    while(*path == '/')
        path++;
    if(*path == 0)
        return 0;
    s = path;
    while(*path != '/' && *path != 0)
        path++;
    len = path - s;
    if(len >= DIRSIZ)
        memmove(name, s, DIRSIZ);
    else {
        memmove(name, s, len);
        name[len] = 0;
    }
    while(*path == '/')
        path++;
    return path;
}


/**
 * @brief 查找并返回指定路径的inode 
 * 如果 nameiparent 非零, 返回父目录的inode
 * 并将path最末尾的元素放入到name中 必须保证name可以容纳DIRSIZ个字节
 * 必须在事务内调用此函数
 * 
 * @param path 
 * @param nameiparent 
 * @param name 
 * @return struct inode* 
 */
static struct inode* namex(char* path, int nameiparent, char* name){
    // 如果以'/' 开头 代表绝对路径 否则是当前路径
    struct inode *ip = (*path == '/') ? iget(ROOTDEV, ROOTINO) : idup(myproc()->cwd);
    while((path = skipelem(path, name)) != 0){
        ilock(ip);
        if(ip->type != T_DIR){
            iunlock(ip);
            return NULL;
        }
        if(nameiparent && *path == '\0'){
            iunlock(ip);
            return ip;
        }
        struct inode *next = dirlookup(ip, name, 0);
        if(next == NULL){
            iunlock(ip);
            return NULL;
        }
        iunlock(ip);
        ip = next;
    }
    if(nameiparent){
        iput(ip);
        return 0;
    }
    return ip;
}

struct inode* nameiparent(char *path, char *name) {
  return namex(path, 1, name);
}
struct inode* namei(char *path){
  char name[DIRSIZ];
  return namex(path, 0, name);
}