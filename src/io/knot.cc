#include "io/knot.h"

static void update(Knot *n) {
  if (!n)
    return;
  if (!n->depth || n->depth == 0)
    return;
  uint32_t left_chars = n->left ? n->left->char_count : 0;
  uint32_t right_chars = n->right ? n->right->char_count : 0;
  n->char_count = left_chars + right_chars;
  uint32_t left_lines = n->left ? n->left->line_count : 0;
  uint32_t right_lines = n->right ? n->right->line_count : 0;
  n->line_count = left_lines + right_lines;
  uint8_t left_depth = n->left ? n->left->depth : 0;
  uint8_t right_depth = n->right ? n->right->depth : 0;
  n->depth = MAX(left_depth, right_depth) + 1;
  n->chunk_size = n->left ? n->left->chunk_size : n->right->chunk_size;
}

uint32_t optimal_chunk_size(uint64_t length) {
  if (length <= MIN_CHUNK_SIZE)
    return MIN_CHUNK_SIZE;
  double target_exponent = MIN(std::log2((double)MAX_CHUNK_SIZE),
                               7.0 + (std::log2((double)length) - 10.0) * 0.25);
  uint32_t final_chunk_size =
      MAX((uint32_t)MIN_CHUNK_SIZE, (uint32_t)std::pow(2.0, target_exponent));
  final_chunk_size = MIN(final_chunk_size, (uint32_t)MAX_CHUNK_SIZE);
  final_chunk_size = 1U << (32 - __builtin_clz(final_chunk_size - 1));
  return final_chunk_size;
}

// str is not consumed and \0 is not handled
// So if str is null terminated then len must be strlen(str)
// and freed by caller
Knot *load(char *str, uint32_t len, uint32_t chunk_size) {
  if (len > (uint32_t)(chunk_size - (chunk_size / 16))) {
    Knot *left = load(str, len / 2, chunk_size);
    Knot *right = load(str + len / 2, len - len / 2, chunk_size);
    Knot *node = (Knot *)malloc(sizeof(Knot));
    if (!node)
      return nullptr;
    node->left = left;
    node->right = right;
    node->chunk_size = chunk_size;
    node->depth = MAX(left->depth, right->depth) + 1;
    node->char_count = left->char_count + right->char_count;
    node->line_count = left->line_count + right->line_count;
    return node;
  } else {
    Knot *node = (Knot *)malloc(sizeof(Knot) + chunk_size + 1);
    if (!node)
      return nullptr;
    node->left = nullptr;
    node->right = nullptr;
    node->chunk_size = chunk_size;
    node->depth = 0;
    node->char_count = len;
    uint32_t newline_count = 0;
    for (uint32_t i = 0; i < len; i++) {
      char c = str[i];
      node->data[i] = c;
      if (c == '\n')
        newline_count++;
    }
    node->line_count = newline_count;
    return node;
  }
}

// leaf if consumed and freed (so dont use or free it after)
// left and right are the new nodes
static void split_leaf(Knot *leaf, uint32_t k, Knot **left, Knot **right) {
  Knot *left_node = (Knot *)malloc(sizeof(Knot) + leaf->chunk_size + 1);
  left_node->left = nullptr;
  left_node->right = nullptr;
  left_node->chunk_size = leaf->chunk_size;
  left_node->depth = 0;
  left_node->char_count = k;
  uint32_t newline_count = 0;
  for (uint32_t i = 0; i < k; i++) {
    char c = leaf->data[i];
    left_node->data[i] = c;
    if (c == '\n')
      newline_count++;
  }
  left_node->line_count = newline_count;
  uint16_t right_line_count = leaf->line_count - newline_count;
  *left = left_node;
  Knot *right_node = (Knot *)malloc(sizeof(Knot) + leaf->chunk_size + 1);
  right_node->left = nullptr;
  right_node->right = nullptr;
  right_node->chunk_size = leaf->chunk_size;
  right_node->depth = 0;
  right_node->char_count = leaf->char_count - k;
  right_node->line_count = right_line_count;
  for (uint32_t i = k; i < leaf->char_count; i++) {
    char c = leaf->data[i];
    right_node->data[i - k] = c;
  }
  *right = right_node;
  free(leaf);
}

