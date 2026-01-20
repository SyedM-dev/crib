#ifndef SYNTAX_TRIE_H
#define SYNTAX_TRIE_H

#include "utils/utils.h"

template <typename T> struct Trie {
  struct TrieNode {
    bool is_word = false;
    std::array<TrieNode *, 128> children{};
    std::conditional_t<std::is_void_v<T>, char, std::optional<T>> value;
    TrieNode() { children.fill(nullptr); }
  };

  Trie() {}
  ~Trie() { clear_trie(root); }

  void build(const std::vector<std::string> &words, bool cs = true) {
    static_assert(std::is_void_v<T>, "This build() is for Trie<void> only");
    case_sensitive = cs;
    for (auto &w : words)
      insert(w);
  }

  template <typename U = T>
  std::enable_if_t<!std::is_void_v<U>>
  build(const std::vector<std::pair<std::string, U>> &words, bool cs = true) {
    static_assert(!std::is_void_v<T>, "This build() is for typed Trie only");
    case_sensitive = cs;
    for (auto &[w, v] : words)
      insert(w, v);
  }

  uint32_t match(const char *text, uint32_t pos, uint32_t len,
                 bool (*is_word_char)(char c)) const {
    const TrieNode *node = root;
    uint32_t max_len = 0;
    for (uint32_t i = pos; i < len; ++i) {
      unsigned char uc = static_cast<unsigned char>(text[i]);
      if (uc >= 128)
        return 0;
      if (!case_sensitive && uc >= 'A' && uc <= 'Z')
        uc = uc - 'A' + 'a';
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

  template <typename U = T>
  uint32_t
  match(const char *text, uint32_t pos, uint32_t len,
        bool (*is_word_char)(char c),
        std::conditional_t<std::is_void_v<T>, void *, std::optional<T> *>
            out_val = nullptr) const {
    const TrieNode *node = root;
    const TrieNode *last_word_node = nullptr;
    uint32_t max_len = 0;
    for (uint32_t i = pos; i < len; ++i) {
      unsigned char uc = static_cast<unsigned char>(text[i]);
      if (uc >= 128)
        break;
      if (!case_sensitive && uc >= 'A' && uc <= 'Z')
        uc = uc - 'A' + 'a';
      if (!node->children[uc])
        break;
      node = node->children[uc];
      if (node->is_word) {
        last_word_node = node;
        max_len = i - pos + 1;
      }
    }
    if (!last_word_node) {
      if (out_val)
        *out_val = std::nullopt;
      return 0;
    }
    if (pos + max_len < len && is_word_char(text[pos + max_len])) {
      if (out_val)
        *out_val = std::nullopt;
      return 0;
    }
    if (out_val)
      *out_val = last_word_node->value;
    return max_len;
  }

private:
  TrieNode *root = new TrieNode();
  bool case_sensitive = true;

  void insert(const std::string &word) {
    TrieNode *node = root;
    for (char c : word) {
      unsigned char uc = static_cast<unsigned char>(c);
      if (uc >= 128)
        return;
      if (!case_sensitive && uc >= 'A' && uc <= 'Z')
        uc = uc - 'A' + 'a';
      if (!node->children[uc])
        node->children[uc] = new TrieNode();
      node = node->children[uc];
    }
    node->is_word = true;
  }

  template <typename U = T>
  std::enable_if_t<!std::is_void_v<U>> insert(const std::string &word,
                                              const U &val) {
    TrieNode *node = root;
    for (char c : word) {
      unsigned char uc = static_cast<unsigned char>(c);
      if (!case_sensitive && uc >= 'A' && uc <= 'Z')
        uc = uc - 'A' + 'a';
      if (!node->children[uc])
        node->children[uc] = new TrieNode();
      node = node->children[uc];
    }
    node->is_word = true;
    node->value = val;
  }

  void clear_trie(TrieNode *node) {
    if (!node)
      return;
    for (auto *child : node->children)
      clear_trie(child);
    delete node;
  }
};

#endif
