#ifndef STRING_H
#define STRING_H
#include "include/types.h"

static inline void* memset(void* v, int c, size_t n)
{
    char* p = (char*)v;
    int m = n;
    while (--m >= 0) *p++ = c;
    return v;
}

#endif // STRING_H