// This makes node nonsensical, so dont use or free it after
void split(Knot *node, uint32_t offset, Knot **left, Knot **right) {
  if (!node) {
    *left = nullptr;
    *right = nullptr;
    return;
  }
  if (node->depth == 0) {
    split_leaf(node, offset, left, right);
    return;
  }
  uint32_t left_size = node->left ? node->left->char_count : 0;
  if (offset < left_size) {
    Knot *L = nullptr, *R = nullptr;
    split(node->left, offset, &L, &R);
    node->left = R;
    update(node);
    *right = node;
    *left = L;
  } else {
    uint32_t new_offset = offset - left_size;
    Knot *L = nullptr, *R = nullptr;
    split(node->right, new_offset, &L, &R);
    node->right = L;
    update(node);
    *left = node;
    *right = R;
  }
}

static inline int get_balance_factor(Knot *n) {
  if (!n)
    return 0;
  return (int)DEPTH(n->left) - (int)DEPTH(n->right);
}

static inline Knot *rotate_right(Knot *y) {
  Knot *x = y->left;
  Knot *T2 = x->right;
  x->right = y;
  y->left = T2;
  update(y);
  update(x);
  return x;
}

static inline Knot *rotate_left(Knot *x) {
  Knot *y = x->right;
  Knot *T2 = y->left;
  y->left = x;
  x->right = T2;
  update(x);
  update(y);
  return y;
}

// Technically n can be used after calling
// but use return value instead
Knot *balance(Knot *n) {
  update(n);
  int bal = get_balance_factor(n);
  if (bal > 1) {
    if (get_balance_factor(n->left) < 0)
      n->left = rotate_left(n->left);
    return rotate_right(n);
  }
  if (bal < -1) {
    if (get_balance_factor(n->right) > 0)
      n->right = rotate_right(n->right);
    return rotate_left(n);
  }
  return n;
}

// Dont free left or right after calling (only free return value)
// Assumes both ropes have equal chunk sizes
Knot *concat(Knot *left, Knot *right) {
  if (!left)
    return right;
  if (!right)
    return left;
  if (!left || left->char_count == 0) {
    if (left)
      free_rope(left);
    return right;
  }
  if (!right || right->char_count == 0) {
    if (right)
      free_rope(right);
    return left;
  }
  if (left->depth == 0 && right->depth == 0) {
    if (left->char_count + right->char_count <= left->chunk_size) {
      Knot *node = (Knot *)malloc(sizeof(Knot) + left->chunk_size + 1);
      node->left = nullptr;
      node->right = nullptr;
      node->chunk_size = left->chunk_size;
      node->depth = 0;
      node->char_count = left->char_count + right->char_count;
      node->line_count = left->line_count + right->line_count;
      memcpy(node->data, left->data, left->char_count);
      memcpy(node->data + left->char_count, right->data, right->char_count);
      free(left);
      free(right);
      return node;
    }
  }
  uint16_t d_left = left->depth;
  uint16_t d_right = right->depth;
  if (d_left > d_right + 1) {
    left->right = concat(left->right, right);
    return balance(left);
  }
  if (d_right > d_left + 1) {
    right->left = concat(left, right->left);
    return balance(right);
  }
  Knot *node = (Knot *)malloc(sizeof(Knot));
  if (!node)
    return nullptr;
  node->left = left;
  node->right = right;
  node->chunk_size = left->chunk_size;
  node->depth = MAX(d_left, d_right) + 1;
  update(node);
  return node;
}

// This makes node nonsensical, so dont use or free it after
// Instead, free the return value or use it in node's place
Knot *insert(Knot *node, uint32_t offset, char *str, uint32_t len) {
  if (!node)
    return nullptr;
  if (node->depth == 0 && node->char_count + len <= node->chunk_size) {
    if (offset < node->char_count)
      memmove(node->data + offset + len, node->data + offset,
              node->char_count - offset);
    memcpy(node->data + offset, str, len);
    node->char_count += len;
    for (uint32_t i = 0; i < len; i++)
      if (str[i] == '\n')
        node->line_count++;
    return node;
  }
  if (node->depth > 0) {
    uint32_t left_count = node->left ? node->left->char_count : 0;
    if (offset < left_count) {
      Knot *new_left = insert(node->left, offset, str, len);
      node->left = new_left;
      update(node);
      return balance(node);
    } else {
      Knot *new_right = insert(node->right, offset - left_count, str, len);
      node->right = new_right;
      update(node);
      return balance(node);
    }
  }
  Knot *left_part = nullptr;
  Knot *right_part = nullptr;
  split(node, offset, &left_part, &right_part);
  Knot *middle_part = load(str, len, left_part->chunk_size);
  return concat(concat(left_part, middle_part), right_part);
}

