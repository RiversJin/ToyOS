#include "console.h"
#include "memory/kalloc.h"

__attribute__((noreturn))
void main(){
    console_init();
    alloc_init();
    panic("hello the world.");
    while(1);
}