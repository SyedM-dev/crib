#ifndef SYNTAX_DECL_H
#define SYNTAX_DECL_H

#include "io/knot.h"
#include "io/sysio.h"

struct Trie {
  struct TrieNode {
    bool is_word = false;
    std::array<TrieNode *, 128> children{};
    TrieNode() { children.fill(nullptr); }
  };

  Trie() : root(new TrieNode()) {}
  ~Trie() { clear_trie(root); }

  void build(const std::vector<std::string> &words) {
    for (const auto &word : words) {
      TrieNode *node = root;
      for (char c : word) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (!node->children[uc])
          node->children[uc] = new TrieNode();
        node = node->children[uc];
      }
      node->is_word = true;
    }
  }

  uint32_t match(const char *text, uint32_t pos, uint32_t len,
                 bool (*is_word_char)(char c)) const {
    const TrieNode *node = root;
    uint32_t max_len = 0;
    for (uint32_t i = pos; i < len; ++i) {
      unsigned char uc = static_cast<unsigned char>(text[i]);
      if (uc >= 128)
        return 0;
      if (!node->children[uc]) {
        if (node->is_word && !is_word_char(text[i]))
          return i - pos;
        break;
      }
      node = node->children[uc];
      if (node->is_word)
        max_len = i - pos + 1;
    }
    if (max_len > 0)
      if (pos + max_len < len && is_word_char(text[pos + max_len]))
        return 0;
    return max_len;
  }

  void clear() {
    clear_trie(root);
    root = new TrieNode();
  }

private:
  TrieNode *root;

  void clear_trie(TrieNode *node) {
    if (!node)
      return;
    for (auto *child : node->children)
      clear_trie(child);
    delete node;
  }
};

struct Highlight {
  uint32_t fg;
  uint32_t bg;
  uint8_t flags;
};

inline static const std::unordered_map<uint8_t, Highlight> highlight_map = {
    {0, {0xFFFFFF, 0, 0}},  {1, {0xAAAAAA, 0, CF_ITALIC}},
    {2, {0xAAD94C, 0, 0}},  {3, {0xFFFFFF, 0, CF_ITALIC}},
    {4, {0xFF8F40, 0, 0}},  {5, {0xFFB454, 0, 0}},
    {6, {0xD2A6FF, 0, 0}},  {7, {0x95E6CB, 0, 0}},
    {8, {0xF07178, 0, 0}},  {9, {0xE6C08A, 0, 0}},
    {10, {0x7dcfff, 0, 0}},
};

struct Token {
  uint32_t start;
  uint32_t end;
  uint8_t type;
};

struct LineData {
  std::shared_ptr<void> in_state{nullptr};
  std::vector<Token> tokens;
  std::shared_ptr<void> out_state{nullptr};
};

#endif
