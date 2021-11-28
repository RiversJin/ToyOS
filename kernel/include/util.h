#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h>

#define ROUNDDOWN(a, n)                                                        \
    ({                                                                         \
        uint64_t __a = (uint64_t)(a);                                          \
        (typeof(a))(__a - __a % (n));                                          \
    })

#define ROUNDUP(a, n)                                                          \
    ({                                                                         \
        uint64_t __n = (uint64_t)(n);                                          \
        (typeof(a))(ROUNDDOWN((uint64_t)(a) + __n - 1, __n));                  \
    })

/* Efficient min and max operations */
#define MIN(_a, _b)                                                            \
    ({                                                                         \
        typeof(_a) __a = (_a);                                                 \
        typeof(_b) __b = (_b);                                                 \
        __a <= __b ? __a : __b;                                                \
    })
#define MAX(_a, _b)                                                            \
    ({                                                                         \
        typeof(_a) __a = (_a);                                                 \
        typeof(_b) __b = (_b);                                                 \
        __a >= __b ? __a : __b;                                                \
    })
#include "arch/aarch64/bitops.h"
#endif // UTIL_H