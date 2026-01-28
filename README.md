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

Some other dependancies are added as submodules or copied.<br>
   - `unicode_width` is compiled by the makefile so nothing to do there.<br>
   - `libgrapheme` needs to be compiled using `make` in it's folder.<br>
   - ``

#### LSPs

Lsp's are defined in the `libcrib.rb` file and you can use your `config/main.rb` file to add more.<br>

The following lsp's are added by default and can be installed anywhere in your `$PATH`<br>

* [clangd](https://clangd.llvm.org/)
* [ruby-lsp](https://shopify.github.io/ruby-lsp/)
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
> It should work even if the lsp is not installed but lsp features will not work.<br>

#### Compiler

`clang++` should work fine but `c++23+` is required.<br>
Can remove `ccache` if you want from the makefile.<br>

#### Compliling

```bash
make release
```

### Running

Preferably add the `bin` folder to PATH or move `bin/crib` to somewhere in PATH.<br>
For some LSP's to work properly `crib` needs to be run from the root folder of the project. *To be fixed*<br>
then do -<br>

```bash
crib ./filename.ext
```

*If `filename.ext` does not exist, it will be created*<br>

## Keybindings

TODO: add keybind information on how to set in `config/main.rb`
      and default / unchangeable keybinds

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
- all instances of current word under cursor highlighting

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
- hover docs
- formatting support
    - Full file formatting on save
    - Ontype formatting when inserting special characters defined by the language server
    - *(few lsp's actually support this - try to configure a few more which can but need configuration and for others need to add support for external formatters)*
- A list of some lsp's can be found [here](#lsps).
- Any lsp can be added to the `config/main.rb` file.
- Though not all might work well. Open an issue if you find a lsp that doesn't work well.

**A lot lot more to come**
