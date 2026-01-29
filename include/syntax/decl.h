#ifndef SYNTAX_DECL_H
#define SYNTAX_DECL_H

#include "io/knot.h"
#include "io/sysio.h"
#include "syntax/trie.h"

struct Highlight {
  uint32_t fg{0xFFFFFF};
  uint32_t bg{0x000000};
  uint8_t flags{0};
};

enum struct TokenKind : uint8_t {
#define ADD(name) name,
#include "syntax/tokens.def"
#undef ADD
  Count
};

constexpr size_t TOKEN_KIND_COUNT = static_cast<size_t>(TokenKind::Count);

const std::unordered_map<std::string, TokenKind> kind_map = {
#define ADD(name) {#name, TokenKind::name},
#include "syntax/tokens.def"
#undef ADD
};

extern std::array<Highlight, TOKEN_KIND_COUNT> highlights;

struct Token {
  uint32_t start;
  uint32_t end;
  TokenKind type;
};

struct LineData {
  std::shared_ptr<void> in_state{nullptr};
  std::vector<Token> tokens;
  std::shared_ptr<void> out_state{nullptr};
};

struct CustomState {
  mrb_value state;
  CustomState(mrb_value s) : state(s) {}
};

#endif
