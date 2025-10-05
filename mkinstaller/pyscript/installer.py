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
DEFAULT_INSTALL_PATH = f"/opt/{PACKAGE_NAME}"
INSTALL_PATH = DEFAULT_INSTALL_PATH

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

    return process.wait()


def create_dir(dir_path):
    if not os.path.exists(dir_path):
        os.mkdir(dir_path)


def update(build_path):
    if not os.path.exists(INSTALL_PATH):
        print(
            Fore.RED + "[Error] The installing path not exists! Please use 'install' instead." + Style.RESET_ALL)
        exit(3)

    installer_path = os.path.dirname(os.path.abspath(__file__))
    mkinstaller_path = os.path.abspath(os.path.join(installer_path, ".."))
    project_base = os.path.abspath(os.path.join(installer_path, "../.."))

    print(Fore.GREEN + ">> Checking file integrity..." + Style.RESET_ALL)

    # 验证构建目录
    if not os.path.isdir(build_path) or not os.path.exists(os.path.join(build_path, "CMakeCache.txt")):
        print(
            Fore.RED + "[Error] Not a valid CMake build directory!" + Style.RESET_ALL)
        exit(-1)

    # 版本文件检查
    for vf in ("WINGHEX_VERSION", "QT_VERSION"):
        if not os.path.exists(os.path.join(build_path, vf)):
            print(Fore.RED + f"[Error] {vf} file not found!" + Style.RESET_ALL)
            exit(-1)

    # 可执行文件检查
    exe_src = os.path.join(build_path, PACKAGE_NAME)
    if not os.path.exists(exe_src):
        print(
            Fore.RED + f"[Error] {PACKAGE_NAME} executable not found!" + Style.RESET_ALL)
        exit(-3)

    # 计算 MD5
    with open(exe_src, 'rb') as f:
        md5sum = hashlib.md5(f.read()).hexdigest().upper()
    print(Fore.GREEN + ">> Get MD5: " + md5sum + Style.RESET_ALL)
    print(Fore.GREEN + ">> Installing..." + Style.RESET_ALL)

    # 复制核心文件
    shutil.copy2(exe_src, os.path.join(INSTALL_PATH, PACKAGE_NAME))
    shutil.copy2(os.path.join(build_path, "WingPlugin", "libWingPlugin.so"),
                 os.path.join(INSTALL_PATH, "libWingPlugin.so"))

    # 目录结构
    for sub in ("plugin", "scripts", "aslib"):
        create_dir(os.path.join(INSTALL_PATH, sub))

    # 共享资源与多语言
    shutil.copytree(os.path.join(installer_path, "share"),
                    os.path.join(INSTALL_PATH, "share"), dirs_exist_ok=True)
    shutil.copytree(os.path.join(build_path, "lang"),
                    os.path.join(INSTALL_PATH, "lang"), dirs_exist_ok=True)
    shutil.copyfile(os.path.join(mkinstaller_path, "config.ini"),
                    os.path.join(INSTALL_PATH, "config.ini"))
    shutil.copytree(os.path.join(build_path, "lsp"), os.path.join(
        INSTALL_PATH, "lsp"), dirs_exist_ok=True)

    # 其他材料
    print(Fore.GREEN + ">> Copying License and other materials..." + Style.RESET_ALL)
    material_files = ["LICENSE", "authorband.svg",
                      "licenseband.svg", "screenshot.png", "README.md"]
    for f in material_files:
        shutil.copyfile(os.path.join(project_base, f),
                        os.path.join(INSTALL_PATH, f))
    shutil.copyfile(os.path.join(project_base, "images", "author.jpg"),
                    os.path.join(INSTALL_PATH, "author.jpg"))

    # 写 md5sums
    with open(os.path.join(INSTALL_PATH, "md5sums"), 'w') as md5f:
        md5f.write(md5sum)

    # 桌面文件
    shutil.copyfile(os.path.join(installer_path, DESKTOP_FILE_NAME),
                    os.path.join(APP_DESKTOP_PATH, DESKTOP_FILE_NAME))

    # 更新 mime 和图标缓存
    cmds = [
        ["xdg-mime", "install",
            os.path.join(INSTALL_PATH, "share", "x-winghex.xml")],
        ["xdg-mime", "default", DESKTOP_FILE_NAME, "application/x-winghex"],
        ["xdg-icon-resource", "install", "--context", "mimetypes", "--size", "32",
         os.path.join(INSTALL_PATH, "share", "winghexpro32.png"), "application-x-winghex"],
        ["xdg-icon-resource", "install", "--context", "mimetypes", "--size", "64",
         os.path.join(INSTALL_PATH, "share", "winghexpro64.png"), "application-x-winghex"],
        ["xdg-icon-resource", "install", "--context", "mimetypes", "--size", "128",
         os.path.join(INSTALL_PATH, "share", "winghexpro128.png"), "application-x-winghex"],
        ["update-mime-database", "/usr/share/mime"],
        ["xdg-icon-resource", "forceupdate"],
        ["gtk-update-icon-cache", "/usr/share/icons/hicolor"],
        ["update-desktop-database", "/usr/share/applications"],
    ]
    for cmd in cmds:
        run_command_interactive(cmd)

    print(Fore.GREEN + ">> Installation finished..." + Style.RESET_ALL)


