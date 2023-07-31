#ifndef __OM_LIST_H__
#define __OM_LIST_H__

#include "om_def.h"

typedef struct _om_list_head {
  struct _om_list_head* next;
} om_list_head_t;

#define OM_LIST_HEAD_INIT(name) \
  { &(name), &(name) }

#define OM_LIST_HEAD(name) om_list_head_t name = OM_LIST_HEAD_INIT(name)

#define om_list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

#define om_list_for_each_safe(pos, n, head) \
  for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#define om_list_entry(ptr, type, member) om_container_of(ptr, type, member)

void _OM_INIT_LIST_HEAD(om_list_head_t* list);

#define OM_INIT_LIST_HEAD(arg) _OM_INIT_LIST_HEAD(arg)

#define om_del_all(source, del_fun)                \
  do {                                             \
    om_list_head_t *pos, *tmp;                     \
    for (pos = (source)->next; pos != (source);) { \
      tmp = pos;                                   \
      pos = pos->next;                             \
      del_fun(tmp);                                \
    }                                              \
  } while (0)

#define OM_PRASE_STRUCT(container, member)            \
  sizeof(container), om_offset_of(container, member), \
      om_member_size_of(container, member)

#define OM_PRASE_VAR(_arg) (&_arg), (sizeof(_arg))

void om_list_add(om_list_head_t* new_data, om_list_head_t* head);

void om_list_del(om_list_head_t* entry);

void om_list_replace(om_list_head_t* old, om_list_head_t* new_data);

int om_list_empty(const om_list_head_t* head);

size_t om_list_get_num(const om_list_head_t* head);
#endif
