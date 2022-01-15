#include "om_list.h"

inline void _INIT_LIST_HEAD(om_list_head_t* list) {
  list->next = list;
  list->prev = list;
}

inline void __list_add(om_list_head_t* new, om_list_head_t* prev,
  om_list_head_t* next) {
  next->prev = new;
  new->next = next;
  new->prev = prev;
  prev->next = new;
}

inline void om_list_add(om_list_head_t* new, om_list_head_t* head) {
  __list_add(new, head, head->next);
}

inline void om_list_add_tail(om_list_head_t* new, om_list_head_t* head) {
  __list_add(new, head->prev, head);
}

inline void __list_del(om_list_head_t* prev, om_list_head_t* next) {
  next->prev = prev;
  prev->next = next;
}

inline void om_list_del(om_list_head_t* entry) {
  __list_del(entry->prev, entry->next);
}

inline void __list_del_entry(om_list_head_t* entry) {
  __list_del(entry->prev, entry->next);
}

inline void om_list_del_init(om_list_head_t* entry) {
  __list_del_entry(entry);
  INIT_LIST_HEAD(entry);
}

inline void om_list_replace(om_list_head_t* old, om_list_head_t* new) {
  new->next = old->next;
  new->next->prev = new;
  new->prev = old->prev;
  new->prev->next = new;
}

inline int om_list_empty(const om_list_head_t* head) {
  return head->next == head;
}