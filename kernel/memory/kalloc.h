#ifndef KALLOC_H
#define KALLOC_H

#include "memory.h"
#include <stddef.h>
/**
 * @brief 内存管理初始化
 * 
 */
void alloc_init(void);
/**
 * @brief 打印当前内存管理系统的状态
 * 
 */
void log_alloc_system_info(void);

void* kalloc(size_t size);
void kfree(void *ptr);
int kalloc_pages(struct page** page,pg_idx_t* pfn,int pages_n);
int kalloc_one_page(struct page** page,pg_idx_t* pfn);
void kfree_page(pg_idx_t pfn);
void kfree(void *ptr);


#endif // KALLOC_H