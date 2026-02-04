Copyright 2025 Syed Daanish

# TODO

# memory usage for debug build (release build will be smaller by about 25%)
```
8K   ->    13.2M
128K ->    13.2M (expected worst case 16.6M)
128M ->   412.0M (expected worst case 2.3G)
```

* Next few super long boring things to do
* redo lsp threads such that no mutex needed for any rope stuff (necessary rn)
    - Also make the classes own the methods in lsp
    - This will mean that parsers/renderers and keystrokes will not need to be individually locked
    - And so it will be much faster
    - At which point the main thread can also be blocked on user input or lsp responses and still be fast
* Add a superclass for editor called Window (which can be popup or tiled) (done)
* Add a recursive tiling class for windows (done)
* Handled by a single renderer that calls and renders each window (done)
* Make editor's functions into its own methods (classify it) (done)
    - While at it
        - Seperate system functions into a class that branches to support local / ssh / server modes.
        - Even lsp shouldnt be directly controlled because it can branch on local and server modes
            - check wolfSSH stuff for remote editing
    - Redo hooks as a engine of its own.
    - And factorize renderer into its own class (and make it just return an array of the render without knowing teh x,y)
        - which is just managed by the renderer
    - which is reused by scrollers/ensurers too
    - this will then allow inlay hints to be possible
    - and also make VAI easier to implement
* Allow keybinds to be set in ruby

* then the fun part:
* Then allow ruby code to create and handle windows as extentions
* Then 3 inbuilt extentions being file manager, theme picker, tab selector

* Extentions can also be used as file openers (for like databases . diffing . images . audio etc)
* Local cache for state management (like undo history, cursor positions etc) (location can be set in config)
* make sure to write inbuilt extentions in cpp and not ruby
* also move default bar and clipboard back into cpp
* all lsp popups are no longer their own classes but instead windows (extention like) in popup mode

* skip opening binary files
* apply themeing in bg log bar lsp popus etc. to keep visual consistency
* searching/replace/Multi-Cursor (for just one lsp command for all) with pcre2 with regex (started by a slash) (disabled for large files)
* add links support in xterm (kitty like clickable links)
* And preprocess markdown in popups to be more like styled than just highlighted
* In the ruby libcrib populate toplevel binding and file and dir and rubybuild stuff and other similar constants
* in require_relative allow requiring without appending .rb if possible.
* Possibly in the future limit memory usage by parser for larger files
* Add a file picker suggestion while typing a path (including stuff like ~ and .. and $HOME etc)
* allow opening directory after filemanger is sorted out.
* commands to:
    change pwd
    load a rb file
    run a ruby command
    close a window etc.
* [ ] Add mgems for most common things and a ruby library to allow combining true ruby with mruby
* add command to set and use a file type at runtime
* [ ] color alpha in ini files
* [ ] Make warning before ctrl+q for saving
* [ ] **LSP Bug:** Check why `fish-lsp` is behaving so off with completions filtering.
* [ ] **Line move:** fix the move line functions to work without the calculations from folds as folds are removed.
* [ ] **Editor Indentation Fix:** - Main : merger indentation with the parser for more accurate results.
    * [ ] Ignore comments/strings from parser when auto-indenting.
    * [ ] Support for stuff like bash \ and math operators in other languages and comma and line starting with a dot (like in ruby)
          etc.
* [ ] **Readme:** Update readme to show ruby based config in detail.
* [ ] **UI Refinement:**
    * [ ] Finish autocomplete box style functions.
* [ ] **Documentation UI:** Capture `Ctrl+h` / `Ctrl+l` for scrolling documentation windows.
* Cap line_tree data limit for large files and just store about a thousand previous lines maybe? (lazily loaded)

* add `:j<n>` command to jump to line \<n> in the current file
* and give warning for invalid commands
* and upon escape clear the current command
* allow multiline logging which captures the input entirely and y will copy the log and anything else will exit
* it will then collapse to being the first line from the log only

* allow flushing functions in ruby to tell c++ to refresh keybinds/themes etc.

* [ ] **LSP:**
    support snippets in completion properly
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

* Allow ruby to config lsp capabilities

* also try to fix why solargraph is the only one breaking on edits near emojis
* ruby-lsp also supports erb so thats a plus

* the ruby should have an api to be able to draw windows and add mappings to them

* **Syntax highlighting**
* ruby done!!
* finish bash then do all the directive-like ones like jsonc (first to help with theme files) / toml / yaml / ini / nginx
* then [ch](++)? then gdscript and python then erb then php
* then markdown / html
* then gitignore / gitattributes
* then fish then sql then css and [jt]sx? then lua (make with type annotations for lsp results)
* then haskell then gomod then go then rust

* [ ] **Undo/Redo:** Add support for undo/redo history.

* [ ] **Auto brace selection:** Add support for auto brace selection.

* [ ] **Tree-sitter Indent:** Attempt to allow Tree-sitter to handle indentation if possible.

### UX

* [ ] **Completion Filtering:**
    * [ ] Stop filtering case-sensitive.
    * [ ] Normalize completion edits if local filtering is used.

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
    * [ ] Double-clicking a bracket selects the whole block and sets mode to `WORD`.

### Visuals, UI & Extensions?

* [ ] Add color picker/palette (as a floating extention).

* [ ] **Git:** Add Git integration (status, diffs).
* [ ] **AI/Snippets:**
    * [ ] Add snippets support (LuaSnip/VSnip style).
    * [ ] Add Codeium/Copilot support (using VAI virtual text) as a test phase.

* [ ] **SQL:** Add SQL support (Viewer and Basic Editor) (as ruby extension).
* [ ] **Prolly?:** Add Splash Screen / Minigame.


### Unimportant optimizations

* [ ] **Performance:**
    * [ ] Switch JSON parser to `RapidJSON` (or similar high-performance lib).
    * [ ] Decrease usage of `std::string` in UI, LSP, warnings etc.
    * [ ] Also for vectors into managed memory especially for completions/lsp-stuff.
