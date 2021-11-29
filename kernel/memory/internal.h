#ifndef INTERNAL_H
#define INTERNAL_H

#include "memory.h"

#define PAGE_BUDDY_MAPCOUNT_VALUE (-128)
typedef int8_t order_t;


static inline pg_idx_t _find_buddy_pfn(pg_idx_t pg_idx, order_t order){
    return pg_idx ^ (1 << order);
}

static inline void set_page_order(struct page *page, order_t order){
    uint64_t order_in_flags = (uint64_t)order << 56;
    uint64_t temp = (1UL << 56) - 1;
    page->flags &= temp;
    page->flags |= order_in_flags;
}

static inline order_t get_buddy_order(struct page *page){
    return (order_t)(page->flags >> 56);
}

static inline bool page_is_buddy(struct page *page,struct page *buddy_page,order_t order){
    if(buddy_page->_mapcount != PAGE_BUDDY_MAPCOUNT_VALUE) return false;

    if(get_buddy_order(buddy_page)!=order) return false;

    if(get_page_refcount(buddy_page)!=0 ) return false;

    return true;
}
/**
 * @brief Set the page buddy object 将指定page标记为buddy
 * 
 * @param page 
 */
static inline void set_page_buddy(struct page *page, order_t order){
    set_page_order(page,order);
    page->_mapcount = PAGE_BUDDY_MAPCOUNT_VALUE;
}

static inline void unset_page_buddy(struct page *page){
    page->_mapcount = 0;
}



#endif // INTERNAL_H