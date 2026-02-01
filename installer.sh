#!/usr/bin/env sh

set -eu

install() {
  BINARY_NAME="crib"
  BIN_URL="https://git.syedm.dev/SyedM/crib/releases/download/v0.0.5-alpha/crib"

  echo "Install or update locally (~/.local/bin) or globally (/usr/bin)? [l/g]"
  read -r choice </dev/tty
  case "$choice" in
  l | L)
    INSTALL_DIR="$HOME/.local/bin"
    SUDO=""
    ;;
  g | G)
    INSTALL_DIR="/usr/bin"
    SUDO="sudo"
    ;;
  *)
    echo "Invalid choice"
    exit 1
    ;;
  esac

  $SUDO mkdir -p "$INSTALL_DIR"

  echo "Downloading binary..."
  curl -L "$BIN_URL" -o /tmp/"$BINARY_NAME"
  $SUDO install -m 755 /tmp/"$BINARY_NAME" "$INSTALL_DIR/$BINARY_NAME"
  rm -f /tmp/"$BINARY_NAME"

  echo
  echo "✔ Crib installed to $INSTALL_DIR"
  echo "Run with: $BINARY_NAME"
  echo "Add $INSTALL_DIR to PATH if needed."
}

install "$@"
