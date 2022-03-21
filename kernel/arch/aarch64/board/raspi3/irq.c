#include "irq.h"
#include "../../timer.h"
#include "../../arm.h"
void irqinit(){
    // AUX里面有mini-uart的中断源
    put32(ENABLE_IRQS_1, AUX_INT);

    // 将所有GPU中断路由给0号核心
    put32(GPU_INTERUPTS_ROUTING, GPUFIQ2CORE(0)|GPUIRQ2CORE(0));
    
    // TOTO: timer_init();


}