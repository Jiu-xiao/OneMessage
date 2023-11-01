#include "om_rbt.h"

#define GET_PARENT(r) ((r)->parent)
#define GET_COLOR(r) ((r)->color)
#define IS_RED(r) ((r)->color == RBT_COLOR_RED)
#define IS_BLACK(r) ((r)->color == RBT_COLOR_BLACK)

#define SET_BLACK(r)              \
  do {                            \
    (r)->color = RBT_COLOR_BLACK; \
  } while (0)

#define SET_RED(r)              \
  do {                          \
    (r)->color = RBT_COLOR_RED; \
  } while (0)

#define SET_PARENT(r, p) \
  do {                   \
    (r)->parent = (p);   \
  } while (0)

#define SET_COLOR(r, c) \
  do {                  \
    (r)->color = (c);   \
  } while (0)

static om_rbt_node_t *_search(om_rbt_t x, const char *key) {
  if (x == NULL) return NULL;

  int ans = strcmp(key, x->key);

  if (ans == 0)
    return x;
  else if (ans < 0)
    return _search(x->left, key);
  else
    return _search(x->right, key);
}

om_rbt_node_t *om_rbtree_search(om_rbt_root_t *root, const char *key) {
  return _search(root->node, key);
}

static void rbtree_left_rotate(om_rbt_root_t *root, om_rbt_node_t *x) {
  om_rbt_node_t *y = x->right;

  x->right = y->left;
  if (y->left != NULL) y->left->parent = x;

  y->parent = x->parent;

  if (x->parent == NULL) {
    root->node = y;
  } else {
    if (x->parent->left == x)
      x->parent->left = y;
    else
      x->parent->right = y;
  }

  y->left = x;
  x->parent = y;
}

static void rbtree_right_rotate(om_rbt_root_t *root, om_rbt_node_t *y) {
  om_rbt_node_t *x = y->left;

  y->left = x->right;
  if (x->right != NULL) x->right->parent = y;

  x->parent = y->parent;

  if (y->parent == NULL) {
    root->node = x;
  } else {
    if (y == y->parent->right)
      y->parent->right = x;
    else
      y->parent->left = x;
  }

  x->right = y;

  y->parent = x;
}

static void rbtree_insert_fixup(om_rbt_root_t *root, om_rbt_node_t *node) {
  om_rbt_node_t *parent, *gparent;

  while ((parent = GET_PARENT(node)) && IS_RED(parent)) {
    gparent = GET_PARENT(parent);

    if (parent == gparent->left) {
      {
        om_rbt_node_t *uncle = gparent->right;
        if (uncle && IS_RED(uncle)) {
          SET_BLACK(uncle);
          SET_BLACK(parent);
          SET_RED(gparent);
          node = gparent;
          continue;
        }
      }

      if (parent->right == node) {
        om_rbt_node_t *tmp;
        rbtree_left_rotate(root, parent);
        tmp = parent;
        parent = node;
        node = tmp;
      }

      SET_BLACK(parent);
      SET_RED(gparent);
      rbtree_right_rotate(root, gparent);
    } else {
      {
        om_rbt_node_t *uncle = gparent->left;
        if (uncle && IS_RED(uncle)) {
          SET_BLACK(uncle);
          SET_BLACK(parent);
          SET_RED(gparent);
          node = gparent;
          continue;
        }
      }

      if (parent->left == node) {
        om_rbt_node_t *tmp;
        rbtree_right_rotate(root, parent);
        tmp = parent;
        parent = node;
        node = tmp;
      }

      SET_BLACK(parent);
      SET_RED(gparent);
      rbtree_left_rotate(root, gparent);
    }
  }

  SET_BLACK(root->node);
}

static void rbtree_insert(om_rbt_root_t *root, om_rbt_node_t *node) {
  om_rbt_node_t *y = NULL;
  om_rbt_node_t *x = root->node;

  while (x != NULL) {
    y = x;
    if (strcmp(node->key, x->key) < 0)
      x = x->left;
    else
      x = x->right;
  }
  GET_PARENT(node) = y;

  if (y != NULL) {
    if (strcmp(node->key, y->key) < 0)
      y->left = node;
    else
      y->right = node;
  } else {
    root->node = node;
  }

  node->color = RBT_COLOR_RED;

  rbtree_insert_fixup(root, node);
}

bool om_rbtree_insert(om_rbt_root_t *root, om_rbt_node_t *node) {
  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;
  node->color = RBT_COLOR_BLACK;

  rbtree_insert(root, node);

  return true;
}

