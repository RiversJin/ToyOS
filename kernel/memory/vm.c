#include "vm.h"
#include "arch/aarch64/mmu.h"
#include "console.h"


pte_t* walk(pagetable_t pagetable_ptr, uint64_t va,bool alloc){
    uint64_t most_significant_bit = va >> (64 - 16) & 0xFFFF;
    if(most_significant_bit!=0 || most_significant_bit!= 0xFFFF){
        panic("walk: invalid virtual address");
    }
    for(int level = 0; level<3; ++level){
        pte_t* pte = &pagetable_ptr[PX(level,va)];
        if( (*pte & PTE_VALID) != 0){
            
        }
    }
}