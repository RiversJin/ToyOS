#include "proc.h"
#include "elf.h"

static int loadseg(pagetable_t *pgdir, uint64_t addr, struct inode *ip, uint32_t offset, uint32_t sz);
int exec(char *path, char **argv){
    // TODO: implement exec
}