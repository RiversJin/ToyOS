#ifndef PROC_H
#define PROC_H
#include <stdint.h>
#include "arch/aarch64/arm.h"
#include "arch/aarch64/include/cpu.h"
#include "memory/memory.h"

/**
 * @brief 返回CPUID 注意要在关中断的情况下使用,
 * 不然可能值还没来得及返回 线程被切换到另一个核心上运行了
 * @return uint64_t 
 */
static inline uint64_t cpuid(){
    return r_mpidr() & 0xFF;
}

/**
 * @brief 返回当前cpu的信息
 * 
 * @return struct cpu* 
 */
struct cpu* mycpu(void);

void init_awake_ap_by_spintable();

void init_cpu_info();
#endif //PROC_H