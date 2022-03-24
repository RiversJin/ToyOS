#include "vm.h"
#include "arch/aarch64/mmu.h"
#include "arch/aarch64/arm.h"
#include "printf.h"
#include "kalloc.h"
#include "lib/string.h"
#include "../proc/proc.h"

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

            if((*pte & 0b11) == PTE_BLOCK) break;
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
        // 如果不是叶子节点的话 这里就只有一个有效位 没有权限位
        if(PTE_FLAG(*pagetable) == PTE_VALID) {
            panic("unmunmap: note a leaf. \n");
        }
        if(do_free){
            uint64_t pa = VA2PA(PTE_ADDR(*pte));
            kfree((void*)pa);
        }
        *pte = 0;
    }
}
/**
 * @brief 给定一个页表和一个虚拟地址,返回其物理地址 只用于用户态的页表
 * 
 * @param pagetable_ptr 页表指针 
 * @param va 虚拟地址
 * @return uint64_t 物理地址
 */
uint64_t walkaddr(pagetable_t pagetable_ptr,uint64_t va){
    pte_t* pte;
    if(!_check_MSB_valid(va)) return 0;
    pte = walk(pagetable_ptr,va,false,NULL);
    if(pte == NULL) return 0;
    if((*pte & PTE_VALID) == 0) return 0;
    if((*pte & PTE_USER) == 0) return 0;
    return (uint64_t)PA2VA(PTE_ADDR(*pte));
}
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
/**
 * @brief 在内核页表上添加一个映射 不会刷新TLB
 * 
 * @param kernel_pagetable_ptr 内核页表
 * @param va 虚拟地址
 * @param pa 物理地址
 * @param size 
 * @param perm 权限位
 */
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
void uvminit(pagetable_t pagetable, uint8_t *src, int64_t sz){
    if(sz >= PGSIZE){
        panic("uvmminit: more than a page.");
    }
    uint8_t *mem = kalloc(PGSIZE);
    if(mem==NULL){
        panic("uvminit: memory allocation failed.");
    }
    memset(mem,0,PGSIZE);
    memmove(mem,src,sz);
    mappages(pagetable,0,(uint64_t)mem,sz,PTE_USER|PTE_RW|PTE_PAGE);
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
    for(uint64_t a = oldsz; a < newsz; a+=PGSIZE){
        uint8_t *mem = kalloc(PGSIZE);
        if(mem == NULL){
            uvmdealloc(pagetable,a,oldsz);
        }
        memset(mem,0,PGSIZE);
        if(mappages(pagetable,a,(uint64_t)mem,PGSIZE,PTE_USER|PTE_RW|PTE_PAGE)){
            kfree(mem);
            uvmdealloc(pagetable,a,oldsz);
            return 0;
        }
    }
    return newsz;
}
uint64_t uvmdealloc(pagetable_t pagetable, uint64_t oldsz, uint64_t newsz){
    if(newsz >= oldsz) return oldsz;
    int npages = (oldsz - newsz) / PGSIZE;
    unmunmap(pagetable,newsz,npages, 1);
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
        if(mappages(new, i, (uint64_t)mem, PGSIZE, flags) != 0){
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
    //由于PTE_FLAG是取低12位的值 如果其不等于零 也就意味着没有此地址没有4k对齐
    if(((uint64_t)pagetable & 0xFFF)!=0){
        panic("uvmfree: invalid pte. \n");
    }
    if(level == 0){
        kfree(pagetable);
        return;
    }
    for(uint64_t i = 0;i < ENTRYSZ; ++i){
        if(pagetable[i] & PTE_VALID){
            uint64_t *v = (uint64_t*)PA2VA(PTE_ADDR(pagetable[i]));
            uvmfree(v,level-1);
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
void uvmclear(uint64_t *pgdir, uint8_t *va){
    uint64_t *pte = walk(pgdir, (uint64_t)va, 0, NULL);
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
        if(pa == 0) return -1;

        uint64_t n = PGSIZE - (dstva - va);
        if(n > len)
            n = len;
        memmove((void*)(pa + (dstva - va)),src,n);
        len -= n;
        src += n;
        dstva = va + PGSIZE;
    }
    return 0;
}
/**
 * @brief 将数据从用户复制到内核
 * 
 * @param pagetable 
 * @param dst 
 * @param srcva 
 * @param len 
 * @return int 
 */
int copyin(pagetable_t pagetable, char* dst, uint64_t srcva, uint64_t len){
    while(len > 0){
        uint64_t va = PGROUNDDOWN(srcva);
        uint64_t pa = walkaddr(pagetable,va);
        if(pa == 0)
            return -1;
        uint64_t n = PGSIZE - (srcva - va);
        if (n > len)
            n = len;
        memmove(dst,(void*)(pa + (srcva - va)),n);
        len -= n;
        dst += n;
        srcva = va + PGSIZE;
    }
    return 0;
}
/**
 * @brief 将一个以'\0'结尾的字符串从进程内存复制到内核
 * 
 * @param pagetable 
 * @param dst 
 * @param srcva 
 * @param max 字符串的最大长度
 * @return int 0 成功 -1 失败
 */
int copyinstr(pagetable_t pagetable, char *dst, uint64_t srcva, uint64_t max){
    int got_null = 0;
    while(got_null == 0 && max > 0){
        uint64_t va = PGROUNDDOWN(srcva);
        uint64_t pa = walkaddr(pagetable,va);
        if(pa==0)
            return -1;
        uint64_t n = PGSIZE - (srcva - va);
        if(n > max)
            n = max;
        char *p = (char*)(pa + (srcva - va));
        while(n > 0){
            if(*p == '\0'){
                *dst = '\0';
                got_null = 1;
                break;
            } else {
                *dst = *p;
            }
            n -= 1;
            max -= 1;
            p += 1;
            dst += 1;
        }
        srcva = va + PGSIZE;
    }
    if(got_null){
        return 0;
    } else {
        return -1;
    }
}
pagetable_t alloc_pagetable(void){
    pagetable_t pgt;
    if((pgt = kalloc(PGSIZE)) == NULL){
        return NULL;
    }
    memset(pgt, 0, PGSIZE);
    return pgt;
}
/**
 * @brief 将此核心MMU的用户页表切换为指定进程的页表
 * 
 * @param p 
 */
void uvmswitch(struct proc *p){
    if (p == NULL){
        panic("uvmswitch: process invalid.\n");
    }
    if (p->kstack == NULL){
        panic("uvmswitch: process' kstack invalid.\n");
    }
    if (p->pagetable == NULL){
        panic("uvmswitch: process' pagetable invalid.\n");
    }
    lttbr0(VA2PA(p->pagetable));
}