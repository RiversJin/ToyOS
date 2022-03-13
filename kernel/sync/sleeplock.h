#ifndef SLEEPLOCK_H
#define SLEEPLOCK_H
#include "spinlock.h"
struct sleeplock {
    uint32_t locked;
    struct spinlock lk;
    //调试用的字段
    char *name; // 锁的名称
    int pid;  // 持有锁的进程
};

/**
 * @brief 初始化sleeplock
 * 
 * @param lock 
 * @param name 锁名称 调试用
 */
void init_sleep_lock(struct sleeplock* lock, char *name);
/**
 * @brief 获取sleeplock
 * 
 * @param lock 
 */
void acquire_sleep_lock(struct sleeplock *lock);
/**
 * @brief 释放sleeplock
 * 
 * @param lock 
 */
void release_sleep_lock(struct sleeplock *lock);
/**
 * @brief 判断此sleeplock是否由当前cpu持有
 * 
 * @param lock 
 * @return true 
 * @return false 
 */
bool is_current_cpu_holing_sleep_lock(struct sleeplock *lock);

#endif /* SLEEPLOCK_H */