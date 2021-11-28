#ifndef LIST_H
#define LIST_H

#include "compiler_attributes.h"
#include "types.h"

#define LIST_HEAD_INIT(var_name) {&(var_name),&(var_name)}
#define LIST_HEAD(var_name) \
    struct list_head var_name = LIST_HEAD_INIT(var_name)

static inline void INIT_LIST_HEAD(struct list_head* list){
    list->next = list;
    list->prev = list;
}
/**
 * @brief 私有函数 在已知两个相邻节点后,向这两个节点中间添加一个新的节点
 * 
 * @param new_node 
 * @param prev 
 * @param next 
 */
static inline void _list_add_between(struct list_head* new_node,struct list_head* prev,struct list_head* next){
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}
/**
 * @brief 向链表头部添加一个新的节点
 * 
 * @param new_node 
 * @param head 
 */
static inline void list_add(struct list_head* new_node, struct list_head* head){
    _list_add_between(new_node,head,head->next);
}
/**
 * @brief 向链表尾部添加一个新的节点
 * 
 * @param new_node 
 * @param head 
 */
static inline void list_add_tail(struct list_head* new_node, struct list_head* head){
    _list_add_between(new_node,head->prev,head);
}

static inline void _list_del(struct list_head* prev, struct list_head* next){
    next->prev = prev;
    WRITE_ONCE(prev->next,next);
}
static inline void _list_del_entry(struct list_head* entry){
    _list_del(entry->prev,entry->next);
}
static inline void list_del(struct list_head* entry){
    _list_del(entry)
}


#endif // LIST_H