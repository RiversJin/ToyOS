#include "arg.h"
#include "../include/stdint.h"
#include "../proc/proc.h"

int64_t sys_exit(){
    int64_t n;
    if(argint(0, (uint64_t*)&n) < 0){ 
        return -1;
    }
    exit(n);
    return 0;
}


// int getpid(void)
int64_t sys_getpid(){
    return myproc()->pid;
}

// int fork(void)
int64_t sys_fork(){
    return fork();
}

int64_t sys_wait(){ 
    uint64_t *p;
    if(argptr(0,(char**)&p,sizeof(uint64_t)) < 0){
        return -1;
    }
    return wait((int64_t*)p);
}

int64_t sys_yield(){
    yield();
    return 0;
}

// int kill(int pid)
int64_t sys_kill(void){
    int64_t pid;
    if(argint(0,(uint64_t*)&pid) < 0){
        return -1;
    }
    return kill(pid);
}
// void* sbrk(int delta)
int64_t sys_sbrk(){
    int64_t delta;
    if(argint(0,(uint64_t*)&delta) < 0){
        return -1;
    }
    int64_t oldaddr = myproc()->sz;
    if(growproc(delta) < 0){
        return -1;
    }
    return oldaddr;
}
extern uint64_t uptime();

int64_t sys_uptime(){
    return uptime();
}

extern struct spinlock tickslock;
extern uint64_t ticks;

int64_t sys_sleep(){
    int64_t n;
    if(argint(0, (uint64_t*)&n) < 0){
        return -1;
    }
    
    acquire_spin_lock(&tickslock);
    uint64_t ticks0 = ticks;
    while(ticks < ticks0 + n){
        if(myproc()->killed != 0){
            release_spin_lock(&tickslock);
            return -1;
        }
        sleep(&ticks,&tickslock);
    }
    release_spin_lock(&tickslock);
    return 0;
}