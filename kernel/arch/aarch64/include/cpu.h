#ifndef CPU_H
#define CPU_H
#include <stdbool.h>
#include "context.h"

struct cpu{
    struct context context;
    
    bool is_interupt_enabled;
    int depth_spin_lock;
    int cpuid; // 调试用
};

#endif /* CPU_H */