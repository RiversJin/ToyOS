#ifndef MMU_H
#define MMU_H

#define PGSIZE 4096
/* 
    在AARCH64的MMU页表项中,在4K页粒度下,格式如下
    |63 --- 52|51 --- 48|47 --- 12|11 --- 2| 1 | 0|
    [63:52] : Upper attributes
    [11: 2] : Lower attributes
    这两个合起来才是所有的属性,在这些属性中,我们要关注的位有这些
    [47:12] : 输出地址(有可能是下一级页表的地址,也有可能是物理页框的地址)
    在Stage1翻译的情况下,我们要关注的有
    [54] UXN/XN Execute-never bit 也就是说,此位置1时,禁止EL0特权时执行此内存内容
    [53] PXN    Priviledged execute-never bit 当此位置1时,禁止EL1特权执行此内存内容
    [10] AF Access flag 当此位置0时,访问此位会产生异常 可以使用此位用来实现页面回收 如果不想要这个功能 置1即可
    [9:8] SH Shareability field 控制共享域 00 Non-share 10 Outer-Share 11 Inner-Share
    [7:6] AP Access permissions 控制读写权限
            AP      EL1     EL0
            00      R/W     None
            01      R/W     R/W
            10      R       None
            11      R       R
    [4:2] AttrIdx 控制内存属性的索引,指向寄存器MAIR_ELx的八个元素,而这个寄存器中,需要哪些属性,在哪个位置,由用户决定
    [1:0] 页表类型,参见MM_TYPE_INVALID MM_TYPE_BLOCK MM_TYPE_TABLE
*/

#define MM_TYPE_INVALID 0b00
#define MM_TYPE_BLOCK 0b01
#define MM_TYPE_TABLE 0b11L

/* Memory Attributes 控制这个页表项对应的内存区域的内存类型,缓存策略 */
#define MA_DEVICE_nGnRnE_Flags 0x0   // 设备内存 禁止聚集(non Gathering) 禁止重排(non re-order) 禁止提前的写入ACK(Early Write Acknowledgement)
#define MA_MEMORY_Flags 0xFF         // 普通内存 使用写回写分配 读分配 (最快的形式,各种缓存buff拉满)
#define MA_MEMORY_NoCache_Flags 0x44 // 普通内存 禁止所有缓存策略

// MAIR_ELx 可以放置8种Memory Attributes,但是我们只需要这三种就够了
#define MA_DEVICE_nGnRnE 0
#define MA_MEMORY 1
#define MA_MEMORY_NoCache 2

#define PTE_AIDX_DEVICE_nGnRn (MA_DEVICE_nGnRnE << 2)
#define PTE_AIDX_MEMORY (MA_MEMORY << 2)
#define PTE_AIDX_MEMORY_NOCACHE (MA_MEMORY_NoCache << 2)
// 这个值 我们一会放到MAIR_EL1 寄存器中
#define MAIR_VALUE \
    (MA_DEVICE_nGnRnE_Flags << (8 * MA_DEVICE_nGnRnE)) | (MA_MEMORY_Flags << (8 * MA_MEMORY)) | (MA_MEMORY_NoCache_Flags << (8 * MA_MEMORY_NoCache))

#define AP_RW 0           // read and write
#define AP_RO 1           // read only
#define AP_PRIVILEGED 0   // 特权级别才可访问
#define AP_UNPREVILEGED 1 // 允许EL0访问

#define PTE_KERNEL (AP_PRIVILEGED << 6)
#define PTE_USER (AP_UNPREVILEGED << 6)
#define PTE_RW (AP_RW << 7)
#define PTE_RO (AP_RO << 7)
#define PTE_SH (0b11 << 8) // 对于SMP系统来说,全部设置为Inner-share就可以了
#define PTE_AF_USED (1 << 10)
#define PTE_AF_UNUSED (0 << 10)

