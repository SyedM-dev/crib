// #ifndef UI_COMPLETIONBOX_H
// #define UI_COMPLETIONBOX_H
//
// #include "io/sysio.h"
// #include "pch.h"
// #include "utils/utils.h"
//
// struct CompletionBox {
//   std::shared_mutex mtx;
//   struct CompletionSession *session;
//   bool hidden = true;
//   std::vector<ScreenCell> cells;
//   Coord size;
//   Coord position;
//
//   CompletionBox(CompletionSession *s) : session(s) {}
//   void render_update();
//   void render(Coord pos);
// };
//
// #endif
