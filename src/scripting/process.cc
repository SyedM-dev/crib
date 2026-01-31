#include "io/sysio.h"
#include "main.h"
#include "scripting/decl.h"
#include "scripting/ruby_compiled.h"
#include "utils/utils.h"
#include <mruby/boxing_word.h>

std::unordered_map<std::string, std::pair<mrb_value, mrb_value>>
    custom_highlighters;

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
  std::vector<std::string> extensions;
  std::vector<std::string> filenames;
  std::string lsp_command; // link to LSP by name
};

mrb_state *mrb = nullptr;
RClass *C_module;

namespace fs = std::filesystem;

void ruby_start() {
  mrb = mrb_open();
  if (!mrb) {
    fprintf(stderr, "Failed to init mruby\n");
    return;
  }
  fs::path exe_dir = get_exe_dir();
  std::vector<fs::path> candidates;
  candidates.emplace_back("./crib.rb");
  const char *xdg = std::getenv("XDG_CONFIG_HOME");
  const char *home = std::getenv("HOME");
  if (xdg) {
    candidates.emplace_back(fs::path(xdg) / "crib/crib.rb");
    candidates.emplace_back(fs::path(xdg) / "crib/main.rb");
    candidates.emplace_back(fs::path(xdg) / "crib.rb");
  } else if (home) {
    fs::path base = fs::path(home) / ".config";
    candidates.emplace_back(base / "crib/crib.rb");
    candidates.emplace_back(base / "crib/main.rb");
    candidates.emplace_back(base / "crib.rb");
  }
  candidates.emplace_back(exe_dir / "../config/main.rb");
  candidates.emplace_back(exe_dir / "../config/crib.rb");
  mrb_load_irep(mrb, _tmp___crib_precompiled_mrb);
  C_module = mrb_module_get(mrb, "C");
  setup_ruby_bindings(mrb, C_module);
  for (const auto &p : candidates) {
    if (fs::exists(p)) {
      FILE *f = fopen(p.string().c_str(), "r");
      if (f) {
        mrb_load_file(mrb, f);
        if (mrb->exc)
          exit(1);
        fclose(f);
      }
      break;
    }
  }
  mrb_value mod_val = mrb_obj_value(C_module);
  mrb_value block = mrb_funcall(mrb, mod_val, "b_startup", 0);
  if (!mrb_nil_p(block))
    mrb_funcall(mrb, block, "call", 0);
}

inline static std::vector<BarLight>
convert_highlights(mrb_state *mrb, mrb_value highlights_val) {
  std::vector<BarLight> result;
  if (!mrb_array_p(highlights_val))
    return result;
  mrb_int len = RARRAY_LEN(highlights_val);
  for (mrb_int i = 0; i < len; i++) {
    mrb_value item = mrb_ary_ref(mrb, highlights_val, i);
    if (!mrb_hash_p(item))
      continue;
    auto get_sym = [&](const char *name) {
      return mrb_symbol_value(mrb_intern_cstr(mrb, name));
    };
    mrb_value fg_v = mrb_hash_get(mrb, item, get_sym("fg"));
    mrb_value bg_v = mrb_hash_get(mrb, item, get_sym("bg"));
    mrb_value flags_v = mrb_hash_get(mrb, item, get_sym("flags"));
    mrb_value start_v = mrb_hash_get(mrb, item, get_sym("start"));
    mrb_value length_v = mrb_hash_get(mrb, item, get_sym("length"));
    BarLight bl{};
    if (!mrb_nil_p(fg_v))
      bl.highlight.fg = (uint32_t)mrb_fixnum(fg_v);
    if (!mrb_nil_p(bg_v))
      bl.highlight.bg = (uint32_t)mrb_fixnum(bg_v);
    if (!mrb_nil_p(flags_v))
      bl.highlight.flags = (uint32_t)mrb_fixnum(flags_v);
    uint32_t start = !mrb_nil_p(start_v) ? (uint32_t)mrb_fixnum(start_v) : 0;
    uint32_t length = !mrb_nil_p(length_v) ? (uint32_t)mrb_fixnum(length_v) : 0;
    bl.start = start;
    bl.end = start + length;
    result.push_back(bl);
  }
  return result;
}

