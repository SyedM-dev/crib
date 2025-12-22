Copyright 2025 Syed Daanish

# Crib

A TUI IDE.

# TODO

- [ ] Add a virtual text support (text that is rendered in the editor but not in the actual text)
    - Add a whitespace highlighter (nerd font). for spaces and tabs at start/end of line. as a test.
- [ ] Add support for LSP & autocomplete / snippets.
    - First research
        - `textDocument/documentHighlight` - for highlighting stuff (probably tree-sitter is enough)
        - `textDocument/selectionRange` //
        - `textDocument/completion` - Obviously
        - `textDocument/onTypeFormatting` - seems promising for auto formatting (indentation etc)
        - `textDocument/inlayHint` & `textDocument/inlineHint` & `textDocument/codeLens`
        - `textDocument/formatting` & `textDocument/rangeFormatting`
        - `textDocument/semanticTokens/*` (probably tree-sitter is enough)
        - `textDocument/linkedEditingRange` - probably useful
        - `textDocument/foldingRange` - i will never use this for folding but it might be useful for other things.
        - `textDocument/rename` & `textDocument/prepareRename` - probably useful
        - And a lot more (just go through each for `clangd` and then expand to say `solargraph`).
    - Make a universal plug for lsp. So focus more on making a general purpose solid communication interface. Instead of something specific.
        - With a 4ish pass system. (more like each returned value from the lsp is used in 4 ways)
            1. One for stuff like jump to x position. or rename symbol x to y. (stuff that explicitly requires user request to do something)
                - Maybe even hover goes here
            2. One for stuff that only affects highlighting and styles . like symbol highlighting etc.
            3. One for Warnings/errors and inlay hints etc. (stuff that adds virtual text to the editor)
            4. One for fromatting and stuff like that. (stuff that edits the buffer text)
- [ ] Add snippets from wherever i get them. (like luasnip or vsnip)
- [ ] Add this thing where select at end of screen scrolls down. (and vice versa)
    - Can be acheived by updating `main.cc` to send drag events to the selected editor instead of just under cursor.
    - Then a drag event above or below will scroll the selected editor.
- [ ] Add support for virtual cursor where edits apply at all the places.
- [ ] Add alt + click to set multiple cursors.
- [ ] Add search / replace along with search / virtual cursors are searched pos.
- [ ] Add support for undo/redo.
- [ ] Add `.scm` files for all the supported languages. (2/14) Done.
- [ ] Add splash screen / minigame jumping.
- [ ] Add codeium/copilot support.
- [ ] Normalize / validate unicode on file open.
- [ ] Add git stuff.
- [ ] Add SQL support. (viewer and basic editor)
- [ ] Add color picker/palette for hex or other css colors.
- [ ] Fix bug where alt+up at eof adds extra line.
- [ ] Think about how i would keep fold states sensical if i added code prettying/formatting.
- [ ] Use tree-sitter to get the node path of the current node under cursor and add an indicator bar. 
    - (possibly with a picker to jump to any node)
- [ ] Add the highlight of block edges when cursor is on a bracket (or in). (prolly from lsp)
- [ ] Add this thing where selection double click on a bracket selects whole block.
    - (only on the first time) and sets mode to `WORD`.
- [ ] Redo folding system and its relation to move_line_* functions. (Currently its a mess)
