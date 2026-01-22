#include "ruby/internal/gc.h"
#include "ruby/internal/value.h"
#include "scripting/decl.h"
#include "utils/utils.h"

std::unordered_map<std::string, std::pair<VALUE, VALUE>> custom_highlighters;

struct R_ThemeEntry {
  std::string key;
  uint32_t fg = 0xFFFFFF;
  uint32_t bg = 0x000000;
  bool italic = false;
  bool bold = false;
  bool underline = false;
  bool strikethrough = false;
};

struct R_Language {
  std::string name;
  uint32_t color = 0xFFFFFF;
  std::string symbol;
  std::vector<std::string> extensions;
  std::vector<std::string> filenames;
  std::vector<std::string> mimetypes;
  std::string lsp_command; // link to LSP by name
};

VALUE C_module = Qnil;
std::mutex ruby_mutex;

void ruby_start(const char *main_file) {
  std::lock_guard lock(ruby_mutex);
  ruby_init_loadpath();
  int state = 0;
  rb_load_protect(rb_str_new_cstr(main_file), 0, &state);
  if (state) {
    rb_errinfo();
    rb_set_errinfo(Qnil);
    fprintf(stderr, "%d: Failed to load Ruby file\n", state);
    return;
  }
  C_module = rb_const_get(rb_cObject, rb_intern("C"));
  if (C_module == Qnil)
    return;
  VALUE block = rb_funcall(C_module, rb_intern("b_startup"), 0);
  if (block != Qnil)
    rb_funcall(block, rb_intern("call"), 0);
}

void ruby_shutdown() {
  std::lock_guard lock(ruby_mutex);
  if (C_module == Qnil)
    return;
  VALUE block = rb_funcall(C_module, rb_intern("b_shutdown"), 0);
  if (block != Qnil)
    rb_funcall(block, rb_intern("call"), 0);
}

inline std::vector<std::string> ruby_array_to_vector(VALUE rb_array) {
  std::vector<std::string> result;
  if (NIL_P(rb_array) || !RB_TYPE_P(rb_array, T_ARRAY))
    return result;
  for (long i = 0; i < RARRAY_LEN(rb_array); ++i) {
    VALUE item = rb_ary_entry(rb_array, i);
    if (RB_TYPE_P(item, T_STRING))
      result.push_back(StringValueCStr(item));
  }
  return result;
}

void ruby_log(std::string msg) {
  std::lock_guard lock(ruby_mutex);
  VALUE str = rb_str_new(msg.c_str(), msg.size());
  rb_funcall(C_module, rb_intern("queue_log"), 1, str);
}

void load_custom_highlighters() {
  std::lock_guard lock(ruby_mutex);
  if (C_module == Qnil)
    return;
  VALUE hashmap = rb_funcall(C_module, rb_intern("highlighters"), 0);
  if (NIL_P(hashmap))
    return;
  VALUE keys = rb_funcall(hashmap, rb_intern("keys"), 0);
  for (long i = 0; i < RARRAY_LEN(keys); ++i) {
    VALUE key_sym = rb_ary_entry(keys, i);
    std::string key = rb_id2name(SYM2ID(key_sym));
    VALUE val_hash = rb_hash_aref(hashmap, key_sym);
    if (NIL_P(val_hash))
      continue;
    VALUE parse_block = rb_hash_aref(val_hash, ID2SYM(rb_intern("parser")));
    VALUE match_block = rb_hash_aref(val_hash, ID2SYM(rb_intern("matcher")));
    rb_gc_register_address(&match_block);
    rb_gc_register_address(&parse_block);
    custom_highlighters[key] = {parse_block, match_block};
  }
}

bool custom_compare(VALUE match_block, VALUE state1, VALUE state2) {
  std::lock_guard lock(ruby_mutex);
  return RTEST(rb_funcall(match_block, rb_intern("call"), 2, state1, state2));
}

VALUE parse_custom(std::vector<Token> *tokens, VALUE parser_block,
                   const char *line, uint32_t len, VALUE state) {
  std::lock_guard lock(ruby_mutex);
  tokens->clear();
  if (NIL_P(parser_block))
    return {};
  VALUE ruby_line = rb_str_new(line, len);
  VALUE tokens_and_state_hash =
      rb_funcall(parser_block, rb_intern("call"), 2, ruby_line, state);
  VALUE tokens_rb =
      rb_hash_aref(tokens_and_state_hash, ID2SYM(rb_intern("tokens")));
  for (long i = 0; i < RARRAY_LEN(tokens_rb); ++i) {
    VALUE token = rb_ary_entry(tokens_rb, i);
    Token tok;
    tok.type =
        (TokenKind)NUM2INT(rb_hash_aref(token, ID2SYM(rb_intern("type"))));
    tok.start = NUM2UINT(rb_hash_aref(token, ID2SYM(rb_intern("start"))));
    tok.end = NUM2UINT(rb_hash_aref(token, ID2SYM(rb_intern("end"))));
    if (tok.type < TokenKind::Count && tok.end > tok.start && tok.end <= len)
      tokens->push_back(tok);
  }
  return rb_hash_aref(tokens_and_state_hash, ID2SYM(rb_intern("state")));
}

