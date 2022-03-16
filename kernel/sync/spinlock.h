#ifndef SPINLOCK_H
#define SPINLOCK_H
#include <stdint.h>
#include <stdbool.h>

struct spinlock {
    bool locked;

    //调试用
    const char *name; // 锁名称
    struct cpu* cpu; // 此锁当前由哪个cpu持有
};
/**
 * @brief 判断此自旋锁是否由当前cpu持有
 * @warning 在调用此函数时,必须关闭中断
 * @param lock 
 * @return true 
 * @return false 
 */
bool is_current_cpu_holding_spin_lock(struct spinlock *lock);
/**
 * @brief 尝试获得一个锁
 * 
 * @param lock 
 */
void acquire_spin_lock(struct spinlock *lock);
/**
 * @brief 释放锁
 * 
 * @param lock 
 */
void release_spin_lock(struct spinlock *lock);
/**
 * @brief 初始化
 * 
 * @param lock 
 * @param name 自旋锁名
 */
void init_spin_lock(struct spinlock *lock,const char *name);


void push_off(void);

void pop_off(void);
#endif //SPINLOCK_H