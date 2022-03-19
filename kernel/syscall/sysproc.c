#include "arg.h"
#include "../include/stdint.h"
#include "../proc/proc.h"

int64_t sys_exit(){
    int n;
    if(argint(0, &n) < 0){ 
        return -1;
    }
    exit(n);
    return 0;
}