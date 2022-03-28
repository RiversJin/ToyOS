#ifndef SYSPROC_H
#define SYSPROC_H
#include "../include/stdint.h"

extern int64_t sys_exit();
extern int64_t sys_exec();
extern int64_t sys_fork();
extern int64_t sys_getpid();
extern int64_t sys_wait();
extern int64_t sys_yield();
extern int64_t sys_kill();
extern int64_t sys_sbrk();
extern int64_t sys_pipe();
extern int64_t sys_uptime();
extern int64_t sys_sleep();
extern int64_t sys_mknod();
extern int64_t sys_mkdir();
extern int64_t sys_dup();
extern int64_t sys_open();
extern int64_t sys_close();
extern int64_t sys_read();
extern int64_t sys_write();
extern int64_t sys_chdir();
extern int64_t sys_fstat();
extern int64_t sys_link(void);
extern int64_t sys_unlink(void);

#endif /* SYSPROC_H */