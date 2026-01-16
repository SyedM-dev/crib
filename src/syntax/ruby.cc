#include "syntax/langs.h"

const static std::vector<std::string> base_keywords = {
    // style 4
    "if",    "else",  "elsif", "case",  "rescue", "ensure", "do",  "for",
    "while", "until", "def",   "class", "module", "begin",  "end", "unless",
};

const static std::vector<std::string> operator_keywords = {
    // style 5
    "alias", "and", "BEGIN", "break",  "catch", "defined?", "in",    "next",
    "not",   "or",  "redo",  "rescue", "retry", "return",   "super", "yield",
    "self",  "nil", "true",  "false",  "undef", "when",
};

const static std::vector<std::string> operators = {
    "+",   "-",   "*",   "/",  "%",  "**", "==",  "!=",  "===",
    "<=>", ">",   ">=",  "<",  "<=", "&&", "||",  "!",   "&",
    "|",   "^",   "~",   "<<", ">>", "=",  "+=",  "-=",  "*=",
    "/=",  "%=",  "**=", "&=", "|=", "^=", "<<=", ">>=", "..",
    "...", "===", "=",   "=>", "&.", "[]", "[]=", "`",   "->",
};

struct HeredocInfo {
  std::string delim;
  bool allow_interpolation{true};
  bool allow_indentation{false};

  bool operator==(const HeredocInfo &other) const {
    return delim == other.delim &&
           allow_interpolation == other.allow_interpolation &&
           allow_indentation == other.allow_indentation;
  }
};

struct RubyFullState {
  // TODO: use this to highlight each level seperaletly like vscode colored
  // braces extention thingy does
  int brace_level = 0;
  int paren_level = 0;
  int bracket_level = 0;

  enum : uint8_t { NONE, STRING, REGEXP, COMMENT, HEREDOC, END };
  uint8_t in_state = RubyFullState::NONE;

  struct Lit {
    char delim_start = '\0';
    char delim_end = '\0';
    // For stuff like %Q{ { these braces are valid } this part is still str }
    int brace_level = 1;
    bool allow_interp = false;

    bool operator==(const RubyFullState::Lit &other) const {
      return delim_start == other.delim_start && delim_end == other.delim_end &&
             brace_level == other.brace_level &&
             allow_interp == other.allow_interp;
    }
  } lit;

  bool operator==(const RubyFullState &other) const {
    return in_state == other.in_state && lit == other.lit &&
           brace_level == other.brace_level &&
           paren_level == other.paren_level &&
           bracket_level == other.bracket_level;
  }
};

struct RubyState {
  int interp_level = 0;
  std::stack<std::shared_ptr<RubyFullState>> interp_stack;
  std::shared_ptr<RubyFullState> full_state;
  std::deque<HeredocInfo> heredocs;

  bool operator==(const RubyState &other) const {
    return interp_level == other.interp_level &&
           interp_stack == other.interp_stack &&
           ((full_state && other.full_state &&
             *full_state == *other.full_state)) &&
           heredocs == other.heredocs;
  }
};

inline std::shared_ptr<RubyState>
ensure_state(std::shared_ptr<RubyState> state) {
  if (!state)
    state = std::make_shared<RubyState>();
  if (state.unique())
    return state;
  return std::make_shared<RubyState>(*state);
}

inline std::shared_ptr<RubyState>
ensure_full_state(std::shared_ptr<RubyState> state) {
  state = ensure_state(state);
  if (!state->full_state)
    state->full_state = std::make_shared<RubyFullState>();
  else if (!state->full_state.unique())
    state->full_state = std::make_shared<RubyFullState>(*state->full_state);
  return state;
}

bool identifier_start_char(char c) {
  return !isascii(c) || isalpha(c) || c == '_';
}

bool identifier_char(char c) { return !isascii(c) || isalnum(c) || c == '_'; }

uint32_t get_next_word(const char *text, uint32_t i, uint32_t len) {
  if (i >= len || !identifier_start_char(text[i]))
    return 0;
  uint32_t width = 1;
  while (i + width < len && identifier_char(text[i + width]))
    width++;
  if (i + width < len && (text[i + width] == '!' || text[i + width] == '?'))
    width++;
  return width;
}

bool compare(const char *a, const char *b, size_t n) {
  size_t i = 0;
  for (; i < n; ++i)
    if (a[i] != b[i])
      return false;
  return true;
}

