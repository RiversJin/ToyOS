#include "console.h"
#include "printf.h"
#include "memory/kalloc.h"
#include "proc/proc.h"
#include "interupt/interupt.h"
#include "arch/aarch64/timer.h"
#include "file/file.h"
#include "buffer/buf.h"
#include "driver/mmc/sd.h"
#include "lib/string.h"

extern void irqinit();
extern char edata[], edata_end[];

__attribute__((noreturn))
void main(){
    if(cpuid() == 0){
        memset(edata,0, edata_end - edata);
        consoleinit();
        printfinit();
        cprintf("kernel booing...\n");
        
        alloc_init();
        procinit();
        exception_handler_init();
        irqinit();
        timer_init();
        enable_interrupt();

        sd_init();
        iinit();
        fileinit();

        //init_awake_ap_by_spintable();
        init_user();
        //scheduler();
        //while(1);
    }
    
    // timer_init();
    // enable_interrupt();
    cprintf("main: [CPU %d] started.\n", cpuid());
    scheduler();
    while(1){
        

    }
}