Copyright 2025 Syed Daanish

# TODO

# memory usage for debug build (release build will be smaller by about 25%)
```
8K   ->    13.2M
128K ->    13.2M (expected worst case 16.6M)
128M ->   412.0M (expected worst case 2.3G)
```

##### BTW Check each lsp with each of the features implemented

* Possibly in the future limit memory usage by parser for larger files
* Also redo lsp threads such that no mutex needed for any rope stuff
    - This will mean that parsers/renderers and keystrokes will not need to be individually locked
    - And so it will be much faster
    - While at it also make the lsp instead of a single thread be a pool of threads blocking on their stdio
    - So one lsp being slower wont affect others and fps based reading wont be necessary saving cpu
    - At which point the main thread can also be blocked on user input or lsp responses and still be fast
* [ ] Add mgems for most common things and a ruby library to allow combining true ruby with mruby
* add command to set and use a file type at runtime
* [ ] color alpha in ini files
* [ ] Make warning before ctrl+q for saving
* [ ] **LSP Bug:** Check why `fish-lsp` is behaving so off with completions filtering.
* [ ] **Line move:** fix the move line functions to work without the calculations from folds as folds are removed.
* [ ] **Editor Indentation Fix:** - Main : merger indentation with the parser for more accurate results.
    * [ ] Ignore comments/strings from parser when auto-indenting.
* [ ] **Readme:** Update readme to show ruby based config in detail.
* [ ] **UI Refinement:**
    * [ ] Finish autocomplete box style functions.
* [ ] **Documentation UI:** Capture `Ctrl+h` / `Ctrl+l` for scrolling documentation windows.
* [ ] Redo hooks as a struct.
* [ ] breakdown the render function into smaller functions.
    - Might allow for VAI integration easier

* Try to make all functions better now that folds have been purged
* Cleanup syntax and renderer files

* add `:j<n>` command to jump to line \<n> in the current file
* and many more stuff like ruby execution
* and give warning for invalid commands
* and upon escape clear the current command
* allow multiline logging which captures the input entirely and y will copy the log and anything else will exit
* it will then collapse to being the first line from the log only

* **RN** Add a thing called view which is a rect with speacial type editor . but otherwise a ruby or c++ view
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

