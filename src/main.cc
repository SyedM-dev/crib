#include "main.h"
#include "editor/editor.h"
#include "extentions/hover.h"
#include "io/sysio.h"
#include "lsp/lsp.h"
#include "ruby/decl.h"
#include "ui/bar.h"
#include "utils/utils.h"
#include "windows/decl.h"

std::atomic<bool> running{true};
Queue<KeyEvent> event_queue;
fs::path pwd;
std::atomic<uint8_t> mode = NORMAL;

namespace ui {
Bar bar;
TileRoot *hover_popup = nullptr;
} // namespace ui

void background_lsp() {
  while (running)
    throttle(8ms, lsp_worker);
}

int main(int argc, char *argv[]) {
  ruby_start();
  load_theme();
  load_languages_info();
  load_custom_highlighters();

  Coord screen = start_screen();
  const char *filename = (argc > 1) ? argv[1] : "";
  uint8_t eol = read_line_endings();

  ui::hover_popup = init_hover();

  root_tile.size = {screen.row - 2, screen.col - 1};
  root_tile.pos = {0, 0};

  root_tile.tile = std::make_unique<Editor>(filename, eol);
  focused_window = static_cast<Editor *>(root_tile.tile.get());
  ui::bar.init(screen);

  std::thread lsp_thread(background_lsp);

  while (running) {
    KeyEvent event = throttle(1ms, read_key);
    if (event.key_type != KEY_NONE) {
      if (event.key_type == KEY_CHAR && event.len == 1 &&
          event.c[0] == CTRL('q')) {
        free(event.c);
        running = false;
        break;
      }
      if (mode != RUNNER) {
        if (event.key_type == KEY_MOUSE) {
          handle_click(event);
        } else {
          focused_window->handle_event(event);
        }
      } else {
        ui::bar.handle_event(event);
      }
      if ((event.key_type == KEY_CHAR || event.key_type == KEY_PASTE) &&
          event.c)
        free(event.c);
    }
    for (auto &lsp_inst : lsp::active_lsps)
      lsp_inst.second->callbacks();
    focused_window->work();
    throttle(4ms, render);
    throttle(4ms, io_render);
  }

  if (lsp_thread.joinable())
    lsp_thread.join();

  end_screen();

  ruby_shutdown();

  return 0;
}
