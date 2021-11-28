#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#define PHYSTOP 0x3F000000 // 在这之后就是MMIO的区域 也就是说 可用的物理内存范围就是从0-0x3eFFFFFF 共1008M
#define TOTAL_STOP 0x40000000
/* 在我们使用48位有效虚拟地址的情况下 地址空间分为两个部分 
   0x0000_0000_0000_0000至0x0000_FFFF_FFFF_FFFF 和 
   0xFFFF_0000_0000_0000至0xFFFF_FFFF_FFFF_FFFF 
   而我们会将内核放在高地址区,故KERNEL_BASE定义为此
*/
#define KERNEL_BASE 0xFFFF000000000000

#ifndef __ASSEMBLER__
#include <stdint.h>
// 对于给定的一个恒等映射的高地址区虚拟地址 将其转换为物理地址
#define VA2PA(a) (((uint64_t)(a)) - KERNEL_BASE)
#define PA2VA(a) (((uint64_t)(a)) + KERNEL_BASE)
#endif


#endif // MEMLAYOUT_H