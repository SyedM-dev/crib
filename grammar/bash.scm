;; #BFBDB6 #000000 0 0 0 1
[
  "("
  ")"
  "{"
  "}"
  "["
  "]"
  "[["
  "]]"
  "(("
  "))"
] @punctuation.bracket

;; #BFBDB6 #000000 0 0 0 1
[
  ";"
  ";;"
  ";&"
  ";;&"
  "&"
] @punctuation.delimiter

;; #F29668 #000000 0 1 0 1
[
  ">"
  ">>"
  "<"
  "<<"
  "&&"
  "|"
  "|&"
  "||"
  "="
  "+="
  "=~"
  "=="
  "!="
  "&>"
  "&>>"
  "<&"
  ">&"
  ">|"
  "<&-"
  ">&-"
  "<<-"
  "<<<"
  ".."
  "!"
] @operator

;; #AAD94C #000000 0 0 0 1
[
  (string)
  (raw_string)
  (ansi_c_string)
  (heredoc_body)
] @string

;; #E6C08A #000000 0 0 0 1
[
  (heredoc_start)
  (heredoc_end)
] @label

(variable_assignment
  (word) @variable)

(command
  argument: "$" @string) ; bare dollar

(concatenation
  (word) @string)

;; #FF8F40 #000000 0 0 0 1
[
  "if"
  "then"
  "else"
  "elif"
  "fi"
  "case"
  "in"
  "esac"
] @keyword.conditional

;; #FF8F40 #000000 0 0 0 1
[
  "for"
  "do"
  "done"
  "select"
  "until"
  "while"
] @keyword.repeat

;; #FF8F40 #000000 0 0 0 1
[
  "declare"
  "typeset"
  "readonly"
  "local"
  "unset"
  "unsetenv"
] @keyword

;; #FF8F40 #000000 0 0 0 1
"export" @keyword.import

;; #FF8F40 #000000 0 0 0 1
"function" @keyword.function

;; #D2A6FF #000000 0 0 0 1
(special_variable_name) @constant

