#include "vm.h"
#include "arch/aarch64/mmu.h"
#include "console.h"
#include "kalloc.h"
#include "lib/string.h"

static inline bool _check_MSB_valid(uint64_t va){
    uint64_t most_significant_bits = va >> (64 - 16) & 0xFFFF;
    return (most_significant_bits == 0xFFFF) || (most_significant_bits == 0);
}
pte_t* walk(pagetable_t pagetable_ptr, uint64_t va,bool alloc){
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
    return &pagetable_ptr[PX(level,va)];
}

uint64_t walkaddr(pagetable_t pagetable_ptr,uint64_t va){
    pte_t* pte;
    if(!_check_MSB_valid(va)) return 0;
    pte = walk(pagetable_ptr,va,false);
    if(pte == NULL) return 0;
    if((*pte & PTE_VALID) == 0) return 0;
    if((*pte & PTE_USER) == 0) return 0;
    return (uint64_t)PA2VA(PTE_ADDR(*pte));
}
/*
int mappages(pagetable_t pagetable_ptr,uint64_t va, uint64_t pa, uint64_t size, int perm){
    if(size == 0){
        panic("mappages: size couldn't be zero");
    }
    uint64_t _va = 
}
*/