def install(build_path):
    if os.path.exists(INSTALL_PATH):
        print(
            Fore.RED + "[Error] The installing path exists! Please use 'update' or 'uninstall' first." + Style.RESET_ALL)
        exit(3)
    os.makedirs(INSTALL_PATH)
    update(build_path)


def remove(path):
    """Remove file or directory recursively."""
    if os.path.isfile(path) or os.path.islink(path):
        os.remove(path)
    elif os.path.isdir(path):
        shutil.rmtree(path)


def is_empty(path):
    return os.path.isdir(path) and not os.listdir(path)


def uninstall():
    cmds = [
        ["xdg-mime", "uninstall",
            os.path.join(INSTALL_PATH, "share", "x-winghex.xml")],
        ["xdg-icon-resource", "uninstall", "--context",
            "mimetypes", "--size", "32", "application-x-winghex"],
        ["xdg-icon-resource", "uninstall", "--context",
            "mimetypes", "--size", "64", "application-x-winghex"],
        ["xdg-icon-resource", "uninstall", "--context",
            "mimetypes", "--size", "128", "application-x-winghex"],
        ["update-mime-database", "/usr/share/mime"],
        ["xdg-icon-resource", "forceupdate"],
        ["update-icon-caches", "/usr/share/icons/hicolor"],
        ["update-desktop-database", "/usr/share/applications"],
    ]
    for cmd in cmds:
        run_command_interactive(cmd)

    remove(os.path.join(APP_DESKTOP_PATH, DESKTOP_FILE_NAME))
    remove(INSTALL_PATH)

    print(Fore.GREEN + ">> Uninstallation finished..." + Style.RESET_ALL)


CFG_PATH = ".config/WingCloudStudio"
AUTO_START = ".config/autostart"
CONF_FILE = "WingHexExplorer2.conf"
CONTENT_PATH = ".local/share/WingCloudStudio"


def clear_usrdata(user):
    if user == "root":
        home = "/root"
    else:
        home = f"/home/{user}"

    remove(os.path.join(home, AUTO_START, DESKTOP_FILE_NAME))
    cfgdir = os.path.join(home, CFG_PATH)
    remove(os.path.join(cfgdir, CONF_FILE))
    if is_empty(cfgdir):
        remove(cfgdir)

    contentdir = os.path.join(home, CONTENT_PATH, PACKAGE_NAME)
    remove(contentdir)
    parent = os.path.join(home, CONTENT_PATH)
    if is_empty(parent):
        remove(parent)

    print(Fore.GREEN + f">> Purged {user}..." + Style.RESET_ALL)


def purge():
    uninstall()
    for user in os.listdir("/home"):
        if user == "lost+found":
            continue
        clear_usrdata(user)
    clear_usrdata("root")
    print(Fore.GREEN + ">> Clean-up finished..." + Style.RESET_ALL)


def main():
    parser = argparse.ArgumentParser(prog="installer.py",
                                     description=f"A installing manager for {PACKAGE_NAME}")
    parser.add_argument("action",
                        choices=["install", "update", "uninstall", "purge"],
                        nargs="?",
                        default="install",
                        help="Action to perform: install (default), update, uninstall, or purge.")
    parser.add_argument("-b", "--build",
                        type=str,
                        required=False,
                        help="Path to the build directory (required for install/update).")
    parser.add_argument("-p", "--prefix",
                        dest="install_path",
                        type=str,
                        default=DEFAULT_INSTALL_PATH,
                        help=f"Installation directory (default: {DEFAULT_INSTALL_PATH})")
    args = parser.parse_args()

    # 覆盖全局变量
    global INSTALL_PATH
    INSTALL_PATH = args.install_path

    if check_is_running(PACKAGE_NAME):
        print(
            Fore.RED + f"[Error] Please exit {PACKAGE_NAME} before installation." + Style.RESET_ALL)
        exit(1)
    if not is_root():
        print(
            Fore.RED + "[Error] root privilege is required." + Style.RESET_ALL)
        exit(2)

    if args.action in ("install", "update") and not args.build:
        print(
            Fore.RED + "[Error] --build path is required for install/update." + Style.RESET_ALL)
        exit(2)

    if args.action == "install":
        install(args.build)
    elif args.action == "update":
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
