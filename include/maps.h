#ifndef MAPS_H
#define MAPS_H

#include "./lsp.h"
#include "./pch.h"
#include "./ts_def.h"
#include <unordered_map>

static const std::unordered_map<std::string, Language> kLanguages = {
    {"bash", {"bash", tree_sitter_bash}},
    {"c", {"c", tree_sitter_c, 1}},
    {"cpp", {"cpp", tree_sitter_cpp, 1}},
    {"h", {"h", tree_sitter_cpp, 1}},
    {"css", {"css", tree_sitter_css}},
    {"fish", {"fish", tree_sitter_fish}},
    {"go", {"go", tree_sitter_go}},
    {"haskell", {"haskell", tree_sitter_haskell}},
    {"html", {"html", tree_sitter_html}},
    {"javascript", {"javascript", tree_sitter_javascript}},
    {"json", {"json", tree_sitter_json}},
    {"lua", {"lua", tree_sitter_lua}},
    {"make", {"make", tree_sitter_make}},
    {"python", {"python", tree_sitter_python}},
    {"ruby", {"ruby", tree_sitter_ruby}},
};

static const std::unordered_map<uint8_t, LSP> kLsps = {
    {1,
     {"clangd",
      {
          "clangd",
          "--background-index",
          "--clang-tidy",
          "--completion-style=detailed",
          "--header-insertion=iwyu",
          "--log=error",
          nullptr,
      }}},
};

static const std::unordered_map<std::string, std::string> kExtToLang = {
    {"sh", "bash"},       {"bash", "bash"},  {"c", "c"},       {"cpp", "cpp"},
    {"cxx", "cpp"},       {"cc", "cpp"},     {"hpp", "h"},     {"hh", "h"},
    {"hxx", "h"},         {"h", "h"},        {"css", "css"},   {"fish", "fish"},
    {"go", "go"},         {"hs", "haskell"}, {"html", "html"}, {"htm", "html"},
    {"js", "javascript"}, {"json", "json"},  {"lua", "lua"},   {"mk", "make"},
    {"makefile", "make"}, {"py", "python"},  {"rb", "ruby"},
};

static const std::unordered_map<std::string, std::string> kMimeToLang = {
    {"text/x-c", "c"},
    {"text/x-c++", "cpp"},
    {"text/x-shellscript", "bash"},
    {"application/json", "json"},
    {"text/javascript", "javascript"},
    {"text/html", "html"},
    {"text/css", "css"},
    {"text/x-python", "python"},
    {"text/x-ruby", "ruby"},
    {"text/x-go", "go"},
    {"text/x-haskell", "haskell"},
    {"text/x-lua", "lua"},
};

#endif
