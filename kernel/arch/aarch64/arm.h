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
static inline uint64_t read_esr_el1(){
    uint64_t reg;
    asm volatile (
        "mrs %0, esr_el1"
        : "=r" (reg)
    );
    return reg;
}
static inline uint64_t read_elr_el(){
    uint64_t reg;
    asm volatile (
        "mrs %0, elr_el1"
        : "=r" (reg)
    );
    return reg;
}
static inline void isb(){
    asm volatile (
        "isb"
    );
}
static inline void disb(){
    asm volatile("dsb sy; isb");
}

static inline void dccivac(void* p, int n)
{
    while (n--) asm volatile("dc civac, %[x]" : : [x] "r"(p + n));
}

static inline uint64_t timestamp(){
    uint64_t t;
    asm volatile("mrs %[cnt], cntpct_el0" : [cnt] "=r"(t));
    return t;
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

/**
 * @brief 读取时钟频率
 * 
 * @return uint64_t 
 */
static inline uint64_t r_cntfrq_el0(){
    uint64_t value;
    asm volatile(
        "mrs %0, cntfrq_el0"
        : "=r" (value)
    );
    return value;
}

static inline uint64_t r_cntp_ctl_el0(){
    uint64_t value;
    asm volatile (
        "mrs %0, cntp_ctl_el0"
        : "=r"(value)
    );
    return value;
}
static inline void l_cntp_ctl_el0(uint64_t value){
    asm volatile (
        "msr cntp_ctl_el0,%0"
        :
        :"r"(value)
    );
}
static inline uint64_t r_cntp_tval_el0(){
    uint64_t value;
    asm volatile (
        "mrs %0, cntp_tval"
        : "=r"(value)
    );
    return value;
}
static inline void l_cntp_tval_el0(uint64_t value){
    asm volatile (
        "msr cntp_tval_el0,%0"
        :
        :"r"(value)
    );
}


static inline void clear_daif()
{
    asm volatile("msr daif, %[x]" : : [x] "r"(0));
}


static inline void set_daif()
{
    asm volatile("msr daif, %[x]" : : [x] "r"(0xF << 6));
}

static inline uint32_t get_daif(){
    uint32_t value;
    asm volatile(
        "mrs %0, daif"
        : "=r" (value)
    );
    return value;
}

static inline void delayus(uint32_t n){
    uint64_t f,t,r;
    //首先 获得当前计数器频率
    asm volatile("mrs %[freq], cntfrq_el0" : [freq] "=r"(f));
    //获得当前计数器的值
    asm volatile("mrs %[cnt], cntpct_el0" : [cnt] "=r"(t));
    
    t += f / 1000000 * n;
    do {
        asm volatile("mrs %[cnt], cntpct_el0" : [cnt] "=r"(r));
    } while (r < t);
}

/* Load Translation Table Base Register 0 (EL1). */
static inline void lttbr0(uint64_t p) {
    asm volatile("msr ttbr0_el1, %[x]" : : [x] "r"(p));
    disb();
    asm volatile("tlbi vmalle1");
    disb();
}

/* Load Translation Table Base Register 1 (EL1). */
static inline void lttbr1(uint64_t p){
    asm volatile("msr ttbr1_el1, %[x]" : : [x] "r"(p));
    disb();
    asm volatile("tlbi vmalle1");
    disb();
}


typedef uint64_t pte_t;
typedef uint64_t * pagetable_t;


#endif // ARM_Hf