;; #D2A6FF #000000 0 0 0 1
((word) @constant.builtin
  (#match? @constant.builtin "^(SIGHUP|SIGINT|SIGQUIT|SIGILL|SIGTRAP|SIGABRT|SIGBUS|SIGFPE|SIGKILL|SIGUSR1|SIGSEGV|SIGUSR2|SIGPIPE|SIGALRM|SIGTERM|SIGSTKFLT|SIGCHLD|SIGCONT|SIGSTOP|SIGTSTP|SIGTTIN|SIGTTOU|SIGURG|SIGXCPU|SIGXFSZ|SIGVTALRM|SIGPROF|SIGWINCH|SIGIO|SIGPWR|SIGSYS|SIGRTMIN|SIGRTMIN\+1|SIGRTMIN\+2|SIGRTMIN\+3|SIGRTMIN\+4|SIGRTMIN\+5|SIGRTMIN\+6|SIGRTMIN\+7|SIGRTMIN\+8|SIGRTMIN\+9|SIGRTMIN\+10|SIGRTMIN\+11|SIGRTMIN\+12|SIGRTMIN\+13|SIGRTMIN\+14|SIGRTMIN\+15|SIGRTMAX\-14|SIGRTMAX\-13|SIGRTMAX\-12|SIGRTMAX\-11|SIGRTMAX\-10|SIGRTMAX\-9|SIGRTMAX\-8|SIGRTMAX\-7|SIGRTMAX\-6|SIGRTMAX\-5|SIGRTMAX\-4|SIGRTMAX\-3|SIGRTMAX\-2|SIGRTMAX\-1|SIGRTMAX)$"))

;; #D2A6FF #000000 0 0 0 1
((word) @boolean.true
  (#match? @boolean.true "^true$"))

;; #D2A6FF #000000 0 0 0 1
((word) @boolean.false
  (#match? @boolean.false "^false$"))

;; #99ADBF #000000 0 1 0 1
(comment) @comment @spell

;; #F29668 #000000 0 0 0 1
(test_operator) @operator

;; #7dcfff #000000 0 0 0 2
(command_substitution
  "$(" @punctuation.special
  ")" @punctuation.special)

;; #7dcfff #000000 0 0 0 2
(process_substitution
  [
    "<("
    ">("
  ] @punctuation.special
  ")" @punctuation.special)

;; #7dcfff #000000 0 0 0 2
(arithmetic_expansion
  [
    "$(("
    "(("
  ] @punctuation.special
  "))" @punctuation.special)

;; #BFBDB6 #000000 0 0 0 1
(arithmetic_expansion
  "," @punctuation.delimiter)

;; #F29668 #000000 0 0 0 1
(ternary_expression
  [
    "?"
    ":"
  ] @keyword.conditional.ternary)

;; #F29668 #000000 0 0 0 1
(binary_expression
  operator: _ @operator)

;; #F29668 #000000 0 0 0 1
(unary_expression
  operator: _ @operator)

;; #F29668 #000000 0 0 0 1
(postfix_expression
  operator: _ @operator)

;; #FFB454 #000000 0 0 0 3
(function_definition
  name: (word) @function)

;; #FFB454 #000000 0 0 0 3
(command_name
  (word) @function.call)

;; #FFB454 #000000 0 0 0 3
(command_name
  (word) @function.builtin
  (#match? @function.builtin "^(\.|\:|alias|bg|bind|break|builtin|caller|cd|command|compgen|complete|compopt|continue|coproc|dirs|disown|echo|enable|eval|exec|exit|false|fc|fg|getopts|hash|help|history|jobs|kill|let|logout|mapfile|popd|printf|pushd|pwd|read|readarray|return|set|shift|shopt|source|suspend|test|time|times|trap|true|type|typeset|ulimit|umask|unalias|wait)$"))

;; #FFFFFF #000000 0 0 0 1
(command
  argument: [
    (word) @variable.parameter
    (concatenation
      (word) @variable.parameter)
  ])

;; #FFFFFF #000000 0 0 0 1
(declaration_command
  (word) @variable.parameter)

;; #FFFFFF #000000 0 0 0 1
(unset_command
  (word) @variable.parameter)

;; #D2A6FF #000000 0 0 0 2
(number) @number

;; #D2A6FF #000000 0 0 0 2
((word) @number
  (#match? @number "^[0-9]+$"))

;; #AAD94C #000000 0 0 0 1
(file_redirect
  (word) @string.special.path)

;; #AAD94C #000000 0 0 0 1
(herestring_redirect
  (word) @string)

;; #F29668 #000000 0 0 0 1
(file_descriptor) @operator

;; #7dcfff #000000 0 0 0 2
(simple_expansion
  "$" @punctuation.special) @none

;; #7dcfff #000000 0 0 0 2
(expansion
  "${" @punctuation.special
  "}" @punctuation.special) @none

;; #7dcfff #000000 0 0 0 2
(expansion
  operator: _ @punctuation.special)

;; #7dcfff #000000 0 0 0 2
(expansion
  "@"
  .
  operator: _ @character.special)

;; #7dcfff #000000 0 0 0 2
((expansion
  (subscript
    index: (word) @character.special))
  (#any-of? @character.special "@" "*"))

;; #7dcfff #000000 0 0 0 2
"``" @punctuation.special

;; #FFFFFF #000000 0 0 0 1
(variable_name) @variable

;; #D2A6FF #000000 0 0 0 1
((variable_name) @constant
  (#match? @constant "^[A-Z][A-Z_0-9]*$"))

;; #F07178 #000000 0 0 0 1
((variable_name) @variable.builtin
  (#match? @variable.builtin "^(CDPATH|HOME|IFS|MAIL|MAILPATH|OPTARG|OPTIND|PATH|PS1|PS2|_|BASH|BASHOPTS|BASHPID|BASH_ALIASES|BASH_ARGC|BASH_ARGV|BASH_ARGV0|BASH_CMDS|BASH_COMMAND|BASH_COMPAT|BASH_ENV|BASH_EXECUTION_STRING|BASH_LINENO|BASH_LOADABLES_PATH|BASH_REMATCH|BASH_SOURCE|BASH_SUBSHELL|BASH_VERSINFO|BASH_VERSION|BASH_XTRACEFD|CHILD_MAX|COLUMNS|COMP_CWORD|COMP_LINE|COMP_POINT|COMP_TYPE|COMP_KEY|COMP_WORDBREAKS|COMP_WORDS|COMPREPLY|COPROC|DIRSTACK|EMACS|ENV|EPOCHREALTIME|EPOCHSECONDS|EUID|EXECIGNORE|FCEDIT|FIGNORE|FUNCNAME|FUNCNEST|GLOBIGNORE|GROUPS|histchars|HISTCMD|HISTCONTROL|HISTFILE|HISTFILESIZE|HISTIGNORE|HISTSIZE|HISTTIMEFORMAT|HOSTFILE|HOSTNAME|HOSTTYPE|IGNOREEOF|INPUTRC|INSIDE_EMACS|LANG|LC_ALL|LC_COLLATE|LC_CTYPE|LC_MESSAGES|LC_NUMERIC|LC_TIME|LINENO|LINES|MACHTYPE|MAILCHECK|MAPFILE|OLDPWD|OPTERR|OSTYPE|PIPESTATUS|POSIXLY_CORRECT|PPID|PROMPT_COMMAND|PROMPT_DIRTRIM|PS0|PS3|PS4|PWD|RANDOM|READLINE_ARGUMENT|READLINE_LINE|READLINE_MARK|READLINE_POINT|REPLY|SECONDS|SHELL|SHELLOPTS|SHLVL|SRANDOM|TIMEFORMAT|TMOUT|TMPDIR|UID)$"))

;; #FFFFFF #000000 0 0 0 1
(case_item
  value: (word) @variable.parameter)

;; #AAD94C #000000 0 0 0 3
((program
  .
  (comment) @keyword.directive @nospell)
  (#match? @keyword.directive "^#!/"))

; Injections

;; !regex
[
  (regex)
  (extglob_pattern)
] @string.regexp

;; !bash
((heredoc_body) @bash_injection
  ((heredoc_end) @lang
    (#match? @lang "BASH")))

;; !c
((heredoc_body) @c_injection
  ((heredoc_end) @lang
    (#match? @lang "C$")))

;; !cpp
((heredoc_body) @cpp_injection
  ((heredoc_end) @lang
    (#match? @lang "CPP")))

;; !css
((heredoc_body) @css_injection
  ((heredoc_end) @lang
    (#match? @lang "CSS")))

;; !fish
((heredoc_body) @fish_injection
  ((heredoc_end) @lang
    (#match? @lang "FISH")))

;; !go
((heredoc_body) @go_injection
  ((heredoc_end) @lang
    (#match? @lang "GO")))

;; !haskell
((heredoc_body) @haskell_injection
  ((heredoc_end) @lang
    (#match? @lang "HASKELL")))

;; !html
((heredoc_body) @html_injection
  ((heredoc_end) @lang
    (#match? @lang "HTML")))

;; !javascript
((heredoc_body) @javascript_injection
  ((heredoc_end) @lang
    (#match? @lang "JAVASCRIPT")))

;; !json
((heredoc_body) @json_injection
  ((heredoc_end) @lang
    (#match? @lang "JSON")))

;; !lua
((heredoc_body) @lua_injection
  ((heredoc_end) @lang
    (#match? @lang "LUA")))

;; !make
((heredoc_body) @make_injection
  ((heredoc_end) @lang
    (#match? @lang "MAKE")))

;; !python
((heredoc_body) @python_injection
  ((heredoc_end) @lang
    (#match? @lang "PYTHON")))

;; !ruby
((heredoc_body) @ruby_injection
  ((heredoc_end) @lang
    (#match? @lang "RUBY")))

;; !rust
((heredoc_body) @rust_injection
  ((heredoc_end) @lang
    (#match? @lang "RUST")))

;; !diff
((heredoc_body) @diff_injection
  ((heredoc_end) @lang
    (#match? @lang "DIFF")))

;; !embedded_template
((heredoc_body) @embedded_template_injection
  ((heredoc_end) @lang
    (#match? @lang "ERB")))

;; !gdscript
((heredoc_body) @gdscript_injection
  ((heredoc_end) @lang
    (#match? @lang "GDSCRIPT")))

;; !gitattributes
((heredoc_body) @gitattributes_injection
  ((heredoc_end) @lang
    (#match? @lang "GITATTRIBUTES")))

;; !gitignore
((heredoc_body) @gitignore_injection
  ((heredoc_end) @lang
    (#match? @lang "GITIGNORE")))

;; !gomod
((heredoc_body) @gomod_injection
  ((heredoc_end) @lang
    (#match? @lang "GOMOD")))

;; !ini
((heredoc_body) @ini_injection
  ((heredoc_end) @lang
    (#match? @lang "INI")))

;; !markdown
((heredoc_body) @markdown_injection
  ((heredoc_end) @lang
    (#match? @lang "MARKDOWN")))

;; !nginx
((heredoc_body) @nginx_injection
  ((heredoc_end) @lang
    (#match? @lang "NGINX")))

;; !php
((heredoc_body) @php_injection
  ((heredoc_end) @lang
    (#match? @lang "PHP")))

;; !query
((heredoc_body) @query_injection
  ((heredoc_end) @lang
    (#match? @lang "QUERY")))

;; !regex
((heredoc_body) @regex_injection
  ((heredoc_end) @lang
    (#match? @lang "REGEX")))

;; !sql
((heredoc_body) @sql_injection
  ((heredoc_end) @lang
    (#match? @lang "SQL")))

;; !toml
((heredoc_body) @toml_injection
  ((heredoc_end) @lang
    (#match? @lang "TOML")))

;; !yaml
((heredoc_body) @yaml_injection
  ((heredoc_end) @lang
    (#match? @lang "YAML")))

;; !cabal
((heredoc_body) @cabal_injection
  ((heredoc_end) @lang
    (#match? @lang "CABAL")))
