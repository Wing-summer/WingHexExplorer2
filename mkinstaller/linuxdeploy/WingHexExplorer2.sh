#!/usr/bin/env bash
 
SCRIPT_DIR="/opt/WingHexExplorer2"
cd "$SCRIPT_DIR" || exit 1
env LD_LIBRARY_PATH="$SCRIPT_DIR/lib" "$SCRIPT_DIR/WingHexExplorer2" "$@"
