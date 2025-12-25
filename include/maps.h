#ifndef MAPS_H
#define MAPS_H

#include "./lsp.h"
#include "./pch.h"
#include "./ts_def.h"
#include <unordered_map>

static const std::unordered_map<std::string, Language> kLanguages = {
    {"bash", {"bash", LANG(bash)}},
    {"c", {"c", LANG(c), 1}},
    {"cpp", {"cpp", LANG(cpp), 1}},
    {"h", {"h", LANG(cpp), 1}},
    {"css", {"css", LANG(css)}},
    {"fish", {"fish", LANG(fish)}},
    {"go", {"go", LANG(go)}},
    {"haskell", {"haskell", LANG(haskell)}},
    {"html", {"html", LANG(html)}},
    {"javascript", {"javascript", LANG(javascript)}},
    {"json", {"json", LANG(json)}},
    {"lua", {"lua", LANG(lua)}},
    {"make", {"make", LANG(make)}},
    {"python", {"python", LANG(python)}},
    {"ruby", {"ruby", LANG(ruby)}},
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
