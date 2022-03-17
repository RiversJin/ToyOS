#ifndef VM_H
#define VM_H
#include <stdbool.h>
#include <stddef.h>
#include "arch/aarch64/arm.h"
struct proc;

pagetable_t alloc_pagetable(void);
uint64_t walkaddr(pagetable_t pagetable_ptr,uint64_t va);
void kvmmap(pagetable_t kernel_pagetable_ptr,uint64_t va, uint64_t pa, uint64_t size, uint32_t perm);
int mappages(pagetable_t pagetable_ptr,uint64_t va, uint64_t pa, uint64_t size, int perm);
pagetable_t uvmcreate(void);
void uvminit(pagetable_t pagetable, uint8_t *src, int64_t sz);
uint64_t uvmalloc(pagetable_t pagetable,uint64_t oldsz, uint64_t newsz);
uint64_t uvmdealloc(pagetable_t, uint64_t, uint64_t);
int uvmcopy(pagetable_t, pagetable_t, uint64_t);
void uvmfree(pagetable_t, uint64_t);
void unmunmap(pagetable_t pagetable,uint64_t va,uint64_t npages, int do_free);
void unmclear(uint64_t *pgdir, uint8_t *va);
int copyout(pagetable_t pagetable, uint64_t dstva, char* src,  uint64_t len);
int copyin(pagetable_t pagetable, char *dst, uint64_t srcva, uint64_t len);
int copyinstr(pagetable_t pagetable, char *dst, uint64_t srcva, uint64_t max);
void uvmswitch(struct proc *p);
#endif // VM_H