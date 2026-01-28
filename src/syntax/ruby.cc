#include "syntax/decl.h"
#include "syntax/langs.h"

// TODO: in regex better highlighting of regex structures

const static std::vector<std::string> types = {
    "BasicObject", "Object",     "NilClass",
    "TrueClass",   "FalseClass", "Integer",
    "Fixnum",      "Bignum",     "Float",
    "Rational",    "Complex",    "Numeric",
    "String",      "Symbol",     "Array",
    "Hash",        "Range",      "Regexp",
    "Struct",      "Enumarator", "Enumerable",
    "Time",        "Date",       "IO",
    "File",        "Dir",        "Thread",
    "Proc",        "Method",     "Module",
    "Class",       "Mutex",      "ConditionVariable",
    "MatchData",   "Encoding",   "Fiber",
};

const static std::vector<std::string> builtins = {
    "ARGF",
    "ARGV",
    "ENV",
    "STDIN",
    "STDOUT",
    "STDERR",
    "DATA",
    "TOPLEVEL_BINDING",
    "RUBY_PLATFORM",
    "RUBY_VERSION",
    "RUBY_RELEASE_DATE",
    "RUBY_PATCHLEVEL",
    "RUBY_ENGINE",
    "__LINE__",
    "__FILE__",
    "__ENCODING__",
    "__dir__",
    "__callee__",
    "__method__",
    "__id__",
    "__send__",
};

const static std::vector<std::string> methods = {
    "abort",
    "at_exit",
    "binding",
    "block_given?",
    "caller",
    "catch",
    "chomp",
    "chomp!",
    "chop",
    "chop!",
    "eval",
    "exec",
    "exit",
    "exit!",
    "fail",
    "fork",
    "format",
    "gets",
    "global_variables",
    "gsub",
    "gsub!",
    "iterator?",
    "lambda",
    "load",
    "loop",
    "open",
    "print",
    "printf",
    "proc",
    "putc",
    "puts",
    "raise",
    "rand",
    "readline",
    "readlines",
    "require",
    "require_relative",
    "select",
    "sleep",
    "spawn",
    "split",
    "sprintf",
    "srand",
    "sub",
    "sub!",
    "syscall",
    "system",
    "test",
    "throw",
    "trace_var",
    "trap",
    "untrace_var",
    "attr",
    "attr_reader",
    "attr_writer",
    "attr_accessor",
    "class_variable_get",
    "class_variable_set",
    "define_method",
    "instance_variable_get",
    "instance_variable_set",
    "private",
    "protected",
    "public",
    "public_class_method",
    "module_function",
    "remove_method",
    "undef_method",
    "method",
    "methods",
    "singleton_methods",
    "private_methods",
    "protected_methods",
    "public_methods",
    "send",
    "extend",
    "include",
    "prepend",
    "clone",
    "dup",
    "freeze",
    "taint",
    "untaint",
    "trust",
    "untrust",
    "untaint?",
    "trust?",
    "each",
    "each_with_index",
    "each_with_object",
    "map",
    "collect",
    "select",
    "reject",
    "reduce",
    "inject",
    "find",
    "detect",
    "all?",
    "any?",
    "none?",
    "one?",
    "count",
    "cycle",
    "drop",
    "drop_while",
    "take",
    "take_while",
    "chunk",
    "chunk_while",
    "group_by",
    "partition",
    "slice_before",
    "slice_after",
    "nil?",
    "is_a?",
    "kind_of?",
    "instance_of?",
    "respond_to?",
    "equal?",
    "object_id",
    "class",
    "singleton_class",
    "clone",
    "freeze",
    "tap",
    "then",
};

const static std::vector<std::string> errors = {
    "Exception", "SignalException", "Interrupt", "StopIteration",
    "Errno",     "SystemExit",      "fatal",
};

const static std::vector<std::string> base_keywords = {
    "class", "module", "begin", "end", "else", "rescue", "ensure", "do", "when",
};

const static std::vector<std::string> expecting_keywords = {
    "if", "elsif", "case", "for", "while", "until", "unless",
};

const static std::vector<std::string> operator_keywords = {
    "alias", "BEGIN",  "break", "catch", "defined?", "in",  "next",
    "redo",  "rescue", "retry", "super", "self",     "nil", "undef",
};

