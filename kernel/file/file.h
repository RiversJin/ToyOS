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
int32_t filewrite(struct file* f, char* addr, int32_t n);
#endif