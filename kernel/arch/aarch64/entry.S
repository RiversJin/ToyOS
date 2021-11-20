#include "mmu.h"
#include "./board/raspi3/memlayout.h"
#include "sysreg.h"

.section ".text._entry"

.global _entry

_entry:
    // 读取当前CPUID,如果id不为0,使其跳至halt休眠
    // mrs -- Move the contents of a special register to a general-purpose register.
    // mpidr_el1 用来读取核心ID用
    mrs     x1, mpidr_el1
    and     x1, x1, #3 // 只取[2:0]即可拿到ID
    cbnz    x1, halt // Compare and Branch on Non-Zero 若不为0,跳转至指定位置 
    mrs     x2, CurrentEL // 读取当前异常级别 在此寄存器中 我们需要的是[3:2]
    and     x2, x2, #0b1100
    cmp     x2, #0b1000
    beq     switch_to_el1_from_el2 // branch on equal
    bgt     switch_to_el2_from_el3
    b       halt // 这种是未定义情况 让cpu halt住方便调试

halt:
    wfe     // wait for event 休眠CPU
    b halt

switch_to_el2_from_el3:
    // 向scr_el3写入0101_1011_0001 0x5B1
    ldr     x1, =(SCR_VALUE)
    msr     scr_el3, x1
    // 向spsr_el3写入0001_1100_1001
    ldr     x2, =(SPSR_EL3_VALUE)
    msr     spsr_el3, x2
    // 加载函数switch_to_el1_from_el2的地址,设置从EL3返回时的地址,这样调用eret即可跳转至此函数继续执行
    adr     x3, switch_to_el1_from_el2
    msr     elr_el3, x3
    eret

switch_to_el1_from_el2:
    /* 使能el1与el0对el物理计数器reg,定时器reg的访问 */
    mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr
    // hcr_el2.rw = 1 设置在el1时使用aarch64 (否则为aarch32)
    mov     x1, #(1 << 31)
    // hcr_el2.swio = 1 设置在el1时 Set/Way指令有效
    orr     x1, x1, #(1 << 1)   /* SWIO hardwired on Pi3 */
    msr     hcr_el2, x1
    mrs     x1, hcr_el2

    /* Setup SCTLR access. */
    ldr     x3, =(SCTLR_VALUE_MMU_DISABLED_VALUE)
    msr     sctlr_el1, x3
    
    ldr     x4, =(SPSR_EL2_VALUE)
    msr     spsr_el2, x4
    // 读取符号"jump_to_main"的位置
    adr     x4, jump_to_main
    // 设定从"jump_to_main"处进入当前的EL2状态,这样在使用eret指令后即可以EL1的状态跳转至"jump_to_main"
    msr     elr_el2, x4
    eret

jump_to_main:
// 此时CPU已经工作在EL1状态
    /* 配置页表 */
    adr     x2, kpgd // 取得页表地址
    msr     ttbr0_el1, x2
    msr     ttbr1_el1, x2
    ldr     x3, =(TCR_VALUE)
    msr     tcr_el1, x3
    ldr     x4, =(MAIR_VALUE)
    msr     mair_el1, x4

    /* 使能MMU */
    mrs     x5, sctlr_el1
    orr     x5, x5, #SCTLR_MMU_ENABLED
    msr     sctlr_el1, x5     

    ldr     x0, =_entry
    mov     sp, x0 // 先暂时将_entry作为内核栈的顶部
    
    ldr     x1, =main
    br      x1

