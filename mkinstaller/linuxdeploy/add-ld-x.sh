#!/usr/bin/env bash

F_RED="\e[31m"
A_DEFAULT="\033[0m"

BUILD_PATH="$1"

if [ "$(id -u)" -ne 0 ]; then
    echo -e "$F_RED Please run this script as root or using sudo! $A_DEFAULT"
    exit 1
fi

if [ ! -d "$BUILD_PATH" ]; then
    echo -e "$F_RED Not exists: $BUILD_PATH$A_DEFAULT"
    exit 1
fi

LD_FILE="$BUILD_PATH/LD_PATH"
ld_file=$(<"$LD_FILE")

if [ -z "$ld_file" ]; then
    echo -e "$F_RED LD_PATH not exists. Ignoring... $A_DEFAULT"
    exit 0
fi

ld_path="$BUILD_PATH/lib/$ld_file"

if [ ! -e "$ld_path" ]; then
    echo -e "$F_RED LD_PATH is INVALID $A_DEFAULT"
    exit 1
fi

chattr +x "$ld_path"

exit 0
