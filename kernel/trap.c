#include "arch/aarch64/include/exception.h"
#include <stdint.h>
#include "console.h"
__attribute__((noreturn))
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
        default:panic("Unknown exception");  
    }
}

