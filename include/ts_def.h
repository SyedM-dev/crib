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

TS_DEF(bash);
TS_DEF(c);
TS_DEF(cpp);
TS_DEF(css);
TS_DEF(fish);
TS_DEF(go);
TS_DEF(haskell);
TS_DEF(html);
TS_DEF(javascript);
TS_DEF(json);
TS_DEF(lua);
TS_DEF(make);
TS_DEF(python);
TS_DEF(ruby);
TS_DEF(rust);
TS_DEF(diff);
TS_DEF(embedded_template);
TS_DEF(gdscript);
TS_DEF(gitattributes);
TS_DEF(gitignore);
TS_DEF(gomod);
TS_DEF(ini);
TS_DEF(markdown);
TS_DEF(nginx);
TS_DEF(php);
TS_DEF(query);
TS_DEF(regex);
TS_DEF(sql);
TS_DEF(toml);
TS_DEF(yaml);
TS_DEF(cabal);

#endif