std::shared_ptr<void> ruby_parse(std::vector<Token> *tokens,
                                 std::shared_ptr<void> in_state,
                                 const char *text, uint32_t len) {
  static bool keywords_trie_init = false;
  static Trie base_keywords_trie;
  static Trie operator_keywords_trie;
  static Trie operator_trie;
  if (!keywords_trie_init) {
    base_keywords_trie.build(base_keywords);
    operator_keywords_trie.build(operator_keywords);
    operator_trie.build(operators);
    keywords_trie_init = true;
  }
  tokens->clear();
  if (!in_state)
    in_state = std::make_shared<RubyState>();
  std::shared_ptr<RubyState> state =
      std::static_pointer_cast<RubyState>(in_state);
  if (!state->full_state)
    state->full_state = std::make_shared<RubyFullState>();
  uint32_t i = 0;
  while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r' ||
                     text[len - 1] == '\t' || text[len - 1] == ' '))
    len--;
  if (len == 0)
    return state;
  bool heredoc_first = false;
  while (i < len) {
    if (state->full_state->in_state == RubyFullState::END) {
      tokens->clear();
      return state;
    }
    if (state->full_state->in_state == RubyFullState::COMMENT) {
      tokens->push_back({i, len, 1});
      if (i == 0 && len == 4 && text[i] == '=' && text[i + 1] == 'e' &&
          text[i + 2] == 'n' && text[i + 3] == 'd') {
        state = ensure_full_state(state);
        state->full_state->in_state = RubyFullState::NONE;
      }
      return state;
    }
    if (!heredoc_first &&
        state->full_state->in_state == RubyFullState::HEREDOC) {
      if (i == 0) {
        uint32_t start = 0;
        if (state->heredocs.front().allow_indentation)
          while (start < len && (text[start] == ' ' || text[start] == '\t'))
            start++;
        if (len - start == state->heredocs.front().delim.length() &&
            compare(text + start, state->heredocs.front().delim.c_str(),
                    state->heredocs.front().delim.length())) {
          state = ensure_full_state(state);
          state->heredocs.pop_front();
          if (state->heredocs.empty())
            state->full_state->in_state = RubyFullState::NONE;
          tokens->push_back({i, len, 10});
          return state;
        }
      }
      uint32_t start = i;
      if (!state->heredocs.front().allow_interpolation) {
        tokens->push_back({i, len, 2});
        return state;
      } else {
        while (i < len) {
          if (text[i] == '\\') {
            // TODO: highlight the escape character
            i++;
            if (i < len)
              i++;
            continue;
          }
          if (text[i] == '#' && i + 1 < len && text[i + 1] == '{') {
            tokens->push_back({start, i, 2});
            tokens->push_back({i, i + 2, 10});
            i += 2;
            state = ensure_state(state);
            state->interp_stack.push(state->full_state);
            state->full_state = std::make_shared<RubyFullState>();
            state->interp_level = 1;
            break;
          }
          i++;
        }
        if (i == len)
          tokens->push_back({start, len, 2});
        continue;
      }
    }
    if (state->full_state->in_state == RubyFullState::STRING) {
      uint32_t start = i;
      while (i < len) {
        if (text[i] == '\\') {
          // TODO: highlight the escape character - need to make priority work
          // and this have higher
          i++;
          if (i < len)
            i++;
          continue;
        }
        if (state->full_state->lit.allow_interp && text[i] == '#' &&
            i + 1 < len && text[i + 1] == '{') {
          tokens->push_back({start, i, 2});
          tokens->push_back({i, i + 2, 10});
          i += 2;
          state = ensure_state(state);
          state->interp_stack.push(state->full_state);
          state->full_state = std::make_shared<RubyFullState>();
          state->interp_level = 1;
          break;
        }
        if (text[i] == state->full_state->lit.delim_start &&
            state->full_state->lit.delim_start !=
                state->full_state->lit.delim_end) {
          state = ensure_full_state(state);
          state->full_state->lit.brace_level++;
        }
        if (text[i] == state->full_state->lit.delim_end) {
          state = ensure_full_state(state);
          if (state->full_state->lit.delim_start ==
              state->full_state->lit.delim_end) {
            i++;
            tokens->push_back({start, i, 2});
            state->full_state->in_state = RubyFullState::NONE;
            break;
          } else {
            state->full_state->lit.brace_level--;
            if (state->full_state->lit.brace_level == 0) {
              i++;
              tokens->push_back({start, i, 2});
              state->full_state->in_state = RubyFullState::NONE;
              break;
            }
          }
        }
        i++;
      }
      if (i == len)
        tokens->push_back({start, len, 2});
      continue;
    }
    if (i == 0 && len == 6) {
      if (text[i] == '=' && text[i + 1] == 'b' && text[i + 2] == 'e' &&
          text[i + 3] == 'g' && text[i + 4] == 'i' && text[i + 5] == 'n') {
        state = ensure_full_state(state);
        state->full_state->in_state = RubyFullState::COMMENT;
        tokens->push_back({0, len, 1});
        return state;
      }
    }
    if (i == 0 && len == 7) {
      if (text[i] == '_' && text[i + 1] == '_' && text[i + 2] == 'E' &&
          text[i + 3] == 'N' && text[i + 4] == 'D' && text[i + 5] == '_' &&
          text[i + 6] == '_') {
        state = ensure_full_state(state);
        tokens->clear();
        state->full_state->in_state = RubyFullState::END;
        return state;
      }
    }
    if (i + 3 <= len && text[i] == '<' && text[i + 1] == '<') {
      uint32_t j = i + 2;
      bool indented = false;
      if (text[j] == '~')
        indented = true;
      if (text[j] == '~' || text[j] == '-')
        j++;
      tokens->push_back({i, j, 10});
      if (j >= len)
        continue;
      std::string delim;
      bool interpolation = true;
      uint32_t s = j;
      if (text[j] == '\'' || text[j] == '"') {
        char q = text[j++];
        if (q == '\'')
          interpolation = false;
        while (j < len && text[j] != q)
          delim += text[j++];
      } else {
        while (j < len && identifier_char(text[j]))
          delim += text[j++];
      }
      if (!delim.empty()) {
        tokens->push_back({s, j, 10});
        state = ensure_full_state(state);
        state->heredocs.push_back({delim, interpolation, indented});
        state->full_state->in_state = RubyFullState::HEREDOC;
        heredoc_first = true;
      }
      i = j;
      continue;
    }
    if (text[i] == '#') {
      tokens->push_back({i, len, 1});
      return state;
    } else if (text[i] == ':') {
      uint32_t start = i;
      i++;
      if (i >= len) {
        tokens->push_back({start, i, 3});
        continue;
      }
      if (text[i] == '\'' || text[i] == '"') {
        tokens->push_back({start, i, 6});
        continue;
      }
      if (text[i] == '$' || text[i] == '@') {
        uint32_t var_start = i;
        i++;
        if (i < len && text[var_start] == '@' && text[var_start + 1] == '@')
          i++;
        while (i < len && identifier_char(text[i]))
          i++;
        tokens->push_back({start, i, 6});
        continue;
      }
      uint32_t op_len = operator_trie.match(text, i, len, identifier_char);
      if (op_len > 0) {
        tokens->push_back({start, i + op_len, 6});
        i += op_len;
        continue;
      }
      if (identifier_start_char(text[i])) {
        uint32_t word_len = get_next_word(text, i, len);
        tokens->push_back({start, i + word_len, 6});
        i += word_len;
        continue;
      }
      tokens->push_back({start, i, 3});
      continue;
    } else if (text[i] == '@') {
      uint32_t start = i;
      i++;
      if (i >= len)
        continue;
      if (text[i] == '@')
        i++;
      if (i < len && identifier_start_char(text[i]))
        i++;
      else
        continue;
      while (i < len && identifier_char(text[i]))
        i++;
      tokens->push_back({start, i, 7});
      continue;
    } else if (text[i] == '$') {
      uint32_t start = i;
      i++;
      if (i >= len)
        continue;
      if (identifier_start_char(text[i])) {
        i++;
        while (i < len && identifier_char(text[i]))
          i++;
      } else if (i + 1 < len && text[i] == '-' && isalpha(text[i + 1])) {
        i += 2;
      } else if (isdigit(text[i])) {
        i++;
        while (i < len && isdigit(text[i]))
          i++;
      } else if (text[i] != '-' && !isalnum(text[i])) {
        i++;
      } else {
        continue;
      }
      tokens->push_back({start, i, 8});
      continue;
    } else if (text[i] == '?') {
      uint32_t start = i;
      i++;
      if (i < len && text[i] == '\\') {
        i++;
        if (i < len && text[i] == 'x') {
          i++;
          if (i < len && isxdigit(text[i]))
            i++;
          else
            continue;
          if (i < len && isxdigit(text[i]))
            i++;
          tokens->push_back({start, i, 7});
          continue;
        } else if (i < len && text[i] == 'u') {
          i++;
          if (i < len && isxdigit(text[i]))
            i++;
          else
            continue;
          if (i < len && isxdigit(text[i]))
            i++;
          else
            continue;
          if (i < len && isxdigit(text[i]))
            i++;
          else
            continue;
          if (i < len && isxdigit(text[i]))
            i++;
          else
            continue;
          tokens->push_back({start, i, 7});
          continue;
        } else if (i < len) {
          i++;
          tokens->push_back({start, i, 7});
          continue;
        }
      } else if (i < len && text[i] != ' ') {
        i++;
        tokens->push_back({start, i, 7});
        continue;
      } else {
        tokens->push_back({start, i, 3});
        continue;
      }
    } else if (text[i] == '{') {
      tokens->push_back({i, i + 1, 3});
      state = ensure_state(state);
      state->interp_level++;
      i++;
      continue;
    } else if (text[i] == '}') {
      state = ensure_full_state(state);
      state->interp_level--;
      if (state->interp_level == 0 && !state->interp_stack.empty()) {
        state->full_state = state->interp_stack.top();
        state->interp_stack.pop();
        tokens->push_back({i, i + 1, 10});
      } else {
        tokens->push_back({i, i + 1, 3});
      }
      i++;
      continue;
    } else if (text[i] == '\'') {
      tokens->push_back({i, i + 1, 2});
      state = ensure_full_state(state);
      state->full_state->in_state = RubyFullState::STRING;
      state->full_state->lit.delim_start = '\'';
      state->full_state->lit.delim_end = '\'';
      state->full_state->lit.allow_interp = false;
      i++;
      continue;
    } else if (text[i] == '"') {
      tokens->push_back({i, i + 1, 2});
      state = ensure_full_state(state);
      state->full_state->in_state = RubyFullState::STRING;
      state->full_state->lit.delim_start = '"';
      state->full_state->lit.delim_end = '"';
      state->full_state->lit.allow_interp = true;
      i++;
      continue;
    } else if (text[i] == '`') {
      tokens->push_back({i, i + 1, 2});
      state = ensure_full_state(state);
      state->full_state->in_state = RubyFullState::STRING;
      state->full_state->lit.delim_start = '`';
      state->full_state->lit.delim_end = '`';
      state->full_state->lit.allow_interp = true;
      i++;
      continue;
    } else if (text[i] == '%') {
      if (i + 1 >= len) {
        i++;
        continue;
      }
      char type = text[i + 1];
      char delim_start = '\0';
      char delim_end = '\0';
      bool allow_interp = true;
      int prefix_len = 1;
      switch (type) {
      case 'Q':
      case 'x':
        allow_interp = true;
        prefix_len = 2;
        break;
      case 'w':
      case 'q':
      case 'i':
        allow_interp = false;
        prefix_len = 2;
        break;
      default:
        allow_interp = true;
        prefix_len = 1;
        break;
      }
      if (i + prefix_len >= len) {
        i += prefix_len;
        continue;
      }
      delim_start = text[i + prefix_len];
      if (!isascii(delim_start) ||
          (isalnum(delim_start) || delim_start == '_' || delim_start == ' ')) {
        i += prefix_len;
        continue;
      }
      switch (delim_start) {
      case '(':
        delim_end = ')';
        break;
      case '{':
        delim_end = '}';
        break;
      case '[':
        delim_end = ']';
        break;
      case '<':
        delim_end = '>';
        break;
      default:
        delim_end = delim_start;
        break;
      }
      tokens->push_back({i, i + prefix_len + 1, 2});
      state = ensure_full_state(state);
      state->full_state->in_state = RubyFullState::STRING;
      state->full_state->lit.delim_start = delim_start;
      state->full_state->lit.delim_end = delim_end;
      state->full_state->lit.allow_interp = allow_interp;
      state->full_state->lit.brace_level = 1;
      i += prefix_len + 1;
      continue;
    } else if (isdigit(text[i])) {
      uint32_t start = i;
      if (text[i] == '0') {
        i++;
        if (i < len && text[i] == 'x') {
          i++;
          if (i < len && isxdigit(text[i]))
            i++;
          else
            continue;
          bool is_underscore = false;
          while (i < len && (isxdigit(text[i]) || text[i] == '_')) {
            if (text[i] == '_')
              is_underscore = true;
            else
              is_underscore = false;
            i++;
          }
          if (is_underscore)
            i--;
        } else if (i < len && text[i] == 'b') {
          i++;
          if (i < len && (text[i] == '0' || text[i] == '1'))
            i++;
          else
            continue;
          bool is_underscore = false;
          while (i < len &&
                 (text[i] == '0' || text[i] == '1' || text[i] == '_')) {
            if (text[i] == '_')
              is_underscore = true;
            else
              is_underscore = false;
            i++;
          }
          if (is_underscore)
            i--;
        } else if (i < len && text[i] == 'o') {
          i++;
          if (i < len && text[i] >= '0' && text[i] <= '7')
            i++;
          else
            continue;
          bool is_underscore = false;
          while (i < len &&
                 ((text[i] >= '0' && text[i] <= '7') || text[i] == '_')) {
            if (text[i] == '_')
              is_underscore = true;
            else
              is_underscore = false;
            i++;
          }
          if (is_underscore)
            i--;
        }
      } else {
        bool is_underscore = true;
        while (i < len &&
               (isdigit(text[i]) || (text[i] == '_' && !is_underscore))) {
          if (text[i] == '_')
            is_underscore = true;
          else
            is_underscore = false;
          i++;
        }
        if (is_underscore)
          i--;
        if (i < len && text[i] == '.') {
          i++;
          bool is_underscore = true;
          while (i < len &&
                 (isdigit(text[i]) || (text[i] == '_' && !is_underscore))) {
            if (text[i] == '_')
              is_underscore = true;
            else
              is_underscore = false;
            i++;
          }
          if (is_underscore)
            i--;
        }
        if (i < len && (text[i] == 'E' || text[i] == 'e')) {
          i++;
          if (i < len && (text[i] == '+' || text[i] == '-'))
            i++;
          bool is_underscore = true;
          while (i < len &&
                 (isdigit(text[i]) || (text[i] == '_' && !is_underscore))) {
            if (text[i] == '_')
              is_underscore = true;
            else
              is_underscore = false;
            i++;
          }
          if (is_underscore)
            i--;
        }
      }
      tokens->push_back({start, i, 9});
      continue;
    } else if (identifier_start_char(text[i])) {
      uint32_t length;
      if ((length = base_keywords_trie.match(text, i, len, identifier_char)) >
          0) {
        tokens->push_back({i, i + length, 4});
        i += length;
        continue;
      } else if ((length = operator_keywords_trie.match(text, i, len,
                                                        identifier_char)) > 0) {
        tokens->push_back({i, i + length, 5});
        i += length;
        continue;
      } else if (text[i] >= 'A' && text[i] <= 'Z') {
        uint32_t start = i;
        i += get_next_word(text, i, len);
        tokens->push_back({start, i, 10});
        continue;
      } else {
        uint32_t start = i;
        while (i < len && identifier_char(text[i]))
          i++;
        if (i < len && text[i] == ':') {
          i++;
          tokens->push_back({start, i, 6});
          continue;
        } else if (i < len && (text[i] == '!' || text[i] == '?')) {
          i++;
        }
        continue;
      }
    } else {
      uint32_t op_len;
      if ((op_len = operator_trie.match(text, i, len,
                                        [](char) { return false; })) > 0) {
        tokens->push_back({i, i + op_len, 3});
        i += op_len;
        continue;
      }
    }
    i += utf8_codepoint_width(text[i]);
  }
  return state;
}

