#ifndef LIST_H
#define LIST_H

#include "compiler_attributes.h"
#include "types.h"
#include "poison.h"

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
/**
 * @brief 对于待删除的节点,若已知其相邻的两个节点,
 * 那么就可以通过让这个两个节点越过此节点互相连接而实现目的
 * 
 * @param prev 
 * @param next 
 */
static inline void _list_del(struct list_head* prev, struct list_head* next){
    next->prev = prev;
    WRITE_ONCE(prev->next,next);
}
static inline void _list_del_entry(struct list_head* entry){
    _list_del(entry->prev,entry->next);
}
/**
 * @brief 在链表中删除指定元素
 * 
 * @param entry 
 */
static inline void list_del(struct list_head* entry){
    _list_del_entry(entry);
    entry->next = (struct list_head*)LIST_POISON1;
    entry->prev = (struct list_head*)LIST_POISON2;
}
/**
 * @brief 在链表中替换指定元素
 * 
 * @param old_node 
 * @param new_node 
 */
static inline void list_replace(struct list_head* old_node, struct list_head* new_node){
    new_node->next = old_node->next;
    new_node->next->prev = new_node;
    new_node->prev = old_node->prev;
    new_node->prev->next = new_node;
}

/**
 * @brief 在链表中替换指定元素 并将old元素的前后指向自身(初始化)
 * 
 * @param old_node 
 * @param new_node 
 */
static inline void list_replace_init(struct list_head* old_node, struct list_head* new_node){
    list_replace(old_node, new_node);
    INIT_LIST_HEAD(old_node);
}
/**
 * @brief 在链表中交换两个节点
 * ---- 用node2替换node1, 然后将node1加入到node2原来的位置
 * 
 * @param node1 
 * @param node2 
 */
static inline void list_swap(struct list_head* node1, struct list_head* node2){
    struct list_head* ptr = node2->prev;
    list_del(node2);
    list_replace(node1,node2);
    if(ptr == node1){
        ptr = node2;
    }
    list_add(node1,ptr);
}
/**
 * @brief 将一个节点从原来的链表移除,并放置在另一个链表的头部
 * 
 * @param entry 
 * @param head 
 */
static inline void list_move(struct list_head* entry, struct list_head* head){
    _list_del_entry(entry);
    list_add(entry,head);
}
/**
 * @brief 将一个节点从原来的链表移除,并放置在另一个链表的尾部
 * 
 * @param entry 
 * @param head 
 */
static inline void list_move_tail(struct list_head* entry, struct list_head* head){
    _list_del_entry(entry);
    list_add_tail(entry,head);
}
/**
 * @brief 将一段链表加入到新链表的尾部
 * 
 * @param head 新链表
 * @param first 
 * @param last 
 */
static inline void list_bulk_move_tail(struct list_head* head, struct list_head* first, struct list_head* last){
    /* 从原链表移除 */
    first->prev->next = last->next;
    last->next->prev = first->prev;

    /* 将此段链表加入到新的链表 */
    head->prev->next = first;
    first->prev = head->prev;

    last->next = head;
    head->prev = last;
}

/**
 * @brief 判断给定节点是否为链表中第一个元素
 * 
 * @param entry 
 * @param head 
 * @return true 
 * @return false 
 */
static inline bool list_is_first(const struct list_head *entry, const struct list_head *head){
    return entry->prev == head;
}
/**
 * @brief 判断给定元素是否为尾元素
 * 
 * @param entry 
 * @param head 
 * @return true 
 * @return false 
 */
static inline bool list_is_last(const struct list_head *entry, const struct list_head *head){
    return entry->next == head;
}
/**
 * @brief 判断给定链表是否为空
 * 
 * @param head 
 * @return true 
 * @return false 
 */
static inline bool list_is_empty(const struct list_head* head){
    return READ_ONCE(head->next) == head;
}
/**
 * @brief 左旋链表
 * 
 * @param head 
 */
static inline void list_rotate_left(struct list_head *head)
{
	struct list_head *first;

	if (!list_is_empty(head)) {
		first = head->next;
		list_move_tail(first, head);
	}
}
/**
 * @brief 将指定元素放置在链表尾部 (也可以说是放在了链表的"front")
 * 
 * @param entry 
 * @param head 
 */
static inline void list_rotate_to_front(struct list_head* entry, struct list_head* head){
    list_move_tail(entry,head);
}
/**
 * @brief 判断给定链表是否只有一个元素
 * 
 * @param head 
 * @return true 
 * @return false 
 */
static inline bool list_is_singular(struct list_head* head){
    return !list_is_empty(head) && (head->next == head->prev);
}

static inline void _list_cut_position(struct list_head *list,struct list_head *head, struct list_head *entry){
	struct list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}
/**
 * @brief 将head链表指定位置之后的元素加入到list链表中
 * list链表中的内容会被清除
 * 
 * @param list 一个新的list_head 会将原有链表中被切掉的元素放置在这个链表中
 * @param head 被切的链表
 * @param entry 在head中的节点 从这里切 这个节点将在原链表中
 */
static inline void list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	if (list_is_empty(head))
		return;
	if (list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		_list_cut_position(list, head, entry);
}
/**
 * @brief 将head链表指定位置之前的元素加入到list链表中
 *        list链表中的内容会被清除
 * @param list 
 * @param head 
 * @param entry 
 */
static inline void list_cut_before(struct list_head *list, struct list_head *head, struct list_head *entry){
    if(head->next == entry){
        INIT_LIST_HEAD(list);
        return;
    }
    list->next = head->next;
    list->next->prev = list;
    list->prev = entry->prev;
    list->prev->next = list;
    head->next = entry;
    entry->prev = head;
}

static inline void _list_splice(const struct list_head *list,
				 struct list_head *prev,
				 struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}
/**
 * @brief 连接两个链表 list将被加入在head之后
 * 
 * @param list 被连接的链表
 * @param head 在第一个列表中添加它的位置
 */
static inline void list_splice(const struct list_head *list,struct list_head *head){
    if(!list_is_empty(list)){
        _list_splice(list,head,head->next);
    }
}
/**
 * @brief 连接两个链表 list将被加入在head之前
 * 
 * @param list 被连接的链表
 * @param head 在第一个列表中添加它的位置
 */
static inline void list_splice_tail(const struct list_head *list,struct list_head *head){
    if(!list_is_empty(list)){
        _list_splice(list,head->prev,head);
    }
}
/**
 * @brief 连接两个链表 list将被加入在head之后 list将被重新初始化
 * 
 * @param list 
 * @param head 
 */
static inline void list_splice_init(struct list_head *list,struct list_head *head){
    if(!list_is_empty(list)){
        _list_splice(list,head,head->next);
        INIT_LIST_HEAD(list);
    }
}
/**
 * @brief 连接两个链表 list将被加入在head之前 list会被重新初始化
 * 
 * @param list 被连接的链表
 * @param head 在第一个列表中添加它的位置
 */
static inline void list_splice_tail_init(struct list_head *list,struct list_head *head){
    if(!list_is_empty(list)){
        _list_splice(list,head->prev,head);
        INIT_LIST_HEAD(list);
    }
}
#endif // LIST_H