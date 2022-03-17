/*
    函数原型为 void swtch(struct context **old, struct context *new);
    将当前的context 保存到栈上 然后通知old
    随后将给定的new context 装载到寄存器中
*/
#include "include/context.h"
.global swtch
swtch:
    # 首先保存当前的callee-saved register
    sub     sp, sp, #CONTEXT_SIZE
    stp     x19, x20, [sp, #16*0]
    stp     x21, x22, [sp, #16*1]
    stp     x23, x24, [sp, #16*2]
    stp     x25, x26, [sp, #16*3]
    stp     x27, x28, [sp, #16*4]
    stp     x29, x30, [sp, #16*5]
    # 将这个struct context 赋给**old
    # **old 在x0
    mov     x19, sp
    str     x19, [x0]
    # 获得给定的struct context* new
    # 因为它是第二个参数 所以 *new 在x1
    mov     sp, x1
    # 装载新的context 到寄存器上
    ldp     x19, x20, [sp, #16*0]
    ldp     x21, x22, [sp, #16*1]
    ldp     x23, x24, [sp, #16*2]
    ldp     x25, x26, [sp, #16*3]
    ldp     x27, x28, [sp, #16*4]
    ldp     x29, x30, [sp, #16*5]
    add     sp, sp, #CONTEXT_SIZE
    ret
