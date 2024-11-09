#! /usr/bin/env python3
# -*- coding:utf-8 -*-
# @Time    : 2024/08/27
# @Author  : 寂静的羽夏(wingsummer)
# @FileName: installer.py

import argparse
import os
import shutil
import hashlib
import psutil
import subprocess

from colorama import Fore, Style

PACKAGE_NAME = "WingHexExplorer2"
INSTALL_PATH = f"/opt/{PACKAGE_NAME}"
APP_DESKTOP_PATH = "/usr/share/applications"
DESKTOP_FILE_NAME = "com.wingsummer.winghexexplorer2.desktop"


def is_root():
    return os.geteuid() == 0


def check_is_running(processName):
    '''
    Check if there is any running process that contains the given name processName.
    '''
    # Iterate over the all the running process
    for proc in psutil.process_iter():
        try:
            # Check if process name contains the given name string.
            if processName.lower() in proc.name().lower():
                return True
        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass
    return False


def run_command_interactive(command):
    """
    Run a command interactively, printing its stdout in real-time.

    :param command: List of command arguments (e.g., ["your_command", "arg1", "arg2"])
    :return: The return code of the command
    """
    process = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,  # Capture both stdout and stderr
        text=True  # Ensure the output is in text mode
    )

    while True:
        output = process.stdout.readline()
        if output == "" and process.poll() is not None:
            break
        if output:
            print(Fore.GREEN + output.strip() + Style.RESET_ALL)

    return_code = process.wait()
    return return_code


def create_dir(dir):
    if not os.path.exists(dir):
        os.mkdir(dir)


def update(build_path):
    if (os.path.exists(INSTALL_PATH) == False):
        print(
            Fore.RED + "[Error] The installing path not exists! Please use 'install' instead." + Style.RESET_ALL)
        exit(3)

    installer_path = os.path.dirname(os.path.abspath(__file__))
    projectbase = os.path.abspath(os.path.join(installer_path, "../.."))

    print(Fore.GREEN + ">> Checking file integrity..." + Style.RESET_ALL)

    if (os.path.exists(build_path) == False):
        print(
            Fore.RED + "[Error] Not found a CMake build directory!" + Style.RESET_ALL)
        exit(-1)

    if (os.path.exists(os.path.join(build_path, "CMakeCache.txt")) == False):
        print(
            Fore.RED + "[Error] This is not a CMake build directory!" + Style.RESET_ALL)
        exit(-1)

    version_file_src = os.path.join(build_path, "WINGHEX_VERSION")
    if (os.path.exists(version_file_src) == False):
        print(
            Fore.RED + "[Error] WINGHEX_VERSION file not found, maybe not a CMake build directory!" + Style.RESET_ALL)
        exit(-1)

    version_qt_src = os.path.join(build_path, "QT_VERSION")
    if (os.path.exists(version_qt_src) == False):
        print(
            Fore.RED + "[Error] QT_VERSION file not found, maybe not a CMake build directory!" + Style.RESET_ALL)
        exit(-1)

    # ok, start copying files
    exemain_src = os.path.join(build_path, PACKAGE_NAME)
    if (os.path.exists(exemain_src) == False):
        print(
            Fore.RED + f"[Error] {PACKAGE_NAME} is not found!" + Style.RESET_ALL)
        exit(-3)

    # calculate the md5 checksum
    with open(exemain_src, 'rb') as file_to_check:
        data = file_to_check.read()
        md5_returned = hashlib.md5(data).hexdigest().upper()

    print(Fore.GREEN + ">> Get MD5: " + md5_returned + Style.RESET_ALL)
    print(Fore.GREEN + ">> Installing..." + Style.RESET_ALL)

    shutil.copy2(exemain_src, os.path.join(INSTALL_PATH, PACKAGE_NAME))

    create_dir(os.path.join(INSTALL_PATH, "plugin"))
    create_dir(os.path.join(INSTALL_PATH, "scripts"))
    create_dir(os.path.join(INSTALL_PATH, "aslib"))

    shutil.copytree(os.path.join(installer_path, "share"),
                    os.path.join(INSTALL_PATH, "share"), dirs_exist_ok=True)

    shutil.copytree(os.path.join(build_path, "lang"),
                    os.path.join(INSTALL_PATH, "lang"), dirs_exist_ok=True)

    print(Fore.GREEN + ">> Copying License and other materials..." + Style.RESET_ALL)

    material_files = ["LICENSE", "authorband.svg",
                      "licenseband.svg", "screenshot.png", "README.md", "TODO.txt"]

    for f in material_files:
        shutil.copyfile(os.path.join(projectbase, f),
                        os.path.join(INSTALL_PATH, f))

    shutil.copyfile(os.path.join(projectbase, "images", "author.jpg"),
                    os.path.join(INSTALL_PATH, "author.jpg"))

    with open(os.path.join(INSTALL_PATH, "md5sums"), 'w') as md5_file:
        md5_file.write(md5_returned)

    shutil.copyfile(os.path.join(installer_path, DESKTOP_FILE_NAME),
                    os.path.join(APP_DESKTOP_PATH, DESKTOP_FILE_NAME))

    run_command_interactive(
        ["xdg-mime", "install", "/opt/WingHexExplorer2/share/x-winghex.xml"])
    run_command_interactive(
        ["xdg-mime", "default", "/usr/share/applications/com.wingsummer.winghexexplorer2.desktop", "application/x-winghex"])
    run_command_interactive(
        ["xdg-icon-resource", "install", "--context", "mimetypes", "--size", "32", "/opt/WingHexExplorer2/share/winghexpro32.png", "application-x-winghex"])
    run_command_interactive(
        ["xdg-icon-resource", "install", "--context", "mimetypes", "--size", "64", "/opt/WingHexExplorer2/share/winghexpro64.png", "application-x-winghex"])
    run_command_interactive(
        ["xdg-icon-resource", "install", "--context", "mimetypes", "--size", "128", "/opt/WingHexExplorer2/share/winghexpro128.png", "application-x-winghex"])
    run_command_interactive(
        ["update-mime-database", "/usr/share/mime"])
    run_command_interactive(
        ["xdg-icon-resource", "forceupdate"])
    run_command_interactive(
        ["gtk-update-icon-cache", "/usr/share/icons/hicolor"])
    run_command_interactive(
        ["update-desktop-database", "/usr/share/applications"])
    print(Fore.GREEN + ">> Installation finished..." + Style.RESET_ALL)


