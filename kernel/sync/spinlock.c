#include "spinlock.h"
#include "interupt/interupt.h"
#include "proc/proc.h"
#include "console.h"
#include "arch/aarch64/include/cpu.h"

void init_spin_lock(struct spinlock *lock,const char *name){
    lock->locked = false;
    lock->name = name;
    lock->cpu = NULL;
}

void push_off(void){
    bool old = is_interupt_enabled();
    disable_interrupt();

    if(mycpu()->depth_spin_lock == 0){
        mycpu() -> is_interupt_enabled = old;
    }
    mycpu() -> depth_spin_lock += 1;
}

void pop_off(void){
    struct cpu* cpu = mycpu();
    if(is_interupt_enabled()){
        panic("pop_off: interruptible before pop_off");
    }
    if(cpu->depth_spin_lock < 1){
        panic("pop_off: depth_spin_lock < 1");
    }
    cpu->depth_spin_lock -= 1;
    if(cpu->depth_spin_lock == 0 && cpu->is_interupt_enabled){
        enable_interrupt();
    }
}

void acquire_spin_lock(struct spinlock *lock){
    push_off();
    if(is_current_cpu_holding_spin_lock(lock)){
        panic("acquire_spin_lock: the lock (%s) is already held by %lu \n",lock->name,cpuid());
    }
    while(__sync_lock_test_and_set(&lock->locked,true) != false);
    __sync_synchronize();
    lock->cpu = mycpu();
}

void release_spin_lock(struct spinlock *lock){
    if(!is_current_cpu_holding_spin_lock(lock)){
        panic("release_spin_lock: the lock (%s) held by %lu can't be released by %lu \n",lock->name,lock->cpu->cpuid,cpuid());
    }
    lock->cpu = NULL;
    __sync_synchronize();
    __sync_lock_release(&lock->locked);
    pop_off();
}

bool is_current_cpu_holding_spin_lock(struct spinlock *lock){
    return lock->locked && lock->cpu == mycpu();
}
