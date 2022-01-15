#ifndef __OM_LIST_H__
#define __OM_LIST_H__

#include "om_def.h"
#include "om_lib.h"

typedef struct _om_list_head {
  struct _om_list_head* next, * prev;
} om_list_head_t;

#define LIST_HEAD_INIT(name) \
  { &(name), &(name) }

#define LIST_HEAD(name) om_list_head_t name = LIST_HEAD_INIT(name)

#define om_offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)

#define om_container_of(ptr, type, member)                \
  ({                                                      \
    const typeof(((type *)0)->member) *__mptr = (ptr);    \
    (type *)((char *)__mptr - om_offsetof(type, member)); \
  })

#define om_list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

#define om_list_for_each_safe(pos, n, head) \
  for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#define om_list_entry(ptr, type, member) om_container_of(ptr, type, member)

void _INIT_LIST_HEAD(om_list_head_t* list);

#define INIT_LIST_HEAD(arg) _INIT_LIST_HEAD(arg)

#define om_del_all(_pos, source, del_fun) \
  om_list_for_each(_pos, source) {        \
    if (om_list_empty(source))            \
      break;                              \
    else                                  \
      del_fun(_pos);                      \
  }

void om_list_add(om_list_head_t* new, om_list_head_t* head);

void om_list_add_tail(om_list_head_t* new, om_list_head_t* head);

void om_list_del(om_list_head_t* entry);

void om_list_del_init(om_list_head_t* entry);

void om_list_replace(om_list_head_t* old, om_list_head_t* new);

int om_list_empty(const om_list_head_t* head);

void __list_del(om_list_head_t* prev, om_list_head_t* next);

void __list_add(om_list_head_t* new, om_list_head_t* prev,
  om_list_head_t* next);

void __list_del_entry(om_list_head_t* entry);
#endif