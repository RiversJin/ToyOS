#ifndef FS_H
#define FS_H
#include "../include/stdint.h"
#include "../include/param.h"
#include "../sync/sleeplock.h"
#include "../include/stat.h"

#define ROOTINO 1 
#define BSIZE 512 // 块大小

#define FSMAGIC 0x10203040 // 用于识别文件系统的magic number
#define NDIRECT 12 // 12个直接寻址块
#define NINDIRECT (BSIZE/sizeof(uint32_t))
#define MAXFILE (NDIRECT + NINDIRECT) // 文件最大允许的块数目

// 磁盘布局
// [ boot block | super block | log | inode blocks |
//                                          free bit map | data blocks]

struct superblock {
    uint32_t magic;
    uint32_t size;
    uint32_t nblocks;
    uint32_t ninodes;
    uint32_t nlog;
    uint32_t logstart;
    uint32_t inodestart;
    uint32_t bmapstart;
};
/**
 * @brief 内存中的inode定义
 * 
 */
struct inode {
    uint32_t dev; 
    uint32_t inum; // inode number
    int ref; // reference count
    struct sleeplock lock; // 保护下面的内容
    int valid; // 是否已经从磁盘中读取到内存
    // 从磁盘中即 dinode读取的内容
    uint16_t type;
    uint16_t major;
    uint16_t minor;
    uint16_t nlink;
    uint32_t size;
    uint32_t addrs[NDIRECT + 1];
};

// 磁盘上inode的结构定义
struct dinode{
    uint16_t type;
    uint16_t major;
    uint16_t minor;
    uint16_t nlink;
    uint32_t size;
    uint32_t addrs[NDIRECT + 1];
};
// 在磁盘上 每个块可以装下多少inode
#define IPB (BSIZE / sizeof(struct dinode))
// 给定一个inode号 获得对应的磁盘上的偏移量
#define IBLOCK(i,sb) (((i)/IPB)+sb.inodestart)

/* Bitmap bits per block. */
#define BPB (BSIZE * 8)
#define BBLOCK(b,sb) (((b)/IPB)+sb.bmapstart)

// 目录名称长度
#define DIRSIZ 14

struct dirent {
    uint16_t inum;
    char name[DIRSIZ];
};

void iput(struct inode *ip);
void ilock(struct inode* ip);
void iunlock(struct inode* ip);
struct inode* ialloc(uint32_t dev, uint16_t type);
void itrunc(struct inode *ip);
void stati(struct inode* ip, struct stat* st);
int readi(struct inode *ip, char* dst, uint32_t offset, uint32_t n);
int writei(struct inode *ip, char* src, uint32_t offset, uint32_t n);
void fsinit(int);
void iinit(void);
struct inode* nameiparent(char *path, char *name);
void iunlockandput(struct inode *ip);
struct inode* dirlookup(struct inode* dp, char *name, uint32_t *poff);
void iupdate(struct inode *ip);
int dirlink(struct inode *dp, char *name, uint32_t inum);
struct inode* namei(char *path);
int namecmp(const char *s, const char *t);
struct inode* idup(struct inode *ip);
#endif /* FS_H */