#include "kalloc.h"
#include "internal.h"
#include "../console.h"
#include "include/util.h"

#define MAX_ORDER 11

extern void pages_init(struct pg_range* range);

struct free_area{
    struct list_head free_list;
    uint64_t n_free;
    
};
struct zone{
    int64_t managed_pages; //此内存区域内管理的页数
    struct free_area area[MAX_ORDER];
} zone;



static inline void del_page_from_free_list(struct page* page,order_t order){

}

static void free_pages(pg_idx_t pfn,order_t order){
    uint32_t n_pages = 1 << order;
    struct page * begin = PFn2PAGE(pfn);
    const struct page * const end = begin + n_pages;
    for(const struct page * ptr = begin; ptr < end; ++ptr){
        clear_page_kernel(ptr);
        set_page_unused(ptr);
        set_page_refcount(ptr, 0);
    }

    zone.managed_pages += n_pages;

}

static void free_pages(pg_idx_t begin, pg_idx_t end){
    order_t order;
    while(begin < end){
        order = MIN(MAX_ORDER - 1UL,__ffs(begin));
        while((begin + (1UL << order)) > end) {
            --order;
        }
    }
}
static inline void free_one_page(struct page * page,pg_idx_t pg_idx, order_t order){
    while(order < MAX_ORDER){
        pg_idx_t buddy_page_idx = _find_buddy_pfn(pg_idx, order);
        struct page * buddy_page = page + (buddy_page_idx - pg_idx);
        if(!page_is_buddy(page, buddy_page,order))
            break;


    }
}

void alloc_init(void){
    struct pg_range pg_range;
    pages_init(&pg_range);
    free_pages(pg_range.begin, pg_range.end);
    
    zone.managed_pages = 0;
    for(int i = 0; i<MAX_ORDER;++i){
        struct free_area* free_area_ptr = zone.area + i;
        free_area_ptr->n_free = 0;
        struct list_head* list_head_ptr = &free_area_ptr->free_list;
        list_head_ptr->next = list_head_ptr;
        list_head_ptr->prev = list_head_ptr;
    }
}

