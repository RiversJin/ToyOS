#ifndef PIPE_H
#define PIPE_H
#include "../include/param.h"
#include "../printf.h"
#include "../sync/spinlock.h"
#include "../sync/sleeplock.h"
#include "../file/file.h" 
#include "../memory/kalloc.h"
#include "../include/stdint.h"

#define PIPE_SIZE (512)

struct pipe {
    struct spinlock lock;
    char data[PIPE_SIZE];
    uint32_t nread;
    uint32_t nwrite;

    int32_t is_readopen;
    int32_t is_writeopen;
};

int pipealloc(struct file **f0, struct file **f1);

void pipeclose(struct pipe *pi, int writable);

int32_t pipewrite(struct pipe *pi, void* addr, int32_t n);
int32_t piperead(struct pipe *pi, void* addr, int32_t n);
#endif // PIPE_H