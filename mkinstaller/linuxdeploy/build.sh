#!/usr/bin/env bash

F_RED="\e[31m"
F_GREEN="\e[32m"
A_DEFAULT="\033[0m"

SCRIPT_DIR=$(dirname "$(realpath "$0")")
cd "$SCRIPT_DIR" || exit

if [ "$#" -ne 1 ]; then
    echo "$F_GREEN Usage: $0 <Path>$A_DEFAULT"
    exit 1
fi

BUILD_PATH="$1"

if [ ! -d "$BUILD_PATH" ]; then
    echo -e "$F_RED Not exists: $BUILD_PATH$A_DEFAULT"
fi
if [ ! -d build ]; then
    mkdir build
fi

cd build || exit 1

VERSION_FILE="$BUILD_PATH/VERSION"
LD_FILE="$BUILD_PATH/LD_PATH"

version=$(<"$VERSION_FILE")
ld_file=$(<"$LD_FILE")

if [ -z "$version" ]; then
    echo -e "$F_RED VERSION file not exists $A_DEFAULT"
    exit 1
fi

if [ -n "$ld_file" ]; then
    ld_path="$BUILD_PATH/lib/$ld_file"

    if [ ! -e "$ld_path" ]; then
        echo -e "$F_RED LD_PATH is INVALID $A_DEFAULT"
        exit 1
    fi

    if [ ! -x "$ld_path" ]; then
        echo -e "$F_RED $ld_file is not EXECUTABLE !!! $A_DEFAULT"
        exit 1
    fi
fi

rm "$LD_FILE"
rm "$VERSION_FILE"

set -e

fakeroot tar czvf payload.tar.gz -C "$BUILD_PATH" .

arch=$(uname -m)

PACKAGE_NAME="WingHexExplorer2-v$version-$arch-installer.run"

cat "$SCRIPT_DIR/installheader.sh" payload.tar.gz >"$PACKAGE_NAME"

echo -e "$F_GREEN>> $PACKAGE_NAME was created under build.$A_DEFAULT"

exit 0
