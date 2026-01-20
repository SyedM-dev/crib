#ifndef LINE_TREE_H
#define LINE_TREE_H

#include "syntax/decl.h"

struct LineTree {
  void clear() {
    std::unique_lock lock(mtx);
    clear_node(root);
    root = nullptr;
    stack_size = 0;
  }
  void build(uint32_t x) {
    std::unique_lock lock(mtx);
    root = build_node(x);
  }
  LineData *at(uint32_t x) {
    std::shared_lock lock(mtx);
    LineNode *n = root;
    while (n) {
      uint32_t left_size = n->left ? n->left->size : 0;
      if (x < left_size) {
        n = n->left;
      } else if (x < left_size + n->data.size()) {
        return &n->data[x - left_size];
      } else {
        x -= left_size + n->data.size();
        n = n->right;
      }
    }
    return nullptr;
  }
  LineData *start_iter(uint32_t x) {
    std::shared_lock lock(mtx);
    stack_size = 0;
    LineNode *n = root;
    while (n) {
      uint32_t left_size = n->left ? n->left->size : 0;
      if (x < left_size) {
        push(n, 0);
        n = n->left;
      } else if (x < left_size + n->data.size()) {
        push(n, x - left_size + 1);
        return &n->data[x - left_size];
      } else {
        x -= left_size + n->data.size();
        push(n, UINT32_MAX);
        n = n->right;
      }
    }
    return nullptr;
  }
  void end_iter() { stack_size = 0; }
  LineData *next() {
    std::shared_lock lock(mtx);
    while (stack_size) {
      auto &f = stack[stack_size - 1];
      LineNode *n = f.node;
      if (f.index < n->data.size())
        return &n->data[f.index++];
      stack_size--;
      if (n->right) {
        n = n->right;
        while (n) {
          push(n, 0);
          if (!n->left)
            break;
          n = n->left;
        }
        return &stack[stack_size - 1].node->data[0];
      }
    }
    return nullptr;
  }
  void insert(uint32_t x, uint32_t y) {
    std::unique_lock lock(mtx);
    if (x > subtree_size(root))
      x = subtree_size(root);
    root = insert_node(root, x, y);
  }
  void erase(uint32_t x, uint32_t y) {
    std::unique_lock lock(mtx);
    if (x + y > subtree_size(root))
      x = subtree_size(root) - y;
    root = erase_node(root, x, y);
  }
  uint32_t count() {
    std::shared_lock lock(mtx);
    return subtree_size(root);
  }
  ~LineTree() { clear(); }

private:
  struct LineNode {
    LineNode *left = nullptr;
    LineNode *right = nullptr;
    uint8_t depth = 1;
    uint32_t size = 0;
    std::vector<LineData> data;
  };
  struct Frame {
    LineNode *node;
    uint32_t index;
  };
  void push(LineNode *n, uint32_t x) {
    stack[stack_size].node = n;
    stack[stack_size].index = x;
    stack_size++;
  }
  static void clear_node(LineNode *n) {
    if (!n)
      return;
    clear_node(n->left);
    clear_node(n->right);
    delete n;
  }
  LineNode *root = nullptr;
  Frame stack[32];
  std::atomic<uint8_t> stack_size = 0;
  std::shared_mutex mtx;
  static constexpr uint32_t LEAF_TARGET = 256;
  LineTree::LineNode *erase_node(LineNode *n, uint32_t x, uint32_t y) {
    if (!n || y == 0)
      return n;
    uint32_t left_sz = subtree_size(n->left);
    uint32_t mid_sz = n->data.size();
    if (x < left_sz) {
      uint32_t len = std::min(y, left_sz - x);
      n->left = erase_node(n->left, x, len);
      y -= len;
      x = left_sz;
    }
    if (y > 0 && x < left_sz + mid_sz) {
      uint32_t mid_x = x - left_sz;
      uint32_t len = std::min(y, mid_sz - mid_x);
      n->data.erase(n->data.begin() + mid_x, n->data.begin() + mid_x + len);
      y -= len;
      x += len;
    }
    if (y > 0) {
      n->right = erase_node(n->right, x - left_sz - n->data.size(), y);
    }
    if (n->left && n->right &&
        subtree_size(n->left) + subtree_size(n->right) < 256) {
      return merge(n->left, n->right);
    }
    return rebalance(n);
  }
  LineTree::LineNode *insert_node(LineNode *n, uint32_t x, uint32_t y) {
    if (!n) {
      auto *leaf = new LineNode();
      leaf->data.resize(y);
      leaf->size = y;
      return leaf;
    }
    if (!n->left && !n->right) {
      n->data.insert(n->data.begin() + x, y, LineData());
      fix(n);
      if (n->data.size() > 512)
        return split_leaf(n);
      return n;
    }
    uint32_t left_size = subtree_size(n->left);
    if (x <= left_size)
      n->left = insert_node(n->left, x, y);
    else
      n->right = insert_node(n->right, x - left_size - n->data.size(), y);
    return rebalance(n);
  }
  LineNode *build_node(uint32_t count) {
    if (count <= LEAF_TARGET) {
      auto *n = new LineNode();
      n->data.resize(count);
      n->size = count;
      return n;
    }
    uint32_t left_count = count / 2;
    uint32_t right_count = count - left_count;
    auto *n = new LineNode();
    n->left = build_node(left_count);
    n->right = build_node(right_count);
    fix(n);
    return n;
  }
  static LineNode *split_leaf(LineNode *n) {
    auto *right = new LineNode();
    size_t mid = n->data.size() / 2;
    right->data.assign(n->data.begin() + mid, n->data.end());
    n->data.resize(mid);
    fix(n);
    fix(right);
    auto *parent = new LineNode();
    parent->left = n;
    parent->right = right;
    fix(parent);
    return parent;
  }
  static LineNode *merge(LineNode *a, LineNode *b) {
    a->data.insert(a->data.end(), b->data.begin(), b->data.end());
    delete b;
    fix(a);
    return a;
  }
  static void fix(LineNode *n) {
    n->depth = 1 + MAX(height(n->left), height(n->right));
    n->size = subtree_size(n->left) + n->data.size() + subtree_size(n->right);
  }
  static LineNode *rotate_right(LineNode *y) {
    LineNode *x = y->left;
    LineNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    fix(y);
    fix(x);
    return x;
  }
  static LineNode *rotate_left(LineNode *x) {
    LineNode *y = x->right;
    LineNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    fix(x);
    fix(y);
    return y;
  }
  static LineNode *rebalance(LineNode *n) {
    fix(n);
    int balance = int(height(n->left)) - int(height(n->right));
    if (balance > 1) {
      if (height(n->left->left) < height(n->left->right))
        n->left = rotate_left(n->left);
      return rotate_right(n);
    }
    if (balance < -1) {
      if (height(n->right->right) < height(n->right->left))
        n->right = rotate_right(n->right);
      return rotate_left(n);
    }
    return n;
  }
  static uint8_t height(LineNode *n) { return n ? n->depth : 0; }
  static uint32_t subtree_size(LineNode *n) { return n ? n->size : 0; }
};

#endif
