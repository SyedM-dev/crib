#ifndef MAIN_H
#define MAIN_H

#include "pch.h"
#include "ui/bar.h"

#define NORMAL 0
#define INSERT 1
#define SELECT 2
#define RUNNER 3
#define JUMPER 4

extern std::atomic<bool> running;
extern std::atomic<uint8_t> mode;
extern fs::path pwd;

namespace ui {
extern Bar bar;
extern TileRoot *hover_popup;
} // namespace ui

#endif
