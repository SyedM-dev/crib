#include "syntax/decl.h"
#include "syntax/langs.h"
#include "utils/utils.h"

struct BashFullState {
  int brace_level = 0;

  enum : uint8_t { NONE, STRING, HEREDOC };
  uint8_t in_state = BashFullState::NONE;

  bool line_cont = false;

  struct Lit {
    std::string delim = "";
    int brace_level = 1;
    bool allow_interp = false;

    bool operator==(const BashFullState::Lit &other) const {
      return delim == other.delim && brace_level == other.brace_level &&
             allow_interp == other.allow_interp;
    }
  } lit;

  bool operator==(const BashFullState &other) const {
    return in_state == other.in_state && lit == other.lit &&
           brace_level == other.brace_level && line_cont == other.line_cont;
  }
};

struct BashState {
  using full_state_type = BashFullState;

  int interp_level = 0;
  std::stack<std::shared_ptr<BashFullState>> interp_stack;
  std::shared_ptr<BashFullState> full_state;

  bool operator==(const BashState &other) const {
    return interp_level == other.interp_level &&
           interp_stack == other.interp_stack &&
           ((full_state && other.full_state &&
             *full_state == *other.full_state));
  }
};

bool bash_state_match(std::shared_ptr<void> state_1,
                      std::shared_ptr<void> state_2) {
  if (!state_1 || !state_2)
    return false;
  return *std::static_pointer_cast<BashState>(state_1) ==
         *std::static_pointer_cast<BashState>(state_2);
}

std::shared_ptr<void> bash_parse(std::vector<Token> *tokens,
                                 std::shared_ptr<void> in_state,
                                 const char *text, uint32_t len) {
  static bool keywords_trie_init = false;
  if (!keywords_trie_init) {
    keywords_trie_init = true;
  }
  tokens->clear();
  auto state = ensure_state(std::static_pointer_cast<BashState>(in_state));
  uint32_t i = 0;
  while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r' ||
                     text[len - 1] == '\t' || text[len - 1] == ' '))
    len--;
  if (len == 0)
    return state;
  while (i < len) {
    i += utf8_codepoint_width(text[i]);
  }
  return state;
}