// This makes node nonsensical, so dont use or free it after
// Instead, free the return value or use it in node's place
Knot *erase(Knot *node, uint32_t offset, uint32_t len) {
  if (!node || len == 0 || offset >= node->char_count)
    return node;
  if (offset + len > node->char_count)
    len = node->char_count - offset;
  if (node->depth == 0) {
    uint32_t deleted_newlines = 0;
    for (uint32_t i = offset; i < offset + len; i++)
      if (node->data[i] == '\n')
        deleted_newlines++;
    node->line_count -= deleted_newlines;
    if (offset + len < node->char_count)
      memmove(node->data + offset, node->data + offset + len,
              node->char_count - (offset + len));
    node->char_count -= len;
    return node;
  }
  uint32_t left_count = node->left ? node->left->char_count : 0;
  if (offset + len <= left_count) {
    node->left = erase(node->left, offset, len);
  } else if (offset >= left_count) {
    node->right = erase(node->right, offset - left_count, len);
  } else {
    Knot *left = nullptr, *middle = nullptr, *right = nullptr;
    split(node, offset, &left, &right);
    split(right, len, &middle, &right);
    free_rope(middle);
    return concat(left, right);
  }
  update(node);
  return balance(node);
}

static void _read_into(Knot *node, uint32_t offset, uint32_t len, char *dest) {
  if (!node || len == 0)
    return;
  if (node->depth == 0) {
    memcpy(dest, node->data + offset, len);
    return;
  }
  Knot *left = node->left;
  uint32_t left_count = left ? left->char_count : 0;
  if (offset < left_count) {
    uint32_t chunk_len = left_count - offset;
    if (chunk_len > len)
      chunk_len = len;
    _read_into(left, offset, chunk_len, dest);
    dest += chunk_len;
    len -= chunk_len;
    offset = 0;
  } else {
    offset -= left_count;
  }
  if (len > 0 && node->right)
    _read_into(node->right, offset, len, dest);
}

char *read(Knot *root, uint32_t offset, uint32_t len) {
  if (!root)
    return nullptr;
  if (offset >= root->char_count) {
    char *empty = (char *)malloc(1);
    if (empty)
      empty[0] = '\0';
    return empty;
  }
  if (offset + len > root->char_count) {
    len = root->char_count - offset;
  }
  char *buffer = (char *)malloc((len + 1) * sizeof(char));
  if (!buffer)
    return nullptr;
  _read_into(root, offset, len, buffer);
  buffer[len] = '\0';
  return buffer;
}

// Hopefully free the tree only once at the end of its use using the pointer
// from the last insert or concat or erase call.
// (or use twice if last call was split - for both left and right).
void free_rope(Knot *root) {
  if (!root)
    return;
  free_rope(root->left);
  free_rope(root->right);
  free(root);
}

static uint32_t find_nth_newline_offset(Knot *node, uint32_t n) {
  if (!node || n > node->line_count)
    return UINT32_MAX;
  if (node->depth == 0) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < node->char_count; i++) {
      if (node->data[i] == '\n') {
        if (count == n)
          return i;
        count++;
      }
    }
    return UINT32_MAX;
  }
  uint32_t left_lines = node->left ? node->left->line_count : 0;
  if (n < left_lines) {
    return find_nth_newline_offset(node->left, n);
  } else {
    uint32_t right_offset =
        find_nth_newline_offset(node->right, n - left_lines);
    if (right_offset == UINT32_MAX)
      return UINT32_MAX;
    uint32_t left_chars = node->left ? node->left->char_count : 0;
    return left_chars + right_offset;
  }
}

