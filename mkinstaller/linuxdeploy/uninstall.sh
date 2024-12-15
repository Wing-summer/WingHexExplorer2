#!/usr/bin/env bash

F_RED="\e[31m"
F_GREEN="\e[32m"
A_DEFAULT="\033[0m"

if [ "$(id -u)" -ne 0 ]; then
    echo -e "$F_RED Please run this script as root or using sudo! $A_DEFAULT"
    exit 1
fi

read -r -p "Are you sure you want to uninstall and delete all contents? (y/N): " CONFIRM

if [[ "$CONFIRM" =~ ^[Yy]$ ]]; then
    xdg-mime uninstall /opt/WingHexExplorer2/share/x-winghex.xml
    xdg-icon-resource uninstall --context mimetypes --size 32 application-x-winghex
    xdg-icon-resource uninstall --context mimetypes --size 64 application-x-winghex
    xdg-icon-resource uninstall --context mimetypes --size 128 application-x-winghex

    update-mime-database /usr/share/mime
    xdg-icon-resource forceupdate
    update-icon-caches /usr/share/icons/hicolor

    # remove desktop-icon
    update-desktop-database /usr/share/applications

    # remove all contents
    rm /usr/share/applications/com.wingsummer.winghexexplorer2.desktop
    rm -r /opt/WingHexExplorer2

    echo -e "$F_GREEN Uninstallation is finished... $A_DEFAULT"
else
    echo -e "$F_GREEN Uninstallation canceled. $A_DEFAULT"
fi
