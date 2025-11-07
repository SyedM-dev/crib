#!/usr/bin/env bash

set -euo pipefail

DIR="$(cd -- "$(dirname -- "$0")" && pwd)"

mkdir -p "$DIR/builds"

g++ -O2 -std=c++20 -shared -fPIC -Wall -Wextra \
  -o "$DIR/builds/C-crib.so" $DIR/src/cpp/*.cpp
strip "$DIR/builds/C-crib.so"
