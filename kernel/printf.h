#ifndef PRINTF_h
#define PRINTF_h

#include <stdarg.h>
#include <stdint.h>

void printinit();
void cprintf(const char *fmt, ...);
void panic(const char *fmt, ...);

#define assert(x)                                                              \
    ({                                                                         \
        if (!(x)) { panic("%s:%d: assertion failed.\n", __FILE__, __LINE__); } \
    })

/* Assertion with reason. */
#define asserts(x, ...)                                                        \
    ({                                                                         \
        if (!(x)) {                                                            \
            cprintf("%s:%d: assertion failed.\n", __FILE__, __LINE__);         \
            panic(__VA_ARGS__);                                                \
        }                                                                      \
    })

#endif // PRINTF_h