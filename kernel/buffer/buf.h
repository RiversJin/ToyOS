#ifndef BUF_H
#define BUF_H
#include "file_system.h"
#include "include/stdint.h"

#define BUF_VALID 0x1 // 0b1
#define BUF_DIRTY 0x2 // 0b10

struct buf {
    int flags;  // 持有 valid 和 dirty 这两个标志位
    uint32_t dev; // 设备id
    uint32_t blockno; // 块号
    uint8_t data[BSIZE]; // 储存的数据
    uint32_t refcnt;
    struct sleeplock lock; // 当锁定后 等待设备完成后释放
    struct buf* prev; 
    struct buf* next; // lru算法要用
};
/**
 * @brief buffer系统初始化
 * 
 */
void binit();
/**
 * @brief 读取一个指定设备 偏移量为blockno的块 返回块的锁是被锁定的状态
 * 
 * @param dev 
 * @param blockno 
 * @return struct buf* 
 */
struct buf* read(uint32_t dev, uint32_t blockno);
/**
 * @brief 将对应的buf写入到设备 此时这个buf必须已上锁
 * 
 */
void bwrite(struct buf*);
/**
 * @brief 释放一个已锁定的buffer 将其移入到lru链表中
 * 
 */
void brelease(struct buf*);
/**
 * @brief 增加此buf的引用计数
 * 
 */
void bpin(struct buf*);

/**
 * @brief 减少此buf的引用计数
 * 
 */
void bunpin(struct buf*);

#endif /* BUF_H */