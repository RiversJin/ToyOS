#ifndef STRING_H
#define STRING_H
#include "include/types.h"

/**
 * memset - Fill a region of memory with the given value
 * @s: Pointer to the start of the area.
 * @c: The byte to fill the area with
 * @count: The size of the area.
 *
 * Do not use memset() to access IO space, use memset_io() instead.
 */
void *memset(void *s, int c, size_t count);

void *memcpy(void *dest, const void *src, size_t count);

void *memmove(void *dest, const void *src, size_t count);

int memcmp(const void *s1, const void *s2, size_t count);

char* safestrcpy(char *s, const char *t, int n);
#endif // STRING_H