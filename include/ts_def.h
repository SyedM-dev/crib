#include "../libs/tree-sitter/lib/include/tree_sitter/api.h"
#include <string>

struct Language {
  std::string name;
  const TSLanguage *(*fn)();
};

extern "C" {
const TSLanguage *tree_sitter_bash(void);
const TSLanguage *tree_sitter_c(void);
const TSLanguage *tree_sitter_cpp(void);
const TSLanguage *tree_sitter_css(void);
const TSLanguage *tree_sitter_fish(void);
const TSLanguage *tree_sitter_go(void);
const TSLanguage *tree_sitter_haskell(void);
const TSLanguage *tree_sitter_html(void);
const TSLanguage *tree_sitter_javascript(void);
const TSLanguage *tree_sitter_json(void);
const TSLanguage *tree_sitter_lua(void);
const TSLanguage *tree_sitter_make(void);
const TSLanguage *tree_sitter_python(void);
const TSLanguage *tree_sitter_ruby(void);
}