uint32_t byte_to_line(Knot *node, uint32_t offset, uint32_t *out_col) {
  if (!node) {
    if (out_col)
      *out_col = 0;
    return 0;
  }
  if (offset >= node->char_count) {
    if (out_col)
      *out_col = 0;
    return node->line_count;
  }
  if (node->depth == 0) {
    uint32_t line = 0;
    uint32_t col = 0;
    for (uint32_t i = 0; i < offset; i++) {
      if (node->data[i] == '\n') {
        line++;
        col = 0;
      } else {
        col++;
      }
    }
    if (out_col)
      *out_col = col;
    return line;
  }
  uint32_t left_chars = node->left ? node->left->char_count : 0;
  if (offset < left_chars) {
    return byte_to_line(node->left, offset, out_col);
  } else {
    uint32_t sub_col = 0;
    uint32_t line = (node->left ? node->left->line_count : 0) +
                    byte_to_line(node->right, offset - left_chars, &sub_col);
    if (out_col)
      *out_col = sub_col;
    return line;
  }
}

uint32_t line_to_byte(Knot *node, uint32_t line, uint32_t *out_len) {
  if (!node) {
    if (out_len)
      *out_len = 0;
    return 0;
  }
  uint32_t start_offset = 0;
  uint32_t end_offset = 0;
  if (line == 0) {
    start_offset = 0;
  } else {
    uint32_t prev_newline = find_nth_newline_offset(node, line - 1);
    if (prev_newline == UINT32_MAX)
      start_offset = node->char_count;
    else
      start_offset = prev_newline + 1;
  }
  uint32_t current_newline = find_nth_newline_offset(node, line);
  if (current_newline == UINT32_MAX)
    end_offset = node->char_count;
  else
    end_offset = current_newline + 1;
  if (out_len) {
    if (end_offset > start_offset)
      *out_len = end_offset - start_offset;
    else
      *out_len = 0;
  }
  return start_offset;
}

LineIterator *begin_l_iter(Knot *root, uint32_t start_line) {
  if (!root)
    return nullptr;
  if (start_line > root->line_count)
    return nullptr;
  LineIterator *it = (LineIterator *)malloc(sizeof(LineIterator));
  if (!it)
    return nullptr;
  it->top = 0;
  it->node = nullptr;
  it->capacity = 128;
  it->buffer = (char *)malloc(it->capacity);
  if (!it->buffer) {
    free(it);
    return nullptr;
  }
  if (start_line == 0) {
    it->offset = 0;
    while (root->left) {
      it->stack[it->top++] = root;
      root = root->left;
    }
    if (!root->left && !root->right)
      it->node = root;
    it->stack[it->top++] = root;
    return it;
  }
  Knot *curr = root;
  uint32_t relative_line = --start_line;
  while (curr) {
    it->stack[it->top++] = curr;
    if (!curr->left && !curr->right) {
      it->node = curr;
      break;
    }
    uint32_t left_lines = (curr->left) ? curr->left->line_count : 0;
    if (relative_line < left_lines) {
      curr = curr->left;
    } else {
      relative_line -= left_lines;
      curr = curr->right;
    }
  }
  if (!it->node) {
    free(it->buffer);
    free(it);
    return nullptr;
  }
  it->offset = 0;
  if (relative_line > 0) {
    uint32_t found_newlines = 0;
    uint32_t i = 0;
    for (i = 0; i < it->node->char_count; i++) {
      if (it->node->data[i] == '\n') {
        found_newlines++;
        if (found_newlines == relative_line) {
          it->offset = i + 1;
          break;
        }
      }
    }
  }
  next_line(it, nullptr);
  return it;
}

static inline void iter_retreat_leaf(LineIterator *it) {
  if (it->top == 0) {
    it->node = nullptr;
    return;
  }
  Knot *curr = it->stack[--it->top];
  while (it->top > 0) {
    Knot *parent = it->stack[it->top - 1];
    if (parent->right == curr && parent->left) {
      Knot *target = parent->left;
      while (target) {
        it->stack[it->top++] = target;
        if (!target->left && !target->right) {
          if (target->char_count == 0)
            break;
          it->node = target;
          it->offset = target->char_count;
          return;
        }
        target = (target->right) ? target->right : target->left;
      }
    }
    curr = it->stack[--it->top];
  }
  it->node = nullptr;
}

