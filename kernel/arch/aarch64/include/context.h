#ifndef CONTEXT_H
#define CONTEXT_H

#ifndef __ASSEMBLER__
#include "../../../include/stdint.h"

// 当进行上下文切换时, 要被调用者要保存的寄存器
struct context {
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t x29; // Stack Frame Pointer
    uint64_t x30; // Link register (the address to return)
};
#define CONTEXT_SIZE sizeof(struct context)
#else
#define CONTEXT_SIZE (12*8)
#endif

#endif // CONTEXT_H