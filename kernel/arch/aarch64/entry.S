
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
    // 向scr_el3写入0101_1011_0001
    // [0]:1 NS none secure EL0与EL1处于Non-secure状态
    // [1]:0 IRQ 路由 当设为0时:(在EL3级别以下产生IRQ时,不会带入EL3;当在EL3时,不会产生IRQ) 当设为1时:无论哪一EL,产生IQR时都会带入到EL3
    // [2]:0 FIQ 路由 此位控制IRQ 与上面的IRQ路由同理
    // [3]:0 EA 路由 控制Abort与SErro路由 与上面IRQ同理
    // 
    // [5:4]:1 RES1 保留位 置1 注意Arm手册中,有两种保留位,RES1需要置1;RES0需要置0
    // [6]:0 RES0
    // [7]:1 SMD Secure Monitor Call 在EL1级别及以上禁用SMC指令(Secure Monitor Call)
    // [8]:1 HCE Hypervisor Call instruction enable. 使能hvc指令
    // [9]:0 SIF Secure instruction fetch 允许在stage1的地址翻译时从标记为非安全状态的内存中获取安全状态的指令 置1为不允许,咱们用不上Arm的secure,那么置0就行
    // [10]:1 若此位为0 则EL3级别以下只可使用aarch32 置1后可以使用aarch64
    // 再高位的东西咱们也用不上,就不继续讲解了,有兴趣的可以参考aarch64手册
    // https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/SCR-EL3--Secure-Configuration-Register
    mov     x1, #0b10110110001
    msr     scr_el3, x1

    // 向spsr_el3写入0b0011_1100_1100
    // [3:0]:1100 在el3状态时,使用进入el3前的栈指针,这意味着使用el2时的栈指针
    // [4]:0 此位为0,表示进入el3之前工作在aarch64为上,这样在el3通过eret命令跳转至el2状态时
    // [5]:0 RES0
    // [6]:1 FIQ interrupt mask 当跳转至EL2时,此位复制到EL2的PSTATE,也就关闭了FIQ中断
    // [7]:1 IRQ mask
    // [8]:1 SError mask
    // [9]:1 Debug mask
    // 剩下的用不上 有兴趣参考手册
    // https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/SPSR-EL3--Saved-Program-Status-Register--EL3-
    mov     x2, #0b01111001100
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
    /* Enable AArch64 in EL1. */
    // hcr_el2.rw = 1 设置在el1时使用aarch64 (否则为aarch32)
    mov     x1, #(1 << 31)
    // hcr_el2.swio = 1 设置在el1时 Set/Way指令有效
    orr     x1, x1, #(1 << 1)   /* SWIO hardwired on Pi3 */
    msr     hcr_el2, x1
    mrs     x1, hcr_el2
    /* Setup SCTLR access. */
    mov     x3, #0x0800
    movk    x3, #0x30d0, lsl #16
    // 此时 x2为'0b110000110100000000100000000000'
    // sctlr_el1.EOS = 1 设置在el1返回的异常为同步异常
    // sctlr_el1.TSCXT = 1  禁止el0访问SCXTNUM_EL0(这个寄存器可以用来防止利用分支预测进行侧信道攻击,没什么用,关掉了之)
    // sctlr_el1.EIS = 1 设置向进入el1的异常为同步异常
    // sctlr_el2.SPAN = 1 发生异常到EL1时,PSTATE.PAN 不变(PAN Privileged Access Never 阻止内核访问用户内存,这个保持不变就好,所以置1)
    // sctlr_el1.nTLSMD = 1 在通过A32或T32指令集(这两个都是aarch32)通过multi load/store访问Device Memory时,不要产生trap 我们将会用aarch64,将此位置1更多是为了兼容性
    // sctlr_el1.LSMAOE = 1 A32和T32在EL使用multi load/store时,顺序和中断行为与Armv8.0的定义相同。
    msr     sctlr_el1, x3
    /* Change execution level to EL1. */
    // spsr_el2 Saved Program Status Register of EL2 这个寄存器保存着产生异常并进入EL2的状态
    // 0x3c4 == '0b1111000100'
    // spsr_el2.M = 0b0_0100 M[4]=0代表进入EL2前CPU工作在aarch64,若置位为1说明工作在aarch32
    // M[3:0] = 0b0100 代表使用EL1t栈指针(即el0.sp)
    // spsr_el2.F = 1 当因为异常进入EL2时,PSTATE.F会复制到此位,当从中断返回时,此位会复制回PSTATE.F
    // 这样,即可在跳转到EL1时禁用FIQ中断 spsr_el2.I,A,D位置1同理,分别禁用IRQ,SError,Debug中断
    mov     x4, #0x3c4
    msr     spsr_el2, x4
    // 读取符号"el1"的位置
    adr     x4, jump_to_main
    // 设定从"el1"处进入当前的EL2状态,这样在使用eret指令后即可以EL1的状态跳转至"el1"
    msr     elr_el2, x4
    eret

jump_to_main:
// 此时CPU已经工作在EL1状态
    ldr     x0, =_entry
    mov     sp, x0 // 先暂时将_entry作为内核栈的顶部
    
    ldr     x1, =main
    br      x1

