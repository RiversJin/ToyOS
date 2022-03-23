#ifndef ARG_H
#define ARG_H
#include "../include/stdint.h"
int64_t argstr(int n, char** pp);
int64_t argptr(int n,char** pp,int size);
int64_t argint(int n, uint64_t* ip);
int64_t fetchstr(uint64_t addr,char** p);
#endif /* ARG_H */