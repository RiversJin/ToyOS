#include "proc.h"
#include "include/param.h"
#include "../memory/kalloc.h"
#include "../memory/vm.h"
#include "../lib/string.h"
#include "../printf.h"
#include "../interupt/interupt.h"
#include "../fs/fs.h"
#include "../fs/log.h"

struct cpu cpus[NCPU];
struct process_table{
    struct proc proc[NPROC];
}process_table;

static volatile uint64_t* _spintable = (uint64_t*)PA2VA(0xD8);
extern void _entry();
extern void user_trapret();
extern void _forkret(struct trapframe*);
extern void swtch(struct context *old, struct context *new);

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
void procinit(){
    init_cpu_info();
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
    // 在栈上为trapframe预留空间
    uint8_t *sp = p->kstack + KSTACKSIZE;
    sp -= sizeof(*p->tf);
    p->tf = (struct trapframe *)sp;

    memset(&p->context,0,sizeof(struct context));
    // 构造返回地址
    cprintf("allocproc: kernel sp: 0x%x \n",sp);
    p->context.x29 = (uint64_t)sp;
    p->context.sp_el1 = (uint64_t)sp;
    p->context.x30 = (uint64_t)forkret;

    cprintf("alloc_proc: process %d allocated. \n",p->pid);
    return p;
}

void reparent(struct proc *p){
    for(struct proc *child_proc = process_table.proc; child_proc < &process_table.proc[NPROC]; child_proc ++){
        if(child_proc->parent == p){
            child_proc->parent = initproc;
            wakeup(initproc);
        }
    }
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
        fsinit(ROOTDEV);
    }
    _forkret(p->tf);
}
void init_user(){
    extern char _binary_build_user_initcode_bin_start[];
    extern char _binary_build_user_initcode_bin_size[];
    int64_t code_size = (uint64_t)_binary_build_user_initcode_bin_size;
    struct proc *p = allocproc();
    if(p == NULL){
        panic("init_user: Failed to allocate proc. \n");
    }
    initproc = p;

    p->sz = PGSIZE;
    // 将初始用户程序代码复制到初始进程的空间中
    uvminit(p->pagetable,(uint8_t*)_binary_build_user_initcode_bin_start,code_size);
    memset(p->tf,0,sizeof(*p->tf));
    p->tf->sp = PGSIZE; // 设置用户栈位置
    p->tf->regs[30] = 0; //initcode位于进程地址空间的0处
    
    safestrcpy(p->name,"initcode",sizeof(p->name));
    p->cwd = namei("/");
    p->state = RUNNABLE;
    release_spin_lock(&p->lock);

}

void sched(){
    int intena;
    struct proc *p = myproc();
    if(!is_current_cpu_holding_spin_lock(&p->lock)){
        panic("sched: p->lock not locked.\n");
    }
    if(mycpu()->depth_spin_lock != 1){
        panic("sched: sched locks.\n");
    }
    if(p->state == RUNNING){
        panic("sched: process is running.\n");
    }
    if(is_interupt_enabled()){
        panic("sched: interrupt is enabled.\n");
    }
    intena = mycpu()->depth_spin_lock;
    swtch(&p->context, &mycpu()->context);
    mycpu()->depth_spin_lock = intena;
}

void scheduler(void){
    
    struct cpu *c = mycpu();
    c -> proc = NULL;
    if(cpuid() == 0){
        while(1);
    }
    while(1){
        enable_interrupt(); // 设备可能会中断
        for(struct proc *p = process_table.proc; p < &process_table.proc[NPROC]; ++p){
            acquire_spin_lock(&p->lock);
            if(p->state == RUNNABLE){
                cprintf("cpu %d to %s\n",cpuid(),p->name);
                p->state = RUNNING;
                c->proc = p;
                uvmswitch(p);
                swtch(&c->context, &p->context);
                c->proc = NULL;
            }
            release_spin_lock(&p->lock);
        }
    }
}

void
proc_dump()
{
    static char* states[] = {
        [UNUSED] "UNUSED  ",  [SLEEPING] "SLEEPING", [RUNNABLE] "RUNNABLE",
        [RUNNING] "RUNNING ", [ZOMBIE] "ZOMBIE  ",
    };

    cprintf("\n====== PROCESS DUMP ======\n");
    for (struct proc* p = process_table.proc; p < &process_table.proc[NPROC]; ++p) {
        if (p->state == UNUSED) continue;
        char* state =
            (p->state >= 0 && p->state < ARRAY_SIZE(states) && states[p->state])
                ? states[p->state]
                : "UNKNOWN ";
        cprintf("[%s] %d (%s)\n", state, p->pid, p->name);
    }
    cprintf("====== DUMP END ======\n\n");
}

void trapframe_dump(struct proc* p){
    cprintf("\n====== TRAP FRAME DUMP ======\n");
    cprintf("sp: %d\n", p->tf->sp);
    cprintf("pc: %d\n", p->tf->pc);
    cprintf("pstate: %d\n", p->tf->pstate);
    for(uint64_t i = 0; i<31; ++i){
        cprintf("x%d: %d\n",p->tf->regs[i]);
    }
    cprintf("====== DUMP END ======\n\n");
}
/**
 * @brief 退出当前进程
 * 
 * @param status 
 */
