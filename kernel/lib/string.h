#ifndef STRING_H
#define STRING_H
#include "include/types.h"

void* memset(void* v, int c, size_t n);

void *memcpy(void *dest, const void *src, size_t count);

void *memmove(void *dest, const void *src, size_t count);
#endif // STRING_H