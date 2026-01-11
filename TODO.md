Copyright 2025 Syed Daanish

# TODO

### Critical Fixes

* [ ] **Critical Crash:** Fix bug where closing immediately while LSP is still loading hangs and then segfaults (especially on slow ones like fish-lsp).
* [ ] **Navigation Bug:** Fix bug where `Alt+Up` at EOF adds an extra line.
* [ ] **LSP Bug:** Check why `fish-lsp` is behaving so off with completions filtering.
* [ ] **File IO:** Normalize/validate unicode on file open (enforce UTF-8, handle other types gracefully).


### Core Editing Mechanics

* [ ] **Undo/Redo:** Add support for undo/redo history.
* [ ] **Indentation Engine (Major Task):**
    * **Startup:**
        1. Scan file: Check for lines with 2+ spaces. Least count = indent. If tabs, use tabs.
        2. Fallback: Use table of file types or default to 2 spaces.
        3. Store as: `1 = tab`, `2+ = n spaces`.
        4. Apply: Use this for indent/unindent actions.
        5. Newline: Follow indent of previous line immediately (ignore default).
    * **Indent/Unindent:**
        * Add support for indent/unindent actions.
        * that use indentation of previous line that is not comment or string or whitespace/blank.
        * and try auto indent one level extra if previous line ends with colon or bracket start.
        * and dedent one level extra if previous line ends with bracket end.
    * **Newline:** Add support for newline actions similar to indent.

* [ ] **Tree-sitter Indent:** Attempt to allow Tree-sitter to handle indentation if possible.

* [ ] **Scrolling:** Add logic where selecting at the end of the screen scrolls down (and vice versa).
    * *Implementation:* Update `main.cc` to send drag events to the selected editor.

* [ ] **Documentation UI:** Capture `Ctrl+h` / `Ctrl+l` for scrolling documentation windows.


### UX

* [ ] **Editor word highlighter:** Do not recompute word under cursor if not changed.

* [ ] **Completion Filtering:**
    * [ ] Stop filtering case-sensitive.
    * [ ] Normalize completion edits if local filtering is used.

* [ ] **UI Refinement:**
    * [ ] Allow completion list to be scrolled; show only `x` max items.
    * [ ] Finish autocomplete box style functions.

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

* [ ] **Folding:** Redo folding system and its relation to `move_line_*` functions.

* [ ] **Grammars:**
    * [ ] Manually add redo SCM files (especially cpp/c/h).
    * [ ] Create `lua-typed` and `man pages` Tree-sitter grammars.

* [ ] **Repo Maintenance:** Once renderer is proven check commit `43f443e`.
