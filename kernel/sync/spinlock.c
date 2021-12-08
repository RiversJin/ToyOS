#include "spinlock.h"
#include "proc/proc.h"
#include "console.h"

void init_spin_lock(struct spinlock *lock,const char *name){
    lock->locked = false;
    lock->name = name;
    lock->cpuid = -1;
}

void acquire_spin_lock(struct spinlock *lock){
    //TODO: 关中断
    if(is_current_cpu_held(lock)){
        panic("acquire_spin_lock: the lock (%s) is already held by %lu \n",lock->name,lock->cpuid);
    }
    while(__sync_lock_test_and_set(&lock->locked,true) != false);
    __sync_synchronize();
    lock->cpuid = cpuid();
}

void release_spin_lock(struct spinlock *lock){
    if(!is_current_cpu_held(lock)){
        panic("release_spin_lock: the lock (%s) held by %lu can't be released by %lu \n",lock->name,lock->cpuid,cpuid());
    }
    lock->cpuid = -1;
    __sync_synchronize();
    __sync_lock_release(&lock->locked);
    //TODO: 开中断
}

bool is_current_cpu_held(struct spinlock *lock){
    return lock->locked && lock->cpuid == cpuid();
}