static std::vector<R_ThemeEntry> read_theme() {
  std::lock_guard lock(ruby_mutex);
  std::vector<R_ThemeEntry> result;
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
    R_ThemeEntry entry;
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
  std::vector<R_ThemeEntry> entries = read_theme();
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

std::vector<LSP> read_lsps() {
  std::vector<LSP> result;
  if (C_module == Qnil)
    return result;
  VALUE lsp_hash = rb_funcall(C_module, rb_intern("lsp_config"), 0);
  if (NIL_P(lsp_hash))
    return result;
  VALUE keys = rb_funcall(lsp_hash, rb_intern("keys"), 0);
  for (long i = 0; i < RARRAY_LEN(keys); ++i) {
    VALUE key = rb_ary_entry(keys, i);
    std::string cmd = StringValueCStr(key);
    VALUE args_array = rb_hash_aref(lsp_hash, key);
    std::vector<std::string> args = ruby_array_to_vector(args_array);
    result.push_back({cmd, args});
  }
  return result;
}

std::vector<R_Language> read_languages() {
  std::vector<R_Language> result;
  if (C_module == Qnil)
    return result;
  VALUE lang_hash = rb_funcall(C_module, rb_intern("languages"), 0);
  if (NIL_P(lang_hash))
    return result;
  VALUE keys = rb_funcall(lang_hash, rb_intern("keys"), 0);
  for (long i = 0; i < RARRAY_LEN(keys); ++i) {
    VALUE key = rb_ary_entry(keys, i);
    VALUE val_hash = rb_hash_aref(lang_hash, key);
    if (NIL_P(val_hash))
      continue;
    R_Language lang;
    lang.name = rb_id2name(SYM2ID(key));
    VALUE fg = rb_hash_aref(val_hash, ID2SYM(rb_intern("color")));
    VALUE symbol = rb_hash_aref(val_hash, ID2SYM(rb_intern("symbol")));
    VALUE extensions = rb_hash_aref(val_hash, ID2SYM(rb_intern("extensions")));
    VALUE filenames = rb_hash_aref(val_hash, ID2SYM(rb_intern("filenames")));
    VALUE mimetypes = rb_hash_aref(val_hash, ID2SYM(rb_intern("mimetypes")));
    VALUE lsp = rb_hash_aref(val_hash, ID2SYM(rb_intern("lsp")));
    if (!NIL_P(fg))
      lang.color = NUM2UINT(fg);
    if (!NIL_P(symbol))
      lang.symbol = StringValueCStr(symbol);
    lang.extensions = ruby_array_to_vector(extensions);
    lang.filenames = ruby_array_to_vector(filenames);
    lang.mimetypes = ruby_array_to_vector(mimetypes);
    if (!NIL_P(lsp))
      lang.lsp_command = StringValueCStr(lsp);
    result.push_back(lang);
  }
  return result;
}

void load_languages_info() {
  std::lock_guard lock(ruby_mutex);
  auto langs = read_languages();
  auto lsps_t = read_lsps();
  languages.clear();
  for (auto &lang : langs) {
    Language l;
    l.name = lang.name;
    l.color = lang.color;
    l.lsp_name = lang.lsp_command;
    l.symbol = lang.symbol;
    languages[lang.name] = l;
    for (auto &ext : lang.extensions)
      language_extensions[ext] = lang.name;
    // TODO: seperate extensions and filenames
    for (auto &filename : lang.filenames)
      language_extensions[filename] = lang.name;
    for (auto &mimetype : lang.mimetypes)
      language_mimetypes[mimetype] = lang.name;
  }
  for (auto &lsp : lsps_t)
    lsps[lsp.command] = lsp;
}

bool read_line_endings() {
  std::lock_guard lock(ruby_mutex);
  if (C_module == Qnil)
    return true;
  VALUE le = rb_funcall(C_module, rb_intern("line_endings"), 0);
  if (SYMBOL_P(le))
    return std::string(rb_id2name(SYM2ID(le))) == "unix";
  return true;
}
