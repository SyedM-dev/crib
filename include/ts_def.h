#include "../libs/tree-sitter/lib/include/tree_sitter/api.h"
#include <string>

struct Language {
  std::string name;
  const TSLanguage *(*fn)();
};

extern "C" {
const TSLanguage *tree_sitter_bash();
const TSLanguage *tree_sitter_c();
const TSLanguage *tree_sitter_cpp();
const TSLanguage *tree_sitter_css();
const TSLanguage *tree_sitter_fish();
const TSLanguage *tree_sitter_go();
const TSLanguage *tree_sitter_haskell();
const TSLanguage *tree_sitter_html();
const TSLanguage *tree_sitter_javascript();
const TSLanguage *tree_sitter_json();
const TSLanguage *tree_sitter_lua();
const TSLanguage *tree_sitter_make();
const TSLanguage *tree_sitter_python();
const TSLanguage *tree_sitter_ruby();
}
