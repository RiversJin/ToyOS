#include "vm.h"
#include "arch/aarch64/mmu.h"
#include "console.h"
#include "kalloc.h"
#include "lib/string.h"

static inline bool _check_MSB_valid(uint64_t va){
    uint64_t most_significant_bits = va >> (64 - 16) & 0xFFFF;
    return (most_significant_bits == 0xFFFF) || (most_significant_bits == 0);
}
/**
 * @brief 给定一个页表,和一个虚拟地址. 如果存在此虚拟地址对应项 返回对应的PTE
 * 当不存在时,若alloc为true,创建对应的映射; 若alloc为false返回null
 * rlevel为返回的页表级数 (在四级页表情况下, 可能有1,2,3三种取值) 如果不关注此值可以设为NULL
 * 
 * @param pagetable 
 * @param va 
 * @param alloc 
 * @param rlevel
 * @return pte_t* 
 */
static pte_t* walk(pagetable_t pagetable_ptr, uint64_t va,bool alloc,int* rlevel){
    if(!_check_MSB_valid(va)){
        panic("walk: invalid virtual address");
    }
    int level;
    for(level = 0; level<3; ++level){
        pte_t* pte = &pagetable_ptr[PX(level,va)];
        if( (*pte & PTE_VALID) != 0){
            pagetable_ptr = (pagetable_t)PA2VA(PTE_ADDR(*pte));
            if((*pte & PTE_BLOCK)) break;
        }else{
            if(!alloc) return NULL;
            if((pagetable_ptr = (pagetable_t)kalloc(PGSIZE)) == NULL){
                return NULL;
            }
            memset(pagetable_ptr,0,PGSIZE);
            *pte = VA2PA(pagetable_ptr) | PTE_PAGE;
        }
    }
    if(rlevel != NULL){
        *rlevel = level;
    }
    return &pagetable_ptr[PX(level,va)];
}

uint64_t walkaddr(pagetable_t pagetable_ptr,uint64_t va){
    pte_t* pte;
    if(!_check_MSB_valid(va)) return 0;
    pte = walk(pagetable_ptr,va,false,NULL);
    if(pte == NULL) return 0;
    if((*pte & PTE_VALID) == 0) return 0;
    if((*pte & PTE_USER) == 0) return 0;
    return (uint64_t)PA2VA(PTE_ADDR(*pte));
}

int mappages(pagetable_t pagetable_ptr,uint64_t va, uint64_t pa, uint64_t size, int perm){
    if(size == 0){
        panic("mappages: size couldn't be zero");
    }
    
    pte_t* pte;
    uint64_t _va = PGROUNDDOWN(va);
    uint64_t last = PGROUNDDOWN(va + size -1 );
    while(1){
        if((pte = walk(pagetable_ptr,_va,true,NULL)) == NULL){
            return -1;
        }
        if( *pte & PTE_VALID ){
            panic("mappages: remap");
            return -1;
        }
        *pte = PTE_ADDR(pa) | perm | PTE_VALID | PTE_PAGE | PTE_AF_USED | PTE_SH | PTE_AIDX_MEMORY;
        if(_va == last)break;
        _va += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}

void kvmmap(pagetable_t kernel_pagetable_ptr,uint64_t va, uint64_t pa, uint64_t size, uint32_t perm){
    if(mappages(kernel_pagetable_ptr,va,pa,size,perm)!= 0){
        panic("kvmmap: mappages failed");
    }
}