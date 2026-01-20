#!/usr/bin/env bash

# ----------------------------------------------
#   Bash Syntax Highlighter Test Specification
# ----------------------------------------------

VERSION="1.0.0"
declare -a ITEMS=("alpha" "beta" "gamma" "delta")
declare -A MAP=([one]=1 [two]=2 [three]=3)

log() {
  local level="$1"
  shift
  echo "[$level] $*"
}

# Simulated colored output (no real colors, just placeholders)
colorize() {
  local color="$1"
  shift
  echo "<$color>$*</$color>"
}

# Example of error handling
handle_error() {
  log ERROR "An error occurred on line $1"
}
trap 'handle_error $LINENO' ERR

# Multiline string test
read -r -d '' MULTI <<'CPP'

int main() {
  
}

CPP

log INFO "Multi-line string loaded"

# Arithmetic test
counter=0
while ((counter < 5)); do
  log DEBUG "Counter = $counter"
  ((counter++))
done

# Subshelled loops and alternating quoting
for item in "${ITEMS[@]}"; do
  (
    msg="Processing $item"
    echo "$(colorize blue "$msg")"
  )
done

# Case statement test
name="beta"
case "$name" in
alpha)
  log INFO "Name is alpha"
  ;;
beta)
  log INFO "Name is beta"
  ;;
*)
  log WARN "Unknown name"
  ;;
esac

# Testing associative array
for key in "${!MAP[@]}"; do
  value="${MAP[$key]}"
  log INFO "MAP[$key] = $value"
done

# Function recursion demo
factorial() {
  local n="$1"
  if ((n <= 1)); then
    echo 1
  else
    local prev
    prev=$(factorial $((n - 1)))
    echo $((n * prev))
  fi
}

log INFO "factorial(5) = $(factorial 5)"

# Test of parameter expansion variety
FOO="hello world"
BAR="${FOO/w/r}"
BAZ=${FOO^^}
QUX=${FOO,,}

log DEBUG "BAR=$BAR"
log DEBUG "BAZ=$BAZ"
log DEBUG "QUX=$QUX"

# Simulated config parsing
CONFIG="
key1=value1
key2=value two
key3=42
"

while IFS='=' read -r key val; do
  [[ -z "$key" ]] && continue
  log INFO "Config: $key = $val"
done <<<"$CONFIG"

# Nested loops + array ops
numbers=(1 2 3 4 5)
letters=(a b c)

for n in "${numbers[@]}"; do
  for l in "${letters[@]}"; do
    echo "Pair: $n:$l"
  done
done

# Here-string test
grep "world" <<<"$FOO" >/dev/null && log INFO "FOO contains world"

# Process substitution test
diff <(echo foo) <(echo foo) >/dev/null && log INFO "diff matched"

# Command substitution with pipeline
timestamp=$(date | sed 's/ /_/g')
log INFO "Timestamp: $timestamp"

# Testing array slicing
slice=("${numbers[@]:1:3}")
log INFO "Slice: ${slice[*]}"

# Simple I/O test (safe)
echo "Enter something (test for reading):"
read -r user_input
log INFO "You typed: $user_input"

# End marker
log INFO "Script finished (version $VERSION)"

