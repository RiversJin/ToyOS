#include "kalloc.h"
#include "../console.h"
#include "../util.h"

#define MAX_ORDER 11

extern void pages_init(struct pg_range* range);

struct free_area{
    struct list_head free_list;
    uint64_t n_free;
};
struct zone{
    struct free_area area[MAX_ORDER];
} zone;

static void free_pages(pg_idx_t begin, pg_idx_t end){

}

void alloc_init(void){
    struct pg_range pg_range;
    pages_init(&pg_range);
    free_pages(pg_range.begin, pg_range.end);

}

