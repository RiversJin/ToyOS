#include "arch/aarch64/arm.h"
#include "interupt.h"
#include <stdbool.h>

void enable_interrupt(){
    clear_daif();
}

void disable_interrupt(){
    set_daif();
}

bool is_interupt_enabled(){
    return get_daif() == 0;
}