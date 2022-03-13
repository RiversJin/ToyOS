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

#endif