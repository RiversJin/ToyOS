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
#endif // UTIL_H