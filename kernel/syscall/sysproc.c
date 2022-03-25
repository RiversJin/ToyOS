#include "arg.h"
#include "../include/stdint.h"
#include "../proc/proc.h"

int64_t sys_exit(){
    int n;
    if(argint(0, (uint64_t*)&n) < 0){ 
        return -1;
    }
    exit(n);
    return 0;
}


// int getpid(void)
int64_t sys_getpid(){
    return myproc()->pid;
}

// int fork(void)
int64_t sys_fork(){
    return fork();
}