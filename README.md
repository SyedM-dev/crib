Copyright 2025 Syed Daanish

# Crib

A TUI IDE.

# TODO

- [ ] Add `hooks` in files that can be set/unset/jumped to.
- [ ] Add folding support at tree-sitter level (basic folding is done).
- [ ] Add feature where doing enter uses tree-sitter to add newline with indentation.
    - it should also put stuff like `}` on the next line.
- [ ] Add this thing where selection double click on a bracket selects whole block.
    - (only on the first time) and sets mode to WORD.
- [ ] Add the highlight of block edges when cursor is on a bracket (or in).
- [ ] Add support for brackets/quotes to auto-close. (also for backspace)
- [ ] Add support for virtual cursor where edits apply at all the places.
- [ ] Add search / replace along with search / virtual cursors are searched pos.
- [ ] Add alt + click to set multiple cursors.
- [ ] Add support for undo/redo.
- [ ] Add `.scm` files for all the supported languages. (2/14) Done.
- [ ] Add splash screen / minigame jumping.
- [ ] Add support for LSP & autocomplete / snippets.
- [ ] Add codeium/copilot support.
- [ ] Normalize / validate unicode on file open.
- [ ] Add git stuff.
- [ ] Fix bug where alt+up at eof adds extra line.
