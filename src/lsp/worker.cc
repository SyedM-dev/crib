#include "lsp/lsp.h"

namespace lsp {
std::mutex lsp_mutex;
std::unordered_map<std::string, std::unique_ptr<LSPInstance>> active_lsps;
std::unordered_set<std::string> opened;
Queue<std::string> need_opening;
std::vector<Editor *> new_editors;
} // namespace lsp

void lsp_worker() {
  std::lock_guard lock(lsp::lsp_mutex);
  while (!lsp::need_opening.empty()) {
    auto lsp_id = *lsp::need_opening.front();
    if (!lsp::opened.contains(lsp_id)) {
      lsp::active_lsps[lsp_id] = std::make_unique<LSPInstance>(lsp_id);
      lsp::opened.insert(lsp_id);
    }
    lsp::need_opening.pop();
  }
  for (auto it = lsp::new_editors.begin(); it != lsp::new_editors.end();) {
    auto ed = *it;
    auto lsp_id = ed->lang.lsp_name;
    if (lsp::opened.contains(lsp_id)) {
      auto a_it = lsp::active_lsps.find(lsp_id);
      if (a_it != lsp::active_lsps.end())
        a_it->second->add(ed);
      it = lsp::new_editors.erase(it);
    } else {
      lsp::need_opening.push(lsp_id);
      it++;
    }
  }
  for (auto it = lsp::active_lsps.begin(); it != lsp::active_lsps.end();) {
    auto &lsp_inst = it->second;
    if (!lsp_inst || lsp_inst->pid == -1 || lsp_inst->editors.empty() ||
        lsp_inst->exited) {
      it = lsp::active_lsps.erase(it);
      continue;
    }
    lsp_inst->work();
    ++it;
  }
}
