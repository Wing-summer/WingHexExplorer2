#! /usr/bin/env python3
# -*- coding:utf-8 -*-
# @Time    : 2024/12/15
# @Author  : 寂静的羽夏(wingsummer)
# @FileName: deploy.py

import argparse
import os
import shutil
import hashlib
import subprocess

from colorama import Fore, Style

PACKAGE_NAME = "WingHexExplorer2"


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


def main():
    parser = argparse.ArgumentParser(
        prog="deploy.py", description=f"A deploying tool for {PACKAGE_NAME}")
    parser.add_argument(
        "folder", help="A folder that has contained the binary build")
    args = parser.parse_args()

    # start parsing build directory
    build_path = os.path.abspath(args.folder)

    installer_path = os.path.dirname(os.path.abspath(__file__))
    mkinstaller_path = os.path.abspath(os.path.join(installer_path, ".."))
    projectbase = os.path.abspath(os.path.join(installer_path, "../.."))
    cmake_cache = os.path.join(build_path, "CMakeCache.txt")

    if (os.path.exists(build_path) == False):
        print(
            Fore.RED + "[Error] Not found a CMake build directory!" + Style.RESET_ALL)
        exit(-1)

    if (os.path.exists(cmake_cache) == False):
        print(
            Fore.RED + "[Error] This is not a CMake build directory!" + Style.RESET_ALL)
        exit(-1)

    installer_path_exec = ""

    with open(cmake_cache, 'r') as cmake_config:
        while (True):
            line = cmake_config.readline()
            if not line:
                break
            if (line.startswith("CMAKE_INSTALL_PREFIX:PATH")):
                set = line.split('=', 1)
                installer_path_exec = set[1].strip('\n')
                pass

    print(Fore.GREEN + ">> Checking patchelf..." + Style.RESET_ALL)

    ret = run_command_interactive(["patchelf", "--version"])
    if (ret != 0):
        print(
            Fore.RED + "[Error] patchelf is needed for deploying!" + Style.RESET_ALL)
        exit(-2)

    print(Fore.GREEN + ">> Checking file integrity..." + Style.RESET_ALL)

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

    # check wether it has been installed
    if (os.path.exists(installer_path_exec) == False):
        print(
            Fore.RED + "[Error] Installing directory not exists!" + Style.RESET_ALL)
        exit(-1)

    install_content = ["bin", "lib", "plugins",
                       "translations", PACKAGE_NAME]
    for item in install_content:
        if (os.path.exists(os.path.join(installer_path_exec, item)) == False):
            print(
                Fore.RED + "[Error] Installing contents have been damaged!" + Style.RESET_ALL)
            exit(-1)

    # ok, start deploying
    remove(os.path.join(installer_path_exec, "bin"))

    exemain_src = os.path.join(installer_path_exec, PACKAGE_NAME)
    print(Fore.GREEN + ">> Deploying..." + Style.RESET_ALL)

    create_dir(os.path.join(installer_path_exec, "plugin"))
    create_dir(os.path.join(installer_path_exec, "scripts"))
    create_dir(os.path.join(installer_path_exec, "aslib"))

    shutil.copyfile(version_file_src, os.path.join(
        installer_path_exec, "VERSION"))

    shutil.copytree(os.path.join(installer_path, "share"),
                    os.path.join(installer_path_exec, "share"), dirs_exist_ok=True)

    shutil.copytree(os.path.join(build_path, "lang"),
                    os.path.join(installer_path_exec, "lang"), dirs_exist_ok=True)

    shutil.copytree(os.path.join(build_path, "lsp"), os.path.join(
        installer_path_exec, "lsp"), dirs_exist_ok=True)

    shutil.copyfile(os.path.join(mkinstaller_path, "config.ini"),
                    os.path.join(installer_path_exec, "config.ini"))

    # copying deployment files
    deploy_files = ["qt.conf", "uninstall.sh",
                    "purge.sh", f"{PACKAGE_NAME}.sh"]
    for item in deploy_files:
        shutil.copy2(os.path.join(installer_path, item),
                     os.path.join(installer_path_exec, item))

    shutil.copy2(os.path.join(build_path, "WingPlugin", "libWingPlugin.so"),
                 os.path.join(installer_path_exec, "lib", "libWingPlugin.so"))

    # finally, copy other files
    print(Fore.GREEN + ">> Copying License and other materials..." + Style.RESET_ALL)

    material_files = ["LICENSE", "authorband.svg",
                      "licenseband.svg", "screenshot.png", "README.md"]

    for f in material_files:
        shutil.copyfile(os.path.join(projectbase, f),
                        os.path.join(installer_path_exec, f))

    shutil.copyfile(os.path.join(projectbase, "images", "author.jpg"),
                    os.path.join(installer_path_exec, "author.jpg"))

    # in the end, start patching
    ld_execs = [filename for filename in os.listdir(os.path.join(
        installer_path_exec, "lib")) if filename.startswith("ld-linux")]

    ld_count = len(ld_execs)
    if (ld_count > 1):
        print(
            Fore.RED + "[Error] dynamic linker/loader can not be determined!" + Style.RESET_ALL)
        exit(-3)

    ld_exec = ""

    if (ld_count == 1):
        ld_exec = ld_execs[0]
        ret = run_command_interactive(
            ["patchelf", "--set-interpreter", f"./lib/{ld_exec}", exemain_src])
        if (ret != 0):
            print(
                Fore.RED + "[Error] patchelf error!" + Style.RESET_ALL)
            exit(-4)

    print(Fore.GREEN + ">> Calculating checksum..." + Style.RESET_ALL)

    # calculate the md5 checksum
    with open(exemain_src, 'rb') as file_to_check:
        data = file_to_check.read()
        md5_returned = hashlib.md5(data).hexdigest().upper()

    print(Fore.GREEN + ">> Get MD5: " + md5_returned + Style.RESET_ALL)

    with open(os.path.join(installer_path_exec, "md5sums"), 'w') as md5_file:
        md5_file.write(md5_returned)

    print(Fore.GREEN + ">> Deployment finished..." + Style.RESET_ALL)

    if (ld_count == 1):
        ld_path = os.path.join(installer_path_exec, "lib", ld_exec)
        if (os.access(ld_path, os.X_OK) == False):
            print(
                Fore.YELLOW + f"[Warn] {ld_exec} has no executable permission! You should set it for running a deployed program!" + Style.RESET_ALL)

    with open(os.path.join(installer_path_exec, "LD_PATH"), "w") as ld_file:
        ld_file.write(ld_exec)

    exit(0)


if __name__ == "__main__":
    main()
else:
    print(
        Fore.RED + "[Error] Please run this script in main mode" + Style.RESET_ALL)
