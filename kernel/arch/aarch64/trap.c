#include "board/raspi3/local_peripherals.h"
#include "include/exception.h"
#include "include/trapframe.h"
#include "arm.h"
#include <stdint.h>
#include "console.h"
#include "proc/proc.h"

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
    uint64_t exception_class_id = esr >> 26;
    uint64_t instruction_length = (esr & (1<<25)) != 0;
    panic("el_sync_trap: exception class id: %x, instruction length: %d",exception_class_id, instruction_length);
}
void el0_sync_trap(struct trapframe * frame,uint64_t esr){
    uint64_t exception_class_id = esr >> 26;
    uint64_t instruction_length = (esr & (1<<25)) != 0;
    panic("el0_sync_trap: exception class id: %x, instruction length: %d",exception_class_id, instruction_length);
}


uint32_t read_irq_src(){
    return get32(COREn_IRQ_SOURCE(cpuid()));
}
void handle_arch_irq(struct trapframe * frame_ptr){
    uint32_t irq_src = read_irq_src();

}

void exception_handler_init(void){
    extern char vectors[];
    load_vbar_el1(vectors);
    isb();
}
