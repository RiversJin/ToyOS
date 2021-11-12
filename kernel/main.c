#include "console.h"

__attribute__((noreturn))
void main(){
    console_init();
    panic("hello the world.");
    while(1);
}