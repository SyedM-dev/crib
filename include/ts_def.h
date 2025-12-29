#ifndef TS_DEF_H
#define TS_DEF_H

#include "./pch.h"

#define LANG(name) tree_sitter_##name
#define TS_DEF(name) extern "C" const TSLanguage *LANG(name)()

struct Language {
  std::string name;
  const TSLanguage *(*fn)();
  uint8_t lsp_id = 0;
};

struct Highlight {
  uint32_t fg;
  uint32_t bg;
  uint32_t flags;
  uint8_t priority;
};

struct TSSetBase {
  std::string lang;
  TSParser *parser;
  std::string query_file;
  TSQuery *query;
  TSTree *tree;
  std::map<uint16_t, Highlight> query_map;
  std::map<uint16_t, Language> injection_map;
  const TSLanguage *language;
};

struct TSSet : TSSetBase {
  std::vector<TSRange> ranges;
};

struct TSSetMain : TSSetBase {
  std::unordered_map<std::string, TSSet> injections;
};

TS_DEF(ruby);
TS_DEF(bash);
TS_DEF(cpp);
TS_DEF(css);
TS_DEF(fish);
TS_DEF(go);
TS_DEF(haskell);
TS_DEF(html);
TS_DEF(javascript);
TS_DEF(tsx);
TS_DEF(man);
TS_DEF(json);
TS_DEF(lua);
TS_DEF(regex);
TS_DEF(query);
TS_DEF(markdown);
TS_DEF(markdown_inline);
TS_DEF(embedded_template);
TS_DEF(php);
TS_DEF(python);
TS_DEF(rust);
TS_DEF(sql);
TS_DEF(gitattributes);
TS_DEF(gitignore);
TS_DEF(gomod);
TS_DEF(nginx);
TS_DEF(toml);
TS_DEF(yaml);
TS_DEF(ini);
TS_DEF(diff);
TS_DEF(make);
TS_DEF(gdscript);

#endif