static void str_reverse(char *begin, char *end) {
  char temp;
  while (begin < end) {
    temp = *begin;
    *begin++ = *end;
    *end-- = temp;
  }
}

char *prev_line(LineIterator *it, uint32_t *out_len) {
  if (!it || !it->node)
    return nullptr;
  size_t len = 0;
  while (it->node) {
    if (it->offset == 0) {
      iter_retreat_leaf(it);
      if (!it->node)
        break;
    }
    it->offset--;
    char c = it->node->data[it->offset];
    if (c == '\n') {
      if (len > 0) {
        it->offset++;
        break;
      }
    }
    if (len + 1 >= it->capacity) {
      it->capacity *= 2;
      char *new_buf = (char *)realloc(it->buffer, it->capacity);
      if (!new_buf)
        return nullptr;
      it->buffer = new_buf;
    }
    it->buffer[len++] = c;
  }
  if (len > 0) {
    it->buffer[len] = '\0';
    str_reverse(it->buffer, it->buffer + len - 1);
    if (out_len)
      *out_len = len;
    return it->buffer;
  }
  return nullptr;
}

static inline void iter_advance_leaf(LineIterator *it) {
  if (it->top == 0) {
    it->node = nullptr;
    return;
  }
  Knot *prev = it->stack[--it->top];
  while (it->top > 0) {
    Knot *parent = it->stack[it->top - 1];
    if (parent->left == prev && parent->right) {
      Knot *curr = parent->right;
      while (curr) {
        it->stack[it->top++] = curr;
        if (!curr->left && !curr->right) {
          if (curr->char_count == 0)
            break;
          it->node = curr;
          it->offset = 0;
          return;
        }
        curr = (curr->left) ? curr->left : curr->right;
      }
    }
    prev = it->stack[--it->top];
  }
  it->node = nullptr;
}

char *next_line(LineIterator *it, uint32_t *out_len) {
  if (!it || !it->node)
    return nullptr;
  size_t len = 0;
  while (it->node) {
    if (it->offset >= it->node->char_count) {
      iter_advance_leaf(it);
      if (!it->node)
        break;
    }
    char *start = it->node->data + it->offset;
    char *end = it->node->data + it->node->char_count;
    char *newline_ptr = (char *)memchr(start, '\n', end - start);
    size_t chunk_len;
    int found_newline = 0;
    if (newline_ptr) {
      chunk_len = (newline_ptr - start) + 1;
      found_newline = 1;
    } else {
      chunk_len = end - start;
    }
    if (len + chunk_len + 1 > it->capacity) {
      it->capacity = (it->capacity * 2) + chunk_len;
      char *new_buf = (char *)realloc(it->buffer, it->capacity);
      if (!new_buf) {
        return nullptr;
      }
      it->buffer = new_buf;
    }
    memcpy(it->buffer + len, start, chunk_len);
    len += chunk_len;
    it->offset += chunk_len;
    if (found_newline) {
      it->buffer[len] = '\0';
      if (out_len)
        *out_len = len;
      return it->buffer;
    }
  }
  if (len > 0) {
    it->buffer[len] = '\0';
    if (out_len)
      *out_len = len;
    return it->buffer;
  }
  return nullptr;
}

LeafIterator *begin_k_iter(Knot *root, uint32_t start_offset) {
  if (!root)
    return nullptr;
  LeafIterator *it = (LeafIterator *)malloc(sizeof(LeafIterator));
  if (!it)
    return nullptr;
  it->top = 0;
  it->adjustment = 0;
  Knot *curr = root;
  while (curr) {
    it->stack[it->top++] = curr;
    if (!curr->left && !curr->right) {
      if (start_offset > curr->char_count) {
        free(it);
        return nullptr;
      }
      it->node = curr;
      it->adjustment = start_offset;
      return it;
    }
    uint32_t left_size = (curr->left) ? curr->left->char_count : 0;
    if (start_offset < left_size) {
      curr = curr->left;
    } else {
      start_offset -= left_size;
      curr = curr->right;
    }
  }
  free(it);
  return nullptr;
}