const static std::vector<std::string> expecting_operators = {
    "and", "return", "not", "yield", "or",
};

const static std::vector<std::string> operators = {
    "+",   "-",   "*",  "/",   "%",   "**", "==", "!=",  "===", "<=>", ">",
    ">=",  "<",   "<=", "&&",  "||",  "!",  "&",  "|",   "^",   "~",   "<<",
    ">>",  "=",   "+=", "-=",  "*=",  "/=", "%=", "**=", "&=",  "|=",  "^=",
    "<<=", ">>=", "..", "...", "===", "=",  "=>", "&",   "`",   "->",  "=~",
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
  int brace_level = 0;

  enum : uint8_t { NONE, STRING, REGEXP, COMMENT, HEREDOC, END };
  uint8_t in_state = RubyFullState::NONE;

  bool expecting_expr = false;

  struct Lit {
    char delim_start = '\0';
    char delim_end = '\0';
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
           expecting_expr == other.expecting_expr;
  }
};

struct RubyState {
  using full_state_type = RubyFullState;

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

inline static bool identifier_start_char(char c) {
  return !isascii(c) || isalpha(c) || c == '_';
}

inline static bool identifier_char(char c) {
  return !isascii(c) || isalnum(c) || c == '_';
}

inline static uint32_t get_next_word(const char *text, uint32_t i,
                                     uint32_t len) {
  if (i >= len || !identifier_start_char(text[i]))
    return 0;
  uint32_t width = 1;
  while (i + width < len && identifier_char(text[i + width]))
    width++;
  if (i + width < len && (text[i + width] == '!' || text[i + width] == '?'))
    width++;
  return width;
}

bool ruby_state_match(std::shared_ptr<void> state_1,
                      std::shared_ptr<void> state_2) {
  if (!state_1 || !state_2)
    return false;
  return *std::static_pointer_cast<RubyState>(state_1) ==
         *std::static_pointer_cast<RubyState>(state_2);
}

std::shared_ptr<void> ruby_parse(std::vector<Token> *tokens,
                                 std::shared_ptr<void> in_state,
                                 const char *text, uint32_t len,
                                 uint32_t line_num) {
  static bool keywords_trie_init = false;
  static Trie<void> base_keywords_trie;
  static Trie<void> expecting_keywords_trie;
  static Trie<void> operator_keywords_trie;
  static Trie<void> expecting_operators_trie;
  static Trie<void> operator_trie;
  static Trie<void> types_trie;
  static Trie<void> builtins_trie;
  static Trie<void> methods_trie;
  static Trie<void> errors_trie;
  if (!keywords_trie_init) {
    base_keywords_trie.build(base_keywords);
    expecting_keywords_trie.build(expecting_keywords);
    operator_keywords_trie.build(operator_keywords);
    expecting_operators_trie.build(expecting_operators);
    operator_trie.build(operators);
    types_trie.build(types);
    builtins_trie.build(builtins);
    methods_trie.build(methods);
    errors_trie.build(errors);
    keywords_trie_init = true;
  }
  tokens->clear();
  auto state = ensure_state(std::static_pointer_cast<RubyState>(in_state));
  uint32_t i = 0;
  while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r' ||
                     text[len - 1] == '\t' || text[len - 1] == ' '))
    len--;
  if (len == 0)
    return state;
  bool heredoc_first = false;
  while (i < len) {
    if (state->full_state->in_state == RubyFullState::END)
      return state;
    if (state->full_state->in_state == RubyFullState::COMMENT) {
      tokens->push_back({i, len, TokenKind::K_COMMENT});
      if (i == 0 && len == 4 && text[i] == '=' && text[i + 1] == 'e' &&
          text[i + 2] == 'n' && text[i + 3] == 'd') {
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
          state->heredocs.pop_front();
          if (state->heredocs.empty())
            state->full_state->in_state = RubyFullState::NONE;
          tokens->push_back({i, len, TokenKind::K_ANNOTATION});
          return state;
        }
      }
      uint32_t start = i;
      if (!state->heredocs.front().allow_interpolation) {
        tokens->push_back({i, len, TokenKind::K_STRING});
        return state;
      } else {
        while (i < len) {
          if (text[i] == '\\') {
            tokens->push_back({start, i, TokenKind::K_STRING});
            start = i;
            i++;
            if (i < len && text[i] == 'x') {
              i++;
              if (i < len && isxdigit(text[i]))
                i++;
              if (i < len && isxdigit(text[i]))
                i++;
            } else if (i < len && text[i] == 'u') {
              i++;
              if (i < len && text[i] == '{') {
                i++;
                while (i < len && text[i] != '}')
                  i++;
                if (i < len)
                  i++;
              } else {
                if (i < len && isxdigit(text[i]))
                  i++;
                if (i < len && isxdigit(text[i]))
                  i++;
                if (i < len && isxdigit(text[i]))
                  i++;
                if (i < len && isxdigit(text[i]))
                  i++;
              }
            } else if (i < len && text[i] >= '0' && text[i] <= '7') {
              i++;
              if (i < len && text[i] >= '0' && text[i] <= '7')
                i++;
              if (i < len && text[i] >= '0' && text[i] <= '7')
                i++;
            } else if (i < len && text[i] == 'c') {
              i++;
              if (i < len && text[i] != '\\')
                i++;
            } else if (i < len && (text[i] == 'M' || text[i] == 'C')) {
              i++;
              if (i < len && text[i] == '-') {
                i++;
                if (i < len && text[i] != '\\')
                  i++;
              }
            } else if (i < len && text[i] == 'N') {
              i++;
              if (i < len && text[i] == '{') {
                i++;
                while (i < len && text[i] != '}')
                  i++;
                if (i < len)
                  i++;
              }
            } else {
              if (i < len)
                i++;
            }
            tokens->push_back({start, i, TokenKind::K_ESCAPE});
            continue;
          }
          if (text[i] == '#' && i + 1 < len && text[i + 1] == '{') {
            tokens->push_back({start, i, TokenKind::K_STRING});
            tokens->push_back({i, i + 2, TokenKind::K_INTERPOLATION});
            i += 2;
            state->interp_stack.push(state->full_state);
            state->full_state = std::make_shared<RubyFullState>();
            state->interp_level = 1;
            break;
          }
          i++;
        }
        if (i == len)
          tokens->push_back({start, len, TokenKind::K_STRING});
        continue;
      }
    }
    if (state->full_state->in_state == RubyFullState::STRING) {
      uint32_t start = i;
      while (i < len) {
        if (text[i] == '\\') {
          tokens->push_back({start, i, TokenKind::K_STRING});
          start = i;
          i++;
          if (i < len && text[i] == 'x') {
            i++;
            if (i < len && isxdigit(text[i]))
              i++;
            if (i < len && isxdigit(text[i]))
              i++;
          } else if (i < len && text[i] == 'u') {
            i++;
            if (i < len && text[i] == '{') {
              i++;
              while (i < len && text[i] != '}')
                i++;
              if (i < len)
                i++;
            } else {
              if (i < len && isxdigit(text[i]))
                i++;
              if (i < len && isxdigit(text[i]))
                i++;
              if (i < len && isxdigit(text[i]))
                i++;
              if (i < len && isxdigit(text[i]))
                i++;
            }
          } else if (i < len && text[i] >= '0' && text[i] <= '7') {
            i++;
            if (i < len && text[i] >= '0' && text[i] <= '7')
              i++;
            if (i < len && text[i] >= '0' && text[i] <= '7')
              i++;
          } else if (i < len && text[i] == 'c') {
            i++;
            if (i < len && text[i] != '\\')
              i++;
          } else if (i < len && (text[i] == 'M' || text[i] == 'C')) {
            i++;
            if (i < len && text[i] == '-') {
              i++;
              if (i < len && text[i] != '\\')
                i++;
            }
          } else if (i < len && text[i] == 'N') {
            i++;
            if (i < len && text[i] == '{') {
              i++;
              while (i < len && text[i] != '}')
                i++;
              if (i < len)
                i++;
            }
          } else {
            if (i < len)
              i++;
          }
          tokens->push_back({start, i, TokenKind::K_ESCAPE});
          continue;
        }
        if (state->full_state->lit.allow_interp && text[i] == '#' &&
            i + 1 < len && text[i + 1] == '{') {
          tokens->push_back({start, i, TokenKind::K_STRING});
          tokens->push_back({i, i + 2, TokenKind::K_INTERPOLATION});
          i += 2;
          state->interp_stack.push(state->full_state);
          state->full_state = std::make_shared<RubyFullState>();
          state->interp_level = 1;
          break;
        }
        if (text[i] == state->full_state->lit.delim_start &&
            state->full_state->lit.delim_start !=
                state->full_state->lit.delim_end) {
          state->full_state->lit.brace_level++;
        }
        if (text[i] == state->full_state->lit.delim_end) {
          if (state->full_state->lit.delim_start ==
              state->full_state->lit.delim_end) {
            i++;
            tokens->push_back({start, i, TokenKind::K_STRING});
            state->full_state->in_state = RubyFullState::NONE;
            state->full_state->expecting_expr = false;
            break;
          } else {
            state->full_state->lit.brace_level--;
            if (state->full_state->lit.brace_level == 0) {
              i++;
              tokens->push_back({start, i, TokenKind::K_STRING});
              state->full_state->in_state = RubyFullState::NONE;
              state->full_state->expecting_expr = false;
              break;
            }
          }
        }
        i++;
      }
      if (i == len)
        tokens->push_back({start, len, TokenKind::K_STRING});
      continue;
    }
    if (state->full_state->in_state == RubyFullState::REGEXP) {
      uint32_t start = i;
      while (i < len) {
        if (text[i] == '\\') {
          tokens->push_back({start, i, TokenKind::K_REGEXP});
          start = i;
          i++;
          if (i < len && text[i] == 'x') {
            i++;
            if (i < len && isxdigit(text[i]))
              i++;
            if (i < len && isxdigit(text[i]))
              i++;
          } else if (i < len && text[i] == 'u') {
            i++;
            if (i < len && text[i] == '{') {
              i++;
              while (i < len && text[i] != '}')
                i++;
              if (i < len)
                i++;
            } else {
              if (i < len && isxdigit(text[i]))
                i++;
              if (i < len && isxdigit(text[i]))
                i++;
              if (i < len && isxdigit(text[i]))
                i++;
              if (i < len && isxdigit(text[i]))
                i++;
            }
          } else if (i < len && text[i] >= '0' && text[i] <= '7') {
            i++;
            if (i < len && text[i] >= '0' && text[i] <= '7')
              i++;
            if (i < len && text[i] >= '0' && text[i] <= '7')
              i++;
          } else if (i < len && text[i] == 'c') {
            i++;
            if (i < len && text[i] != '\\')
              i++;
          } else if (i < len && (text[i] == 'M' || text[i] == 'C')) {
            i++;
            if (i < len && text[i] == '-') {
              i++;
              if (i < len && text[i] != '\\')
                i++;
            }
          } else if (i < len && text[i] == 'N') {
            i++;
            if (i < len && text[i] == '{') {
              i++;
              while (i < len && text[i] != '}')
                i++;
              if (i < len)
                i++;
            }
          } else {
            if (i < len)
              i++;
          }
          tokens->push_back({start, i, TokenKind::K_ESCAPE});
          continue;
        }
        if (text[i] == '#' && i + 1 < len && text[i + 1] == '{') {
          tokens->push_back({start, i, TokenKind::K_REGEXP});
          tokens->push_back({i, i + 2, TokenKind::K_INTERPOLATION});
          i += 2;
          state->interp_stack.push(state->full_state);
          state->full_state = std::make_shared<RubyFullState>();
          state->interp_level = 1;
          break;
        }
        if (text[i] == state->full_state->lit.delim_start &&
            state->full_state->lit.delim_start !=
                state->full_state->lit.delim_end) {
          state->full_state->lit.brace_level++;
        }
        if (text[i] == state->full_state->lit.delim_end) {
          if (state->full_state->lit.delim_start ==
              state->full_state->lit.delim_end) {
            i += 1;
            tokens->push_back({start, i, TokenKind::K_REGEXP});
            state->full_state->in_state = RubyFullState::NONE;
            state->full_state->expecting_expr = false;
            break;
          } else {
            state->full_state->lit.brace_level--;
            if (state->full_state->lit.brace_level == 0) {
              i += 1;
              tokens->push_back({start, i, TokenKind::K_REGEXP});
              state->full_state->in_state = RubyFullState::NONE;
              state->full_state->expecting_expr = false;
              break;
            }
          }
        }
        i++;
      }
      if (i == len)
        tokens->push_back({start, len, TokenKind::K_REGEXP});
      continue;
    }
    if (i == 0 && len == 6) {
      if (text[i] == '=' && text[i + 1] == 'b' && text[i + 2] == 'e' &&
          text[i + 3] == 'g' && text[i + 4] == 'i' && text[i + 5] == 'n') {
        state->full_state->in_state = RubyFullState::COMMENT;
        state->full_state->expecting_expr = false;
        tokens->push_back({0, len, TokenKind::K_COMMENT});
        return state;
      }
    }
    if (i == 0 && len == 7) {
      if (text[i] == '_' && text[i + 1] == '_' && text[i + 2] == 'E' &&
          text[i + 3] == 'N' && text[i + 4] == 'D' && text[i + 5] == '_' &&
          text[i + 6] == '_') {
        tokens->clear();
        state->full_state->in_state = RubyFullState::END;
        state->full_state->expecting_expr = false;
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
      tokens->push_back({i, j, TokenKind::K_OPERATOR});
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
        if (j < len && identifier_start_char(text[j])) {
          delim += text[j++];
          while (j < len && identifier_char(text[j]))
            delim += text[j++];
        }
      }
      state->full_state->expecting_expr = false;
      if (!delim.empty()) {
        tokens->push_back({s, j, TokenKind::K_ANNOTATION});
        state->heredocs.push_back({delim, interpolation, indented});
        state->full_state->in_state = RubyFullState::HEREDOC;
        heredoc_first = true;
      }
      i = j;
      continue;
    }
    if (text[i] == '/' && state->full_state->expecting_expr) {
      tokens->push_back({i, i + 1, TokenKind::K_REGEXP});
      state->full_state->in_state = RubyFullState::REGEXP;
      state->full_state->expecting_expr = false;
      state->full_state->lit.delim_start = '/';
      state->full_state->lit.delim_end = '/';
      state->full_state->lit.allow_interp = true;
      i++;
      continue;
    } else if (text[i] == '#') {
      if (line_num == 0 && i == 0 && len > 4 && text[i + 1] == '!') {
        state->full_state->expecting_expr = false;
        tokens->push_back({0, len, TokenKind::K_SHEBANG});
        return state;
      }
      tokens->push_back({i, len, TokenKind::K_COMMENT});
      state->full_state->expecting_expr = false;
      return state;
    } else if (text[i] == '.') {
      uint32_t start = i;
      i++;
      if (i < len && text[i] == '.') {
        i++;
        if (i < len && text[i] == '.') {
          i++;
        }
      }
      tokens->push_back({start, i, TokenKind::K_OPERATOR});
      state->full_state->expecting_expr = false;
      continue;
    } else if (text[i] == ':') {
      state->full_state->expecting_expr = false;
      uint32_t start = i;
      i++;
      if (i >= len) {
        tokens->push_back({start, i, TokenKind::K_OPERATOR});
        state->full_state->expecting_expr = true;
        continue;
      }
      if (text[i] == ':') {
        i++;
        continue;
      }
      if (text[i] == '\'' || text[i] == '"') {
        tokens->push_back({start, i, TokenKind::K_LABEL});
        state->full_state->expecting_expr = true;
        continue;
      }
      if (text[i] == '$' || text[i] == '@') {
        uint32_t var_start = i;
        i++;
        if (i < len && text[var_start] == '@' && text[var_start + 1] == '@')
          i++;
        while (i < len && identifier_char(text[i]))
          i++;
        tokens->push_back({start, i, TokenKind::K_LABEL});
        continue;
      }
      uint32_t op_len = operator_trie.match(text, i, len, identifier_char);
      if (op_len > 0) {
        tokens->push_back({start, i + op_len, TokenKind::K_LABEL});
        i += op_len;
        continue;
      }
      if (identifier_start_char(text[i])) {
        uint32_t word_len = get_next_word(text, i, len);
        tokens->push_back({start, i + word_len, TokenKind::K_LABEL});
        i += word_len;
        continue;
      }
      tokens->push_back({start, i, TokenKind::K_OPERATOR});
      continue;
    } else if (text[i] == '@') {
      state->full_state->expecting_expr = false;
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
      tokens->push_back({start, i, TokenKind::K_VARIABLEINSTANCE});
      continue;
    } else if (text[i] == '$') {
      state->full_state->expecting_expr = false;
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
      tokens->push_back({start, i, TokenKind::K_VARIABLEGLOBAL});
      continue;
    } else if (text[i] == '?') {
      state->full_state->expecting_expr = false;
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
          tokens->push_back({start, i, TokenKind::K_CHAR});
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
          tokens->push_back({start, i, TokenKind::K_CHAR});
          continue;
        } else if (i < len) {
          i++;
          tokens->push_back({start, i, TokenKind::K_CHAR});
          continue;
        }
      } else if (i < len && text[i] != ' ') {
        i++;
        tokens->push_back({start, i, TokenKind::K_CHAR});
        continue;
      } else {
        state->full_state->expecting_expr = true;
        tokens->push_back({start, i, TokenKind::K_OPERATOR});
        continue;
      }
    } else if (text[i] == '{') {
      state->full_state->expecting_expr = true;
      uint8_t brace_color =
          (uint8_t)TokenKind::K_BRACE1 + (state->full_state->brace_level % 5);
      tokens->push_back({i, i + 1, (TokenKind)brace_color});
      state->interp_level++;
      state->full_state->brace_level++;
      i++;
      continue;
    } else if (text[i] == '}') {
      state->full_state->expecting_expr = false;
      state->interp_level--;
      if (state->interp_level == 0 && !state->interp_stack.empty()) {
        state->full_state = state->interp_stack.top();
        state->interp_stack.pop();
        tokens->push_back({i, i + 1, TokenKind::K_INTERPOLATION});
      } else {
        state->full_state->brace_level--;
        uint8_t brace_color =
            (uint8_t)TokenKind::K_BRACE1 + (state->full_state->brace_level % 5);
        tokens->push_back({i, i + 1, (TokenKind)brace_color});
      }
      i++;
      continue;
    } else if (text[i] == '(') {
      state->full_state->expecting_expr = true;
      uint8_t brace_color =
          (uint8_t)TokenKind::K_BRACE1 + (state->full_state->brace_level % 5);
      tokens->push_back({i, i + 1, (TokenKind)brace_color});
      state->full_state->brace_level++;
      i++;
      continue;
    } else if (text[i] == ')') {
      state->full_state->expecting_expr = false;
      state->full_state->brace_level--;
      uint8_t brace_color =
          (uint8_t)TokenKind::K_BRACE1 + (state->full_state->brace_level % 5);
      tokens->push_back({i, i + 1, (TokenKind)brace_color});
      i++;
      continue;
    } else if (text[i] == '[') {
      state->full_state->expecting_expr = true;
      uint8_t brace_color =
          (uint8_t)TokenKind::K_BRACE1 + (state->full_state->brace_level % 5);
      tokens->push_back({i, i + 1, (TokenKind)brace_color});
      state->full_state->brace_level++;
      i++;
      continue;
    } else if (text[i] == ']') {
      state->full_state->expecting_expr = false;
      state->full_state->brace_level--;
      uint8_t brace_color =
          (uint8_t)TokenKind::K_BRACE1 + (state->full_state->brace_level % 5);
      tokens->push_back({i, i + 1, (TokenKind)brace_color});
      i++;
      continue;
    } else if (text[i] == '\'') {
      state->full_state->expecting_expr = false;
      tokens->push_back({i, i + 1, TokenKind::K_STRING});
      state->full_state->in_state = RubyFullState::STRING;
      state->full_state->lit.delim_start = '\'';
      state->full_state->lit.delim_end = '\'';
      state->full_state->lit.allow_interp = false;
      i++;
      continue;
    } else if (text[i] == '"') {
      state->full_state->expecting_expr = false;
      tokens->push_back({i, i + 1, TokenKind::K_STRING});
      state->full_state->in_state = RubyFullState::STRING;
      state->full_state->lit.delim_start = '"';
      state->full_state->lit.delim_end = '"';
      state->full_state->lit.allow_interp = true;
      i++;
      continue;
    } else if (text[i] == '`') {
      state->full_state->expecting_expr = false;
      tokens->push_back({i, i + 1, TokenKind::K_STRING});
      state->full_state->in_state = RubyFullState::STRING;
      state->full_state->lit.delim_start = '`';
      state->full_state->lit.delim_end = '`';
      state->full_state->lit.allow_interp = true;
      i++;
      continue;
    } else if (text[i] == '%') {
      state->full_state->expecting_expr = false;
      if (i + 1 >= len) {
        i++;
        continue;
      }
      char type = text[i + 1];
      char delim_start = '\0';
      char delim_end = '\0';
      bool allow_interp = true;
      int prefix_len = 1;
      bool is_regexp = false;
      switch (type) {
      case 'r':
        is_regexp = true;
        allow_interp = true;
        prefix_len = 2;
        break;
      case 'Q':
      case 'x':
      case 'I':
      case 'W':
        allow_interp = true;
        prefix_len = 2;
        break;
      case 'w':
      case 'q':
      case 'i':
      case 's':
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
      tokens->push_back(
          {i, i + prefix_len + 1,
           (is_regexp ? TokenKind::K_REGEXP : TokenKind::K_STRING)});
      state->full_state->in_state =
          is_regexp ? RubyFullState::REGEXP : RubyFullState::STRING;
      state->full_state->lit.delim_start = delim_start;
      state->full_state->lit.delim_end = delim_end;
      state->full_state->lit.allow_interp = allow_interp;
      state->full_state->lit.brace_level = 1;
      i += prefix_len + 1;
      continue;
    } else if (isdigit(text[i])) {
      state->full_state->expecting_expr = false;
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
      tokens->push_back({start, i, TokenKind::K_NUMBER});
      continue;
    } else if (identifier_start_char(text[i])) {
      state->full_state->expecting_expr = false;
      uint32_t length;
      if ((length = base_keywords_trie.match(text, i, len, identifier_char))) {
        tokens->push_back({i, i + length, TokenKind::K_KEYWORD});
        i += length;
        continue;
      } else if ((length = expecting_keywords_trie.match(text, i, len,
                                                         identifier_char))) {
        state->full_state->expecting_expr = true;
        tokens->push_back({i, i + length, TokenKind::K_KEYWORD});
        i += length;
        continue;
      } else if ((length = operator_keywords_trie.match(text, i, len,
                                                        identifier_char))) {
        tokens->push_back({i, i + length, TokenKind::K_KEYWORDOPERATOR});
        i += length;
        continue;
      } else if ((length = expecting_operators_trie.match(
                      text, i, len, identifier_char)) > 0) {
        state->full_state->expecting_expr = true;
        tokens->push_back({i, i + length, TokenKind::K_KEYWORDOPERATOR});
        i += length;
        continue;
      } else if ((length = types_trie.match(text, i, len, identifier_char))) {
        tokens->push_back({i, i + length, TokenKind::K_TYPE});
        i += length;
        continue;
      } else if ((length = methods_trie.match(text, i, len, identifier_char))) {
        tokens->push_back({i, i + length, TokenKind::K_FUNCTION});
        i += length;
        continue;
      } else if ((length =
                      builtins_trie.match(text, i, len, identifier_char))) {
        tokens->push_back({i, i + length, TokenKind::K_CONSTANT});
        i += length;
        continue;
      } else if ((length = errors_trie.match(text, i, len, identifier_char))) {
        tokens->push_back({i, i + length, TokenKind::K_ERROR});
        i += length;
        continue;
      } else if (text[i] >= 'A' && text[i] <= 'Z') {
        uint32_t start = i;
        i += get_next_word(text, i, len);
        if (i - start >= 5 && text[i - 5] == 'E' && text[i - 4] == 'r' &&
            text[i - 3] == 'r' && text[i - 2] == 'o' && text[i - 1] == 'r') {
          tokens->push_back({start, i, TokenKind::K_ERROR});
          continue;
        }
        tokens->push_back({start, i, TokenKind::K_CONSTANT});
        continue;
      } else {
        uint32_t start = i;
        if (i + 3 < len && text[i] == 't' && text[i + 1] == 'r' &&
            text[i + 2] == 'u' && text[i + 3] == 'e' &&
            ((i + 4 < len && !identifier_char(text[i + 4])) || i + 4 == len)) {
          i += 4;
          tokens->push_back({start, i, TokenKind::K_TRUE});
          continue;
        }
        if (i + 4 < len && text[i] == 'f' && text[i + 1] == 'a' &&
            text[i + 2] == 'l' && text[i + 3] == 's' && text[i + 4] == 'e' &&
            ((i + 5 < len && !identifier_char(text[i + 5])) || i + 5 == len)) {
          i += 5;
          tokens->push_back({start, i, TokenKind::K_FALSE});
          continue;
        }
        if (i + 3 < len && text[i] == 'd' && text[i + 1] == 'e' &&
            text[i + 2] == 'f') {
          i += 3;
          tokens->push_back({start, i, TokenKind::K_KEYWORD});
          while (i < len && (text[i] == ' ' || text[i] == '\t'))
            i++;
          while (i < len) {
            if (identifier_start_char(text[i])) {
              uint32_t width = get_next_word(text, i, len);
              if (text[i] >= 'A' && text[i] <= 'Z')
                tokens->push_back({i, i + width, TokenKind::K_CONSTANT});
              else if (width == 4 && (text[i] >= 's' && text[i + 1] == 'e' &&
                                      text[i + 2] == 'l' && text[i + 3] == 'f'))
                tokens->push_back({i, i + width, TokenKind::K_KEYWORD});
              i += width;
              if (i < len && text[i] == '.') {
                i++;
                continue;
              }
              tokens->push_back({i - width, i, TokenKind::K_FUNCTION});
              break;
            } else {
              break;
            }
          }
          continue;
        }
        while (i < len && identifier_char(text[i]))
          i++;
        if (i < len && text[i] == ':') {
          i++;
          tokens->push_back({start, i, TokenKind::K_LABEL});
          continue;
        } else if (i < len && (text[i] == '!' || text[i] == '?')) {
          i++;
          tokens->push_back({start, i, TokenKind::K_FUNCTION});
        } else {
          uint32_t tmp = i;
          if (tmp < len && (text[tmp] == '(' || text[tmp] == '{')) {
            tokens->push_back({start, i, TokenKind::K_FUNCTION});
            continue;
          } else if (tmp < len && (text[tmp] == ' ' || text[tmp] == '\t')) {
            tmp++;
          } else {
            continue;
          }
          while (tmp < len && (text[tmp] == ' ' || text[tmp] == '\t'))
            tmp++;
          if (tmp >= len)
            continue;
          if (!isascii(text[tmp])) {
            tokens->push_back({start, i, TokenKind::K_FUNCTION});
            continue;
          } else if (text[tmp] == '-' || text[tmp] == '&' || text[tmp] == '%' ||
                     text[tmp] == ':') {
            if (tmp + 1 >= len ||
                (text[tmp + 1] == ' ' || text[tmp + 1] == '>'))
              continue;
          } else if (text[tmp] == ']' || text[tmp] == '}' || text[tmp] == ')' ||
                     text[tmp] == ',' || text[tmp] == ';' || text[tmp] == '.' ||
                     text[tmp] == '+' || text[tmp] == '*' || text[tmp] == '/' ||
                     text[tmp] == '=' || text[tmp] == '?' || text[tmp] == '|' ||
                     text[tmp] == '^' || text[tmp] == '<' || text[tmp] == '>') {
            continue;
          }
          tokens->push_back({start, i, TokenKind::K_FUNCTION});
        }
        continue;
      }
    } else {
      uint32_t op_len;
      if ((op_len =
               operator_trie.match(text, i, len, [](char) { return false; }))) {
        tokens->push_back({i, i + op_len, TokenKind::K_OPERATOR});
        i += op_len;
        state->full_state->expecting_expr = true;
        continue;
      } else {
        i += utf8_codepoint_width(text[i]);
        continue;
      }
    }
  }
  return state;
}
