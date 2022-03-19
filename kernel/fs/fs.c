#include "fs.h"
#include "log.h"
#include "../include/stdint.h"
#include "../buffer/buf.h"
#include "../lib/string.h"
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