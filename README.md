Copyright 2025 Syed Daanish

# Crib

A TUI IDE.

# TODO

- [ ] Add support for virtual cursor where edits apply at all the places.
- [ ] Add alt + click to set multiple cursors.
- [ ] Add search / replace along with search / virtual cursors are searched pos.
- [ ] Add support for undo/redo.
- [ ] Add `.scm` files for all the supported languages. (2/14) Done.
- [ ] Add splash screen / minigame jumping.
- [ ] Add support for LSP & autocomplete / snippets.
- [ ] Add codeium/copilot support.
- [ ] Normalize / validate unicode on file open.
- [ ] Add git stuff.
- [ ] Fix bug where alt+up at eof adds extra line.
- [ ] Think about how i would keep fold states sensical if i added code prettying.

- [ ] Retry get proper blocks from tree-sitter.
    - And use it for full block selection (including inline ones).
    - And for indenting.
    - And highlighting block edges etc.
- [ ] Add feature where doing enter uses tree-sitter to add newline with indentation.
    - it should also put stuff like `}` on the next line.
- [ ] Add the highlight of block edges when cursor is on a bracket (or in).
- [ ] Add this thing where selection double click on a bracket selects whole block.
    - (only on the first time) and sets mode to `WORD`.
- [ ] Redo folding system and its relation to move_line_* functions.
- [ ] Also try regex based indentation.
- [ ] And indentation based blocks
