Copyright 2025 Syed Daanish

# Crib - a text editor

### About

Crib is a TUI based text editor built primaririly for personal use.<br>
Crib has a vim-style editor modes system but navigation and shortcuts are very different.<br>
It supports superfast incremental syntax highlighting.<br>
And LSP for auto-completion, diagnostics, hover docs etc.<br>
It aims to be complete general purpose IDE.<br>
(It is still very much a work in progress so a lot of things may seem incomplete)<br>
For now it is just a single file editor. I plan to add a multi-file support with file pickers and tabs soon.<br>

## Building

### Get started

Make sure the repo is cloned with submodules to get `libgrapheme`.

```bash
git clone --recurse-submodules https://git.syedm.dev/SyedM/crib.git
```

### Dependencies

#### System-wide libraries

Make sure you have the following dependencies installed (apart from the standard C++ libraries):

* **[nlohmann/json](https://github.com/nlohmann/json)**
  Install it via your package manager. Once installed, the header should be available as:
  ```cpp
  #include <nlohmann/json.hpp>
  ```

* **[PCRE2](https://github.com/PCRE2Project/pcre2)**
  Install the library to use its headers:
  ```cpp
  #include <pcre2.h>
  ```

* **libmagic**
  Install it so that you can include it in your code (most *nix systems have it installed):
  ```cpp
  #include <magic.h>
  ```

It also uses `xclip` at runtime for copying/pasting *(TODO: make it os portable)*.
And any modern terminal should work fine - preferably `kitty` or `wezterm`.<br>

#### `./libs` folder

Some other dependancies like `libgrapheme` and `unicode_width` are added as submodules or copied.<br>
    `unicode_width` is compiled by the makefile so nothing to do there.<br>
    `libgrapheme` needs to be compiled using `make` in it's folder.<br>

#### LSPs

The following lsp's are supported and can be installed anywhere in your `$PATH`<br>

* [clangd](https://clangd.llvm.org/)
* [solargraph](https://solargraph.org/)
* [bash-language-server](https://github.com/bash-lsp/bash-language-server)
* [vscode-css-language-server](https://github.com/hrsh7th/vscode-langservers-extracted)
* [vscode-json-language-server](https://github.com/hrsh7th/vscode-langservers-extracted)
* [fish-lsp](https://github.com/ndonfris/fish-lsp)
* [gopls](https://pkg.go.dev/golang.org/x/tools/gopls)
* [haskell-language-server](https://github.com/haskell/haskell-language-server)
* [emmet-language-server](https://github.com/olrtg/emmet-language-server)
* [typescript-language-server](https://github.com/typescript-language-server/typescript-language-server)
* [lua-language-server](https://github.com/LuaLS/lua-language-server)
* [pyright-langserver](https://github.com/microsoft/pyright)
* [rust-analyzer](https://github.com/rust-analyzer/rust-analyzer)
* [intelephense](https://intelephense.com/)
* [marksman](https://github.com/artempyanykh/marksman)
* [nginx-language-server](https://github.com/pappasam/nginx-language-server)
* [taplo](https://taplo.tamasfe.dev/)
* [yaml-language-server](https://github.com/redhat-developer/yaml-language-server)
* [sqls](https://github.com/sqls-server/sqls)
* [make-language-server](https://github.com/Freed-Wu/autotools-language-server)

> As it is still in development, some of these may not work as expected or that well.<br>
> But for c/ruby/lua/python it should work fine (I test more with these).<br>
> It should work even if the lsp is not installed but lsp features will not work.<br>
> See `include/config.h` & `include/ts/decl.h` if you want to add your own lsp and/or tree-sitter grammar.<br>

#### Compiler

`g++` and `clang++` should both work fine but `c++20+` is required.
The makefile uses `clang++` by default.<br>
Can remove `ccache` if you want from the makefile.<br>

#### Compliling

```bash
make release
```

### Running

Preferably add the `bin` folder to PATH or move `bin/crib` to somewhere in PATH.<br>
But make sure that `scripts/` are at `../` relative to the binary or it will crash.<br>
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
| **Backspace** | Deletes previous character or auto-collapses empty pairs (e.g., `{` -> `}`). |
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
- per-language smart auto-indent on new line insert
- bracket/quote auto-pairing
- hooks jumping (bookmarking)
- color hex code highlighting
- current line highlighting
<!-- - TODO: current word under cursor highlighting -->

#### syntax highlighting and filetype detection (using extention or libmagic) for:
- ruby
<!-- TODO: -->
<!-- - bash -->
<!-- - c/cpp (and headers) -->
<!-- - css -->
<!-- - fish -->
<!-- - go/gomod -->
<!-- - haskell -->
<!-- - html/erb -->
<!-- - javascript -->
<!-- - typescript/tsx -->
<!-- - json/jsonc -->
<!-- - lua -->
<!-- - python -->
<!-- - rust -->
<!-- - php -->
<!-- - markdown -->
<!-- - nginx -->
<!-- - toml -->
<!-- - yaml -->
<!-- - sql -->
<!-- - make -->
<!-- - gdscript -->
<!-- - man pages -->
<!-- - diff/patch -->
<!-- - gitattributes/gitignore -->
<!-- - tree-sitter queries -->
<!-- - regex -->
<!-- - ini -->

#### LSP-powered features:
- diagnostics
- autocompletion
- hover docs (with markdown support)
- formatting support
    - Full file formatting on save
    - Ontype formatting when inserting special characters defined by the language server
    - *(few lsp's actually support this - try to configure a few more which can but need configuration and for others need to add support for external formatters)*
- A list of all supported lsp's can be found [here](#lsps).

**A lot lot more to come**
