#!/usr/bin/env bash
 
SCRIPT_DIR="/opt/WingHexExplorer2"

ABS_PATHS=()

# Iterate through each argument
for path in "$@"; do
    abs_path=$(realpath "$path" 2>/dev/null)

    if [ -z "$abs_path" ]; then
        abs_path="$path"
    fi

    ABS_PATHS+=("$abs_path")
done

cd "$SCRIPT_DIR" || exit 1

env LD_LIBRARY_PATH="$SCRIPT_DIR/lib" "$SCRIPT_DIR/WingHexExplorer2" "${ABS_PATHS[@]}"
