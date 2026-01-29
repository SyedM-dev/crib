#!/usr/bin/env sh

set -eu

install() {
  BINARY_NAME="crib"
  VERSION="v0.0.1-alpha"
  BIN_URL="https://git.syedm.dev/SyedM-dev/crib/releases/download/$VERSION/crib"

  ldconfig -p | grep libmagic >/dev/null 2>&1

  if ! ldconfig -p | grep libmagic >/dev/null 2>&1; then
    echo "Missing dependency: libmagic (part of \`file\` package)"
    echo "Install them using your package manager:"
    echo "Ubuntu/Debian: sudo apt install ruby libmagic1"
    echo "Arch: sudo pacman -S file"
    echo "Void: sudo xbps-install -Sy file"
    exit 1
  fi

  echo "Install locally (~/.local/bin) or globally (/usr/bin)? [l/g]"
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
