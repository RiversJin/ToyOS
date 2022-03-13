#ifndef VM_H
#define VM_H
#include <stdbool.h>
#include <stddef.h>
#include "arch/aarch64/arm.h"


/**
 * @brief 给定一个页表和一个虚拟地址,返回其物理地址 只用于用户态的页表
 * 
 * @param pagetable_ptr 页表指针 
 * @param va 虚拟地址
 * @return uint64_t 物理地址
 */
uint64_t walkaddr(pagetable_t pagetable_ptr,uint64_t va);

/**
 * @brief 在内核页表上添加一个映射 不会刷新TLB
 * 
 * @param kernel_pagetable_ptr 内核页表
 * @param va 虚拟地址
 * @param pa 物理地址
 * @param size 
 * @param perm 权限位
 */
void kvmmap(pagetable_t kernel_pagetable_ptr,uint64_t va, uint64_t pa, uint64_t size, uint32_t perm);

/**
 * @brief 在指定页表上,创建va-pa的映射. 非零值代表失败(内存没地方了)
 *        使用4级页表 不要用于内核页表(内核页表含有block项)
 * @param pagetable_ptr 
 * @param va 
 * @param pa 
 * @param size 
 * @param perm 
 * @return int 
 */
int mappages(pagetable_t pagetable_ptr,uint64_t va, uint64_t pa, uint64_t size, int perm);

pagetable_t uvmcreate(void);
void uvminit(pagetable_t pagetable, uint8_t *src, uint8_t sz);
uint64_t uvmalloc(pagetable_t pagetable,uint64_t oldsz, uint64_t newsz);
uint64_t uvmdealloc(pagetable_t, uint64_t, uint64_t);
int uvmcopy(pagetable_t, pagetable_t, uint64_t);
void uvmfree(pagetable_t, uint64_t);
void unmunmap(pagetable_t pagetable,uint64_t va,uint64_t npages, int do_free);
void unmclear(uint64_t *pgdir, uint8_t *va);
int copyout(pagetable_t pagetable, uint64_t dstva, char* src,  uint64_t len);
#endif // VM_H