#include "proc.h"
#include "elf.h"
#include "../printf.h"
#include "../sync/spinlock.h"
#include "../include/param.h"
#include "../fs/fs.h"
#include "../fs/log.h"
#include "../lib/string.h"

static int loadseg(pagetable_t pgdir, uint64_t va, struct inode *ip, uint32_t offset, uint32_t sz);

int exec(char *path, char **argv){
    pagetable_t pagetable = 0, oldpagetable;
    struct inode *ip;
    
    begin_op();
    if((ip = namei(path)) == NULL){
        end_op();
        cprintf("exec: %s not found\n", path);
        return -1;
    }
    ilock(ip);
    struct elfhdr elf;
    // 检查ELF Header
    if(readi(ip,(char*)&elf,0,sizeof(elf)) != sizeof(elf)){
        cprintf("exec: read elf header failed\n");
        goto bad;
    }
    if(elf.magic != ELF_MAGIC){
        cprintf("exec: elf magic check failed\n");
        goto bad;
    }
    pagetable = uvmcreate();
    if(pagetable == NULL){
        cprintf("exec: unable to create pagetable\n");
        goto bad;
    }
    struct proghdr ph;
    uint64_t sz = 0;
    for(int i = 0, off = elf.phoff; i < elf.phnum; ++i, off+=sizeof(ph)){
        if(readi(ip,(char*)&ph,off,sizeof(ph)) != sizeof(ph)){ 
            cprintf("exec: failed to read program header.\n");
            goto bad;
        }
        if(ph.type != ELF_PROG_LOAD) continue;
        if (ph.memsz < ph.filesz) {
            cprintf("exec: memory size < file size.\n");
            goto bad;
        }
        if(ph.vaddr + ph.memsz < ph.vaddr){
            cprintf("exec: addr overflow.\n");
            goto bad;
        }
        uint64_t sz1 = uvmalloc(pagetable, sz, ph.vaddr + ph.memsz);
        if(sz1 == 0){
            cprintf("exec: allocate memory failed.\n");
            goto bad;
        }
        sz = sz1;
        if((ph.vaddr % PGSIZE) != 0){
            cprintf("exec: vaddr %d not aligned.\n", ph.vaddr);
            goto bad;
        }
        if(loadseg(pagetable,ph.vaddr,ip,ph.off,ph.filesz) < 0){
            cprintf("exec: loadseg failed.\n");
            goto bad;
        }
    }
    iunlockandput(ip);
    end_op();
    ip = NULL;

    struct proc *p = myproc();
    
    // 配置用户栈
    sz = PGROUNDUP(sz);
    uint64_t sz1 = uvmalloc(pagetable, sz, sz + 2*PGSIZE);
    if(sz1 == 0)
        goto bad;
    sz = sz1;
    uvmclear(pagetable, (uint8_t*)(sz - 2*PGSIZE));
    uint64_t sp = sz;
    uint64_t stack_base = sp - PGSIZE;
    uint64_t argc;
    uint64_t ustack[MAXARG];
    for(argc = 0; argv[argc]; argc ++ ){
        if(argc >= MAXARG){
            cprintf("exec: too many args.\n");
            goto bad;
        }
        sp -= strlen(argv[argc]) + 1;
        sp -= sp % 16; // aarch64 架构下 栈需要对齐
        if(sp < stack_base){
            cprintf("exec: user stack overflow.\n");
            goto bad;
        }
        if(copyout(pagetable,sp,argv[argc],strlen(argv[argc])+1) < 0){
            cprintf("exec: failed to push argument strings.\n");
            goto bad;
        }
        ustack[argc] = sp;
    }
    ustack[argc] = 0;
    sp -= (argc+1) * sizeof(uint64_t);
    sp -= sp % 16;
    if(sp < stack_base){
        cprintf("exec: user stack overflow after push argument strings.\n");
        goto bad;
    }
    if(copyout(pagetable,sp,(char*)ustack, (argc+1)*sizeof(uint64_t)) < 0){
        cprintf("exec: push argument error.\n");
        goto bad;
    }
    p->tf->regs[1] = sp; // exec返回相当于调用用户进程的main 而x0即argc通过exec返回值配置
    // 调试用 将程序名复制到proc中
    char* last = path;
    for (char* s = path; *s; ++s)
        if (*s == '/') last = s + 1;
    strncpy(p->name, last, sizeof(p->name));

    oldpagetable = p->pagetable; // 先暂存之前的页表
    p->pagetable = pagetable;
    p->sz = sz;
    p->tf->sp = sp;
    p->tf->pc = elf.entry;
    uvmswitch(p);
    if(oldpagetable != NULL)
        uvmfree(oldpagetable,4);
    return argc;

    bad:
        if(pagetable)
            uvmfree(pagetable,4);
        if(ip){
            iunlockandput(ip);
            end_op();
        }
        return -1;
}

static int loadseg(pagetable_t pgdir, uint64_t va, struct inode *ip, uint32_t offset, uint32_t sz){
    for(int i=0; i < sz; i += PGSIZE){
        uint64_t pa = walkaddr(pgdir, va + i);
        if (pa == 0){
            panic("loadseg: address %p not exist.\n", va + i);
        }
        uint32_t n;
        if(sz - i < PGSIZE)
            n = sz - i;
        else
            n = PGSIZE;
        if(readi(ip, (char*)pa, offset+i, n) != n)
            return -1;
    }
    return 0;
}