Copyright 2025 Syed Daanish

# Crib - a text editor

### About

Crib is a TUI based text editor built primaririly for personal use.<br>
Crib has a vim-style editor modes system but navigation and shortcuts are very different.<br>
It supports tree-sitter based text highlighting.<br>
And LSP for auto-completion, diagnostics, hover docs etc.<br>
It aims to be complete general purpose IDE.<br>
(It is still very much a work in progress so a lot of things may seem incomplete)<br>
For now it is just a single file editor. I plan to add a multi-file support with file pickers and tabs soon.<br>

## Building

### Get started

Make sure the repo is cloned with submodules to get most of the dependencies.

```bash
git clone --recurse-submodules https://git.syedm.dev/SyedM/crib.git
```

### Dependencies

#### System-wide libraries

Make sure to install [nlohmann/json](https://github.com/nlohmann/json) from your package manager
(it should be available in the compiler as the header `nlohmann/json.hpp`) and to also have libmagic installed --
`#include <magic.h>` should work. And finally for [pcre2](https://github.com/PCRE2Project/pcre2): `#include <pcre2.h>`<br>

It also uses `xclip` at runtime for copying/pasting.
And any modern terminal should work fine - preferably `kitty` or `wezterm`.<br>

#### `./libs` folder

Some other dependancies like `libgrapheme` and `tree-sitter*` and `unicode_width` are added as submodules or copied.<br>
`unicode_width` is compiled by the makefile so nothing to do there.<br>
`libgrapheme` needs to be compiled using `make` in it's folder.<br>
`tree-sitter` needs to be compiled using `make` in it's folder.<br>
For other tree-sitter grammars, run `make` in their folders except some for which `npm install` needs to be used (see their README.md)<br>
For any problems with `npm install` make sure to have older versions of node installed.<br>
For some even manual clang or gcc compilation may be required.<br>
*TODO: Make a detailed list of how to do compile each*<br>

#### LSPs

The following lsp's are supported and can be installed anywhere in your `$PATH`<br>

* **clangd** — [https://clangd.llvm.org/installation.html](https://clangd.llvm.org/installation.html)
* **solargraph** — [https://solargraph.org/](https://solargraph.org/)
* **bash-language-server** — [https://github.com/bash-lsp/bash-language-server](https://github.com/bash-lsp/bash-language-server)
* **vscode-css-language-server** — [https://github.com/microsoft/vscode-css-languageservice](https://github.com/microsoft/vscode-css-languageservice)
* **vscode-json-language-server** — [https://github.com/microsoft/vscode-langservers-extracted](https://github.com/microsoft/vscode-langservers-extracted)
* **fish-lsp** — [https://github.com/fisho/fish-language-server](https://github.com/fisho/fish-language-server)
* **gopls** — [https://pkg.go.dev/golang.org/x/tools/gopls](https://pkg.go.dev/golang.org/x/tools/gopls)
* **haskell-language-server** — [https://github.com/haskell/haskell-language-server](https://github.com/haskell/haskell-language-server)
* **emmet-ls** — [https://github.com/emmetio/emmet‑ls](https://github.com/emmetio/emmet‑ls)
* **typescript-language-server** — [https://github.com/typescript-language-server/typescript-language-server](https://github.com/typescript-language-server/typescript-language-server)
* **lua-language-server** — [https://github.com/LuaLS/lua-language-server](https://github.com/LuaLS/lua-language-server)
* **pyright-langserver** — [https://github.com/microsoft/pyright](https://github.com/microsoft/pyright)
* **rust-analyzer** — [https://github.com/rust-analyzer/rust-analyzer](https://github.com/rust-analyzer/rust-analyzer)
* **intelephense** — [https://github.com/bmewburn/vscode-intelephense](https://github.com/bmewburn/vscode-intelephense)
* **marksman** — [https://github.com/christianchiarulli/marksman](https://github.com/christianchiarulli/marksman)
* **nginx-language-server** — [https://github.com/nginx/nginx-language-server](https://github.com/nginx/nginx-language-server)
* **taplo** — [https://github.com/taplo‑lang/taplo](https://github.com/taplo‑lang/taplo)
* **yaml-language-server** — [https://github.com/redhat-developer/yaml-language-server](https://github.com/redhat-developer/yaml-language-server)
* **sqls** — [https://github.com/lighttiger2505/sqls](https://github.com/lighttiger2505/sqls)
* **make-language-server** — [https://github.com/make-langserver/make-language-server](https://github.com/make-langserver/make-language-server)

> As it is still in development, some of these may not work as expected or that well.
> But for c/ruby/lua/python it should work fine (I test more with these).
> It should work even if the lsp is not installed but lsp features will not work.
> See `include/config.h` & `include/ts/decl.h` if you want to add your own lsp and/or tree-sitter grammar.<br>

#### Compiler

`g++` and `clang++` should both work fine.
The makefile has been set to use g++ if made with `make -j test` and clang++ if made with `make -j release`<br>
This can be changed but I have found clang++ builds to be slightly faster - also test builds do not have the flags needed to be used system wide or any optimizations.<br>

#### Compliling

```bash
make -j release
```

### Running

Preferably add `bin` folder to PATH or move `bin/crib` to somewhere in PATH.<br>
But make sure that `scripts/` and `grammar/` are at `../` relative to the binary or it will crash.<br>
`scripts/init.sh` and `scripts/exit.sh` can be used to add hooks to the editor on startup and exit
(Make sure to remove my `kitty` hooks from them if you want).<br>
For some LSP's to work properly `crib` needs to be run from the root folder of the project. *To be fixed*<br>
then do -<br>

```bash
crib ./filename.ext
```

*If `filename.ext` does not exist, it will fail to load the editor - use `touch filename.ext` to create it - to be fixed*<br>
*Try out with files in `samples/`*<br>

## Keybindings

### Mouse Interactions

These interactions work globally or generally across the editor canvas.

| Action | Function |
| --- | --- |
| **Scroll Up/Down** | Scrolls the view. |
| **Scroll Left/Right** | Moves the cursor left or right. |
| **Left Click (Press)** | Moves cursor to position; resets selection. |
| **Left Click (Double)** | Selects the **word** under the cursor (enters SELECT mode). |
| **Left Click (Triple)** | Selects the **line** under the cursor (enters SELECT mode). |
| **Left Click (Drag)** | Selects text (Character, Word, or Line based on initial click type). |
| **Left Click (Release)** | If cursor and selection start are the same, returns to NORMAL mode. |

### Navigation (Global / Special Keys)

These keys work primarily in Normal mode but handle movement logic.

| Key | Modifier | Function |
| --- | --- | --- |
| **Arrows** (Up/Down/Left/Right) | None | Move cursor 1 step in that direction. |
| **Arrows** (Up/Down) | `CTRL` | Move cursor **5 steps** in that direction. |
| **Arrows** (Left/Right) | `CTRL` | Jump to the previous/next **word boundary**. |
| **Arrows** (Up/Down) | `ALT` | **Move the current line** Up or Down. |
| **Arrows** (Left/Right) | `ALT` | Move cursor **8 steps** in that direction. |

### NORMAL Mode

This is the default navigation and command mode.

| Key | Function |
| --- | --- |
| **i** | Enter **INSERT** mode (at current position). |
| **a** | Enter **INSERT** mode (append: moves cursor right by 1 first). |
| **s** or **v** | Enter **SELECT** mode (start character selection). |
| **:** or **;** | Enter **RUNNER** mode (Command Bar). |
| **u** | Select the **last line** of the file (Jumps to bottom). |
| **h** | Trigger **LSP Hover** information for the symbol under cursor. |
| **Ctrl + h** | Scroll the hover window **Up**. |
| **Ctrl + l** | Scroll the hover window **Down**. |
| **Ctrl + s** | **Save** the file. |
| **Ctrl + d** | Scroll Page **Down** (1 unit). |
| **Ctrl + u** | Scroll Page **Up** (1 unit). |
| **p** | **Paste** from clipboard at cursor position (moves cursor to end of paste). |
| **>** or **.** | **Indent** the current line. |
| **<** or **,** | **Dedent** (un-indent) the current line. |
| **Space** | Move cursor Right. |
| **Backspace** (`0x7F`) | Move cursor Left. |
| **Enter** (`\n`, `\r`) | Move cursor Down. |
| **\| or \\** | Move cursor Up. |
| **n** | Enter **JUMPER** mode (Set Bookmark). |
| **m** | Enter **JUMPER** mode (Jump to Bookmark). |
| **N** | Clear specific Jumper hook (logic attempts to clear hook at current line). |

### INSERT Mode

Used for typing text.

| Key | Function |
| --- | --- |
| **Esc** (`0x1B`) | Return to **NORMAL** mode. |
| **Tab** (`\t`) | Inserts 2 spaces. |
| **Enter** | Inserts newline + **Auto-indents** based on previous line/context. |
| **Backspace** | Deletes previous character or auto-closes empty pairs (e.g., `{ |
| **Ctrl + w** | **Delete Previous Word**. |
| **Del** | Delete character under cursor. |
| **Ctrl + Del** | Delete **Next Word**. |
| **Typing** | Inserts characters. |
| **Ctrl + Shift + v or as configured in your terminal** | System pasting. |
| **{ ( [ " '** | Auto-inserts closing pair (e.g., typing `{` inserts `{}`). |
| **} ) ] " '** | If the next char matches the typed char, skip insertion (overwrite), otherwise insert. |

#### Autocompletion (Inside Insert Mode)

These function only if LSP and completion are active.

| Key | Function |
| --- | --- |
| **Ctrl + p** | Select **Next** completion item. |
| **Ctrl + o** | Select **Previous** completion item. |
| **Ctrl + \\** | **Accept** selected completion OR trigger new completion request. |
| **Trigger Chars** | (e.g., `.`, `>`) Automatically triggers completion popup. |

### SELECT Mode

Used for highlighting text.

| Key | Function |
| --- | --- |
| **Esc**, **s**, **v** | Cancel selection and return to **NORMAL** mode. |
| **y** | **Yank (Copy)** selection to clipboard → Return to Normal. |
| **x** | **Cut** selection to clipboard → Return to Normal. |
| **p** | **Paste** over selection (Replace text) → Return to Normal. |
| **f** | **Fold** the selected range (collapses code) → Return to Normal. |

### JUMPER Mode

This mode uses a bookmarking system mapped to keyboard characters.

* **Entered via `n` (Set Mode):**
* Pressing any key `!` through `~` assigns the current line number to that key.

* **Entered via `m` (Jump Mode):**
* Pressing any key `!` through `~` jumps the cursor to the line previously assigned to that key.

### RUNNER Mode (Command Bar)

Activated by `:` or `;`.

| Key | Function |
| --- | --- |
| **Esc** | Cancel and return to **NORMAL** mode. |
| **Enter** | Execute the typed command. |
| **Left / Right** | Move cursor within the command bar. |
| **Up / Down** | Intended for command history. (Not implemented) |
| **Typing** | Insert characters into the command bar. (Not implemented) |

## Features Implemented

#### Core workflow:
- NORMAL / INSERT / SELECT / RUNNER / JUMPER modes
- full mouse support for scrolling and multi-click word/line selection.
    - Double click to select word
    - Triple click to select line

#### Core editing tools:
- indent/dedent
- move lines up/down
- folding on a selected range
- yank/cut/paste via system clipboard
- auto-indent on new lines
- bracket/quote auto-pairing
- hooks jumping (bookmarking)

#### Tree-sitter syntax highlighting and filetype detection (using extention or libmagic) for:
- bash
- c/cpp (and headers)
- css
- fish
- go/gomod
- haskell
- html/erb
- javascript
- typescript/tsx
- json/jsonc
- ruby
- lua
- python
- rust
- php
- markdown
- nginx
- toml
- yaml
- sql
- make
- gdscript
- man pages
- diff/patch
- gitattributes/gitignore
- tree-sitter queries
- regex
- ini

#### LSP-powered features:
- diagnostics
- autocompletion
- hover docs (with markdown support)
- A list of all supported lsp's can be found [here](#LSPs).

**A lot lot more to come**