def install(build_path):
    if (os.path.exists(INSTALL_PATH)):
        print(
            Fore.RED + "[Error] The installing path exists! Please use 'update' instead or 'uninstall' before it." + Style.RESET_ALL)
        exit(3)
    os.makedirs(INSTALL_PATH)
    update(build_path)


def remove(path):
    """ param <path> could either be relative or absolute. """
    if os.path.isfile(path) or os.path.islink(path):
        os.remove(path)  # remove the file
    elif os.path.isdir(path):
        shutil.rmtree(path)  # remove dir and all contains


def is_empty(path):
    if os.path.exists(path) and not os.path.isfile(path):
        # Checking if the directory is empty or not
        if not os.listdir(path):
            return True
        else:
            return False
    else:
        return False


def uninstall():
    run_command_interactive(
        ["xdg-mime", "uninstall", "/opt/WingHexExplorer2/share/x-winghex.xml"])
    run_command_interactive(
        ["xdg-icon-resource", "uninstall", "--context", "mimetypes", "--size", "32", "application-x-winghex"])
    run_command_interactive(
        ["xdg-icon-resource", "uninstall", "--context", "mimetypes", "--size", "64", "application-x-winghex"])
    run_command_interactive(
        ["xdg-icon-resource", "uninstall", "--context", "mimetypes", "--size", "128", "application-x-winghex"])
    run_command_interactive(
        ["update-mime-database", "/usr/share/mime"])
    run_command_interactive(
        ["xdg-icon-resource", "forceupdate"])
    run_command_interactive(
        ["update-icon-caches", "/usr/share/icons/hicolor"])
    remove(os.path.join(APP_DESKTOP_PATH, DESKTOP_FILE_NAME))
    run_command_interactive(
        ["update-desktop-database", "/usr/share/applications"])
    remove(INSTALL_PATH)
    print(Fore.GREEN + ">> Uninstallation finished..." + Style.RESET_ALL)


CFG_PATH = ".config/WingCloudStudio"
AUTO_START = ".config/autostart"
CONF_FILE = "WingHexExplorer2.conf"
CONTENT_PATH = ".local/share/WingCloudStudio"


def clear_usrdata(usr):
    usr_path = ""
    if (usr == "root"):
        usr_path = "/root"
    else:
        usr_path = f"/home/{usr}"

    desktop_file = os.path.join(usr_path, AUTO_START, DESKTOP_FILE_NAME)
    remove(desktop_file)

    cfgpath = os.path.join(usr_path, CFG_PATH)

    remove(os.path.join(cfgpath, CONF_FILE))
    if is_empty(cfgpath):
        remove(cfgpath)

    cfgpath = os.path.join(usr_path, CONTENT_PATH, PACKAGE_NAME)
    remove(cfgpath)

    cfgpath = os.path.join(usr_path, CONTENT_PATH)
    if is_empty(cfgpath):
        remove(cfgpath)

    print(Fore.GREEN + f">> Purged ${usr}..." + Style.RESET_ALL)


def purge():
    uninstall()
    for p in os.listdir("/home"):
        if p == "lost+found":
            continue
        clear_usrdata(p)
    clear_usrdata("root")
    print(Fore.GREEN + ">> Clean-up finished..." + Style.RESET_ALL)


def main():
    parser = argparse.ArgumentParser(
        prog="installer.py", description=f"A installing manager for {PACKAGE_NAME}")
    parser.add_argument(
        "action",
        choices=["install", "update", "uninstall", "purge"],
        nargs="?",
        default="install",
        help="Action to perform: install (default), update, uninstall, or purge."
    )
    parser.add_argument(
        "-b", "--build",
        type=str,
        required=False,
        help="Path to the building files for installation (required for 'install' and 'update' action)."
    )
    args = parser.parse_args()

    if check_is_running(PACKAGE_NAME):
        print(
            Fore.RED + f"[Error] You should exit {PACKAGE_NAME} berfore installation." + Style.RESET_ALL)
        exit(1)

    if not is_root():
        print(
            Fore.RED + "[Error] root is required for installation." + Style.RESET_ALL)
        exit(2)

    # checking build toolkits
    if args.action == "install":
        if not args.build:
            print(
                Fore.RED + "[Error] --build path is required for installation." + Style.RESET_ALL)
            exit(2)
        install(args.build)
    elif args.action == "update":
        if not args.build:
            print(
                Fore.RED + "[Error] --build path is required for installation." + Style.RESET_ALL)
            exit(2)
        update(args.build)
    elif args.action == "uninstall":
        uninstall()
    elif args.action == "purge":
        purge()

    exit(0)


if __name__ == "__main__":
    main()
else:
    print(
        Fore.RED + "[Error] Please run this script in main mode" + Style.RESET_ALL)
