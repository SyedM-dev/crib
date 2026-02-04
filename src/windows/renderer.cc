#include "main.h"
#include "windows/decl.h"

TileRoot root_tile;
std::vector<std::unique_ptr<TileRoot>> popups;
Window *focused_window;

void render() {
  ui::bar.render(new_screen);
  root_tile.render(new_screen);
  for (auto &popup : popups)
    popup->render(new_screen);
}

void handle_click(KeyEvent event) {
  for (auto &popup : popups) {
    if (popup->inside(event.mouse_x, event.mouse_y)) {
      popup->handle_click(event);
      return;
    }
  }
  root_tile.handle_click(event);
}
