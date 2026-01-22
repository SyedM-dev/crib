Copyright 2025 Syed Daanish

# TODO

### Critical Fixes

##### Check each lsp with each of the features implemented

* [ ] **LSP Bug:** Check why `fish-lsp` is behaving so off with completions filtering.
* [ ] **File IO:** Normalize/validate unicode on file open (enforce UTF-8, handle other types gracefully).
* [ ] **Critical Crash:** Fix bug where closing immediately while LSP is still loading hangs and then segfaults (especially on slow ones like fish-lsp where quick edits and exit can hang).
* [ ] **Line move:** fix the move line functions to work without the calculations from folds as folds are removed.
* [ ] **Modularize handle_events and renderer functions:** The function is over 700 lines with a lot of repeating blocks. Split into smaller functions.
* [ ] **Editor Indentation Fix:** - Main : merger indentation with the parser for more accurate results.
    * [ ] Keep cache of language maps in engine to reduce lookup time.
    * [ ] In indents add function to support tab which indents if before any content and inserts a pure \t otherwise.
    * [ ] And backspace which undents if before any content.
    * [ ] Add block indentation support.
    * [ ] Ignore comments/strings from parser when auto-indenting.
    * [ ] These will dedent when the block immediately after them is dedented
    * [ ] Dont dedent if ending is valid starting is invalid but also empty
    * [ ] Just leave asis if starting is empty
* [ ] **Readme:** Update readme to show indetation mechanics.
* [ ] **LSP Bug:** Try to find out why emojis are breaking lsp edits. (check the ruby sample)
* [ ] **UI Refinement:**
    * [ ] Allow completion list to be scrolled; show only `x` max items.
    * [ ] Finish autocomplete box style functions.
* [ ] **Documentation UI:** Capture `Ctrl+h` / `Ctrl+l` for scrolling documentation windows.
* [ ] **Redo hooks and folding as proper engines**: With functions to checkstate/cursor like function and edits application.
* [ ] Do trextmate like regex grammar parsing with lsp symbols for semantic highlighting.
    * Probably remove tre--sitter or just keep it for context tree.
    * Making bracket matching andignoring strings/comments easier.
* remove tree-sitter mention from everywhere especially submodules
* make it faster for line inserts/deletes too (treeify the vector)
* Try to make all functions better now that folds have been purged
* Cleanup syntax and renderer files

probably remove solargraph support and use ruby-lsp (or sorbet?) instead

move lsp configs to json and also allow configs for windows-style vs unix-style line endings and utf-8 vs utf-16

* the ruby should have an api to be able to draw windows and add mappings to them

* finish bash then do all the directive-like ones like jsonc (first to help with theme files) / toml / yaml / ini / nginx
* then markdown / html
* then gitignore / gitattributes
* then fish then sql then css and [jt]sx? then python then lua (make with type annotations for lsp results)
* then [ch](++)? then gdscript then erb then php
* then haskell then gomod then go then rust

* [ ] **Undo/Redo:** Add support for undo/redo history.

* [ ] **Auto brace selection:** Add support for auto brace selection.

* [ ] **Tree-sitter Indent:** Attempt to allow Tree-sitter to handle indentation if possible.

* [ ] **Scrolling:** Add logic where selecting at the end of the screen scrolls down (and vice versa).
    * *Implementation:* Update `main.cc` to send drag events to the selected editor.


### UX

* [ ] **Editor word highlighter:** Do not recompute word under cursor if not changed.

* [ ] **Completion Filtering:**
    * [ ] Stop filtering case-sensitive.
    * [ ] Normalize completion edits if local filtering is used.

* [ ] **LSP Features:**
    * [ ] Add LSP jumping support (Go to Definition, Hover).
    * [ ] Add LSP rename support.
    * [ ] Handle snippets properly in autocomplete: use only the last word in signature when replacing and set cursor to the first one.

* [ ] **Basic Autocomplete:** Keep a list of words in the current buffer for non-LSP fallback.
* [ ] **Language Support:**
    * [ ] Add ECMA to JS and make TSX support.
    * [ ] Add formatting for files where LSP doesn't provide it.
    * [ ] Redo grammar files properly (especially cpp).


### Major Features

* [ ] **Search & Replace:**
    * [ ] Add Search/Replace UI.
    * [ ] Support capture groups (`$1`, `$2`) or allow Perl regex directly.
    * [ ] Ensure virtual cursors are included in search positions.

* [ ] **Multi-Cursor:**
    * [ ] Add virtual cursor support (edits apply to all locations).
    * [ ] Add `Alt+Click` to set multiple cursors.
    * [ ] Allow search and place cursor at all matches.

* [ ] **Block Selection:**
    * [ ] Double-clicking a bracket selects the whole block (first time only) and sets mode to `WORD`.

* [ ] **Tree-sitter Context:**
    * [ ] Get code context from Tree-sitter.
    * [ ] Get node path of current cursor and add indicator bar (breadcrumbs).
    * [ ] Highlight block edges when cursor is on/in a bracket.


### Visuals, UI & Extensions?

*Focus: Aesthetics and external integrations.*

* [ ] **Status Bar:** Complete status bar and command runner.

* [ ] **Visual Aids:**
    * [ ] Expand color regex to match CSS colors in CSS files.
    * [ ] Add color picker/palette.

* [ ] **Git:** Add Git integration (status, diffs).
* [ ] **AI/Snippets:**
    * [ ] Add snippets support (LuaSnip/VSnip style).
    * [ ] Add Codeium/Copilot support (using VAI virtual text) as a test phase.

* [ ] **SQL:** Add SQL support (Viewer and Basic Editor).
* [ ] **Prolly?:** Add Splash Screen / Minigame.


### Optimizations & Fluff

* [ ] **Event Loop:**
    * [ ] Make the whole engine event-driven rather than clock-driven.
    * [ ] Mybe keep background thread with dirty flag.
    * [ ] But merge input and render into a single loop that only renders when input affects render or background thread needs refresh and try to couple multiple renders.
    * [ ] LSP and inputs should be blocking (lsp on its fd) and inputs in io/input.cc

* [ ] **Performance:**
    * [ ] Switch JSON parser to `RapidJSON` (or similar high-performance lib).
    * [ ] Decrease usage of `std::string` in UI, LSP, and warnings.
    * [ ] Also for vectors into managed memory especially for completions/lsp-stuff.

* [ ] **Folding:** Redo folding system and its relation to `move_line_*` functions.

* [ ] **Grammars:**
    * [ ] Manually add redo SCM files (especially cpp/c/h).
    * [ ] Create `lua-typed` and `man pages` Tree-sitter grammars.

* [ ] **Repo Maintenance:** Once renderer is proven check commit `43f443e`.
