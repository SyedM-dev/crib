#include "syntax/decl.h"
#include "syntax/langs.h"

struct BashFullState {
  int brace_level = 0;

  enum : uint8_t { NONE, STRING, HEREDOC, PARAMETER };
  uint8_t in_state = BashFullState::NONE;

  bool line_cont = false;

  struct Lit {
    std::string delim = ""; // Only 1 wide for strings
    bool allow_interp = false;

    bool operator==(const BashFullState::Lit &other) const {
      return delim == other.delim && allow_interp == other.allow_interp;
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
                                 const char *text, uint32_t len,
                                 uint32_t line_num) {
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
    if (state->full_state->in_state == BashFullState::PARAMETER) {
      uint32_t start = i;
      while (i < len) {
        if (text[i] == '{') {
          i++;
          state->full_state->brace_level++;
          continue;
        }
        if (text[i] == '}') {
          if (--state->full_state->brace_level == 0 &&
              !state->interp_stack.empty()) {
            tokens->push_back({i - 1, i, TokenKind::K_INTERPOLATION});
            state->full_state = state->interp_stack.top();
            state->interp_stack.pop();
            i++;
            break;
          }
        }
        i++;
      }
      continue;
    }
    if (state->full_state->in_state == BashFullState::STRING) {
      uint32_t start = i;
      while (i < len) {
        if (state->full_state->lit.allow_interp && text[i] == '$') {
          if (++i < len && text[i] == '{') {
            tokens->push_back({start, i - 1, TokenKind::K_STRING});
            tokens->push_back({i - 1, i, TokenKind::K_INTERPOLATION});
            state->interp_stack.push(state->full_state);
            state->full_state = std::make_shared<BashFullState>();
            state->full_state->in_state = BashFullState::PARAMETER;
            state->full_state->brace_level = 1;
            break;
          }
        }
        if (text[i] == state->full_state->lit.delim[0]) {
          i++;
          tokens->push_back({start, i, TokenKind::K_STRING});
          state->full_state->in_state = BashFullState::NONE;
          break;
        }
        i++;
      }
      if (i == len)
        tokens->push_back({start, i, TokenKind::K_STRING});
      continue;
    }
    if (text[i] == '#') {
      if (line_num == 0 && i == 0 && len > 4 && text[i + 1] == '!') {
        tokens->push_back({0, len, TokenKind::K_SHEBANG});
        return state;
      }
      tokens->push_back({i, len, TokenKind::K_COMMENT});
      return state;
    } else if (text[i] == '\'') {
      state->full_state->in_state = BashFullState::STRING;
      state->full_state->lit.delim = "'";
      state->full_state->lit.allow_interp = false;
      tokens->push_back({i, ++i, TokenKind::K_STRING});
      continue;
    } else if (text[i] == '"') {
      state->full_state->in_state = BashFullState::STRING;
      state->full_state->lit.delim = "\"";
      state->full_state->lit.allow_interp = true;
      tokens->push_back({i, ++i, TokenKind::K_STRING});
      continue;
    }
    i++;
  }
  return state;
}

// String literals surrounded by   '  strictly with no escaping inside
// double quoted strings "  allow interpolation and escaping - with $var and
// ${var}  and $((math)) $(command) and `command` expansions ANSI-C quoted
// stirngs - $''  backslash escapes but with \xHH and \uHHHH and \uHHHHHHHH \cX
// too
//
// Lock edit_replace across both delete and insert instead of within to keep the
// parser from glitching
