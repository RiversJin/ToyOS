#ifndef FILE_H
#define FILE_H

#include "../fs/fs.h"
#include "../sync/sleeplock.h"
/**
 * @brief 文件结构体 下面的一些字段也许可以使用union 但先实现了再考虑优化
 * 
 */
struct file {
    enum { FD_NONE, FD_PIPE, FD_INODE, FD_DEVICE } type;
    int ref;
    char readable;
    char writable;
    // 这个字段给pipe用
    struct pipe* pipe;
    // 这个字段给inode和device用
    struct inode* ip;

    size_t off; // inode

    int16_t major; // device
};

#define major(dev) ((dev)>>16 & 0xFFFF)
#define minor(dev) ((dev) & 0xFFFF)
#define mkdev(m,n) ((uint32_t)((m) << 16) | (n))
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

// 通过主设备号映射对应的读写函数
struct devsw{
    int (*read)(char*, int);
    int (*write)(char*, int);
};

extern struct devsw devsw[];
#define CONSOLE 1

void fileinit(void);
struct file* filealloc(void);
struct file* filedup(struct file*);
void fileclose(struct file*);

int32_t filestat(struct file*, struct stat*);
int32_t fileread(struct file*, char*, int32_t);
#endif