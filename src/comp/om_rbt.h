#ifndef __OM_RBT_H__
#define __OM_RBT_H__

#include "om_def.h"

#define RBT_ROOT(name) om_rbt_root_t name = {NULL}

typedef enum {
  RBT_COLOR_RED,
  RBT_COLOR_BLACK,
} om_rbt_color_t;

typedef struct rbt_node {
  om_rbt_color_t color;
  const char *key;
  struct rbt_node *left;
  struct rbt_node *right;
  struct rbt_node *parent;
} om_rbt_node_t, *om_rbt_t;

typedef struct rb_root {
  om_rbt_node_t *node;
} om_rbt_root_t;

bool om_rbtree_insert(om_rbt_root_t *root, om_rbt_node_t *node);

void om_rbtree_delete(om_rbt_root_t *root, om_rbt_node_t *node);

om_rbt_node_t *om_rbtree_search(om_rbt_root_t *root, const char *key);

uint32_t om_rbtree_get_num(om_rbt_root_t *root);

void om_rbtree_foreach(om_rbt_root_t *root,
                       bool (*fun)(om_rbt_node_t *node, void *arg), void *arg);

om_rbt_node_t *om_rbtree_foreach_disc(om_rbt_root_t *rbt, om_rbt_node_t *node);
#endif