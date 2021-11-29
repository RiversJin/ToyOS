#include "memlayout.h"
#include "lib/string.h"
#include "memory/memory.h"

extern char kernel_end[];
struct page pages[TOTAL_PAGES_N];

void pages_init(struct pg_range* range){
    for(int i=0; i<TOTAL_PAGES_N; ++i) {
        static struct page temp_page = {
            flags:0,
            _mapcount:0,
            _refcount:0
        };
        pages[i] = temp_page;
    }
    pg_idx_t kernel_pg_idx_tail = PHY2PFn((void*)VA2PA(kernel_end));
    for(pg_idx_t i=0;i<=kernel_pg_idx_tail;++i){
        set_page_kernel(PFn2PAGE(i));
        set_page_used(PFn2PAGE(i));
    }

    for(pg_idx_t i = PHY2PFn((void*)PHYSTOP);i<=PHY2PFn((void*)TOTAL_STOP);++i){
        set_page_kernel(PFn2PAGE(i));
        set_page_used(PFn2PAGE(i));
    }
    
    range->begin=kernel_pg_idx_tail+1;
    range->end=PHY2PFn((void*)PHYSTOP);
}