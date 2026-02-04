#ifndef TILING_DECL_H
#define TILING_DECL_H

#include "io/sysio.h"
#include "utils/utils.h"

struct Tile {
  bool hidden = false;
  uint32_t weight = 100;

  virtual void render(std::vector<ScreenCell> &buffer, Coord size,
                      Coord pos) = 0;
  virtual void handle_click(KeyEvent event, Coord size) = 0;
  virtual ~Tile() = default;
};

struct Window : Tile {
  virtual ~Window() = default;
  virtual void handle_event(KeyEvent){};
  virtual void handle_command(std::string &) {};
  virtual void work() {};
  virtual std::array<std::string, 5> bar_info() { return {}; };
};

struct TileBlock : Tile {
  bool vertical;
  std::vector<std::unique_ptr<Tile>> tiles;

  void render(std::vector<ScreenCell> &buffer, Coord size, Coord pos) override {
    uint32_t total_weight = 0;
    for (auto &t : tiles) {
      if (!t->hidden)
        total_weight += t->weight;
    }
    if (total_weight == 0)
      return;
    for (auto &t : tiles) {
      if (t->hidden)
        continue;
      uint32_t proportion =
          t->weight * (vertical ? size.row : size.col) / total_weight;
      Coord tile_size = vertical ? Coord{.row = proportion, .col = size.col}
                                 : Coord{.row = size.row, .col = proportion};
      t->render(buffer, tile_size, pos);
      if (vertical)
        pos.row += tile_size.row;
      else
        pos.col += tile_size.col;
    }
  }

  void handle_click(KeyEvent event, Coord size) override {
    uint32_t total_weight = 0;
    for (auto &t : tiles)
      if (!t->hidden)
        total_weight += t->weight;
    if (total_weight == 0)
      return;
    uint32_t i = 0;
    for (auto &t : tiles) {
      if (t->hidden)
        continue;
      uint32_t proportion =
          t->weight * (vertical ? size.row : size.col) / total_weight;
      Coord tile_size = vertical ? Coord{.row = proportion, .col = size.col}
                                 : Coord{.row = size.row, .col = proportion};
      if (vertical) {
        if (event.mouse_y < i + proportion) {
          event.mouse_y -= i;
          t->handle_click(event, tile_size);
          return;
        }
      } else {
        if (event.mouse_x < i + proportion) {
          event.mouse_x -= i;
          t->handle_click(event, tile_size);
          return;
        }
      }
      i += proportion;
    }
  }
};

struct TileRoot {
  std::unique_ptr<Tile> tile;
  Coord pos;
  Coord size;

  void render(std::vector<ScreenCell> &buffer) {
    if (this->tile->hidden)
      return;
    this->tile->render(buffer, size, pos);
  }
  void handle_click(KeyEvent event) {
    event.mouse_x -= this->pos.col;
    event.mouse_y -= this->pos.row;
    this->tile->handle_click(event, size);
  }
  bool inside(uint32_t x, uint32_t y) {
    return x >= pos.col && x < pos.col + size.col && y >= pos.row &&
           y < pos.row + size.row;
  }
};

extern TileRoot root_tile;
extern std::vector<std::unique_ptr<TileRoot>> popups;
extern Window *focused_window;

inline void close_popup(TileRoot *handle) {
  auto it = std::find_if(popups.begin(), popups.end(),
                         [handle](const auto &p) { return p.get() == handle; });
  if (it != popups.end())
    popups.erase(it);
}

void render();
void handle_click(KeyEvent event);

#endif
