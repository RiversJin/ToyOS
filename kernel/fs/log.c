#include "log.h"
#include "../include/stdint.h"
#include "../console.h"
#include "../lib/string.h"
#include "../proc/proc.h"

struct logheader {
    int32_t n;
    int32_t block[LOGSIZE];
};

struct log {
    struct spinlock lock;
    int32_t start;
    int32_t size;
    int32_t outstanding; // 当前有多少文件系统调用正在执行
    int32_t commiting; // 是否正在提交
    int32_t dev;
    struct logheader lh;
};

struct log log;
static void recover_fromlog(void);
static void commit(void);

void initlog(int dev, struct superblock *sb){
    if(sizeof(struct logheader) >= BSIZE){
        panic("initlog: too big logheader.\n");
    }
    init_spin_lock(&log.lock, "log");
    log.start = sb->logstart;
    log.size = sb->nlog;
    log.dev = dev;
    recover_fromlog();   
}
/**
 * @brief 将已提交的块复制到它们的对应的位置
 * 
 * @param recovering 
 */
static void install_trans(int recovering){
    for(int32_t tail = 0; tail < log.lh.n; tail++){
        struct buf *lbuf = bread(log.dev, log.start + tail + 1);
        struct buf *dbuf = bread(log.dev, log.lh.block[tail]);
        memmove(dbuf->data, lbuf->data, BSIZE);
        bwrite(dbuf);
        if(recovering == 0){
            bunpin(dbuf);
        }
        brelease(lbuf);
        brelease(dbuf);
    }
}
/**
 * @brief 从磁盘中读取logheader到内存中
 * 
 */
static void read_head(void){
    struct buf *buf = bread(log.dev, log.start);
    struct logheader *lh = (struct logheader*)(buf->data);
    log.lh.n = lh->n;
    for(int32_t i = 0; i < log.lh.n; ++i){
        log.lh.block[i] = lh->block[i];
    }
    brelease(buf);
}
/**
 * @brief 将内存中的logheader写入到磁盘中
 * 
 */
static void write_head(void){
    struct buf *buf = bread(log.dev, log.start);
    struct logheader *hb = (struct logheader*)(buf->data);
    hb -> n = log.lh.n;
    for(int32_t i = 0; i < log.lh.n; ++i){
        hb->block[i] = log.lh.block[i];
    }
    bwrite(buf);
    brelease(buf);
}

static void recover_fromlog(void){
    read_head();
    install_trans(1); // 如果已提交 将记录写入磁盘
    log.lh.n = 0;
    write_head(); 
}

void begin_op(void){ 
    acquire_spin_lock(&log.lock);
    while(1){
        if(log.commiting){
            sleep(&log, &log.lock);
        } else if(log.lh.n+(log.outstanding+1)*MAXOPBLOCKS > LOGSIZE){
            // 如果正在处理的事务量已满 就等一下
            sleep(&log, &log.lock);
        } else {
            log.outstanding += 1;
            release_spin_lock(&log.lock);
            break;
        }
    }
}

void end_op(void){
    int do_commit = 0;
    acquire_spin_lock(&log.lock);
    log.outstanding -= 1;

    if(log.commiting){ 
        panic("end_op: log.comming = 1");
    }

    if(log.outstanding == 0){
        do_commit = 1;
        log.commiting = 1;
    }else{
        wakeup(&log);
    }
    release_spin_lock(&log.lock);
    if(do_commit){
        commit();
        acquire_spin_lock(&log.lock);
        log.commiting = 0;
        wakeup(&log);
        release_spin_lock(&log.lock);
    }
}
static void write_log(void){
    for(int32_t tail = 0; tail < log.lh.n; tail++){
        struct buf *to = bread(log.dev, log.start + tail + 1);
        struct buf *from = bread(log.dev, log.lh.block[tail]);
        memmove(to->data, from->data, BSIZE);
        bwrite(to);
        brelease(from);
        brelease(to);
    }
}
static void commit(void){
    if(log.lh.n > 0){
        write_log();
        write_head();
        install_trans(0);
        log.lh.n = 0;
        write_head();
    }
}

void log_write(struct buf *b){
    acquire_spin_lock(&log.lock);
    if(log.lh.n >= LOGSIZE || log.lh.n >= log.size - 1){
        panic("log_write: too big a transaction. \n");
    }
    if(log.outstanding < 1){
        panic("log_write: outside of transaction. \n");
    }
    int32_t i;
    for(i = 0; i < log.lh.n; ++i){
        if(log.lh.block[i] == b->blockno){
            break;
        }
    }
    log.lh.block[i] = b->blockno;
    if(i == log.lh.n){
        bpin(b);
        log.lh.n += 1;
    }
    release_spin_lock(&log.lock);
}