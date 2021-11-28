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



#endif // BITOPS_H