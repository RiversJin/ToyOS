#include "buf.h"
#include "sync/spinlock.h"
#include "sync/sleeplock.h"

struct bcache {
    struct spinlock lock;
    struct buf buf[NBUF];

    struct buf head; //lru用
} bcache;

void binit(){
    init_spin_lock(&bcache.lock,"bcache");

    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;

    for(struct buf* b = &bcache.buf; b < bcache.buf + NBUF; ++b){
        b->next = bcache.head.next;
        b->prev = &bcache.head;

        init_sleep_lock(&b->lock, "buffer");
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }

    cprintf("binit: success.\n");
}

static struct buf* bget(uint32_t dev, uint32_t blockno){
    acquire_spin_lock(&bcache.lock);

    // 若当前block已经被缓存
    for(struct buf* b = bcache.head.next; b!= &bcache.head;b = b->next){
        if(b->dev == dev && b->blockno == blockno){
            // 命中 直接返回即可
            b->refcnt += 1;
            release_spin_lock(&bcache.lock);
            acquire_sleep_lock(&b->lock);
            return b;
        }
    }

    // 如果没有命中 首先 从lru中找一个最近使用最少的buffer 希望这个bufer不脏 
    for(struct buf* b = bcache.head.prev; b != &bcache.head; b = b->prev){
        if( b->refcnt == 0 && !(b->flags & BUF_DIRTY)){
            b->dev = dev;
            b->blockno = blockno;
            b->flags = 0;
            b->refcnt = 1;
            release_spin_lock(&bcache.lock);
            acquire_sleep_lock(&b->lock);
            return b;
        }
    }
    //TODO: 如果找不到不脏的 就找一个脏的写回
    panic("bget: no available buffer.\n");
    return NULL;
}

void bpin(struct buf* b){
    acquire_spin_lock(&bcache.lock);
    b->refcnt+=1;
    release_spin_lock(&bcache.lock);
}

void bunpin(struct buf* b){
    acquire_spin_lock(&bcache.lock);
    b->refcnt-=1;
    release_spin_lock(&bcache.lock);
}