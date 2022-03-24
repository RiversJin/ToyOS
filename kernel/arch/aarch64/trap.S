#include "include/linkage.h"
#include "board/raspi3/memlayout.h"
#include "include/trapframe.h"
#include "include/exception.h"

.macro kernel_entry el
    sub     sp, sp, #FRAME_SIZE
    stp	    x0, x1, [sp, #16 * 0]
    stp	    x2, x3, [sp, #16 * 1]
    stp	    x4, x5, [sp, #16 * 2]
    stp	    x6, x7, [sp, #16 * 3]
    stp	    x8, x9, [sp, #16 * 4]
    stp	    x10, x11, [sp, #16 * 5]
    stp	    x12, x13, [sp, #16 * 6]
    stp	    x14, x15, [sp, #16 * 7]
    stp	    x16, x17, [sp, #16 * 8]
    stp	    x18, x19, [sp, #16 * 9]
    stp	    x20, x21, [sp, #16 * 10]
    stp	    x22, x23, [sp, #16 * 11]
    stp	    x24, x25, [sp, #16 * 12]
    stp	    x26, x27, [sp, #16 * 13]
    stp	    x28, x29, [sp, #16 * 14]
    .if     \el == 0
        mrs     x21, sp_el0
    .else
        add     x21, sp, #FRAME_SIZE
    .endif
    stp     lr, x21, [sp, #16*15]
    mrs     x22, elr_el1
    mrs     x23, spsr_el1
    stp     x22, x23, [sp, #16*16]
.endm

.macro kernel_exit el, ret = 0
    ldp     x21,x22, [sp, #16*16]
    .if     \el == 0
        ldr     x23, [sp, #8*31]
        msr     sp_el0, x23
    .endif
    msr     elr_el1, x21
    msr     spsr_el1, x22
    .if     \ret
        ldr     x1, [sp, #8*1]
    .else
        ldp     x0, x1, [sp, #16*0]
    .endif
    ldp	    x2, x3, [sp, #16 * 1]
    ldp	    x4, x5, [sp, #16 * 2]
    ldp	    x6, x7, [sp, #16 * 3]
    ldp	    x8, x9, [sp, #16 * 4]
    ldp	    x10, x11, [sp, #16 * 5]
    ldp	    x12, x13, [sp, #16 * 6]
    ldp	    x14, x15, [sp, #16 * 7]
    ldp	    x16, x17, [sp, #16 * 8]
    ldp	    x18, x19, [sp, #16 * 9]
    ldp	    x20, x21, [sp, #16 * 10]
    ldp	    x22, x23, [sp, #16 * 11]
    ldp	    x24, x25, [sp, #16 * 12]
    ldp	    x26, x27, [sp, #16 * 13]
    ldp	    x28, x29, [sp, #16 * 14]
    ldr     lr, [sp, #8 * 30]
    add     sp, sp, #FRAME_SIZE
    eret
.endm

.macro ventry label
    .align 7
    b   \label
.endm

.macro verror label
    .align 7
    ldr x0, =(\label)
    b   trap_error
.endm

.macro irq_handler
    ldr     x1, =(handle_arch_irq)
    mov     x0, sp
    blr     x1
.endm

.text
.align 6
el_sync:
    kernel_entry 1 // 保存环境
    mrs x1, esr_el1
    mov x0, sp
    b el_sync_trap

el0_sync:
.align 6
    kernel_entry 0
    mrs x1, esr_el1
    mov x0, sp
    bl el0_sync_trap
    b user_trapret

.align 6
el_irq:
    kernel_entry 1
    irq_handler
    kernel_exit 1
ENDPROC(el_irq)

.align 6
el0_irq:
    kernel_entry 0
    irq_handler
    b   ret_to_user
ENDPROC(el_irq)

ret_to_user:
    kernel_exit 0

.align 11
ENTRY(vectors)
el_sp0:
    // 前四个使用sp0 全部不接受
    verror BAD_SYNC_SP0
    verror BAD_IRQ_SP0
    verror BAD_FIQ_SP0
    verror BAD_ERROR_SP0
el_spx:
    // 使用当前异常级别的指针
    ventry el_sync
    ventry el_irq
    verror BAD_FIQ_SPx
    verror BAD_ERROR_SPx
el0:
    ventry el0_sync
    ventry el0_irq
    verror BAD_FIQ_EL0
    verror BAD_ERROR_EL0
// 接下来的是AArch32造成的异常,不必兼容
el0_aarch32:
    verror BAD_AARCH32
    verror BAD_AARCH32
    verror BAD_AARCH32
    verror BAD_AARCH32
END(vectors)

ENTRY(user_trapret)
    #kernel_exit 0, ret = 1
    ldp     x21,x22, [sp, #16*16]

#    .if     \el == 0
    ldr     x23, [sp, #8*31]
    msr     sp_el0, x23
#    .endif
    msr     elr_el1, x21
    msr     spsr_el1, x22
#    .if     \ret
#        ldr     x1, [sp, #8*1]
#    .else
    ldp     x0, x1, [sp, #16*0]
#    .endif
    ldp	    x2, x3, [sp, #16 * 1]
    ldp	    x4, x5, [sp, #16 * 2]
    ldp	    x6, x7, [sp, #16 * 3]
    ldp	    x8, x9, [sp, #16 * 4]
    ldp	    x10, x11, [sp, #16 * 5]
    ldp	    x12, x13, [sp, #16 * 6]
    ldp	    x14, x15, [sp, #16 * 7]
    ldp	    x16, x17, [sp, #16 * 8]
    ldp	    x18, x19, [sp, #16 * 9]
    ldp	    x20, x21, [sp, #16 * 10]
    ldp	    x22, x23, [sp, #16 * 11]
    ldp	    x24, x25, [sp, #16 * 12]
    ldp	    x26, x27, [sp, #16 * 13]
    ldp	    x28, x29, [sp, #16 * 14]
    ldr     lr, [sp, #8 * 30]
    add     sp, sp, #FRAME_SIZE
    eret
END(user_trapret)