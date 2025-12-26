;; #9CDCFE #000000 0 0 0 3
[
 "("
 ")"
 "{"
 "}"
] @punctuation.bracket

;; #C2E8FF #000000 0 1 0 2
[
 ":"
 "&:"
 "::"
 "|"
 ";"
 "\""
 "'"
 ","
] @punctuation.delimiter

;; #FFD700 #000000 0 0 0 2
[
 "$"
 "$$"
] @punctuation.special

;; #FF8F40 #000000 0 0 0 2
(automatic_variable
 [ "@" "%" "<" "?" "^" "+" "/" "*" "D" "F"] @punctuation.special)

;; #FF6347 #000000 0 0 0 2
(automatic_variable
 "/" @error . ["D" "F"])

;; #F29668 #000000 0 1 0 2
[
 "="
 ":="
 "::="
 "?="
 "+="
 "!="
 "@"
 "-"
 "+"
] @operator

;; #FFFFFF #000000 0 0 0 1
[
 (text)
 (string)
 (raw_text)
] @string

;; #9AD4FF #000000 0 0 0 2
(variable_assignment (word) @string)

;; #7AA2F7 #000000 0 0 0 1
[
 "ifeq"
 "ifneq"
 "ifdef"
 "ifndef"
 "else"
 "endif"
 "if"
 "or"  ; boolean functions are conditional in make grammar
 "and"
] @conditional

;; #9ADE7A #000000 0 0 0 2
"foreach" @repeat

;; #D2A6FF #000000 0 0 0 1
[
 "define"
 "endef"
 "vpath"
 "undefine"
 "export"
 "unexport"
 "override"
 "private"
; "load"
] @keyword

;; #C6B5FF #000000 0 0 0 2
[
 "include"
 "sinclude"
 "-include"
] @include

;; #82AAFF #000000 0 0 0 2
[
 "subst"
 "patsubst"
 "strip"
 "findstring"
 "filter"
 "filter-out"
 "sort"
 "word"
 "words"
 "wordlist"
 "firstword"
 "lastword"
 "dir"
 "notdir"
 "suffix"
 "basename"
 "addsuffix"
 "addprefix"
 "join"
 "wildcard"
 "realpath"
 "abspath"
 "call"
 "eval"
 "file"
 "value"
 "shell"
] @keyword.function

;; #FF9D5C #000000 0 0 0 2
[
 "error"
 "warning"
 "info"
] @exception

;; #B8E986 #000000 0 0 0 2
(variable_assignment
  name: (word) @constant)

;; #B8E986 #000000 0 0 0 2
(variable_reference
  (word) @constant)

;; #99ADBF #000000 0 1 0 1
(comment) @comment

;; #F28FAD #000000 0 0 0 2
((word) @clean @string.regex
 (#match? @clean "[%\*\?]"))

;; #F07178 #000000 0 0 0 2
(function_call
  function: "error"
  (arguments (text) @text.danger))

;; #FFC877 #000000 0 0 0 2
(function_call
  function: "warning"
  (arguments (text) @text.warning))

;; #61AFEF #000000 0 0 0 2
(function_call
  function: "info"
  (arguments (text) @text.note))

;; #95E6CB #000000 0 0 0 2
[
 "VPATH"
 ".RECIPEPREFIX"
] @constant.builtin

;; #95E6CB #000000 0 0 0 2
(variable_assignment
  name: (word) @clean @constant.builtin
        (#match? @clean "^(AR|AS|CC|CXX|CPP|FC|M2C|PC|CO|GET|LEX|YACC|LINT|MAKEINFO|TEX|TEXI2DVI|WEAVE|CWEAVE|TANGLE|CTANGLE|RM|ARFLAGS|ASFLAGS|CFLAGS|CXXFLAGS|COFLAGS|CPPFLAGS|FFLAGS|GFLAGS|LDFLAGS|LDLIBS|LFLAGS|YFLAGS|PFLAGS|RFLAGS|LINTFLAGS|PRE_INSTALL|POST_INSTALL|NORMAL_INSTALL|PRE_UNINSTALL|POST_UNINSTALL|NORMAL_UNINSTALL|MAKEFILE_LIST|MAKE_RESTARTS|MAKE_TERMOUT|MAKE_TERMERR|\\.DEFAULT_GOAL|\\.RECIPEPREFIX|\\.EXTRA_PREREQS)$"))

;; #95E6CB #000000 0 0 0 2
(variable_reference
  (word) @clean @constant.builtin
  (#match? @clean "^(AR|AS|CC|CXX|CPP|FC|M2C|PC|CO|GET|LEX|YACC|LINT|MAKEINFO|TEX|TEXI2DVI|WEAVE|CWEAVE|TANGLE|CTANGLE|RM|ARFLAGS|ASFLAGS|CFLAGS|CXXFLAGS|COFLAGS|CPPFLAGS|FFLAGS|GFLAGS|LDFLAGS|LDLIBS|LFLAGS|YFLAGS|PFLAGS|RFLAGS|LINTFLAGS|PRE_INSTALL|POST_INSTALL|NORMAL_INSTALL|PRE_UNINSTALL|POST_UNINSTALL|NORMAL_UNINSTALL|MAKEFILE_LIST|MAKE_RESTARTS|MAKE_TERMOUT|MAKE_TERMERR|\\.DEFAULT_GOAL|\\.RECIPEPREFIX|\\.EXTRA_PREREQS\\.VARIABLES|\\.FEATURES|\\.INCLUDE_DIRS|\\.LOADED)$"))

;; #C792EA #000000 0 0 0 2
(targets
  (word) @constant.macro
  (#match? @constant.macro "^(all|install|install-html|install-dvi|install-pdf|install-ps|uninstall|install-strip|clean|distclean|mostlyclean|maintainer-clean|TAGS|info|dvi|html|pdf|ps|dist|check|installcheck|installdirs)$"))

;; #C792EA #000000 0 0 0 2
(targets
  (word) @constant.macro
  (#match? @constant.macro "^(all|install|install-html|install-dvi|install-pdf|install-ps|uninstall|install-strip|clean|distclean|mostlyclean|maintainer-clean|TAGS|info|dvi|html|pdf|ps|dist|check|installcheck|installdirs)$"))

;; #C792EA #000000 0 0 0 2
(targets
  (word) @constant.macro
  (#match? @constant.macro "^\\.(PHONY|SUFFIXES|DEFAULT|PRECIOUS|INTERMEDIATE|SECONDARY|SECONDEXPANSION|DELETE_ON_ERROR|IGNORE|LOW_RESOLUTION_TIME|SILENT|EXPORT_ALL_VARIABLES|NOTPARALLEL|ONESHELL|POSIX)$"))
