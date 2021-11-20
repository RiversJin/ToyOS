#ifndef SYSREG_H
#define SYSREG_H

/**
 * @file sysreg.h
 * @author Rivers Jin (riversjin@foxmail.com)
 * @brief 将配置CPU各控制寄存器的值收归这里
 * @date 2021-11-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/* SCR_EL3 Secure Configuration Register EL3 */
/*
    [0]:1 NS none secure EL0与EL1处于Non-secure状态
    [1]:0 IRQ 路由 当设为0时:(在EL3级别以下产生IRQ时,不会带入EL3;当在EL3时,不会产生IRQ) 当设为1时:无论哪一EL,产生IQR时都会带入到EL3
    [2]:0 FIQ 路由 此位控制IRQ 与上面的IRQ路由同理
    [3]:0 EA 路由 控制Abort与SErro路由 与上面IRQ同理

    [5:4]:1 RES1 保留位 置1 注意Arm手册中,有两种保留位,RES1需要置1;RES0需要置0
    [6]:0 RES0
    [7]:1 SMD Secure Monitor Call 在EL1级别及以上禁用SMC指令(Secure Monitor Call)
    [8]:1 HCE Hypervisor Call instruction enable. 使能hvc指令
    [9]:0 SIF Secure instruction fetch 允许在stage1的地址翻译时从标记为非安全状态的内存中获取安全状态的指令 置1为不允许,咱们用不上Arm的secure,那么置0就行
    [10]:1 若此位为0 则EL3级别以下只可使用aarch32 置1后可以使用aarch64
    再高位的东西咱们也用不上,就不继续讲解了,有兴趣的可以参考aarch64手册
    https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/SCR-EL3--Secure-Configuration-Register
*/
#define SCR_NS (1)
#define SCR_EXCEPTION_ROUTE (0 << 1)
#define SCR_RES1 (0b11 << 4)
#define SCR_RES0 (0 << 6)
#define SCR_SMD_DISABLE (1 << 7)
#define SCR_HCE_ENABLE (1 << 8)
#define SCR_SIF (0 << 9)
#define SCR_RW (1 << 10)

// SCR_VALUE 的值应该是 0b0101_1011_0001 0x5B1 1457
#define SCR_VALUE                                         \
    (SCR_NS | SCR_EXCEPTION_ROUTE | SCR_RES1 | SCR_RES0 | \
     SCR_SMD_DISABLE | SCR_HCE_ENABLE | SCR_SIF | SCR_RW)

/* SPSR_ELx Saved Program Status Register */
/*
    可以理解为 此寄存器保存了CPSR_EL(x-1),所以当使用eret指令后,
    此寄存器状态控制了返回至EL(x-1)的状态
    向spsr_el3写入0b0001_1100_1001
	[3:0]:1001 
	[4]:0 此位为0,表示进入此EL之前工作在aarch64状态
	[5]:0 RES0
	[6]:1 FIQ interrupt mask 当跳转至EL2时,此位复制到EL2的PSTATE,也就关闭了FIQ中断
	[7]:1 IRQ mask
	[8]:1 SError mask
	[9]:0 Debug mask
	剩下的用不上 有兴趣参考手册
	https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/SPSR-EL3--Saved-Program-Status-Register--EL3-
*/
#define SPSR_MASK_FIS (0b111 << 6)
#define SPSR_EL1h (0b0101 << 0)
#define SPSR_EL2h (0b1001 << 0)
// SPSR_EL3_VALUE 的值应该是 0b0001_1100_1001 0x1C9 457
#define SPSR_EL3_VALUE (SPSR_MASK_FIS | SPSR_EL2h)
// SPSR_EL2_VALUE 的值应该是 0b0001_1100_0101 0x1C5 453
#define SPSR_EL2_VALUE (SPSR_MASK_FIS | SPSR_EL1h)

/* SCTLR_EL1, System Control Register (EL1). */
#define SCTLR_RESERVED \
    ((3 << 28) | (3 << 22) | (1 << 20) | (1 << 11) | (1 << 8) | (1 << 7))
#define SCTLR_EE_LITTLE_ENDIAN (0 << 25)
#define SCTLR_E0E_LITTLE_ENDIAN (0 << 24)
#define SCTLR_I_CACHE (1 << 12)
#define SCTLR_D_CACHE (1 << 2)
#define SCTLR_MMU_DISABLED (0 << 0)
#define SCTLR_MMU_ENABLED (1 << 0)

#define SCTLR_VALUE_MMU_DISABLED_VALUE \
    (SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_E0E_LITTLE_ENDIAN | SCTLR_I_CACHE | SCTLR_D_CACHE | SCTLR_MMU_DISABLED)

#endif //SYSREG_H