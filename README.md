Copyright 2025 Syed Daanish

# Crib

A TUI IDE.

# TODO

- [ ] FIX: bug where `move_line_up/down` dont honor folding.
    - idk .. wyyy but it is soo hard. somehow handle \n seperately without interfering with folding.

- [ ] Do this thing where folds are properly shifted on newline addition / deletion.
- [ ] Add support for brackets/quotes to auto-close. (also for backspace)
    - it also doesnt actually add a closer if it exists and is matched.
- [ ] Add feature where doing enter uses tree-sitter to add newline with indentation.
    - it should also put stuff like `}` on the next line.
- [ ] Add the highlight of block edges when cursor is on a bracket (or in).
- [ ] Add this thing where selection double click on a bracket selects whole block.
    - (only on the first time) and sets mode to `WORD`.
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
