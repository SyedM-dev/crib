#!/usr/bin/env fish
# Fish highlighting torture test

# === Variables ===
set normal_var hello
set -l local_var 123
set -gx GLOBAL_VAR world
set PATH $PATH /usr/local/bin
set --erase OLD_VAR

# Builtin variables
echo $HOME $PWD $USER $FISH_VERSION

# === Strings ===
set single 'single quoted string'
set double "double quoted $normal_var"
set escaped "newline\n tab\t dollar\$"

# === Conditionals ===
if test $normal_var = hello
    echo equal
else if test $normal_var != world
    echo "not equal"
end

# === Logical operators ===
true and echo yes
false or echo fallback
not false

# === Arithmetic ===
set x 10
set y 20
math "$x + $y"
if test (math "$x * 2") -gt 15
    echo "math works"
end

# === Loops ===
for i in 1 2 3
    echo "loop $i"
end

while test $x -gt 0
    set x (math "$x - 1")
end

# === Functions ===
function greet --argument name
    echo "Hello $name"
end

greet world

# === Command substitution ===
set files (ls | grep ".fish")

# === Redirections ===
echo output >/tmp/fish_test.txt
cat </tmp/fish_test.txt >>/tmp/fish_log.txt

# === Process substitution ===
diff (ls /bin) (ls /usr/bin)

# === Case statement ===
switch $argv[1]
    case start
        echo Starting
    case stop
        echo Stopping
    case '*'
        echo Unknown
end

# === Subshell ===
begin
    echo "inside begin/end"
end

# === Comments & operators ===
# && || | & ! should all highlight
true && echo ok || echo fail

# === Regex ===
string match -r '^[a-z]+$' hello

# === Test builtin ===
test -f /etc/passwd
test ! -d /does/not/exist

# === Exit ===
exit 0

