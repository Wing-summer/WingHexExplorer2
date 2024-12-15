#!/usr/bin/env bash

F_RED="\e[31m"
F_GREEN="\e[32m"
A_DEFAULT="\033[0m"

set -e

echo -e "$F_GREEN"
echo "======================================================"
echo "   A Self Extracting Installer for WingHexExplorer2 "
echo "       License: AGPL-3.0"
echo "       Author: Wingsummer"
echo "======================================================"
echo -e "$A_DEFAULT"

if [ "$(id -u)" -ne 0 ]; then
    echo -e "$F_RED Please run this script as root or using sudo! $A_DEFAULT"
    exit 1
fi

# don't try to modified this path
INSTALL_PATH="/opt/WingHexExplorer2"

if [ -d "$INSTALL_PATH" ]; then
    rm -rf "$INSTALL_PATH"
fi

mkdir -p "$INSTALL_PATH"

ARCHIVE=$(awk '/^__ARCHIVE_BELOW__/ {print NR + 1; exit 0; }' "$0")

tail -n+"$ARCHIVE" "$0" | tar xzv -C "$INSTALL_PATH"

echo -e "$F_GREEN"
echo ">> Registering componments of WingHexExplorer2!"
echo -e "$A_DEFAULT"

mv $INSTALL_PATH/share/com.wingsummer.winghexexplorer2.desktop /usr/share/applications/com.wingsummer.winghexexplorer2.desktop

xdg-mime install $INSTALL_PATH/share/x-winghex.xml
xdg-mime default /usr/share/applications/com.wingsummer.winghexexplorer2.desktop application/x-winghex

xdg-icon-resource install --context mimetypes --size 32 $INSTALL_PATH/share/winghexpro32.png application-x-winghex
xdg-icon-resource install --context mimetypes --size 64 $INSTALL_PATH/share/winghexpro64.png application-x-winghex
xdg-icon-resource install --context mimetypes --size 128 $INSTALL_PATH/share/winghexpro128.png application-x-winghex
update-mime-database /usr/share/mime
xdg-icon-resource forceupdate
gtk-update-icon-cache /usr/share/icons/hicolor
update-desktop-database /usr/share/applications

echo -e "$F_GREEN"
echo ">> WingHexExplorer2 is installed on your computer!"
echo -e "$A_DEFAULT"
exit 0

__ARCHIVE_BELOW__
