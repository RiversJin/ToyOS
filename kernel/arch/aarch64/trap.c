#include "board/raspi3/local_peripherals.h"
#include "board/raspi3/irq.h"
#include "include/exception.h"
#include "include/trapframe.h"
#include "arm.h"
#include "timer.h"
#include <stdint.h>
#include "printf.h"
#include "proc/proc.h"

#include "board/raspi3/uart.h"
extern int64_t syscall(struct trapframe *frame);

void trap_error(uint64_t error_type){
    switch(error_type){
        case(BAD_SYNC_SP0):panic("BAD_SYNC_SP0");break;
        case(BAD_IRQ_SP0):panic("BAD_IRQ_SP0");break;
        case(BAD_FIQ_SP0):panic("BAD_FIQ_SP0");break; 
        case(BAD_ERROR_SP0):panic("BAD_ERROR_SP0");break;
        case(BAD_FIQ_SPx):panic("BAD_FIQ_SPx");break;
        case(BAD_ERROR_SPx):panic("BAD_ERROR_SPx");break;
        case(BAD_FIQ_EL0):panic("BAD_FIQ_EL0");break;
        case(BAD_ERROR_EL0):panic("BAD_ERROR_EL0");break;
        case(BAD_AARCH32):panic("BAD_AARCH32");break;
    }
    panic("Unknown exception");
}

void el_sync_trap(struct trapframe * frame_ptr,uint64_t esr){
    uint64_t exception_class_id = (esr >> 26)&0b111111;
    uint64_t instruction_length = (esr & (1<<25)) != 0;
    panic("el_sync_trap: exception class id: 0x%x, instruction length: %d",exception_class_id, instruction_length);
}
void el0_sync_trap(struct trapframe * frame,uint64_t esr){
    cprintf("el0_sync_trap.\n");
    uint64_t exception_class_id = (esr >> 26)&0b111111;
    uint64_t iss = (esr & ISS_MASK);
    if(exception_class_id == EC_SVC64 && iss == 0){
        struct proc *p = myproc();
        if(p->killed){
            exit(-1);
        }
        p->tf = frame;
        //enable_interrupt();
        p->tf->regs[0] = syscall(frame);
        
        if(p->killed){
            exit(-1);
        }
    }else{
        panic("el0_sync_trap: exception class id: 0x%x, iss: %d",exception_class_id, iss);
    }

}


uint32_t read_irq_src(){
    return get32(COREn_IRQ_SOURCE(cpuid()));
}

extern void uartintr(void);

void handle_arch_irq(struct trapframe * frame_ptr){
    uint32_t irq_src = read_irq_src();
    // 如果当前核心有时间中断
    // cprintf("..");
    if(irq_src & IRQ_CNTPNSIRQ){
        // yield();
        timer_reset();
        
    }else{

        uint32_t irq_pending_1 = get32(IRQ_PENDING_1);
        uint32_t irq_pending_2 = get32(IRQ_PENDING_2);
        if(irq_pending_1 & AUX_INT){
            uartintr();
        }
        
    }
}

void exception_handler_init(void){
    extern char vectors[];
    load_vbar_el1(vectors);
    isb();
}
