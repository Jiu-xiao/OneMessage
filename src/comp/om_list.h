#include "om_def.h"
#include "om_lib.h"

typedef struct _node {
  void* data;
  size_t size;
  struct _node* next;
} om_node_data_t;

typedef om_node_data_t* om_node_t;

typedef struct list {
  om_node_t head;
  uint16_t number;
} om_list_data_t;

typedef om_list_data_t* om_list_t;

om_node_t om_node_create(void* data, size_t size);

void om_node_delete(om_node_t node);

void om_node_connect(om_node_t left, om_node_t right);

void om_node_across(om_node_t node);

om_node_t om_list_get(om_list_t list, uint16_t number);

om_node_t om_list_end(om_list_t list);

om_status_t om_list_add(om_list_t list, om_node_t new_node);

om_list_t om_list_init(void* data, size_t size);

om_status_t om_list_delete(om_list_t list, uint16_t number);

uint16_t om_list_find(const om_list_t list,
                      bool (*check)(const void* data, const void* source),
                      void* arg);

om_status_t om_list_through(om_list_t list,
                            om_status_t (*check)(const void* data,
                                                 const void* source),
                            void* arg);