#ifndef TRAPFRAME_H
#define TRAPFRAME_H

#ifndef __ASSEMBLY__
#include <stdint.h>
struct trapframe {
    uint64_t regs[31]; // x0-x30 通用寄存器
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
};
#define FRAME_SIZE sizeof(struct trapframe)
#else
#define FRAME_SIZE 272
#endif

#endif //TRAPFRAME_H