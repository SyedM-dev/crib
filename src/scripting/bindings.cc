#include "main.h"
#include "scripting/decl.h"

mrb_value get_mode(mrb_state *mrb, mrb_value self) {
  return mrb_fixnum_value(mode);
}

void setup_ruby_bindings(mrb_state *mrb, RClass *C_module) {
  mrb_define_module_function(mrb, C_module, "mode", get_mode, MRB_ARGS_NONE());
}
