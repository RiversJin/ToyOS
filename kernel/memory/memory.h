#ifndef MEMORY_H
#define MEMORY_H
#include <stddef.h>
#include <stdint.h>
#include "../arch/aarch64/mmu.h"
#include "../arch/aarch64/board/raspi3/memlayout.h"

#define PAGE_USED (1<<0)
#define PAGE_KERNEL (1<<1)
#define TOTAL_PAGES_N (TOTAL_STOP/PGSIZE)



typedef uint64_t pg_idx_t;

struct list_head {
    struct list_head *prev, *next;
};


struct pg_range {
    pg_idx_t begin, end;
};

struct page{
    uint64_t flags;
    int64_t count;
    struct list_head lru;
    void* virtual_address;
};
extern struct page pages[TOTAL_PAGES_N];
extern struct pg_range free_zone;

inline static void set_page_used(struct page *page){
    page->flags |= PAGE_USED;
}

inline static int is_page_used(struct page *page){
    return page->flags & PAGE_USED;
}

inline static void set_page_kernel(struct page *page){
    page->flags |= PAGE_KERNEL;
}
inline static int is_page_kernel(struct page *page){
    return page->flags & PAGE_KERNEL;
}

inline static pg_idx_t PHY2PFn(void* ptr){
    return (uint64_t)(ptr) >> PAGE_SHIFT;
}
inline static void* PFn2PHY(pg_idx_t idx){
    return (void*)(idx << PAGE_SHIFT);
}
inline static struct page * PFn2PAGE(pg_idx_t idx){
    return &pages[idx];
}
inline static pg_idx_t PAGE2PFn(struct page *page){
    return (pg_idx_t)(page - pages);
}


#endif // MEMORY_H