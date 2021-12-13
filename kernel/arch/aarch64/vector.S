#include "include/linkage.h"
#include "board/memlayout.h"
#include "include/exception.h"

.macro ventry label
    .align 7
    b   \label
.endm

.macro verror label
    .align 7
    ldr x0, =(\label)
    b   trap_error
.endm

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