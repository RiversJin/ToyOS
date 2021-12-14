#include "console.h"
#include "memory/kalloc.h"
#include "proc/proc.h"
#include "interupt/interupt.h"

__attribute__((noreturn))
void main(){
    if(cpuid() == 0){
        console_init();
        alloc_init();
        exception_handler_init();
        init_awake_ap_by_spintable();
    }
    cprintf("main: [CPU %d] started.\n", cpuid());
    while(1){}
}