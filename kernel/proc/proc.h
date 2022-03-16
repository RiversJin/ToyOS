#ifndef PROC_H
#define PROC_H
#include "include/stdint.h"
#include "include/param.h"
#include "../file/file.h"
#include "arch/aarch64/arm.h"
#include "arch/aarch64/include/context.h"
#include "memory/memory.h"
#include "sync/spinlock.h"
#include "arch/aarch64/include/trapframe.h"

enum process_state {
    UNUSED,
    EMBRYO,
    SLEEPING,
    RUNNABLE,
    RUNNING,
    ZOMBIE
};

struct proc{
    struct spinlock lock;

    // p->lock must be held when using these:
    enum process_state state;
    void* chan; // 如果非零 即意味着此进程在目标队列上等待
    int killed;
    int xstate; // 退出状态
    int pid; // process id
    // wait_lock must be held when using this:
    struct proc* parent; 
    // these are private to the process, so p->lock need not be held.

    uint8_t *kstack; // 内核栈的底部
    uint64_t sz; // 进程使用的内存大小 以字节计

    pagetable_t pagetable; // 进程的页表
    struct trapframe* tf;  // 进程发起系统调用时使用
    struct context* context; // 进程切换时使用
    struct file* ofile[NOFILE]; // 进程打开的文件
    struct inode* cwd; // 当前工作目录
    char name[16]; // 进程名 调试用
};

struct cpu{
    struct context context;
    struct proc* proc;
    bool is_interupt_enabled;
    int depth_spin_lock;
    int cpuid; // 调试用
};

/**
 * @brief 返回CPUID 注意要在关中断的情况下使用,
 * 不然可能值还没来得及返回 线程被切换到另一个核心上运行了
 * @return uint64_t 
 */
static inline uint64_t cpuid(){
    return r_mpidr() & 0xFF;
}


/* 函数声明 */

struct cpu* mycpu(void);
struct proc* myproc(void);

void init_awake_ap_by_spintable();
void init_cpu_info();

void init_proc();
#endif //PROC_H