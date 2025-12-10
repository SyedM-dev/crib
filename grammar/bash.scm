[
  (function_definition)
  (if_statement)
  (case_statement)
  (for_statement)
  (while_statement)
  (c_style_for_statement)
  (heredoc_redirect)
] @fold

;; #bd9ae6 #000000 0 0 0 1
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

;; #bd9ae6 #000000 0 0 0 1
[
  ";"
  ";;"
  ";&"
  ";;&"
  "&"
] @punctuation.delimiter

;; #ffffff #000000 0 0 0 1
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

;; #aad84c #000000 0 0 0 1
[
  (string)
  (raw_string)
  (ansi_c_string)
  (heredoc_body)
] @string

;; #fbb152 #000000 0 0 0 1
[
  (heredoc_start)
  (heredoc_end)
] @label

(variable_assignment
  (word) @string)

(command
  argument: "$" @string) ; bare dollar

(concatenation
  (word) @string)

;; #fbb152 #000000 0 0 0 1
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

;; #fbb152 #000000 0 0 0 1
[
  "for"
  "do"
  "done"
  "select"
  "until"
  "while"
] @keyword.repeat

;; #fbb152 #000000 0 0 0 1
[
  "declare"
  "typeset"
  "readonly"
  "local"
  "unset"
  "unsetenv"
] @keyword

;; #fbb152 #000000 0 0 0 1
"export" @keyword.import

;; #fbb152 #000000 0 0 0 1
"function" @keyword.function

;; #ebda8c #000000 0 0 0 1
(special_variable_name) @constant

