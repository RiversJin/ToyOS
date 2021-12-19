#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>

/**
 * @brief 初始化 Arm Generic Timer
 * 
 */
void timer_init();

/**
 * @brief 如果一切顺利的话(irq mask之类的),设定下一次触发时间
 * 
 * @param us 
 */
void timer_tick_in(uint64_t us);

/**
 * @brief 充值计时器 (timer_tick_in的语法糖而已)
 * 
 */
void timer_reset();

#endif //TIMER_H