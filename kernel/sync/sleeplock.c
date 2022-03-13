#include "sleeplock.h"
#include "spinlock.h"

void init_sleep_lock(struct sleeplock* lock, char *name){
    init_spin_lock(&lock->lk,name);
    lock->locked = 0;
    lock->pid = 0;
}

void acquire_sleep_lock(struct sleeplock *lock){
    acquire_spin_lock(&lock->lk);
    while(lock->locked){
        // sleep()
    }
    
    lock->locked = 1;
    // TODO: this...
    // lock->pid = 
    release_spin_lock(&lock->lk);
}

void release_sleep_lock(struct sleeplock* lock){
    acquire_spin_lock(&lock->lk);
    lock->locked = 0;
    lock->pid = 0;
    // TODO: this
    // wakeup(lk)
    release_spin_lock(&lock->lk);
}

bool is_current_cpu_holing_sleep_lock(struct sleeplock *lock){
    // FIX: this
    bool r = lock->locked;
    acquire_spin_lock(&lock->lk);
    // TODO: this
    // r = (lock->locked&&(lock->pid == thisproc()->pid));
    release_spin_lock(&lock->lk);
    return r;
}