BarLine bar_contents(uint8_t mode, std::string lang_name, uint32_t warnings,
                     std::string lsp_name, std::string filename,
                     std::string foldername, uint32_t line, uint32_t max_line,
                     uint32_t width) {
  BarLine bar_line;
  mrb_value info = mrb_hash_new(mrb);
  mrb_value key_mode = mrb_symbol_value(mrb_intern_cstr(mrb, "mode"));
  mrb_value val_mode;
  switch (mode) {
  case NORMAL:
    val_mode = mrb_symbol_value(mrb_intern_cstr(mrb, "normal"));
    break;
  case INSERT:
    val_mode = mrb_symbol_value(mrb_intern_cstr(mrb, "insert"));
    break;
  case SELECT:
    val_mode = mrb_symbol_value(mrb_intern_cstr(mrb, "select"));
    break;
  case RUNNER:
    val_mode = mrb_symbol_value(mrb_intern_cstr(mrb, "runner"));
    break;
  case JUMPER:
    val_mode = mrb_symbol_value(mrb_intern_cstr(mrb, "jumper"));
    break;
  }
  mrb_hash_set(mrb, info, key_mode, val_mode);
  mrb_value key_lang_name = mrb_symbol_value(mrb_intern_cstr(mrb, "lang_name"));
  mrb_value val_lang_name =
      mrb_symbol_value(mrb_intern_cstr(mrb, lang_name.c_str()));
  mrb_hash_set(mrb, info, key_lang_name, val_lang_name);
  mrb_value key_filename = mrb_symbol_value(mrb_intern_cstr(mrb, "filename"));
  mrb_value val_filename =
      mrb_str_new(mrb, filename.c_str(), filename.length());
  mrb_hash_set(mrb, info, key_filename, val_filename);
  mrb_value key_width = mrb_symbol_value(mrb_intern_cstr(mrb, "width"));
  mrb_value val_width = mrb_fixnum_value(width);
  mrb_hash_set(mrb, info, key_width, val_width);
  mrb_value mod_val = mrb_obj_value(C_module);
  mrb_value block = mrb_funcall(mrb, mod_val, "b_bar", 0);
  mrb_value val_line = mrb_funcall(mrb, block, "call", 1, info);
  if (mrb->exc) {
    end_screen();
    fputs("Error when executing Ruby code:\n", stderr);
    mrb_print_error(mrb);
    mrb_close(mrb);
    exit(1);
  }
  mrb_value text_val = mrb_hash_get(
      mrb, val_line, mrb_symbol_value(mrb_intern_cstr(mrb, "text")));
  const char *ptr = RSTRING_PTR(text_val);
  mrb_int len = RSTRING_LEN(text_val);
  bar_line.line = std::string(ptr, len);
  mrb_value highlights_val = mrb_hash_get(
      mrb, val_line, mrb_symbol_value(mrb_intern_cstr(mrb, "highlights")));
  bar_line.highlights = convert_highlights(mrb, highlights_val);
  return bar_line;
}

void ruby_copy(const char *text, size_t len) {
  if (C_module == nullptr)
    return;
  mrb_value mod_val = mrb_obj_value(C_module);
  mrb_value block = mrb_funcall(mrb, mod_val, "b_copy", 0);
  if (!mrb_nil_p(block))
    mrb_funcall(mrb, block, "call", 1, mrb_str_new(mrb, text, len));
  if (mrb->exc) {
    end_screen();
    fputs("Error when executing Ruby code:\n", stderr);
    mrb_print_error(mrb);
    mrb_close(mrb);
    exit(1);
  }
}

std::string ruby_paste() {
  if (C_module == nullptr)
    return "";
  mrb_value mod_val = mrb_obj_value(C_module);
  mrb_value block = mrb_funcall(mrb, mod_val, "b_paste", 0);
  if (!mrb_nil_p(block)) {
    mrb_value val = mrb_funcall(mrb, block, "call", 0);
    if (mrb->exc) {
      end_screen();
      fputs("Error when executing Ruby code:\n", stderr);
      mrb_print_error(mrb);
      mrb_close(mrb);
      exit(1);
    }
    if (mrb_string_p(val))
      return std::string(RSTRING_PTR(val), RSTRING_LEN(val));
    return "";
  }
  return "";
}