// Caller must never free the returned string
char *next_leaf(LeafIterator *it, uint32_t *out_len) {
  if (!it || !it->node)
    return nullptr;
  char *data_to_return = it->node->data + it->adjustment;
  if (out_len)
    *out_len = it->node->char_count - it->adjustment;
  it->node->data[it->node->char_count] = '\0';
  it->adjustment = 0;
  Knot *prev_leaf = it->node;
  Knot *parent = nullptr;
  while (it->top > 0) {
    parent = it->stack[--it->top];
    if (parent->right && parent->right != prev_leaf) {
      Knot *curr = parent->right;
      while (curr) {
        it->stack[it->top++] = curr;
        if (!curr->left && !curr->right) {
          it->node = curr;
          return data_to_return;
        }
        curr = curr->left;
        if (!curr)
          curr = it->stack[it->top - 1]->right;
      }
    }
    prev_leaf = parent;
  }
  it->node = nullptr;
  return data_to_return;
}

ByteIterator *begin_b_iter(Knot *root) {
  ByteIterator *b_it = (ByteIterator *)malloc(sizeof(ByteIterator));
  LeafIterator *l_it = begin_k_iter(root, 0);
  b_it->it = l_it;
  b_it->offset_g = 0;
  b_it->offset_l = 0;
  b_it->char_count = 0;
  b_it->data = nullptr;
  return b_it;
}

char next_byte(ByteIterator *it) {
  if (it->data && it->offset_l < it->char_count) {
    return it->data[it->offset_l++];
  } else {
    it->offset_g += it->offset_l;
    it->offset_l = 1;
    char *data = next_leaf(it->it, &it->char_count);
    if (!data)
      return '\0';
    while (it->char_count <= 0) {
      data = next_leaf(it->it, &it->char_count);
      if (!data)
        return '\0';
    }
    it->data = data;
    return *it->data;
  }
}

// Caller must NOT free returned string.
// Returns nullptr if offset is invalid or no leaf found.
char *leaf_from_offset(Knot *root, uint32_t start_offset, uint32_t *out_len) {
  if (!root)
    return nullptr;
  Knot *curr = root;
  while (curr) {
    if (!curr->left && !curr->right) {
      if (start_offset > curr->char_count)
        return nullptr;
      char *result = curr->data + start_offset;
      if (out_len)
        *out_len = curr->char_count - start_offset;
      curr->data[curr->char_count] = '\0';
      return result;
    }
    uint32_t left_size = curr->left ? curr->left->char_count : 0;
    if (start_offset < left_size) {
      curr = curr->left;
    } else {
      start_offset -= left_size;
      curr = curr->right;
    }
  }
  return nullptr;
}

