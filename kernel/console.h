#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdarg.h>
#include <stdint.h>

void console_init();
void cprintf(const char *fmt, ...);
void panic(const char *fmt, ...);

#endif // CONSOLE_H