#!/usr/bin/env bash
set -e

BINARY_NAME="crib"
GITHUB_URL="https://github.com/SyedM-dev/crib/releases/download/v0.0.1-alpha/crib-linux-x86_64"

missing=()
command -v ruby >/dev/null 2>&1 || missing+=("ruby")
ldconfig -p | grep libmagic >/dev/null 2>&1 || missing+=("libmagic")

if [ ${#missing[@]} -ne 0 ]; then
  echo "Missing dependencies: ${missing[*]}"
  echo "Install them using your package manager:"
  echo "Ubuntu/Debian: sudo apt install ruby libmagic1"
  echo "Arch: sudo pacman -S ruby file"
  echo "Void: sudo xbps-install -Sy ruby file"
  exit 1
fi

echo "Install locally (~/.local/bin) or globally (/usr/bin)? [l/g]"
read -r choice
case "$choice" in
l | L) INSTALL_DIR="$HOME/.local/bin" ;;
g | G) INSTALL_DIR="/usr/bin" ;;
*)
  echo "Invalid choice"
  exit 1
  ;;
esac

mkdir -p "$INSTALL_DIR"

echo "Downloading binary..."
curl -L "$GITHUB_URL" -o "$INSTALL_DIR/$BINARY_NAME"
chmod +x "$INSTALL_DIR/$BINARY_NAME"

echo "$BINARY_NAME installed to $INSTALL_DIR"
echo "You can run it with: $BINARY_NAME"
echo "Remember to add $INSTALL_DIR to PATH if not already."
