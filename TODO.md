Copyright 2025 Syed Daanish

# TODO

##### BTW Check each lsp with each of the features implemented

* [ ] Add mgems for most common things and a ruby library to allow combining true ruby with mruby
      - Or revert to cruby and retry with manual linking . maybe it might work?
* [ ] color alpha in ini files
* [ ] Make warning before ctrl+q for saving
* [ ] **LSP Bug:** Check why `fish-lsp` is behaving so off with completions filtering.
* [ ] **Line move:** fix the move line functions to work without the calculations from folds as folds are removed.
* [ ] **Editor Indentation Fix:** - Main : merger indentation with the parser for more accurate results.
    * [ ] Keep cache of language maps in engine to reduce lookup time.
    * [ ] In indents add function to support tab which indents if before any content and inserts a pure \t otherwise.
    * [ ] And backspace which undents if before any content.
    * [ ] Add block indentation support.
    * [ ] Ignore comments/strings from parser when auto-indenting.
    * [ ] These will dedent when the block immediately after them is dedented
    * [ ] Dont dedent if ending is valid starting is invalid but also empty
    * [ ] Just leave asis if starting is empty
* [ ] **Readme:** Update readme to show ruby based config.
* [ ] **UI Refinement:**
    * [ ] Allow completion list to be scrolled; show only `x` max items.
    * [ ] Finish autocomplete box style functions.
* [ ] **Documentation UI:** Capture `Ctrl+h` / `Ctrl+l` for scrolling documentation windows.
* [ ] Redo hooks as a struct.
* [ ] breakdown the render function into smaller functions.

* Try to make all functions better now that folds have been purged
* Cleanup syntax and renderer files

* Add a thing called view which is a rect with speacial type editor . but otherwise a ruby or c++ view
* can be used for stuff like file manager/git manager/theme picker.
* allow flushing functions in ruby to tell c++ to refresh keybinds/themes etc.
* allow keybinds to be set in ruby

check::
        pull diagnostics for ruby-lsp
        lsp selection range - use to highlight start / end of range maybe?
        goto definiton
        signature help
        document symbol for the top bar maybe? (or workspace symbol)
        also setup workspaces
        Semantic highlighting
        Quick fixes
        Rename symbols

probably remove solargraph support and use ruby-lsp (or sorbet?) instead because it is a pain for utf stuff
ruby-lsp also supports erb so thats a plus

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
    * *Implementation:* Update `main.cc` to send drag events to the selected editor even if coordinates are out of bounds.


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


### Visuals, UI & Extensions?

* [ ] **Status Bar:** Complete status bar and command runner.

* [ ] Add color picker/palette.

* [ ] **Git:** Add Git integration (status, diffs).
* [ ] **AI/Snippets:**
    * [ ] Add snippets support (LuaSnip/VSnip style).
    * [ ] Add Codeium/Copilot support (using VAI virtual text) as a test phase.

* [ ] **SQL:** Add SQL support (Viewer and Basic Editor).
* [ ] **Prolly?:** Add Splash Screen / Minigame.


### Optimizations & Fluff

* [ ] **Performance:**
    * [ ] Switch JSON parser to `RapidJSON` (or similar high-performance lib).
    * [ ] Decrease usage of `std::string` in UI, LSP, and warnings.
    * [ ] Also for vectors into managed memory especially for completions/lsp-stuff.