std::vector<std::pair<size_t, size_t>> search_rope_dfa(Knot *root,
                                                       const char *pattern) {
  std::vector<std::pair<size_t, size_t>> results;
  int errorcode;
  PCRE2_SIZE erroffset;
  pcre2_code *re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, 0,
                                 &errorcode, &erroffset, nullptr);
  if (!re) {
    fprintf(stderr, "PCRE2 compile error: %d\n", errorcode);
    return results;
  }
  pcre2_match_data *mdata = pcre2_match_data_create(128, nullptr);
  int workspace[PCRE_WORKSPACE_SIZE];
  LeafIterator *it = begin_k_iter(root, 0);
  if (!it) {
    pcre2_code_free(re);
    pcre2_match_data_free(mdata);
    return results;
  }
  size_t limit = 256;
  results.reserve(limit);
  size_t chunk_abs_offset = 0;
  size_t saved_match_start = 0;
  bool match_in_progress = false;
  int flags = PCRE2_PARTIAL_SOFT;
  while (1) {
    uint32_t chunk_len;
    const char *chunk_start = next_leaf(it, &chunk_len);
    if (!chunk_start)
      break;
    const char *current_ptr = chunk_start;
    size_t remaining_len = chunk_len;
    while (remaining_len > 0) {
      int rc =
          pcre2_dfa_match(re, (PCRE2_SPTR)current_ptr, remaining_len, 0, flags,
                          mdata, nullptr, workspace, PCRE_WORKSPACE_SIZE);
      if (rc >= 0) {
        PCRE2_SIZE *ov = pcre2_get_ovector_pointer(mdata);
        size_t match_start_abs;
        size_t match_end_abs;
        if (match_in_progress) {
          match_start_abs = saved_match_start;
          match_end_abs =
              chunk_abs_offset + (current_ptr - chunk_start) + ov[1];
        } else {
          match_start_abs =
              chunk_abs_offset + (current_ptr - chunk_start) + ov[0];
          match_end_abs =
              chunk_abs_offset + (current_ptr - chunk_start) + ov[1];
        }
        size_t total_len = match_end_abs - match_start_abs;
        if (results.size() >= limit) {
          limit *= 2;
          results.reserve(limit);
        }
        results.push_back(std::make_pair(match_start_abs, total_len));
        size_t consumed = ov[1];
        if (consumed == 0)
          consumed = 1;
        current_ptr += consumed;
        if (consumed > remaining_len)
          remaining_len = 0;
        else
          remaining_len -= consumed;
        match_in_progress = false;
        flags = PCRE2_PARTIAL_SOFT | PCRE2_NOTBOL;
        continue;
      } else if (rc == PCRE2_ERROR_PARTIAL) {
        PCRE2_SIZE *ov = pcre2_get_ovector_pointer(mdata);
        if (!match_in_progress) {
          saved_match_start =
              chunk_abs_offset + (current_ptr - chunk_start) + ov[0];
          match_in_progress = true;
        }
        flags |= PCRE2_DFA_RESTART;
        flags |= PCRE2_NOTBOL;
        break;
      } else {
        if (match_in_progress) {
          match_in_progress = false;
          flags = PCRE2_PARTIAL_SOFT | PCRE2_NOTBOL;
          current_ptr++;
          remaining_len--;
        } else {
          break;
        }
      }
    }
    chunk_abs_offset += chunk_len;
    if (!match_in_progress)
      flags = PCRE2_PARTIAL_SOFT | PCRE2_NOTBOL;
  }
  pcre2_match_data_free(mdata);
  pcre2_code_free(re);
  free(it);
  return results;
}

static const size_t MAX_OVERLAP = 1024;

std::vector<Match> search_rope(Knot *root, const char *pattern) {
  std::vector<Match> results;
  int errorcode;
  PCRE2_SIZE erroffset;
  pcre2_code *re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, 0,
                                 &errorcode, &erroffset, nullptr);
  if (!re)
    return results;
  pcre2_match_data *mdata = pcre2_match_data_create_from_pattern(re, nullptr);
  LeafIterator *it = begin_k_iter(root, 0);
  if (!it) {
    pcre2_match_data_free(mdata);
    pcre2_code_free(re);
    return results;
  }
  size_t limit = 256;
  results.reserve(limit);
  std::string buffer;
  buffer.reserve(MAX_OVERLAP * 2);
  size_t buffer_abs_offset = 0;
  size_t processed_up_to = 0;
  while (true) {
    uint32_t chunk_len;
    const char *chunk = next_leaf(it, &chunk_len);
    if (!chunk)
      break;
    buffer.append(chunk, chunk_len);
    PCRE2_SPTR subject = (PCRE2_SPTR)buffer.data();
    size_t subject_len = buffer.size();
    size_t start_offset = 0;
    while (true) {
      int rc = pcre2_match(re, subject, subject_len, start_offset, 0, mdata,
                           nullptr);
      if (rc < 0)
        break;
      PCRE2_SIZE *ov = pcre2_get_ovector_pointer(mdata);
      size_t local_start = ov[0];
      size_t local_end = ov[1];
      size_t abs_start = buffer_abs_offset + local_start;
      if (abs_start >= processed_up_to) {
        if (results.size() >= limit) {
          limit *= 2;
          results.reserve(limit);
        }
        results.push_back({abs_start, abs_start + local_end - local_start,
                           std::string(buffer.data() + local_start,
                                       local_end - local_start)});
        processed_up_to = abs_start + 1;
      }
      start_offset = (local_end > local_start) ? local_end : local_start + 1;
      if (start_offset >= subject_len)
        break;
    }
    if (buffer.size() > MAX_OVERLAP) {
      size_t trim = buffer.size() - MAX_OVERLAP;
      buffer.erase(0, trim);
      buffer_abs_offset += trim;
    }
  }
  pcre2_match_data_free(mdata);
  pcre2_code_free(re);
  free(it);
  return results;
}
