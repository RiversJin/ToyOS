#include "../file/file.h"
#include "../proc/proc.h"
#include "pipe.h"

int pipealloc(struct file **f0, struct file **f1){
    struct pipe *pi = NULL;
    *f0 = NULL;
    *f1 = NULL;
    if((*f0 = filealloc()) == NULL || (*f1 = filealloc()) == NULL){
        goto bad;
    }
    if((pi = (struct pipe *)kalloc(sizeof(struct pipe))) == NULL){
        goto bad;
    }
    pi->is_readopen = 1;
    pi->is_writeopen = 1;
    pi->nread = 0;
    pi->nwrite = 0;
    init_spin_lock(&pi->lock,"pipe");
    (*f0)->type = FD_PIPE;
    (*f0)->readable = 1;
    (*f0)->writable = 0;
    (*f0)->pipe = pi;

    (*f1)->type = FD_PIPE;
    (*f1)->readable = 0;
    (*f1)->writable = 1;
    (*f1)->pipe = pi;

    bad:
    if(pi != NULL){
        kfree(pi);
    }
    if(*f0 != NULL){
        fileclose(*f0);
    }
    if(*f1 != NULL){
        fileclose(*f1);
    }
    return -1;
}

void pipeclose(struct pipe *pi, int writable){
    acquire_spin_lock(&pi->lock);
    if(writable){
        pi->is_writeopen = 0;
        wakeup(&pi->nread);
    }else{
        pi->is_readopen = 0;
        wakeup(&pi->nwrite);
    }
    if(pi->is_readopen == 0 && pi->is_writeopen == 0){
        release_spin_lock(&pi->lock);
        kfree(pi);
    }else{
        release_spin_lock(&pi->lock);
    }
}