#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct atomic_t {
    int64_t counter;
} atomic_t;

struct list_head {
	struct list_head *next, *prev;
};


#endif //TYPES_H