;; #ebda8c #000000 0 0 0 1
((word) @constant.builtin
  (#match? @constant.builtin "^(SIGHUP|SIGINT|SIGQUIT|SIGILL|SIGTRAP|SIGABRT|SIGBUS|SIGFPE|SIGKILL|SIGUSR1|SIGSEGV|SIGUSR2|SIGPIPE|SIGALRM|SIGTERM|SIGSTKFLT|SIGCHLD|SIGCONT|SIGSTOP|SIGTSTP|SIGTTIN|SIGTTOU|SIGURG|SIGXCPU|SIGXFSZ|SIGVTALRM|SIGPROF|SIGWINCH|SIGIO|SIGPWR|SIGSYS|SIGRTMIN|SIGRTMIN\+1|SIGRTMIN\+2|SIGRTMIN\+3|SIGRTMIN\+4|SIGRTMIN\+5|SIGRTMIN\+6|SIGRTMIN\+7|SIGRTMIN\+8|SIGRTMIN\+9|SIGRTMIN\+10|SIGRTMIN\+11|SIGRTMIN\+12|SIGRTMIN\+13|SIGRTMIN\+14|SIGRTMIN\+15|SIGRTMAX\-14|SIGRTMAX\-13|SIGRTMAX\-12|SIGRTMAX\-11|SIGRTMAX\-10|SIGRTMAX\-9|SIGRTMAX\-8|SIGRTMAX\-7|SIGRTMAX\-6|SIGRTMAX\-5|SIGRTMAX\-4|SIGRTMAX\-3|SIGRTMAX\-2|SIGRTMAX\-1|SIGRTMAX)$"))

;; #51eeba #000000 0 0 0 1
((word) @boolean.true
  (#match? @boolean.true "^true$"))

;; #ee513a #000000 0 0 0 1
((word) @boolean.false
  (#match? @boolean.false "^false$"))

;; #AAAAAA #000000 0 1 0 1
(comment) @comment @spell

;; #ffffff #000000 0 0 0 1
(test_operator) @operator

;; #e6a24c #000000 0 0 0 2
(command_substitution
  "$(" @punctuation.special
  ")" @punctuation.special)

;; #e6a24c #000000 0 0 0 2
(process_substitution
  [
    "<("
    ">("
  ] @punctuation.special
  ")" @punctuation.special)

;; #e6a24c #000000 0 0 0 2
(arithmetic_expansion
  [
    "$(("
    "(("
  ] @punctuation.special
  "))" @punctuation.special)

;; #bd9ae6 #000000 0 0 0 1
(arithmetic_expansion
  "," @punctuation.delimiter)

;; #ffffff #000000 0 0 0 1
(ternary_expression
  [
    "?"
    ":"
  ] @keyword.conditional.ternary)

;; #ffffff #000000 0 0 0 1
(binary_expression
  operator: _ @operator)

;; #ffffff #000000 0 0 0 1
(unary_expression
  operator: _ @operator)

;; #ffffff #000000 0 0 0 1
(postfix_expression
  operator: _ @operator)

;; #aad84c #000000 0 0 0 3
(function_definition
  name: (word) @function)

;; #aad84c #000000 0 0 0 3
(command_name
  (word) @function.call)

;; #aad84c #000000 0 0 0 3
(command_name
  (word) @function.builtin
  (#match? @function.builtin "^(\.|\:|alias|bg|bind|break|builtin|caller|cd|command|compgen|complete|compopt|continue|coproc|dirs|disown|echo|enable|eval|exec|exit|false|fc|fg|getopts|hash|help|history|jobs|kill|let|logout|mapfile|popd|printf|pushd|pwd|read|readarray|return|set|shift|shopt|source|suspend|test|time|times|trap|true|type|typeset|ulimit|umask|unalias|wait)$"))

;; #ffffff #000000 0 0 0 1
(command
  argument: [
    (word) @variable.parameter
    (concatenation
      (word) @variable.parameter)
  ])

;; #ffffff #000000 0 0 0 1
(declaration_command
  (word) @variable.parameter)

;; #ffffff #000000 0 0 0 1
(unset_command
  (word) @variable.parameter)

;; #ebda8c #000000 0 0 0 2
(number) @number

;; #ebda8c #000000 0 0 0 2
((word) @number
  (#match? @number "^[0-9]+$"))

;; #aad84c #000000 0 0 0 1
(file_redirect
  (word) @string.special.path)

;; #aad84c #000000 0 0 0 1
(herestring_redirect
  (word) @string)

;; #ffffff #000000 0 0 0 1
(file_descriptor) @operator

;; #e6a24c #000000 0 0 0 2
(simple_expansion
  "$" @punctuation.special) @none

;; #e6a24c #000000 0 0 0 2
(expansion
  "${" @punctuation.special
  "}" @punctuation.special) @none

;; #e6a24c #000000 0 0 0 2
(expansion
  operator: _ @punctuation.special)

;; #e6a24c #000000 0 0 0 2
(expansion
  "@"
  .
  operator: _ @character.special)

;; #e6a24c #000000 0 0 0 2
((expansion
  (subscript
    index: (word) @character.special))
  (#any-of? @character.special "@" "*"))

;; #e6a24c #000000 0 0 0 2
"``" @punctuation.special

;; #ffffff #000000 0 0 0 1
(variable_name) @variable

;; #ebda8c #000000 0 0 0 1
((variable_name) @constant
  (#match? @constant "^[A-Z][A-Z_0-9]*$"))

;; #ffffff #000000 0 0 0 1
((variable_name) @variable.builtin
  (#match? @variable.builtin "^(CDPATH|HOME|IFS|MAIL|MAILPATH|OPTARG|OPTIND|PATH|PS1|PS2|_|BASH|BASHOPTS|BASHPID|BASH_ALIASES|BASH_ARGC|BASH_ARGV|BASH_ARGV0|BASH_CMDS|BASH_COMMAND|BASH_COMPAT|BASH_ENV|BASH_EXECUTION_STRING|BASH_LINENO|BASH_LOADABLES_PATH|BASH_REMATCH|BASH_SOURCE|BASH_SUBSHELL|BASH_VERSINFO|BASH_VERSION|BASH_XTRACEFD|CHILD_MAX|COLUMNS|COMP_CWORD|COMP_LINE|COMP_POINT|COMP_TYPE|COMP_KEY|COMP_WORDBREAKS|COMP_WORDS|COMPREPLY|COPROC|DIRSTACK|EMACS|ENV|EPOCHREALTIME|EPOCHSECONDS|EUID|EXECIGNORE|FCEDIT|FIGNORE|FUNCNAME|FUNCNEST|GLOBIGNORE|GROUPS|histchars|HISTCMD|HISTCONTROL|HISTFILE|HISTFILESIZE|HISTIGNORE|HISTSIZE|HISTTIMEFORMAT|HOSTFILE|HOSTNAME|HOSTTYPE|IGNOREEOF|INPUTRC|INSIDE_EMACS|LANG|LC_ALL|LC_COLLATE|LC_CTYPE|LC_MESSAGES|LC_NUMERIC|LC_TIME|LINENO|LINES|MACHTYPE|MAILCHECK|MAPFILE|OLDPWD|OPTERR|OSTYPE|PIPESTATUS|POSIXLY_CORRECT|PPID|PROMPT_COMMAND|PROMPT_DIRTRIM|PS0|PS3|PS4|PWD|RANDOM|READLINE_ARGUMENT|READLINE_LINE|READLINE_MARK|READLINE_POINT|REPLY|SECONDS|SHELL|SHELLOPTS|SHLVL|SRANDOM|TIMEFORMAT|TMOUT|TMPDIR|UID)$"))

;; #ffffff #000000 0 0 0 1
(case_item
  value: (word) @variable.parameter)

;; #e6a24c #000000 0 0 0 2
[
  (regex)
  (extglob_pattern)
] @string.regexp

;; #51eeba #000000 0 0 0 3
((program
  .
  (comment) @keyword.directive @nospell)
  (#match? @keyword.directive "^#!/"))
