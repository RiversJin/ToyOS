#include "../proc/proc.h"
#include "file.h"
#include "fs/fs.h"
#include "fs/log.h"
#include "../printf.h"
#include "../include/param.h"
#include "../include/stat.h"
#include "../pipe/pipe.h"

struct devsw devsw[NDEV];
struct ftable {
    struct spinlock lock;
    struct file file[NFILE];
} ftable;

void fileinit(void){
    init_spin_lock(&ftable.lock, "ftable");
}
/**
 * @brief 分配一个文件结构体
 * 
 * @return struct file* 
 */
struct file* filealloc(void){
    acquire_spin_lock(&ftable.lock);
    for(struct file *f = ftable.file; f < ftable.file+NFILE; f++){ 
        if(f->ref == 0){ 
            f->ref = 1;
            release_spin_lock(&ftable.lock);
            return f;
        }
    }
    release_spin_lock(&ftable.lock);
    return NULL;
}
/**
 * @brief 增加一个文件的引用
 * 
 * @param f 
 * @return struct file* 
 */
struct file* filedup(struct file *f){
    acquire_spin_lock(&ftable.lock);
    if(f->ref < 1){
        panic("filedup: ref: %d .\n", f->ref);
    }
    f->ref++;
    release_spin_lock(&ftable.lock);
    return f;
}

/**
 * @brief 关闭指定文件 (减少引用数 如果当引用为零时 从OS层面彻底关闭)
 * 
 * @param f 
 */
void fileclose(struct file *f){
    acquire_spin_lock(&ftable.lock);
    if(f->ref < 1){
        panic("fileclose: ref: %d .\n", f->ref);
    }
    if(--f->ref > 0){
        release_spin_lock(&ftable.lock);
        return;
    }
    struct file ff = *f;
    f -> ref = 0;
    f -> type = FD_NONE;
    release_spin_lock(&ftable.lock);

    if(ff.type == FD_PIPE){
        pipeclose(ff.pipe, ff.writable);
    } else if(ff.type == FD_INODE || ff.type == FD_DEVICE){
        begin_op();
        iput(ff.ip);
        end_op();
    }
}
int32_t filestat(struct file *f, struct stat* st){
    if(f->type == FD_INODE || f->type == FD_DEVICE){
        ilock(f->ip);
        stati(f->ip, st);
        iunlock(f->ip);
        return 0;
    }
    return -1;
}

int32_t fileread(struct file *f, char *addr, int32_t n){
    if(!f->readable) return -1;
    int r = 0;
    if(f->type == FD_PIPE){
        f = piperead(f->pipe, addr, n);
    } else if( f->type == FD_DEVICE ){
        if(f->major < 0 || f->major >= NDEV || !devsw[f->major].read)
            return -1;
        r = devsw[f->major].read(addr, n);
    } else if( f->type == FD_INODE ){ 
        ilock(f->ip);
        if((r = readi(f->ip, addr, f->off, n)) > 0){
            f->off += r;
        }
        iunlock(f->ip);
    } else { 
        panic("unsupported file type: %d .\n", f->type);
    }
    return r;
}

int32_t filewrite(struct file* f, char* addr, int32_t n){
    if(!f->writable) return -1;
    int r = 0;
    int ret = 0;
    switch(f->type){
        case FD_PIPE:
            ret = pipewrite(f->pipe, addr, n);
            break;
        case FD_DEVICE:
            if(f->major < 0 || f->major >= NDEV || !devsw[f->major].write){
                return -1;
            }
            ret = devsw[f->major].write(addr,n);
            break;
        case FD_INODE:{
            int32_t max = ((MAXOPBLOCKS - 1 - 1 - 2)/2) * BSIZE;
            int32_t i = 0;
            while(i < n){
                int n1 = n - i;
                if(n1 > max)
                    n1 = max;
                begin_op();
                ilock(f->ip);
                if((r = writei(f->ip, addr + i, f->off, n1)) > 0) {
                    f->off += r;
                }
                iunlock(f->ip);
                end_op();
                if(r != n1) {
                    break;
                }
                i += r;
            }
            ret = (i == n) ? n : -1 ;
        }
        break;
        default:
            panic("unsupported file type %d. \n", f->type);
            break;
    }
    return ret;
}