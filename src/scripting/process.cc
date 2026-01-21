#include "scripting/decl.h"
#include "syntax/decl.h"
#include "utils/utils.h"

struct ThemeEntry {
  std::string key;
  uint32_t fg = 0xFFFFFF;
  uint32_t bg = 0x000000;
  bool italic = false;
  bool bold = false;
  bool underline = false;
  bool strikethrough = false;
};

VALUE C_module = Qnil;

void ruby_start(const char *main_file) {
  USING(Language);
  ruby_init();
  ruby_init_loadpath();
  int state = 0;
  rb_load_protect(rb_str_new_cstr(main_file), 0, &state);
  if (state) {
    VALUE err = rb_errinfo();
    rb_set_errinfo(Qnil);
    fprintf(stderr, "Failed to load Ruby file\n");
  }
  C_module = rb_const_get(rb_cObject, rb_intern("C"));
  if (C_module == Qnil)
    return;
  VALUE block = rb_funcall(C_module, rb_intern("b_startup"), 0);
  if (block != Qnil)
    rb_funcall(block, rb_intern("call"), 0);
}

void ruby_shutdown() {
  if (C_module == Qnil)
    return;
  VALUE block = rb_funcall(C_module, rb_intern("b_shutdown"), 0);
  if (block != Qnil)
    rb_funcall(block, rb_intern("call"), 0);
  ruby_finalize();
}

static std::vector<ThemeEntry> read_theme() {
  std::vector<ThemeEntry> result;
  if (C_module == Qnil)
    return result;
  VALUE theme_hash = rb_funcall(C_module, rb_intern("theme"), 0);
  if (NIL_P(theme_hash))
    return result;
  VALUE keys = rb_funcall(theme_hash, rb_intern("keys"), 0);
  for (long i = 0; i < RARRAY_LEN(keys); ++i) {
    VALUE key_sym = rb_ary_entry(keys, i);
    std::string key = rb_id2name(SYM2ID(key_sym));
    VALUE val_hash = rb_hash_aref(theme_hash, key_sym);
    if (NIL_P(val_hash))
      continue;
    ThemeEntry entry;
    entry.key = key;
    VALUE fg = rb_hash_aref(val_hash, ID2SYM(rb_intern("fg")));
    VALUE bg = rb_hash_aref(val_hash, ID2SYM(rb_intern("bg")));
    VALUE italic = rb_hash_aref(val_hash, ID2SYM(rb_intern("italic")));
    VALUE bold = rb_hash_aref(val_hash, ID2SYM(rb_intern("bold")));
    VALUE underline = rb_hash_aref(val_hash, ID2SYM(rb_intern("underline")));
    VALUE strikethrough =
        rb_hash_aref(val_hash, ID2SYM(rb_intern("strikethrough")));
    if (!NIL_P(fg))
      entry.fg = NUM2UINT(fg);
    if (!NIL_P(bg))
      entry.bg = NUM2UINT(bg);
    if (!NIL_P(italic))
      entry.italic = RTEST(italic);
    if (!NIL_P(bold))
      entry.bold = RTEST(bold);
    if (!NIL_P(underline))
      entry.underline = RTEST(underline);
    if (!NIL_P(strikethrough))
      entry.strikethrough = RTEST(strikethrough);
    result.push_back(entry);
  }
  return result;
}

void load_theme() {
  std::vector<ThemeEntry> entries = read_theme();
  Highlight default_hl = {0xFFFFFF, 0, 0};
  for (auto &entry : entries) {
    if (entry.key == "default") {
      default_hl.fg = entry.fg;
      default_hl.bg = entry.bg;
      if (entry.italic)
        default_hl.flags |= CF_ITALIC;
      if (entry.bold)
        default_hl.flags |= CF_BOLD;
      if (entry.underline)
        default_hl.flags |= CF_UNDERLINE;
      if (entry.strikethrough)
        default_hl.flags |= CF_STRIKETHROUGH;
      break;
    }
  }
  for (auto &hl : highlights)
    hl = default_hl;
  for (auto &entry : entries) {
    if (entry.key == "default")
      continue;
    std::string key = "k_" + entry.key;
    for (char &c : key)
      c = std::toupper(static_cast<unsigned char>(c));
    auto it = kind_map.find(key);
    if (it == kind_map.end())
      continue;
    Highlight hl = {0xFFFFFF, 0, 0};
    hl.fg = entry.fg;
    hl.bg = entry.bg;
    if (entry.italic)
      hl.flags |= CF_ITALIC;
    if (entry.bold)
      hl.flags |= CF_BOLD;
    if (entry.underline)
      hl.flags |= CF_UNDERLINE;
    if (entry.strikethrough)
      hl.flags |= CF_STRIKETHROUGH;
    highlights[static_cast<uint8_t>(it->second)] = hl;
  }
}

std::string read_line_endings() {
  if (C_module == Qnil)
    return "";
  VALUE le = rb_funcall(C_module, rb_intern("line_endings"), 0);
  if (SYMBOL_P(le))
    return rb_id2name(SYM2ID(le));
  return "";
}

std::string read_utf_mode() {
  if (C_module == Qnil)
    return "";
  VALUE utf = rb_funcall(C_module, rb_intern("utf_mode"), 0);
  if (SYMBOL_P(utf))
    return rb_id2name(SYM2ID(utf));
  return "";
}
