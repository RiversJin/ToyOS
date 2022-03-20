#include "sleeplock.h"
#include "spinlock.h"
#include "../proc/proc.h"

void init_sleep_lock(struct sleeplock* lock, char *name){
    init_spin_lock(&lock->lk,"sleeplock");
    lock->name = name;
    lock->locked = 0;
    lock->pid = 0;
}

void acquire_sleep_lock(struct sleeplock *lock){
    acquire_spin_lock(&lock->lk);
    while(lock->locked){
        sleep(lock, &lock->lk);
    }
    
    lock->locked = 1;
    lock->pid = myproc()->pid;
    release_spin_lock(&lock->lk);
}

void release_sleep_lock(struct sleeplock* lock){
    acquire_spin_lock(&lock->lk);
    lock->locked = 0;
    lock->pid = 0;
    wakeup(lock);
    release_spin_lock(&lock->lk);
}

bool is_current_cpu_holing_sleep_lock(struct sleeplock *lock){
    bool r;
    acquire_spin_lock(&lock->lk);
    r = (lock->locked&&(lock->pid == myproc()->pid));
    release_spin_lock(&lock->lk);
    return r;
}