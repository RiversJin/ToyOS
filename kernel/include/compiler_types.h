#ifndef COMPILER_TYPES_H
#define COMPILER_TYPES_H
#include "compiler_attributes.h"

#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))


#define __compiletime_assert(condition, msg, prefix, suffix)		\
	do {								\
		extern void prefix ## suffix(void) __compiletime_error(msg); \
		if (!(condition))					\
			prefix ## suffix();				\
	} while (0)


#define _compiletime_assert(condition, msg, prefix, suffix) \
	__compiletime_assert(condition, msg, prefix, suffix)

#define compiletime_assert(condition, msg) \
	_compiletime_assert(condition, msg, __compiletime_assert_, __COUNTER__)

#endif // COMPILER_TYPES_H