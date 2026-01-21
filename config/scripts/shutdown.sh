#!/usr/bin/env sh

# This file can be used to execute commands right after the editor session ends.
#
# The location of this file is defined in the editor's config.json and can be changed
#
# It can for example be used to unset environment variables for any lsp(s) used
# Or like this example to set kitty padding back higher
#   kitty @ --to="$KITTY_LISTEN_ON" set-spacing padding=8 margin=0 2>/dev/null || true

echo "Exiting crib editor..."