void ruby_shutdown() {
  if (C_module == nullptr)
    return;
  mrb_value mod_val = mrb_obj_value(C_module);
  mrb_value block = mrb_funcall(mrb, mod_val, "b_shutdown", 0);
  if (!mrb_nil_p(block))
    mrb_funcall(mrb, block, "call", 0);
  mrb_close(mrb);
  mrb = nullptr;
  C_module = nullptr;
}

std::vector<std::string> array_to_vector(mrb_value ary) {
  std::vector<std::string> result;
  if (mrb_nil_p(ary) || mrb_type(ary) != MRB_TT_ARRAY)
    return result;
  mrb_int len = RARRAY_LEN(ary);
  for (mrb_int i = 0; i < len; i++) {
    mrb_value item = mrb_ary_ref(mrb, ary, i);
    if (mrb_string_p(item))
      result.push_back(std::string(RSTRING_PTR(item), RSTRING_LEN(item)));
  }
  return result;
}

void load_custom_highlighters() {
  if (!C_module)
    return;
  mrb_value mod_val = mrb_obj_value((struct RObject *)C_module);
  mrb_value hashmap = mrb_funcall(mrb, mod_val, "highlighters", 0);
  if (mrb_nil_p(hashmap) || mrb_type(hashmap) != MRB_TT_HASH)
    return;
  mrb_value keys = mrb_funcall(mrb, hashmap, "keys", 0);
  mrb_int len = RARRAY_LEN(keys);
  for (mrb_int i = 0; i < len; i++) {
    mrb_value key_sym = mrb_ary_ref(mrb, keys, i);
    mrb_sym sym_id = mrb_symbol(key_sym);
    const char *key_cstr = mrb_sym_dump(mrb, sym_id);
    std::string key(key_cstr);
    mrb_value val_hash = mrb_hash_get(mrb, hashmap, key_sym);
    if (mrb_nil_p(val_hash) || mrb_type(val_hash) != MRB_TT_HASH)
      continue;
    mrb_sym parser_sym = mrb_intern_lit(mrb, "parser");
    mrb_sym matcher_sym = mrb_intern_lit(mrb, "matcher");
    mrb_value parse_block =
        mrb_hash_get(mrb, val_hash, mrb_symbol_value(parser_sym));
    mrb_value match_block =
        mrb_hash_get(mrb, val_hash, mrb_symbol_value(matcher_sym));
    custom_highlighters[key] = {parse_block, match_block};
  }
}

bool custom_compare(mrb_value match_block, mrb_value state1, mrb_value state2) {
  if (mrb_type(match_block) != MRB_TT_PROC)
    return false;
  mrb_value ret = mrb_funcall(mrb, match_block, "call", 2, state1, state2);
  return mrb_test(ret);
}

mrb_value parse_custom(std::vector<Token> *tokens, mrb_value parser_block,
                       const char *line, uint32_t len, mrb_value state,
                       uint32_t c_line) {
  tokens->clear();
  if (mrb_nil_p(parser_block))
    return mrb_nil_value();
  mrb_value ruby_line = mrb_str_new(mrb, line, len);
  mrb_value line_idx = mrb_fixnum_value(c_line);
  mrb_value tokens_and_state_hash =
      mrb_funcall(mrb, parser_block, "call", 3, ruby_line, state, line_idx);
  mrb_sym tokens_sym = mrb_intern_lit(mrb, "tokens");
  mrb_value tokens_rb =
      mrb_hash_get(mrb, tokens_and_state_hash, mrb_symbol_value(tokens_sym));
  if (mrb_type(tokens_rb) == MRB_TT_ARRAY) {
    mrb_int len_tokens = RARRAY_LEN(tokens_rb);
    for (mrb_int i = 0; i < len_tokens; i++) {
      mrb_value token = mrb_ary_ref(mrb, tokens_rb, i);
      Token tok;
      tok.type = (TokenKind)mrb_fixnum(mrb_hash_get(
          mrb, token, mrb_symbol_value(mrb_intern_lit(mrb, "type"))));
      tok.start = (uint32_t)mrb_fixnum(mrb_hash_get(
          mrb, token, mrb_symbol_value(mrb_intern_lit(mrb, "start"))));
      tok.end = (uint32_t)mrb_fixnum(mrb_hash_get(
          mrb, token, mrb_symbol_value(mrb_intern_lit(mrb, "end"))));
      if (tok.type < TokenKind::Count && tok.end > tok.start && tok.end <= len)
        tokens->push_back(tok);
    }
  }
  mrb_sym state_sym = mrb_intern_lit(mrb, "state");
  return mrb_hash_get(mrb, tokens_and_state_hash, mrb_symbol_value(state_sym));
}

