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
void free(void *ptr);

#endif // KALLOC_H