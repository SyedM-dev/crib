#ifndef SYNTAX_DECL_H
#define SYNTAX_DECL_H

#include "io/knot.h"
#include "io/sysio.h"
#include "syntax/trie.h"

struct Highlight {
  uint32_t fg;
  uint32_t bg;
  uint8_t flags;
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

inline void load_theme(std::string filename) {
  uint32_t len = 0;
  char *raw = load_file(filename.c_str(), &len);
  if (!raw)
    return;
  std::string data(raw, len);
  free(raw);
  json j = json::parse(data);
  Highlight default_hl = {0xFFFFFF, 0, 0};
  if (j.contains("Default")) {
    auto def = j["Default"];
    if (def.contains("fg") && def["fg"].is_string())
      default_hl.fg = HEX(def["fg"]);
    if (def.contains("bg") && def["bg"].is_string())
      default_hl.bg = HEX(def["bg"]);
    if (def.contains("italic") && def["italic"].get<bool>())
      default_hl.flags |= CF_ITALIC;
    if (def.contains("bold") && def["bold"].get<bool>())
      default_hl.flags |= CF_BOLD;
    if (def.contains("underline") && def["underline"].get<bool>())
      default_hl.flags |= CF_UNDERLINE;
    if (def.contains("strikethrough") && def["strikethrough"].get<bool>())
      default_hl.flags |= CF_STRIKETHROUGH;
  }
  for (auto &hl : highlights)
    hl = default_hl;
  for (auto &[key, value] : j.items()) {
    if (key == "Default")
      continue;
    auto it = kind_map.find(key);
    if (it == kind_map.end())
      continue;
    Highlight hl = {0xFFFFFF, 0, 0};
    if (value.contains("fg") && value["fg"].is_string())
      hl.fg = HEX(value["fg"]);
    if (value.contains("bg") && value["bg"].is_string())
      hl.bg = HEX(value["bg"]);
    if (value.contains("italic") && value["italic"].get<bool>())
      hl.flags |= CF_ITALIC;
    if (value.contains("bold") && value["bold"].get<bool>())
      hl.flags |= CF_BOLD;
    if (value.contains("underline") && value["underline"].get<bool>())
      hl.flags |= CF_UNDERLINE;
    if (value.contains("strikethrough") && value["strikethrough"].get<bool>())
      hl.flags |= CF_STRIKETHROUGH;
    highlights[static_cast<uint8_t>(it->second)] = hl;
  }
}

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

#endif
