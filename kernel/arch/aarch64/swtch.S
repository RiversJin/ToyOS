/*
    函数原型为 void swtch(struct context *old, struct context *new);
    将当前的context 保存到栈上 然后通知old
    随后将给定的new context 装载到寄存器中
*/
#include "include/context.h"
.global swtch
swtch:
    # 首先保存当前的callee-saved register
    # 第一个参数在x0
    stp     x19, x20, [x0, #16*0]
    stp     x21, x22, [x0, #16*1]
    stp     x23, x24, [x0, #16*2]
    stp     x25, x26, [x0, #16*3]
    stp     x27, x28, [x0, #16*4]
    stp     x29, x30, [x0, #16*5]

    # 获得给定的struct context* new
    # 因为它是第二个参数 所以 *new 在x1
    # 装载新的context 到寄存器上
    ldp     x19, x20, [x1, #16*0]
    ldp     x21, x22, [x1, #16*1]
    ldp     x23, x24, [x1, #16*2]
    ldp     x25, x26, [x1, #16*3]
    ldp     x27, x28, [x1, #16*4]
    ldp     x29, x30, [x1, #16*5]

    ret
