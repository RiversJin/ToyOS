#ifndef FILE_H
#define FILE_H

#include "../fs/fs.h"
#include "../sync/sleeplock.h"

struct file {
    enum { FD_NONE, FD_PIPE, FD_INODE } type;
    int ref;
    char readable;
    char writable;
    struct pipe* pipe;
    struct inode* ip;
    size_t off;
};

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
    int (*read)(int, uint64_t, int);
    int (*write)(int, uint64_t, int);
};

#endif