// 取得页表项中的物理地址 我们需要[47:12]范围的内容
#define PTE_ADDR(pte) ((uint64_t)(pte)&0xfffffffff000)
// 取得页表项中的flags 需要除了[47:12]以外的内容
#define PTE_FLAG(pte) ((uint64_t)(pte) & (~0xfffffffff000))

// 默认页表项 带缓存的memory,inner share,特权读写 AF置位
#define PTE_NORMAL_MEMORY ( MM_TYPE_BLOCK | PTE_AIDX_MEMORY | PTE_SH | PTE_AF_USED | PTE_RW )
// 设备内存页表项
#define PTE_DEVICE_MEMORY (MM_TYPE_BLOCK | PTE_AIDX_DEVICE_nGnRn | PTE_AF_USED)
// 指向下一级页表
#define PTE_TABLE MM_TYPE_TABLE
/* 接下来配置TCR 翻译控制寄存器 */
/*
    TCR_ELx的格式如下
    [63:39] RES0
    [38] TBI1 Top Byte ignored 指示地址的顶部字节是用于TTBR1_EL1区域的地址匹配(置0),还是被忽略并用于标记的地址(置1)
    [37] TBI0 同TBI1 只不过是TTBR0_EL1区域地址
    [36] ASID Size 置1时ASID为16位 0为8位 若实现不支持16位,那么此位视为RES0
    [35] RES0
    [34:32] 中间物理地址大小
        000 32bits 4G
        001 26bits 64G
        010 40bits 1TB
        011 42bits 4TB
        100 44bits 16TB
        101 48bits 256TB
        其余值视为101,但未来架构可能修改
    [31:30] TG1 TTBR1_EL1的粒度
        01 16K
        10 4K
        11 64K
    [29:28] SH1 TTBR1_EL1的Shareability
        01 Non-shareable
        10 Outer-shareable
        11 Inner-shareable
    [27:26] ORGN1 TTBR1_EL1的Outer cacheability
        00 Outer Non-cacheable
        01 Outer 写回 写分配
        10 写直通
        11 写回 不写分配
    [25:24] IRGN1 TTBR1_EL1的Inner cacheability 编码与ORNG1相同
    [23] EPD1 当使用TTBR1时 若此为置1,当遍历页表时TLB发生miss,会发生translation fault
    [22] 当为0时,TTBR0_EL1.ASID定义ASID 为1时TTBR1_EL1.ASID定义ASID
    [21:16] T1SZ  TTBR1_EL1的内存区域为64-T1SZ 比如我们需要48位的虚地址空间,那就需要将TxSZ设置为64-48
    [15:14] TG0 TTBR0_EL1的粒度
        00 4K
        01 64K
        10 16K
    [13:12] SH0 TTBR0_EL1的Shareability 与SH1编码相同
    [11:10] ORGN0 TTBR0_EL1的Outer cacheability 编码与ORGN1相同
    [9:8] IRGN0
    [7] EPD0 见EPD1
    [6] RES0
    [5:0] T0SZ
*/
#define TCR_IPS (0 << 32)
#define TCR_TG1_4K (0b10 << 30)
#define TCR_SH1_INNER (0b11 << 28)
#define TCR_ORGN1_IRGN1_WRITEBACK_WRITEALLOC ((0b01 << 26) | (0b01 << 24))
#define TCR_TG0_4K (0 << 14)
#define TCR_SH0_INNER (0b11 << 12)
#define TCR_ORGN0_IRGN0_WRITEBACK_WRITEALLOC ((0b01 << 10) | (0b01 << 8))
#define TCR_VALUE                                                        \
    (TCR_IPS |                                                           \
     TCR_TG1_4K | TCR_SH1_INNER | TCR_ORGN1_IRGN1_WRITEBACK_WRITEALLOC | \
     TCR_TG0_4K | TCR_SH0_INNER | TCR_ORGN0_IRGN0_WRITEBACK_WRITEALLOC)


/* 在System Control Register 中 使能MMU的值 */
#define SCTLR_MMU_ENABLED               (1 << 0)
#endif // MMU_H