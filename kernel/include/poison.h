#ifndef POISON_H
#define POISON_H
/* 为了list.h 需要提供一个非法的指针值 如果访问到了,就会产生段错误,方便调试
 * 之前提到过 对于aarch64,虚地址高有效位必须为全0或全1 所以除此之外就都可以了
 */
#define ILLEGAL_POINTER_VALUE 0xdead000000000000

#define LIST_POISON1 ((void*) 0x1 + ILLEGAL_POINTER_VALUE)
#define LIST_POISON2 ((void*) 0x10 + ILLEGAL_POINTER_VALUE)

#endif //POISON_H