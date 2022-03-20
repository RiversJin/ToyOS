#ifndef ARG_H
#define ARG_H
#include "../include/stdint.h"
int64_t argstr(int n, char** pp);
int64_t argptr(int n,char** pp,int size);
int64_t argint(int n, uint64_t* ip);

#endif /* ARG_H */