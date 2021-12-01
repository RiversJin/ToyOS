/* 参考 linux kernel 源码 include/linux/list.h */
#ifndef LIST_H
#define LIST_H

#include "kernel.h"
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
/**
 * @brief 取得此链表entry所在的结构体
 * @ptr: 指向 &struct list_head
 * @type: list_head所在的结构体类型
 * @member: 结构体中,list_head的成员名
 */
#define list_entry(ptr,type,member) \
        container_of(ptr,type,member)

/**
 * @brief 取得链表中第一个元素
 * @ptr: 链表头
 * @type: list_head所在的结构体类型
 * @member: 结构体中,list_head的成员名
 */
#define list_first_entry(ptr,type,member) \
        list_entry(((ptr)->next),type,member)
/**
 * @brief 取得链表中最后一个元素
 * 
 */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)
/**
 * @brief 获得链表的第一个元素,若链表为空,则返回NULL
 * 
 */
#define list_first_entry_or_null(ptr, type, member) ({ \
	struct list_head *head__ = (ptr); \
	struct list_head *pos__ = READ_ONCE(head__->next); \
	pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
})
/**
 * @brief 获取下一个元素 
 * pos是指向含有list_head结构体的指针
 * member: 结构体中,list_head的成员名
 */
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)
/**
 * @brief 获取上一个元素
 * pos是指向含有list_head结构体的指针
 * member: 结构体中,list_head的成员名
 */
#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_continue - continue iteration over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 *
 * Continue to iterate over a list, continuing after the current position.
 */
#define list_for_each_continue(pos, head) \
	for (pos = pos->next; pos != (head); pos = pos->next)


/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_safe - 安全迭代链表,即使用此宏迭代时,可以移除当前迭代的对象
 * @pos:	the &struct list_head to use as a loop cursor.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop cursor.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     pos != (head); \
	     pos = n, n = pos->prev)

/**
 * list_entry_is_head - test if the entry points to the head of the list
 * @pos:	the type * to cursor
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_entry_is_head(pos, head, member)				\
	(&pos->member == (head))

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_last_entry(head, typeof(*pos), member);		\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = list_prev_entry(pos, member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @head:	the head of the list
 * @member:	the name of the list_head within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, member) \
	((pos) ? : list_entry(head, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, member) 		\
	for (pos = list_next_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = list_prev_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member);			\
	     pos = list_prev_entry(pos, member))
/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, head, member) 			\
	for (; !list_entry_is_head(pos, head, member);			\
	     pos = list_next_entry(pos, member))

/**
 * list_for_each_entry_from_reverse - iterate backwards over list of given type
 *                                    from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, continuing from current position.
 */
#define list_for_each_entry_from_reverse(pos, head, member)		\
	for (; !list_entry_is_head(pos, head, member);			\
	     pos = list_prev_entry(pos, member))


/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, typeof(*pos), member),	\
		n = list_next_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_next_entry(n, member))


/**
 * list_for_each_entry_safe_continue - continue list iteration safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, head, member) 		\
	for (pos = list_next_entry(pos, member), 				\
		n = list_next_entry(pos, member);				\
	     !list_entry_is_head(pos, head, member);				\
	     pos = n, n = list_next_entry(n, member))

/**
 * list_for_each_entry_safe_from - iterate over list from current point safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, head, member) 			\
	for (n = list_next_entry(pos, member);					\
	     !list_entry_is_head(pos, head, member);				\
	     pos = n, n = list_next_entry(n, member))

/**
 * list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_last_entry(head, typeof(*pos), member),		\
		n = list_prev_entry(pos, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_prev_entry(n, member))

/**
 * list_safe_reset_next - reset a stale list_for_each_entry_safe loop
 * @pos:	the loop cursor used in the list_for_each_entry_safe loop
 * @n:		temporary storage used in list_for_each_entry_safe
 * @member:	the name of the list_head within the struct.
 *
 * list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define list_safe_reset_next(pos, n, member)				\
	n = list_next_entry(pos, member)

#endif // LIST_H