bool ruby_state_match(std::shared_ptr<void> state_1,
                      std::shared_ptr<void> state_2) {
  if (!state_1 || !state_2)
    return false;
  return *std::static_pointer_cast<RubyState>(state_1) ==
         *std::static_pointer_cast<RubyState>(state_2);
}

//    function calls matched with alphanumeric names followed immediately by !
//      or ? or `(` immediately or siwth space or are followed by a non-keyword
//      or non-operator (some operators like - for negating and ! for not or {
//      for block might be allowed?)
//    a word following :: or . is matched as a property
//    and any random word is matched as a variable name
//      or as a class/module name if it starts with a capital letter
//
//    regex are matched as text within / and / as long as
//    the first / is not
//      following a literal (int/float/string) or variable or brace close
//      and is following a keyword or operator liek return /regex/  or  x =
//      /regex/ . so maybe add feild expecting_expr to state that is true right
//      after keyword or some operators like = , =~ , `,` etc?
//
//    (left to implement) -
//
//        words - breaks up into these submatches
//          - Constants that start with a capital letter
//          - a word following :: or . is matched as a property
//          - function call if ending with ! or ? or ( or are followed by a
//            non-keyword or non-operator . ill figure it out
//
//        regex (and distinguish between / for division and / for regex) and
//        %r{} ones too
//
//        Matching brace colors by brace depth
//
