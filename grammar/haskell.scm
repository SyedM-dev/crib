;; #FFFFFF #000000 0 0 0 0 1
(variable) @variable

;; Lambdas / patterns keep params white
;; #FFFFFF #000000 0 1 0 0 5
(expression/lambda
  (_)+ @variable.parameter
  "->")
;; #FFFFFF #000000 0 0 0 0 1
(decl/function
  patterns: (patterns
    (_) @variable.parameter))
;; #FFFFFF #000000 0 0 0 0 1
(decl/function
  (infix
    (pattern) @variable.parameter))

;; Types / builtins
;; #F07178 #000000 0 0 0 0 6
((name) @type.builtin
  (#match? @type.builtin "^(Int|Integer|Bool|Char|String|Float|Double|Word)$"))

;; Strings / chars
;; #9ADE7A #000000 0 0 0 0 1
(char) @literal.char
;; #9ADE7A #000000 0 0 0 0 1
(string) @literal.string

;; Comments
;; #99ADBF #000000 0 1 0 0 5
(comment) @comment.general
;; #99ADBF #000000 0 1 0 0 5
(haddock) @comment.documentation
;; #99ADBF #000000 0 1 0 0 1
(comment) @spell

;; Punctuation
;; #BFBDB6 #000000 0 0 0 0 1
[
  "(" ")" "{" "}" "[" "]"
] @punctuation.bracket
;; #BFBDB6 #000000 0 0 0 0 1
[ "," ";" ] @punctuation.delimiter

;; Keywords (orange)
;; #FF8F40 #000000 0 0 0 0 1
[ "forall" ] @keyword.quantifier
;; #FF8F40 #000000 0 0 0 0 1
(pragma) @keyword.directive
;; #FF8F40 #000000 0 0 0 0 1
[
  "if" "then" "else" "case" "of"
] @keyword.conditional
;; #FF8F40 #000000 0 0 0 0 1
[ "import" "qualified" "module" ] @keyword.import
;; #FF8F40 #000000 0 0 0 0 1
[
  "where" "let" "in" "class" "instance" "pattern" "data"
  "newtype" "family" "type" "as" "hiding" "deriving" "via"
  "stock" "anyclass" "do" "mdo" "rec" "infix" "infixl" "infixr"
] @keyword.definition
;; #FF8F40 #000000 0 0 0 0 1
[ "forall" ] @keyword.repeat

;; Operators (italic white, high priority)
;; #FFFFFF #000000 0 1 0 0 6
[
  (operator)
  (constructor_operator)
  (all_names)
  "." ".." "=" "|" "::" "=>" "->" "<-" "\\" "`" "@"
] @operator.general
;; #FFFFFF #000000 0 1 0 0 6
(infix_id
  [
    (variable) @operator.infix
    (qualified (variable) @operator.infix)
  ])
;; #FFFFFF #000000 0 1 0 0 6
[
  (operator)
  (constructor_operator)
  (all_names)
  (wildcard)
  "." ".." "=" "|" "::" "=>" "->" "<-" "\\" "`" "@"
] @operator

;; Modules
;; #7dcfff #000000 0 0 0 0 1
(module
  (module_id) @module.name)
;; #7dcfff #000000 0 0 0 0 1
(module
  (module_id) @module)

;; Functions / calls (leave blue for function identifiers, but vars stay white due to higher priority var rules)
;; #82AAFF #000000 0 0 0 0 3
(decl/signature
  [
    name: (variable) @function.name
    names: (binding_list (variable) @function.name)
  ])
;; #82AAFF #000000 0 0 0 0 3
(decl/function
  [
    name: (variable) @function.name
    names: (binding_list (variable) @function.name)
  ])
;; #82AAFF #000000 0 0 0 0 3
(decl/bind
  [
    name: (variable) @function.name
    names: (binding_list (variable) @function.name)
  ])
;; #82AAFF #000000 0 0 0 0 2
(decl
  [
   name: (variable) @function
   names: (binding_list (variable) @function)
  ])
;; #82AAFF #000000 0 0 0 0 3
(decl/signature
  name: (variable) @function.io
  type: (type/apply
    constructor: (name) @_io)
  (#match? @_io "^IO$"))

;; Function calls kept white via var priority; ensure explicit call rule stays neutral/white
;; #FFFFFF #000000 0 0 0 0 3
(apply
  [
    (expression/variable) @function.call
    (expression/qualified (variable) @function.call)
  ])

;; Types / constructors
;; #82AAFF #000000 0 0 0 0 3
(name) @type
;; #82AAFF #000000 0 0 0 0 3
(type/star) @type
;; #C6B5FF #000000 0 0 0 0 1
(constructor) @constructor.general
;; #9ADE7A #000000 0 0 0 0 2
((constructor) @boolean
  (#match? @boolean "^(True|False)$"))
;; #9ADE7A #000000 0 0 0 0 1
((variable) @boolean
  (#match? @boolean "^otherwise$"))

;; Quoters / quasiquotes
;; #82AAFF #000000 0 0 0 0 3
(quoter) @function.call
;; #9ADE7A #000000 0 0 0 0 1
(quasiquote
  [
    (quoter) @_name
    (_
      (variable) @_name)
  ]
  (#match? @_name "^qq$")
  (quasiquote_body) @string)
;; #9ADE7A #000000 0 0 0 0 1
(quasiquote
  (_
    (variable) @_name)
  (#match? @_name "^qq$")
  (quasiquote_body) @string)
;; #82AAFF #000000 0 0 0 0 3
(quasiquote
  (_
    (module) @module
    .
    (variable) @function.call))

;; Exceptions / Debug
;; #F07178 #000000 0 0 0 0 1
((variable) @keyword.exception
  (#match? @keyword.exception "^(error|undefined|try|tryJust|tryAny|catch|catches|catchJust|handle|handleJust|throw|throwIO|throwTo|throwError|ioError|mask|mask_|uninterruptibleMask|uninterruptibleMask_|bracket|bracket_|bracketOnErrorSource|finally|fail|onException|expectationFailure)$"))
;; #F07178 #000000 0 0 0 0 1
((variable) @keyword.debug
  (#match? @keyword.debug "^(trace|traceId|traceShow|traceShowId|traceWith|traceShowWith|traceStack|traceIO|traceM|traceShowM|traceEvent|traceEventWith|traceEventIO|flushEventLog|traceMarker|traceMarkerIO)$"))

;; Misc remaining structural
;; #C6B5FF #000000 0 0 0 0 1
(wildcard) @literal.special
;; #BFBDB6 #000000 0 0 0 0 1
[ "," ";" ] @punctuation.delimiter
;; #BFBDB6 #000000 0 0 0 0 1
[
  "(" ")" "{" "}" "[" "]"
] @punctuation.bracket
;; #7dcfff #000000 0 0 0 0 1
(type/unit) @type.unit
(type/list) @type.list
(type/star) @type.star
;; #FFFFFF #000000 0 0 0 0 1
(field_name (variable) @variable.member)
(import_name (name) . (children (variable) @variable.member))

;; Numbers (bright yellow-green)
;; #DFFFA0 #000000 0 0 0 0 2
(integer) @number.integer
;; #DFFFA0 #000000 0 0 0 0 2
(negation) @number.integer
;; #DFFFA0 #000000 0 0 0 0 2
(expression/literal
  (float) @number.float)