static std::vector<R_ThemeEntry> read_theme() {
  std::vector<R_ThemeEntry> result;
  if (!C_module)
    return result;
  mrb_value mod_val = mrb_obj_value((struct RObject *)C_module);
  mrb_value theme_hash = mrb_funcall(mrb, mod_val, "theme", 0);
  if (mrb_nil_p(theme_hash) || mrb_type(theme_hash) != MRB_TT_HASH)
    return result;
  mrb_value keys = mrb_funcall(mrb, theme_hash, "keys", 0);
  mrb_int len_keys = RARRAY_LEN(keys);
  for (mrb_int i = 0; i < len_keys; i++) {
    mrb_value key_sym = mrb_ary_ref(mrb, keys, i);
    mrb_sym sym_id = mrb_symbol(key_sym);
    const char *key_cstr = mrb_sym_dump(mrb, sym_id);
    std::string key(key_cstr);
    mrb_value val_hash = mrb_hash_get(mrb, theme_hash, key_sym);
    if (mrb_nil_p(val_hash) || mrb_type(val_hash) != MRB_TT_HASH)
      continue;
    R_ThemeEntry entry;
    entry.key = key;
    mrb_value fg = mrb_hash_get(mrb, val_hash,
                                mrb_symbol_value(mrb_intern_lit(mrb, "fg")));
    mrb_value bg = mrb_hash_get(mrb, val_hash,
                                mrb_symbol_value(mrb_intern_lit(mrb, "bg")));
    mrb_value italic = mrb_hash_get(
        mrb, val_hash, mrb_symbol_value(mrb_intern_lit(mrb, "italic")));
    mrb_value bold = mrb_hash_get(
        mrb, val_hash, mrb_symbol_value(mrb_intern_lit(mrb, "bold")));
    mrb_value underline = mrb_hash_get(
        mrb, val_hash, mrb_symbol_value(mrb_intern_lit(mrb, "underline")));
    mrb_value strikethrough = mrb_hash_get(
        mrb, val_hash, mrb_symbol_value(mrb_intern_lit(mrb, "strikethrough")));
    if (!mrb_nil_p(fg))
      entry.fg = (uint32_t)mrb_fixnum(fg);
    if (!mrb_nil_p(bg))
      entry.bg = (uint32_t)mrb_fixnum(bg);
    if (!mrb_nil_p(italic))
      entry.italic = mrb_test(italic);
    if (!mrb_nil_p(bold))
      entry.bold = mrb_test(bold);
    if (!mrb_nil_p(underline))
      entry.underline = mrb_test(underline);
    if (!mrb_nil_p(strikethrough))
      entry.strikethrough = mrb_test(strikethrough);
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
  if (!C_module)
    return result;
  mrb_value mod_val = mrb_obj_value((struct RObject *)C_module);
  mrb_value lsp_hash = mrb_funcall(mrb, mod_val, "lsp_config", 0);
  if (mrb_nil_p(lsp_hash) || mrb_type(lsp_hash) != MRB_TT_HASH)
    return result;
  mrb_value keys = mrb_funcall(mrb, lsp_hash, "keys", 0);
  mrb_int len_keys = RARRAY_LEN(keys);
  for (mrb_int i = 0; i < len_keys; i++) {
    mrb_value key = mrb_ary_ref(mrb, keys, i);
    std::string cmd;
    if (mrb_string_p(key))
      cmd = std::string(RSTRING_PTR(key), RSTRING_LEN(key));
    else if (mrb_symbol_p(key))
      cmd = std::string(mrb_sym_dump(mrb, mrb_symbol(key)));
    mrb_value args_array = mrb_hash_get(mrb, lsp_hash, key);
    std::vector<std::string> args = array_to_vector(args_array);
    result.push_back({cmd, args});
  }
  return result;
}

std::vector<R_Language> read_languages() {
  std::vector<R_Language> result;
  if (!C_module)
    return result;
  mrb_value mod_val = mrb_obj_value((struct RObject *)C_module);
  mrb_value lang_hash = mrb_funcall(mrb, mod_val, "languages", 0);
  if (mrb_nil_p(lang_hash) || mrb_type(lang_hash) != MRB_TT_HASH)
    return result;
  mrb_value keys = mrb_funcall(mrb, lang_hash, "keys", 0);
  mrb_int len_keys = RARRAY_LEN(keys);
  for (mrb_int i = 0; i < len_keys; i++) {
    mrb_value key = mrb_ary_ref(mrb, keys, i);
    mrb_value val_hash = mrb_hash_get(mrb, lang_hash, key);
    if (mrb_nil_p(val_hash) || mrb_type(val_hash) != MRB_TT_HASH)
      continue;
    R_Language lang;
    if (mrb_symbol_p(key))
      lang.name = std::string(mrb_sym_dump(mrb, mrb_symbol(key)));
    else if (mrb_string_p(key))
      lang.name = std::string(RSTRING_PTR(key), RSTRING_LEN(key));
    mrb_value fg = mrb_hash_get(mrb, val_hash,
                                mrb_symbol_value(mrb_intern_lit(mrb, "color")));
    mrb_value extensions = mrb_hash_get(
        mrb, val_hash, mrb_symbol_value(mrb_intern_lit(mrb, "extensions")));
    mrb_value filenames = mrb_hash_get(
        mrb, val_hash, mrb_symbol_value(mrb_intern_lit(mrb, "filenames")));
    mrb_value lsp = mrb_hash_get(mrb, val_hash,
                                 mrb_symbol_value(mrb_intern_lit(mrb, "lsp")));
    if (!mrb_nil_p(fg))
      lang.color = (uint32_t)mrb_fixnum(fg);
    lang.extensions = array_to_vector(extensions);
    if (!mrb_nil_p(filenames))
      lang.filenames = array_to_vector(filenames);
    if (!mrb_nil_p(lsp))
      lang.lsp_command = std::string(RSTRING_PTR(lsp), RSTRING_LEN(lsp));
    result.push_back(lang);
  }
  return result;
}

void load_languages_info() {
  auto langs = read_languages();
  auto lsps_t = read_lsps();
  languages.clear();
  for (auto &lang : langs) {
    Language l;
    l.name = lang.name;
    l.color = lang.color;
    l.lsp_name = lang.lsp_command;
    languages[lang.name] = l;
    for (auto &ext : lang.extensions)
      language_extensions[ext] = lang.name;
    // TODO: seperate extensions and filenames
    for (auto &filename : lang.filenames)
      language_extensions[filename] = lang.name;
  }
  for (auto &lsp : lsps_t)
    lsps[lsp.command] = lsp;
}

uint8_t read_line_endings() {
  if (!C_module)
    return 1;
  mrb_value mod_val = mrb_obj_value((struct RObject *)C_module);
  mrb_value le = mrb_funcall(mrb, mod_val, "line_endings", 0);
  if (!mrb_symbol_p(le))
    return 1;
  uint8_t flags = 1;
  const char *name = mrb_sym_dump(mrb, mrb_symbol(le));
  if (std::strcmp(name, "unix") == 0)
    flags = 0b01;
  else if (std::strcmp(name, "windows") == 0)
    flags = 0b00;
  else if (std::strcmp(name, "auto_unix") == 0)
    flags = 0b11;
  else if (std::strcmp(name, "auto_windows") == 0)
    flags = 0b10;
  return flags;
}
