#include "proc.h"
#include "include/param.h"

struct cpu cpus[NCPU];
struct process_table{
    struct proc proc[NPROC];
}process_table;

static volatile uint64_t* _spintable = (uint64_t*)PA2VA(0xD8);
extern void _entry();

static struct proc *initproc;
int nextpid = 1;

static struct spinlock pid_lock;
static struct spinlock wait_lock;

void init_awake_ap_by_spintable(){
    for(int i = 1; i < NCPU; ++i){
        _spintable[i] = (uint64_t)VA2PA(&_entry);
    }
    asm volatile(
        "dsb st \n\t"
        "sev"
    );
}

/**
 * @brief 返回当前cpu的信息
 * 
 * @return struct cpu* 
 */
struct cpu* mycpu(void){
    return &cpus[cpuid()];
}

void init_cpu_info(){
    for(int i = 0; i < NCPU; ++i){
        cpus[i].depth_spin_lock = 0;
        cpus[i].cpuid = i;
    }
}
/**
 * @brief 获得当前进程
 * 
 * @return struct proc* 
 */
struct proc* myproc(void){
    push_off();
    struct cpu *cpu = mycpu();
    struct proc *proc = cpu->proc;
    pop_off();
    return proc;
}
/**
 * @brief 初始化进程管理
 * 
 */
void init_proc(){
    init_spin_lock(&wait_lock,"wait_lock");
    init_spin_lock(&pid_lock,"pid_lock");
    for(struct proc *p = process_table.proc; p < &process_table.proc[NPROC]; ++p){
        init_spin_lock(&p->lock,"proc");
    }
}