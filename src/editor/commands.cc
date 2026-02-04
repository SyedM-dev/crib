#include "editor/editor.h"
#include "main.h"

void Editor::handle_command(std::string &command) {
  if (command == "w") {
    this->save();
  }
  if (command == "wq") {
    this->save();
    command = "q";
    ui::bar.handle_command(command);
  }
}
