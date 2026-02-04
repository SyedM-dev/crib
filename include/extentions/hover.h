#ifndef EXTENTION_HOVER_H
#define EXTENTION_HOVER_H

#include "io/sysio.h"
#include "pch.h"
#include "utils/utils.h"
#include "windows/decl.h"

TileRoot *init_hover();

struct HoverBox : Window {
  std::string text;
  std::atomic<bool> is_markup;
  uint32_t scroll_;
  bool scroll_dirty;

  HoverBox() : scroll_(0) { this->hidden = true; }
  void clear() {
    this->text = "";
    this->hidden = true;
    this->is_markup = false;
    this->scroll_ = 0;
    this->scroll_dirty = true;
  }
  void scroll(int32_t number);
  void render(std::vector<ScreenCell> &buffer, Coord size, Coord pos) override;
  void handle_click(KeyEvent, Coord) override { this->hidden = true; };
  ~HoverBox() {};
};

#endif
