#ifndef ARM_H
#define ARM_H

#include <stdint.h>

/**
 * @brief 向指定内存位置写入32位的值 会控制编译器不要进行优化此次写入
 * 
 * @param ptr 指针
 * @param val 值
 */
static inline void put32(uint64_t ptr, uint32_t val){
    *(volatile uint32_t *)ptr = val;
}
/**
 * @brief 从指定内存位置直接读取 会控制编译器不要进行优化
 * 
 * @param ptr 
 * @return uint32_t 
 */
static inline uint32_t get32(uint64_t ptr){
    return *(volatile uint32_t *)ptr;
}

/**
 * @brief 设置EL1的异常处理函数表基址
 * 
 * @param ptr 
 */
static inline void load_vbar_el1(void *ptr){
    asm volatile (
        "msr vbar_el1, %0"
        : /* 没有输出 */
        : "r" (ptr)
    );
}
/**
 * @brief 设置EL1的栈指针
 * 
 * @param ptr 
 */
static inline void load_sp_el1(void *ptr){
    asm volatile (
        "msr sp_el1, %0"
        : /* no output */
        : "r" (ptr)
    );
}
/**
 * @brief 延迟一段时间 用来等待串口 但是count并不精确 只是靠一段循环而已
 * 
 * @param count 
 */
static inline void delay(int32_t count){
    asm volatile(
        "__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
        :"=r"(count)
        :[count]"0"(count)
        :"cc"
    );
}

static inline uint64_t r_mpidr(){
    uint64_t value;
    asm volatile(
        "mrs %0, mpidr_el1"
        : "=r"(value)
    );
    return value;
}

typedef uint64_t pte_t;
typedef uint64_t * pagetable_t;


#endif // ARM_Hf