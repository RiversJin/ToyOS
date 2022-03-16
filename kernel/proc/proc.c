#include "proc.h"
#include "include/param.h"
#include "../memory/kalloc.h"
#include "../memory/vm.h"
#include "../lib/string.h"
#include "../console.h"

struct cpu cpus[NCPU];
struct process_table{
    struct proc proc[NPROC];
}process_table;

static volatile uint64_t* _spintable = (uint64_t*)PA2VA(0xD8);
extern void _entry();
extern void user_trapret(struct trapframe*);

static struct proc *initproc;
int nextpid = 1;

static struct spinlock pid_lock;
static struct spinlock wait_lock;

static void freeproc(struct proc *p);
void forkret();

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
/**
 * @brief 分配一个可用的pid
 * 
 * @return int 
 */
static int allocpid(){
    int pid;
    acquire_spin_lock(&pid_lock);
    pid = nextpid;
    nextpid += 1;
    release_spin_lock(&pid_lock);
    return pid;
}
/**
 * @brief 在进程表中寻找一个处于UNSED状态的进程 
 * 找到后 初始化 并运行 将process->lock锁定
 * 如果没有可用进程或内存分配失败 返回NULL
 * 
 * @return struct proc* 
 */
static struct proc * allocproc(void){
    struct proc *p;
    for(p = process_table.proc;p < &process_table.proc[NPROC]; ++p){
        acquire_spin_lock(&p->lock);
        if(p->state == UNUSED){
            goto found;
        } else {
            release_spin_lock(&p->lock);
        }
    }
    return 0;
    found:
    p->pid = allocpid();
    p->state = EMBRYO;
    // 为内核栈分配空间 
    if((p->kstack = kalloc(KSTACKSIZE)) == NULL){
        freeproc(p);
        release_spin_lock(&p->lock);
        return NULL;
    }
    // 分配页表
    if((p->pagetable = alloc_pagetable()) == NULL){
        freeproc(p);
        release_spin_lock(&p->lock);
        return NULL;
    }
    // 在栈上为trapframe和context预留空间
    uint8_t *sp = p->kstack + KSTACKSIZE;
    sp -= sizeof(*p->tf);
    p->tf = (struct trapframe *)sp;

    sp -= sizeof(*p->context);
    p->context = (struct context *)sp;
    memset(p->context,0,sizeof(struct context));
    // 构造返回地址
    p->context->x30 = (uint64_t)forkret;

    cprintf("alloc_proc: process %d allocated. \n");
    return p;
}
/**
 * @brief 释放一个进程
 * 
 * @param p 
 */
static void freeproc(struct proc *p){
    p->chan = NULL;
    p->killed = 0;
    p->xstate = 0;
    p->pid = 0;
    p->parent = NULL;
    if(p->kstack){
        kfree(p->kstack);
    }
    p->kstack = NULL;
    p->sz = 0;
    if(p->pagetable){
        uvmfree(p->pagetable,4);
    }
    p->pagetable = NULL;
    p->tf = NULL;
    p->name[0] = '\0';
    p->state = UNUSED;
}

void forkret(){
    volatile static int first = 1;
    struct proc *p = myproc();
    release_spin_lock(&p->lock);
    if(first){
        first = 0;
        // TODO: 记得在这里初始化文件系统 因为文件系统依赖进程的存在 所以不能直接在main中初始化
    }
    user_trapret(p->tf);
}
void init_user(){
    extern char *user_init_begin;
    extern char *user_init_end;
    int64_t size = user_init_end - user_init_begin;
}