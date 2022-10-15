#include "om_list.h"

void _INIT_LIST_HEAD(om_list_head_t* list) {
  list->next = list;
  list->prev = list;
}

void __list_add(om_list_head_t* new_data, om_list_head_t* prev,
                om_list_head_t* next) {
  next->prev = new_data;
  new_data->next = next;
  new_data->prev = prev;
  prev->next = new_data;
}

void om_list_add(om_list_head_t* new_data, om_list_head_t* head) {
  __list_add(new_data, head, head->next);
}

void om_list_add_tail(om_list_head_t* new_data, om_list_head_t* head) {
  __list_add(new_data, head->prev, head);
}

void __list_del(om_list_head_t* prev, om_list_head_t* next) {
  next->prev = prev;
  prev->next = next;
}

void om_list_del(om_list_head_t* entry) {
  __list_del(entry->prev, entry->next);
}

void __list_del_entry(om_list_head_t* entry) {
  __list_del(entry->prev, entry->next);
}

void om_list_del_init(om_list_head_t* entry) {
  __list_del_entry(entry);
  INIT_LIST_HEAD(entry);
}

void om_list_replace(om_list_head_t* old, om_list_head_t* new_data) {
  new_data->next = old->next;
  new_data->next->prev = new_data;
  new_data->prev = old->prev;
  new_data->prev->next = new_data;
}

int om_list_empty(const om_list_head_t* head) { return head->next == head; }

size_t om_list_get_num(const om_list_head_t* head) {
  uint32_t num = 0;

  om_list_head_t* pos;
  om_list_for_each(pos, head) { num++; }

  return num;
}
