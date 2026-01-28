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

constexpr const char tokens_def[] = "module Tokens\n"
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define ADD(name) "  " #name " = " STRINGIFY(__COUNTER__) "\n"
#include "syntax/tokens.def"
#undef ADD
#undef STRINGIFY
#undef STRINGIFY_HELPER
                                    "  freeze\n"
                                    "end";

constexpr const char crib_module[] = {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc23-extensions"
#embed "libcrib.rb"
#pragma clang diagnostic pop
    , '\0'};

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
  VALUE state;
  CustomState(VALUE s) : state(s) { rb_gc_register_address(&state); }
  ~CustomState() { rb_gc_unregister_address(&state); }
};

#endif
