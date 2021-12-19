#include "timer.h"
#include "proc/proc.h"
#include "console.h"
#include "arm.h"
#include "board/raspi3/interupt.h"
#include "board/raspi3/local_peripherals.h"
#define CNTP_CTL_EL0_ENABLE 1
#define CNTP_CTL_EL0_IMASK (1 << 1)
#define CNTP_CTL_EL0_ISTATUS (1 << 2)

static uint64_t _timer_uint;

void timer_init(){

    uint64_t timer_frq = r_cntfrq_el0();
    cprintf("[cpu %d] timer frequency: %lu\n",cpuid(),timer_frq);
    _timer_uint = timer_frq / 1000; // 以1ms为最小时间单位
    timer_tick_in(10000);
    put32(COREn_TIMER_INTERUPT_CONTROL(cpuid()),CORE_TIMER_ENABLE); //使能对应的Core timer
    l_cntp_ctl_el0(CNTP_CTL_EL0_ENABLE);
}

void timer_tick_in(uint64_t ms){
    uint64_t value = ms * _timer_uint;
    l_cntp_tval_el0(value);
}

void timer_reset(){
    timer_tick_in(100); // 暂时设置一个时间片0.1s
}