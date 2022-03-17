#include "console.h"
#include "memory/kalloc.h"
#include "proc/proc.h"
#include "interupt/interupt.h"
#include "arch/aarch64/timer.h"

#include "buffer/buf.h"
#include "driver/mmc/sd.h"

extern uint32_t read_irq_src();

__attribute__((noreturn))
void main(){
    if(cpuid() == 0){
        console_init();
        alloc_init();
        init_proc();
        exception_handler_init();
        timer_init();
        enable_interrupt();

        sd_init();
        //init_awake_ap_by_spintable();
        init_user();
    }
    
    // timer_init();
    // enable_interrupt();
    cprintf("main: [CPU %d] started.\n", cpuid());
    scheduler();
    while(1){
        

    }
}