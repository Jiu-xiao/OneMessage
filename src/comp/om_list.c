#include "om_list.h"

void _OM_INIT_LIST_HEAD(om_list_head_t* list) { list->next = list; }

static void __list_add(om_list_head_t* new_data, om_list_head_t* prev,
                       om_list_head_t* next) {
  new_data->next = next;
  prev->next = new_data;
}

void om_list_add(om_list_head_t* new_data, om_list_head_t* head) {
  __list_add(new_data, head, head->next);
}

static void __list_del(om_list_head_t* prev, om_list_head_t* next) {
  prev->next = next;
}

static om_list_head_t* __list_get_prev(om_list_head_t* entry) {
  om_list_head_t* prev = entry;
  while (prev->next != entry) {
    prev = prev->next;
  }
  return prev;
}

void om_list_del(om_list_head_t* entry) {
  __list_del(__list_get_prev(entry), entry->next);
}

void om_list_replace(om_list_head_t* old, om_list_head_t* new_data) {
  new_data->next = old->next;
  __list_get_prev(old)->next = new_data;
}

int om_list_empty(const om_list_head_t* head) { return head->next == head; }

size_t om_list_get_num(const om_list_head_t* head) {
  uint32_t num = 0;

  om_list_head_t* pos;
  om_list_for_each(pos, head) { num++; }

  return num;
}
