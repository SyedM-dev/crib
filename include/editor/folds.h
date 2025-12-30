#ifndef EDITOR_FOLDS_H
#define EDITOR_FOLDS_H

#include "editor/editor.h"

inline std::vector<Fold>::iterator find_fold_iter(Editor *editor,
                                                  uint32_t line) {
  auto &folds = editor->folds;
  auto it = std::lower_bound(
      folds.begin(), folds.end(), line,
      [](const Fold &fold, uint32_t value) { return fold.start < value; });
  if (it != folds.end() && it->start == line)
    return it;
  if (it != folds.begin()) {
    --it;
    if (it->contains(line))
      return it;
  }
  return folds.end();
}

inline bool add_fold(Editor *editor, uint32_t start, uint32_t end) {
  if (!editor || !editor->root)
    return false;
  if (start > end)
    std::swap(start, end);
  if (start >= editor->root->line_count)
    return false;
  end = std::min(end, editor->root->line_count - 1);
  if (start == end)
    return false;
  Fold new_fold{start, end};
  auto &folds = editor->folds;
  auto it = std::lower_bound(
      folds.begin(), folds.end(), new_fold.start,
      [](const Fold &fold, uint32_t value) { return fold.start < value; });
  if (it != folds.begin()) {
    auto prev = std::prev(it);
    if (prev->end + 1 >= new_fold.start) {
      new_fold.start = std::min(new_fold.start, prev->start);
      new_fold.end = std::max(new_fold.end, prev->end);
      it = folds.erase(prev);
    }
  }
  while (it != folds.end() && it->start <= new_fold.end + 1) {
    new_fold.end = std::max(new_fold.end, it->end);
    it = folds.erase(it);
  }
  folds.insert(it, new_fold);
  return true;
}

inline bool remove_fold(Editor *editor, uint32_t line) {
  auto it = find_fold_iter(editor, line);
  if (it == editor->folds.end())
    return false;
  editor->folds.erase(it);
  return true;
}

inline void apply_line_insertion(Editor *editor, uint32_t line, uint32_t rows) {
  for (auto it = editor->folds.begin(); it != editor->folds.end();) {
    if (line <= it->start) {
      it->start += rows;
      it->end += rows;
      ++it;
    } else if (line <= it->end) {
      it = editor->folds.erase(it);
    } else {
      ++it;
    }
  }
}

inline void apply_line_deletion(Editor *editor, uint32_t removal_start,
                                uint32_t removal_end) {
  if (removal_start > removal_end)
    return;
  uint32_t rows_removed = removal_end - removal_start + 1;
  std::vector<Fold> updated;
  updated.reserve(editor->folds.size());
  for (auto fold : editor->folds) {
    if (removal_end < fold.start) {
      fold.start -= rows_removed;
      fold.end -= rows_removed;
      updated.push_back(fold);
      continue;
    }
    if (removal_start > fold.end) {
      updated.push_back(fold);
      continue;
    }
  }
  editor->folds.swap(updated);
}

inline const Fold *fold_for_line(const std::vector<Fold> &folds,
                                 uint32_t line) {
  auto it = std::lower_bound(
      folds.begin(), folds.end(), line,
      [](const Fold &fold, uint32_t value) { return fold.start < value; });
  if (it != folds.end() && it->start == line)
    return &(*it);
  if (it != folds.begin()) {
    --it;
    if (it->contains(line))
      return &(*it);
  }
  return nullptr;
}

inline Fold *fold_for_line(std::vector<Fold> &folds, uint32_t line) {
  const auto *fold =
      fold_for_line(static_cast<const std::vector<Fold> &>(folds), line);
  return const_cast<Fold *>(fold);
}

inline bool line_is_fold_start(const std::vector<Fold> &folds, uint32_t line) {
  const Fold *fold = fold_for_line(folds, line);
  return fold && fold->start == line;
}

inline bool line_is_folded(const std::vector<Fold> &folds, uint32_t line) {
  return fold_for_line(folds, line) != nullptr;
}

inline uint32_t next_unfolded_row(const Editor *editor, uint32_t row) {
  uint32_t limit = editor && editor->root ? editor->root->line_count : 0;
  while (row < limit) {
    const Fold *fold = fold_for_line(editor->folds, row);
    if (!fold)
      return row;
    row = fold->end + 1;
  }
  return limit;
}

inline uint32_t prev_unfolded_row(const Editor *editor, uint32_t row) {
  while (row > 0) {
    const Fold *fold = fold_for_line(editor->folds, row);
    if (!fold)
      return row;
    if (fold->start == 0)
      return 0;
    row = fold->start - 1;
  }
  return 0;
}

#endif
