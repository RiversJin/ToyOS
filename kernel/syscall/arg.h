#ifndef ARG_H
#define ARG_H
#include "../include/stdint.h"
int argstr(int n, char** pp);
int argptr(int n,char** pp,int size);
int argint(int n, uint64_t* ip);

#endif /* ARG_H */