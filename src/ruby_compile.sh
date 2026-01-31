#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
INPUT="$SCRIPT_DIR/../include/syntax/tokens.def"
TMP="/tmp/__crib_precompiled.rb"
OUTPUT="/tmp/__crib_precompiled.mrb"

echo "module Tokens" >"$TMP"

counter=0

while read -r line; do
  if [[ $line =~ ADD\(([^\)]+)\) ]]; then
    name="${BASH_REMATCH[1]}"
    echo "  $name = $counter" >>"$TMP"
    counter=$((counter + 1))
  fi
done <"$INPUT"

OS="$(uname -s)"
OS_TYPE="unknown"

case "$OS" in
Linux*)
  OS_TYPE="linux"
  ;;
Darwin*)
  OS_TYPE="mac"
  ;;
CYGWIN* | MINGW* | MSYS*)
  OS_TYPE="windows"
  ;;
esac

{
  echo "  freeze"
  echo "end"
  echo
  cat "$SCRIPT_DIR/../include/scripting/libcrib.rb" | sed "s/os_name_placed_here/$OS_TYPE/g"
} >>"$TMP"

"$SCRIPT_DIR/../libs/mruby/bin/mrbc" -o$OUTPUT $TMP

{
  echo "#pragma once"
  xxd -i $OUTPUT | sed 's/^unsigned char /constexpr unsigned char /' |
    sed 's/^unsigned int /constexpr unsigned int /'
} >"$SCRIPT_DIR/../include/scripting/ruby_compiled.h"

rm $TMP
rm $OUTPUT