static void rbtree_delete_fixup(om_rbt_root_t *root, om_rbt_node_t *node,
                                om_rbt_node_t *parent) {
  om_rbt_node_t *other;

  while ((!node || IS_BLACK(node)) && node != root->node) {
    if (parent->left == node) {
      other = parent->right;
      if (IS_RED(other)) {
        SET_BLACK(other);
        SET_RED(parent);
        rbtree_left_rotate(root, parent);
        other = parent->right;
      }
      if ((!other->left || IS_BLACK(other->left)) &&
          (!other->right || IS_BLACK(other->right))) {
        SET_RED(other);
        node = parent;
        parent = GET_PARENT(node);
      } else {
        if (!other->right || IS_BLACK(other->right)) {
          SET_BLACK(other->left);
          SET_RED(other);
          rbtree_right_rotate(root, other);
          other = parent->right;
        }
        SET_COLOR(other, GET_COLOR(parent));
        SET_BLACK(parent);
        SET_BLACK(other->right);
        rbtree_left_rotate(root, parent);
        node = root->node;
        break;
      }
    } else {
      other = parent->left;
      if (IS_RED(other)) {
        SET_BLACK(other);
        SET_RED(parent);
        rbtree_right_rotate(root, parent);
        other = parent->left;
      }
      if ((!other->left || IS_BLACK(other->left)) &&
          (!other->right || IS_BLACK(other->right))) {
        SET_RED(other);
        node = parent;
        parent = GET_PARENT(node);
      } else {
        if (!other->left || IS_BLACK(other->left)) {
          SET_BLACK(other->right);
          SET_RED(other);
          rbtree_left_rotate(root, other);
          other = parent->left;
        }
        SET_COLOR(other, GET_COLOR(parent));
        SET_BLACK(parent);
        SET_BLACK(other->left);
        rbtree_right_rotate(root, parent);
        node = root->node;
        break;
      }
    }
  }
  if (node) SET_BLACK(node);
}

void om_rbtree_delete(om_rbt_root_t *root, om_rbt_node_t *node) {
  om_rbt_node_t *child, *parent;
  int color;

  if ((node->left != NULL) && (node->right != NULL)) {
    om_rbt_node_t *replace = node;

    replace = replace->right;
    while (replace->left != NULL) replace = replace->left;

    if (GET_PARENT(node)) {
      if (GET_PARENT(node)->left == node)
        GET_PARENT(node)->left = replace;
      else
        GET_PARENT(node)->right = replace;
    } else
      root->node = replace;

    child = replace->right;
    parent = GET_PARENT(replace);
    color = GET_COLOR(replace);

    if (parent == node) {
      parent = replace;
    } else {
      if (child) SET_PARENT(child, parent);
      parent->left = child;

      replace->right = node->right;
      SET_PARENT(node->right, replace);
    }

    replace->parent = node->parent;
    replace->color = node->color;
    replace->left = node->left;
    node->left->parent = replace;

    if (color == RBT_COLOR_BLACK) rbtree_delete_fixup(root, child, parent);

    return;
  }

  if (node->left != NULL)
    child = node->left;
  else
    child = node->right;

  parent = node->parent;
  color = node->color;

  if (child) child->parent = parent;

  if (parent) {
    if (parent->left == node)
      parent->left = child;
    else
      parent->right = child;
  } else
    root->node = child;

  if (color == RBT_COLOR_BLACK) rbtree_delete_fixup(root, child, parent);
}

void _rbtree_get_num(om_rbt_node_t *node, uint32_t *num) {
  if (node == NULL) return;

  (*num)++;

  _rbtree_get_num(node->left, num);
  _rbtree_get_num(node->right, num);
}

uint32_t om_rbtree_get_num(om_rbt_root_t *root) {
  uint32_t num;
  num = 0;
  _rbtree_get_num(root->node, &num);
  return num;
}

static bool _om_rbtree_foreach(om_rbt_node_t *node,
                               bool (*fun)(om_rbt_node_t *node, void *arg),
                               void *arg) {
  if (node == NULL) {
    return false;
  }

  if (_om_rbtree_foreach(node->left, fun, arg) && fun(node, arg)) {
    return _om_rbtree_foreach(node->right, fun, arg);
  }

  return false;
}

void om_rbtree_foreach(om_rbt_root_t *root,
                       bool (*fun)(om_rbt_node_t *node, void *arg), void *arg) {
  _om_rbtree_foreach(root->node, fun, arg);
}

om_rbt_node_t *om_rbtree_foreach_disc(om_rbt_root_t *rbt, om_rbt_node_t *node) {
  if (node == NULL) {
    node = rbt->node;
    while (node->left != NULL) {
      node = node->left;
    }
    return node;
  }

  if (node->right != NULL) {
    node = node->right;
    while (node->left != NULL) {
      node = node->left;
    }
    return node;
  }

  if (node->parent != NULL) {
    if (node == node->parent->left) {
      return node->parent;
    } else {
      while (node->parent != NULL && node == node->parent->right) {
        node = node->parent;
      }
      return node->parent;
    }
  }

  return NULL;
}
