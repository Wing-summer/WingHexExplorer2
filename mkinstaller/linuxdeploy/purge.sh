#!/usr/bin/env bash

F_RED="\e[31m"
F_GREEN="\e[32m"
A_DEFAULT="\033[0m"

CFG_PATH=".config/WingCloudStudio"
AUTO_START=".config/autostart"
CONF_FILE="WingHexExplorer2.conf"
CONTENT_PATH=".local/share/WingCloudStudio"
DESKTOP_FILE_NAME="com.wingsummer.winghexexplorer2.desktop" 
PACKAGE_NAME="WingHexExplorer2"           

# Function to check if a directory is empty
is_empty() {
    local dir="$1"
    if [ -d "$dir" ] && [ -z "$(ls -A "$dir")" ]; then
        return 0 # True (empty)
    else
        return 1 # False (not empty)
    fi
}

# Function to remove a file or directory safely
remove() {
    local target="$1"
    if [ -e "$target" ]; then
        rm -rf "$target"
        echo -e "$F_GREEN >> Removed: $target$A_DEFAULT"
    fi
}

# Function to clear user data
clear_usrdata() {
    local usr="$1"
    local usr_path

    if [ "$usr" = "root" ]; then
        usr_path="/root"
    else
        usr_path="/home/$usr"
    fi

    local desktop_file="${usr_path}/${AUTO_START}/${DESKTOP_FILE_NAME}"
    remove "$desktop_file"

    local cfgpath="${usr_path}/${CFG_PATH}"
    remove "${cfgpath}/${CONF_FILE}"
    if is_empty "$cfgpath"; then
        remove "$cfgpath"
    fi

    cfgpath="${usr_path}/${CONTENT_PATH}/${PACKAGE_NAME}"
    remove "$cfgpath"

    cfgpath="${usr_path}/${CONTENT_PATH}"
    if is_empty "$cfgpath"; then
        remove "$cfgpath"
    fi

    echo -e "$F_GREEN >> Purged $usr...$A_DEFAULT"
}

# Function to purge all data
purge() {
    # Loop through user directories
    for p in /home/*; do
        local usr
        usr=$(basename "$p")
        if [ "$usr" = "lost+found" ]; then
            continue
        fi
        clear_usrdata "$usr"
    done

    # Handle root user
    clear_usrdata "root"
    echo -e "$F_GREEN >> Clean-up finished...$A_DEFAULT"
}

if [ "$(id -u)" -ne 0 ]; then
    echo -e "$F_RED Please run this script as root or using sudo! $A_DEFAULT"
    exit 1
fi

read -r -p "Are you sure you want to purge all contents? (y/N): " CONFIRM

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
    purge
    rm -r /opt/WingHexExplorer2

    echo -e "$F_GREEN Uninstallation is finished... $A_DEFAULT"
else
    echo -e "$F_GREEN Uninstallation canceled. $A_DEFAULT"
fi
