Copyright 2025 Syed Daanish

# Crib

A TUI IDE.

# TODO

- [ ] Fix problem highlighting selection when line is empty.
- [ ] Add mouse support.
- [ ] Add struct as union of structs that can take input. or similar
    - then send input to them.
    - Also background thread will run worker function for whatever struct is selected.
    - And maybe even a list of active editors etc?
- [ ] Add ui api for setting cursor types.
- [ ] Use that api to set cursor for selection/insertion/normal etc properly.
    - Sorta unrelated but check kutuwm + kitty problem not keeping cursor mode properly.
- [ ] Add line numbers.
- [ ] Add bg highlight for current line.
- [ ] Make function to get selected text. (selection itself is done)
- [ ] Add support for copy/cut/paste.
- [ ] Add support for ctrl + arrow key for moving words.
- [ ] Add support for ctrl + backspace / delete for deleting words.
- [ ] Add underline highlight for current word and all occurences.
- [ ] Add `hooks` in files that can be set/unset/jumped to.
- [ ] Add folding support at tree-sitter level (basic folding is done).
- [ ] Add feature where doing enter uses tree-sitter to add newline with indentation.
    - it should also put stuff like `}` on the next line.
- [ ] Add support for brackets/quotes to auto-close.
- [ ] Add support for virtual cursor where edits apply at all the places.
- [ ] Add search / replace along with search / virtual cursors are searched pos.
- [ ] Add support for undo/redo.
- [ ] Add `.scm` files for all the supported languages. (2/14) Done.
- [ ] Add support for LSP & autocomplete / snippets.
- [ ] Add codeium/copilot support.
- [ ] Add git stuff.
- [ ] Add splash screen / minigame jumping.
