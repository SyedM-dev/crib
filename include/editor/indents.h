#ifndef EDITOR_INDENTS_H
#define EDITOR_INDENTS_H

#include "utils/utils.h"

static const std::unordered_map<std::string, uint8_t> kLangtoIndent = {
    {"make", 1}, {"yaml", 2}};

// this indents the newline one level when the line (on the curser before \n  is
// inserted) matches this at its end (stripped of whitespace)
static const std::unordered_map<std::string, const std::vector<std::string>>
    kLangtoBlockStartsEnd = {
        {"bash", {"then", "do", "in", "{", "(", "\\", "&&", "||", "|"}},
        {"c", {"{", "(", ":"}},
        {"cpp", {"{", "(", ":"}},
        {"h", {"{", "(", ":"}},
        {"css", {"{", "("}},
        {"fish", {"{", "(", "^", "&&", "||", "|"}},
        {"go", {"{", "(", ":"}},
        {"gomod", {"{", "(", ":"}},
        {"haskell", {"do", "where", "then", "else", "of"}},
        {"javascript", {"{", "(", "[", ":"}},
        {"typescript", {"{", "(", "[", ":"}},
        {"json", {"{", "[", ":"}},
        {"jsonc", {"{", "[", ":"}},
        {"ruby", {"then", "else", "begin", "{", "(", "["}},
        {"lua", {"then", "do", "else", "repeat", "{", "(", "["}},
        {"python", {":", "(", "[", "{"}},
        {"rust", {"{", "(", "[", ":"}},
        {"php", {"{", "(", "[", ":"}},
        {"nginx", {"{"}},
        {"yaml", {":"}},
        {"sql", {"("}},
        {"make", {":"}},
        {"gdscript", {":", "(", "[", "{"}},
};

// this indents the newline one level when the line (on the curser before \n is
// inserted) matches this at its start (stripped of whitespace)
static const std::unordered_map<std::string, const std::vector<std::string>>
    kLangtoBlockStartsStart = {
        {"c", {"if", "for", "while"}},
        {"cpp", {"if", "for", "while"}},
        {"h", {"if", "for", "while"}},
        {"fish", {"if", "else", "for", "while", "switch", "case", "function"}},
        {"javascript", {"if", "for", "while"}},
        {"typescript", {"if", "for", "while"}},
        {"ruby",
         {"if", "do", "when", "rescue", "class", "module", "def", "unless",
          "until", "elsif", "ensure"}},
        {"lua", {"function"}},
        {"nginx", {"{"}},
};

// This dedents the line (under the cursor before \n is inserted) when the line
// matches this fully (stripped of whitespace)
static const std::unordered_map<std::string, const std::vector<std::string>>
    kLangtoBlockEndsFull = {
        {"bash", {"fi", "done", "esac", "}", ")"}},
        {"c", {"}", ")"}},
        {"cpp", {"}", ")"}},
        {"h", {"}", ")"}},
        {"css", {"}", ")"}},
        {"fish", {"end"}},
        {"go", {"}", ")"}},
        {"gomod", {"}", ")"}},
        {"javascript", {"}", ")", "]"}},
        {"typescript", {"}", ")", "]"}},
        {"json", {"}", "]"}},
        {"jsonc", {"}", "]"}},
        {"ruby", {"end", "else", "}", ")", "]"}},
        {"lua", {"else", "}", ")", "]"}},
        {"python", {"}", ")", "]", "else:"}},
        {"rust", {"}", ")", "]"}},
        {"php",
         {"}", ")", "]", "else:", "endif;", "endfor;", "endwhile;",
          "endswitch;", "endcase;", "endfunction;"}},
        {"nginx", {"}"}},
        {"sql", {")"}},
        {"gdscript", {"}", ")", "]"}},
};

// This dedents the line (under the cursor before \n is inserted) when the line
// matches this at its start (stripped of whitespace)
static const std::unordered_map<std::string, const std::vector<std::string>>
    kLangtoBlockEndsStart = {
        {"c", {"case", "default:", "} else"}},
        {"cpp", {"case", "default:", "} else"}},
        {"h", {"case", "default:", "} else"}},
        {"fish", {"else if"}},
        {"go", {"case", "default:", "} else"}},
        {"gomod", {"}", ")"}},
        {"javascript", {"case", "default:"}},
        {"typescript", {"case", "default:"}},
        {"json", {"}", "]"}},
        {"python", {"elif"}},
        {"jsonc", {"}", "]"}},
        {"ruby", {"when", "elsif", "rescue", "ensure"}},
        {"lua", {"end", "elseif", "until"}},
        {"rust", {"case", "default:", "} else"}},
        {"php", {"case", "default:", "} else"}},
};

struct IndentationEngine {
  // tabs = 1, spaces = 2+
  uint8_t indent = 0;
  struct Editor *editor = nullptr;

  void compute_indent(Editor *n_editor);
  void insert_new_line(Coord cursor);
  void insert_tab(Coord cursor);
  uint32_t set_indent(uint32_t row, int64_t indent_level);
  uint32_t indent_line(uint32_t row);
  uint32_t dedent_line(uint32_t row);
  void indent_block(uint32_t start_row, uint32_t end_row, int delta);
  void indent_block(uint32_t start, uint32_t end);
  void dedent_block(uint32_t start, uint32_t end);
  // fixes a autocomplete block's indentation
  char *block_to_asis(Coord cursor, std::string source, uint32_t *out_len);

private:
  const std::vector<std::string> *start_end = nullptr;
  const std::vector<std::string> *start_start = nullptr;
  const std::vector<std::string> *end_full = nullptr;
  const std::vector<std::string> *end_start = nullptr;

  // TODO: Ignore comments/strings too
  // returns the indent level of the line itself or of the previous non-empty
  uint32_t indent_expected(uint32_t row);
  // returns the indent level of the line
  uint32_t indent_real(char *line, uint32_t len);
};

inline static bool ends_with(const std::string &str,
                             const std::string &suffix) {
  const size_t str_len = str.size();
  const size_t suf_len = suffix.size();
  if (suf_len > str_len)
    return false;
  for (size_t i = 0; i < suf_len; i++)
    if (str[str_len - suf_len + i] != suffix[i])
      return false;
  return true;
}

inline static bool starts_with(const std::string &str,
                               const std::string &prefix) {
  const size_t str_len = str.size();
  const size_t pre_len = prefix.size();
  if (pre_len > str_len)
    return false;
  for (size_t i = 0; i < pre_len; i++)
    if (str[i] != prefix[i])
      return false;
  return true;
}

#endif
