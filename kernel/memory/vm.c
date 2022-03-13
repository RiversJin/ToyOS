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
void unmunmap(pagetable_t pagetable,uint64_t va,uint64_t npages, int do_free){
    if((va % PGSIZE) != 0 ){
        panic("unmunmap: invalid virtual address, which is not aligned.\n");
    }
    for(uint64_t a = va; a < va + npages*PGSIZE; a += PGSIZE){
        pte_t* pte = walk(pagetable,va,0,NULL);
        if(pte != NULL){
            panic("unmunmap: walk. \n");
        }
        if((*pte & PTE_VALID) == 0){
            panic("unmunmap: not mapped. \n");
        }
        // TODO: 验证此pte是否为页表的叶子节点
        if(do_free){
            uint64_t pa = VA2PA(PTE_ADDR(*pte));
            kfree((void*)pa);
        }
        *pte = 0;
    }
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

/**
 * @brief 创建一个空的用户页表 如果没空间了就返回0
 * 
 * @return pagetable_t 
 */
pagetable_t uvmcreate(void){
    pagetable_t pagetable;
    pagetable = (pagetable_t)kalloc(PGSIZE);
    if(pagetable==NULL){
        return NULL;
    }
    memset(pagetable,0,PGSIZE);
    return pagetable;
}
/**
 * @brief 将用户态程序的初始代码放在给定页表的地址0处
 * 只用于最初的用户进程
 * 并且要保证sz小于一个页的大小(以后可能会取消这个限制 现在就这样)
 * @param pagetable 
 * @param src 
 * @param sz 
 */
void uvminit(pagetable_t pagetable, uint8_t *src, uint8_t sz){
    if(sz >= PGSIZE){
        panic("uvmminit: more than a page.");
    }
    uint8_t *mem = kalloc(PGSIZE);
    if(mem==NULL){
        panic("uvminit: memory allocation failed.");
    }
    memset(mem,0,PGSIZE);
    memmove(mem,src,sz);
    mappages(pagetable,0,src,sz,PTE_USER|PTE_RW|PTE_PAGE);
}

/**
 * @brief 为进程分配虚拟内存 不需要对齐 如果成功返回新的大小 否则返回0
 * 
 * @param pagetable 
 * @param oldsz 
 * @param newsz 
 * @return uint64_t 
 */
uint64_t uvmalloc(pagetable_t pagetable,uint64_t oldsz, uint64_t newsz){
    if(newsz<oldsz){
        return oldsz;
    }
    oldsz = PGROUNDUP(oldsz);
    for(uint64_t *a = oldsz; a < newsz; a+=PGSIZE){
        uint8_t *mem = kalloc(PGSIZE);
        if(mem == NULL){
            uvmdealloc(pagetable,a,oldsz);
        }
        memset(mem,0,PGSIZE);
        if(mappages(pagetable,a,mem,PGSIZE,PTE_USER|PTE_RW|PTE_PAGE)){
            kfree(mem);
            uvmdealloc(pagetable,a,oldsz);
            return 0;
        }
    }
    return newsz;
}
/**
 * @brief 给定一个父进程的页表,将此进程的内存拷贝到子进程的页表中
 * 内容和页表项都会复制
 * 当执行失败时 会清理所有在此过程中分配的页
 * @param old 
 * @param new 
 * @param sz 
 * @return int 0代表成功 -1失败
 */
int uvmcopy(pagetable_t old, pagetable_t new, uint64_t sz){
    
    for(uint64_t i = 0; i < sz; i += PGSIZE){
        pte_t *pte = walk(old, i, false, NULL);
        if(pte == NULL){
            panic("uvmcopy: pte should exist. \n");
        }
        if(((*pte)&PTE_VALID) == 0){
            panic("uvmcopy: page note present. \n");
        }
        uint64_t pa = VA2PA(PTE_ADDR(*pte));
        uint64_t flags = PTE_FLAG(*pte);
        uint8_t *mem = kalloc(PGSIZE);
        if(mem == NULL){
            unmunmap(new, 0, i/PGSIZE, 1);
            return -1;
        }
        memmove(mem, (void*)pa, PGSIZE);
        if(mappages(new, i, mem, PGSIZE, flags) != 0){
            kfree(mem);
            unmunmap(new, 0, i/PGSIZE, 1);
            return -1;
        }
    }
    return 0;
}
/**
 * @brief free用户态内存 然后释放存放页表的页
 * 
 * @param pagetable 
 * @param level
 */
void uvmfree(pagetable_t pagetable, uint64_t level){
    if(pagetable == NULL || level < 0){
        return;
    }
    // TODO: ???
    if(PTE_FLAG(pagetable)){
        panic("uvmfree: invalid pte. \n");
    }
    if(level == 0){
        kfree(pagetable);
        return;
    }
    for(uint64_t i = 0;i < ENTRYSZ; ++i){
        if(pagetable[i] & PTE_VALID){
            uint64_t *v = (uint64_t*)PA2VA(PTE_ADDR(pagetable[i]));
            vm_free(v,level-1);
        }
        kfree(pagetable);
    }
}
/**
 * @brief 将一个PTE标记为用户态不可使用 用来给stack做内存保护
 * 
 * @param pgdir 
 * @param va 
 */
void unmclear(uint64_t *pgdir, uint8_t *va){
    uint64_t *pte = walk(pgdir, va, 0, NULL);
    if(pte == NULL){
        panic("uvmclear: failed to get the PTE. \n");
    }
    *pte  &= ~PTE_USER;
}

/**
 * @brief 将数据从内核复制到用户
 * 
 * @param pagetable 
 * @param dstva 
 * @param src 
 * @param len 
 * @return int 
 */
int copyout(pagetable_t pagetable, uint64_t dstva, char* src,  uint64_t len){
    while(len > 0){
        uint64_t va = PGROUNDDOWN(dstva);
        uint64_t pa = walkaddr(pagetable,va);
        if(pa == NULL) return -1;

        uint64_t n = PGSIZE - (dstva - va);
        if(n > len)
            n = len;
        memmove((pa + (dstva - va)),src,n);
        len -= n;
        src += n;
        dstva = va + PGSIZE;
    }
    return 0;
}