void exit(int status){
    struct proc* p = myproc();
    if(p == initproc){
        panic("init proc exit with status code %d\n",status);
    }
    
    // 关闭进程所有打开的文件
    for(int fd = 0; fd < NOFILE; ++fd){
        if(p->ofile[fd] != NULL){
            struct file* f = p->ofile[fd];
            fileclose(f);
            p->ofile[fd] = NULL;
        }
    }
    begin_op();
    iput(p->cwd);
    end_op();

    p->cwd = NULL;

    acquire_spin_lock(&wait_lock);
    // 1. 将此已退出进程的子进程设置为此进程的父进程的子进程
    reparent(p);
    // 2. 如果父进程调用wait()的话 苏醒父进程
    wakeup(p->parent);
    // 3. 调用sched重新调度
    acquire_spin_lock(&p->lock);
    p->xstate = status;
    p->state = ZOMBIE;
    release_spin_lock(&wait_lock);
    sched();
    panic("exit: zombie exit.\n");
}

void sleep(void* chan, struct spinlock* lk){ 
    struct proc* p = myproc();
    acquire_spin_lock(&p->lock);
    release_spin_lock(lk);
    p->chan = chan;
    p->state = SLEEPING;

    sched();

    p->chan = 0;
    release_spin_lock(&p->lock);
    acquire_spin_lock(lk);
}

void wakeup(void *chan){ 
    for(struct proc* p = process_table.proc; p < &process_table.proc[NPROC]; p++){
        if(p != myproc()){
            acquire_spin_lock(&p->lock);
            if(p->state == SLEEPING && p->chan == chan){
                p -> state = RUNNABLE;
            }
            release_spin_lock(&p->lock);
        }
    }
}
int32_t kill(int pid){
    for(struct proc* p = process_table.proc; p < &process_table.proc[NPROC]; p++){ 
        acquire_spin_lock(&p->lock);
        if(p->pid == pid){
            p->killed = 1;
            if(p->state == SLEEPING){
                p->state = RUNNABLE;
            }
            release_spin_lock(&p->lock);
            return 0;
        }
        release_spin_lock(&p->lock);
    }
    return -1;
}

int32_t fork(void){
    struct proc* parent_proc = myproc();
    struct proc* child_proc = allocproc();
    if(child_proc == NULL){
        return -1;
    }
    // 将父进程内存复制到子进程中
    if(uvmcopy(parent_proc->pagetable, child_proc->pagetable, parent_proc->sz) < 0){
        freeproc(child_proc);
        release_spin_lock(&child_proc->lock);
        cprintf("fork: copy memory to child process failed.\n");
        return -1;
    }
    child_proc->sz = parent_proc->sz;
    // 复制寄存器
    *(child_proc->tf) = *(parent_proc->tf);
    // 子进程返回值为0
    child_proc->tf->regs[0] = 0;
    // 父子进程打开文件同步
    for(int i = 0; i < NOFILE; ++i){
        if(parent_proc->ofile[i] != NULL){
            child_proc->ofile[i] = filedup(parent_proc->ofile[i]);
        }
    }
    // 配置工作目录
    child_proc->cwd = parent_proc->cwd;
    strncpy(child_proc->name, parent_proc->name, sizeof(child_proc->name));

    int child_pid = child_proc->pid;
    release_spin_lock(&child_proc->lock);

    acquire_spin_lock(&wait_lock);
    child_proc->parent = parent_proc;
    release_spin_lock(&wait_lock);

    acquire_spin_lock(&child_proc->lock);
    child_proc->state = RUNNABLE;
    release_spin_lock(&child_proc->lock);

    return child_pid;
}
/**
 * @brief 等待子进程返回 返回值为子进程的pid xstate是子进程的返回值
 * 如果此进程没有子进程 返回-1
 * 
 * @param xstate 
 * @return int32_t 
 */
int32_t wait(int64_t *xstate){
    struct proc *p = myproc();
    int is_have_child = 0;
    int pid;
    acquire_spin_lock(&wait_lock);
    while(1){
        is_have_child = 0;
        for(struct proc *np = process_table.proc; np < process_table.proc+NPROC; np++){
            if(np->parent == p){
                acquire_spin_lock(&np->lock);
                is_have_child = 1;
                if(np->state == ZOMBIE){
                    // 找到一个已经退出的子进程
                    pid = np->pid;
                    if(xstate != NULL){
                        *xstate = (int64_t)np->xstate;
                    }
                    freeproc(np);
                    release_spin_lock(&np->lock);
                    release_spin_lock(&wait_lock);
                    return pid;
                }
                release_spin_lock(&np->lock);
            }
        }
        if(is_have_child == 0 || p->killed){
            release_spin_lock(&wait_lock);
            return -1;
        }
        sleep(p, &wait_lock);
    }
}

void yield(void){
    struct proc *p = myproc();
    acquire_spin_lock(&p->lock);
    p->state = RUNNABLE;
    sched();
    release_spin_lock(&p->lock);
}