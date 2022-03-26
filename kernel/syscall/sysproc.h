#ifndef SYSPROC_H
#define SYSPROC_H
#include "../include/stdint.h"

extern int64_t sys_exit();
extern int64_t sys_exec();
extern int64_t sys_fork();
extern int64_t sys_getpid();
extern int64_t sys_wait();

extern int64_t sys_mknod();
extern int64_t sys_dup();
extern int64_t sys_open();
extern int64_t sys_close();
extern int64_t sys_read();
extern int64_t sys_write();

#endif /* SYSPROC_H */