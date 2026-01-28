#!/usr/bin/env bash
set -e

BINARY_NAME="crib"
VERSION="v0.0.1-alpha"

if [ -z "$RUBY_VERSION" ]; then
  if ldconfig -p | grep -q libruby.so.3.4; then
    HAVE_34=1
  fi
  if ldconfig -p | grep -q libruby-3.2.so; then
    HAVE_32=1
  fi

  if [ "$HAVE_34" = "1" ] && [ "$HAVE_32" = "1" ]; then
    echo "Multiple Ruby versions detected."
    echo "Select Ruby ABI:"
    echo "  1) Ruby 3.4"
    echo "  2) Ruby 3.2"
    read -r choice
    case "$choice" in
    1) RUBY_VERSION="3.4" ;;
    2) RUBY_VERSION="3.2" ;;
    *)
      echo "Invalid choice"
      exit 1
      ;;
    esac
  elif [ "$HAVE_34" = "1" ]; then
    RUBY_VERSION="3.4"
  elif [ "$HAVE_32" = "1" ]; then
    RUBY_VERSION="3.2"
  else
    echo "No compatible Ruby library found (need Ruby 3.2 or 3.4)."
    exit 1
  fi
fi

GITHUB_URL="https://github.com/SyedM-dev/crib/releases/download/$VERSION/crib-linux-x86_64-ruby$RUBY_VERSION"

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

echo "Installing Crib (Ruby $RUBY_VERSION)"

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

echo
echo "✔ Crib installed to $INSTALL_DIR"
echo "Run with: $BINARY_NAME"
echo "Ruby ABI: $RUBY_VERSION"
echo "Add $INSTALL_DIR to PATH if needed."
