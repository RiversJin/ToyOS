#include "include/stdint.h"
#include "syscall.h"
#include "arch/aarch64/include/trapframe.h"
#include "proc/proc.h"
#include "sysproc.h"
#include "../printf.h"

static int64_t (*syscalls[])(void) = {
    [SYS_exec] sys_exec,
    [SYS_exit] sys_exit,
    [SYS_getpid] sys_getpid,
    [SYS_fork] sys_fork,
    [SYS_wait] sys_wait,
    [SYS_pipe] sys_pipe,
    [SYS_yield] sys_yield,
    [SYS_chdir] sys_chdir,
    [SYS_kill] sys_kill,
    [SYS_sbrk] sys_sbrk,
    [SYS_uptime] sys_uptime,

    [SYS_fstat] sys_fstat,
    [SYS_mknod] sys_mknod,
    [SYS_mkdir] sys_mkdir,
    [SYS_open] sys_open,
    [SYS_close] sys_close,
    [SYS_read] sys_read,
    [SYS_write] sys_write,
    [SYS_dup] sys_dup,
    [SYS_link] sys_link,
    [SYS_unlink] sys_unlink
};

int64_t syscall(struct trapframe *frame){
    int64_t call_number = (int64_t)frame->regs[8];
    if(call_number >0 && call_number<ARRAY_SIZE(syscalls)){
        int64_t (*fun)(void) = syscalls[call_number];
        if(fun == NULL){
            goto unsupported_syscall;
        }
        return fun();
    }
    unsupported_syscall:
    panic("syscall: unsupported syscall number %d\n",call_number);

    return -1;
}
/**
 * @brief 获得位于用户虚地址addr处的int64 将其赋值给*ip 若失败返回-1
 * 
 * @param addr 
 * @param ip 
 * @return int 
 */
int64_t fetchint64ataddr(uint64_t addr, uint64_t *ip){
    struct proc *proc = myproc();
    if(addr >= proc->sz || addr + sizeof(*ip) > proc->sz){
        return -1;
    }
    *ip = *(int64_t*)(addr);
    return 0;
}
/**
 * @brief 将*p指向用户虚地址中给定的以'\0'结束的字符串 返回字符串长度
 * 若失败返回-1
 * 
 * @param addr 
 * @param p 
 * @return int 
 */
int64_t fetchstr(uint64_t addr,char** p){
    struct proc *proc = myproc();
    if(addr >= proc->sz)return -1;
    char* ep = (char*) proc -> sz;
    for (char* s = (char*) addr; s < ep; ++s) {
        if (*s == '\0'){
            *p = (char*) addr;
            return ((int64_t)s - (int64_t)addr);
        }
    }
    return -1;
}
/**
 * @brief 获得系统调用中第n个参数 最高支持6个 即第0-5个参数
 * 
 * @param n 
 * @param ip 
 * @return int 
 */
int64_t argint(int n, uint64_t* ip){
    if(n < 0 || n > 5){
        cprintf("argint: invalid argument number %d\n", n);
        return -1;
    }
    struct proc* proc = myproc();
    *ip = proc->tf->regs[n];
    return 0;
}
/**
 * @brief 将系统调用中第n个参数作为一个指针 赋值给pp
 * 并检查这个指针是否在进程有效边界之内
 * 
 * @param n 
 * @param pp 
 * @param size 
 * @return int 
 */
int64_t argptr(int n,char** pp,int size){
    uint64_t i;
    if(argint(n,&i)<0) return -1;
    struct proc* p = myproc();
    if( i + size > p->sz ) return -1;
    *pp = (char*)i;
    return 0;
}
/**
 * @brief 将系统调用中第n个参数作为一个指针 获得字符串
 * 
 * @param n 
 * @param pp 
 * @return int 
 */
int64_t argstr(int n, char** pp){
    uint64_t addr;
    if(argint(n,&addr) < 0)return -1;
    return fetchstr(addr,pp);
}
