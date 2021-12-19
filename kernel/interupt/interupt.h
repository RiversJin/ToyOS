#ifndef INTERUPT_H
#define INTERUPT_H
#include <stdbool.h>
extern void exception_handler_init(void);

void enable_interrupt();

void disable_interrupt();

bool is_interupt_enabled();

#endif /* INTERUPT_H */