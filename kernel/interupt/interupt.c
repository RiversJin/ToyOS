#include "arch/aarch64/arm.h"
#include "interupt.h"

void enable_interrupt(){
    clear_daif();
}

void disable_interrupt(){
    set_daif();
}