#include "om_list.h"

inline om_node_t om_node_create(void* data, size_t size) {
  om_node_t node = om_malloc(sizeof(om_node_data_t));
  node->data = data;
  node->next = NULL;
  node->size = size;
  return node;
}

inline void om_node_delete(om_node_t node) { om_free(node); }

inline void om_node_connect(om_node_t left, om_node_t right) {
  left->next = right;
}

inline void om_node_across(om_node_t node) { node->next = node->next->next; }

om_node_t om_list_get(om_list_t list, uint16_t number) {
  om_node_t end = list->head;
  for (int i = 1; i < number; i++) end = end->next;
  return end;
}

inline om_node_t om_list_end(om_list_t list) {
  return om_list_get(list, list->number);
}

om_status_t om_list_add(om_list_t list, om_node_t new_node) {
  OM_ASSENT(list == NULL || new_node == NULL);

  if (list->number == 0) {
    list->head = new_node;
  } else {
    om_list_end(list)->next = new_node;
  }
  list->number++;

  return OM_OK;
}

om_list_t om_list_init(void* data, size_t size) {
  OM_ASSENT(data == NULL);
  OM_CHECK(size != 0);

  om_list_t list = om_malloc(sizeof(om_list_data_t));
  om_list_add(list, om_node_create(data, size));

  return list;
}

om_status_t om_list_delete(om_list_t list, uint16_t number) {
  OM_ASSENT(list == NULL || number == 0);
  OM_CHECK(list->number < number);

  om_node_t del = list->head;
  if (number == 1) {
    list->head = list->head->next;
  } else {
    om_node_t left = om_list_get(list, number - 1);
    del = left->next;
    om_node_across(left);
  }

  list->number--;

  om_node_delete(del);
}

uint16_t om_list_find(const om_list_t list,
                      bool (*check)(const void* data, const void* source),
                      void* arg) {
  OM_ASSENT(list == NULL || check == NULL);
  OM_CHECK(list->number != 0);

  om_node_t node = list->head;
  for (uint16_t i = 1; i <= list->number; i++) {
    if (check(node->data, arg)) return i;
    node = node->next;
  }

  return 0;
}

om_status_t om_list_through(om_list_t list,
                            om_status_t (*check)(const void* data,
                                                 const void* source),
                            void* arg) {
  OM_ASSENT(list == NULL || check == NULL);

  om_status_t status = OM_OK;

  om_node_t node = list->head;
  for (uint16_t i = 1; i <= list->number; i++) {
    if (check(node->data, arg)) status = OM_ERROR;
    node = node->next;
  }
};