#include "ruby/decl.h"

mrb_value get_config_file(mrb_state *mrb, mrb_value self) {
  return mrb_str_new_cstr(mrb, ruby_config_path.string().c_str());
}

void setup_ruby_bindings(mrb_state *mrb, RClass *C_module) {
  mrb_define_module_function(mrb, C_module, "config_file", get_config_file,
                             MRB_ARGS_NONE());
}
