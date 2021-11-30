#ifndef BITOPS_H
#define BITOPS_H

#include "include/compiler_attributes.h"

/**
 * __ffs - find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static __always_inline unsigned long __ffs(unsigned long word)
{
	return __builtin_ctzl(word);
}

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static __always_inline int __fls(unsigned long x)
{
	return x ? sizeof(x) * 8 - __builtin_clz(x) : 0;
}


#endif // BITOPS_H