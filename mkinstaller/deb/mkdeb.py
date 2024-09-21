#! /usr/bin/env python3
# -*- coding:utf-8 -*-
# @Time    : 2024/08/22
# @Author  : 寂静的羽夏(wingsummer)
# @FileName: mdeb.py

import argparse
import os
import shutil
import hashlib
import platform
import subprocess

from colorama import Fore, Style

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

def check_command_exists(command):
    try:
        # Try to run the command with --version to check if it exists
        subprocess.run([command, '--version'], check=True,
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return True
    except subprocess.CalledProcessError:
        # The command exists but returned a non-zero exit code
        return True
    except FileNotFoundError:
        # The command does not exist
        return False


def detect_architecture():
    machine_arch = platform.machine()

    # Map common machine architectures to Debian architecture names
    arch_map = {
        "x86_64": "amd64",     # 64-bit x86 architecture
        "i386": "i386",        # 32-bit x86 architecture
        "i686": "i386",        # 32-bit x86 architecture
        "aarch64": "arm64",    # 64-bit ARM architecture
        "armv7l": "armhf",     # 32-bit ARM architecture with hardware floating-point
    }

    # Return the Debian architecture or use the machine architecture as fallback
    return arch_map.get(machine_arch, machine_arch)


def main():
    parser = argparse.ArgumentParser(
        prog="mkdeb.py", description="A deb installer maker for WingHexExplorer2")

    parser.add_argument(
        "folder", help="A folder that has contained the binary build")
    parser.add_argument("-o", "--output", help="where to put the deb file")
    args = parser.parse_args()

    # checking build toolkits
    if (check_command_exists("fakeroot") == False):
        print(Fore.RED +
              "[Error] fakeroot is not installed on your system." + Style.RESET_ALL)
        exit(-5)

    if (check_command_exists("dpkg") == False):
        print(Fore.RED +
              "[Error] dpkg is not installed on your system." + Style.RESET_ALL)
        exit(-5)

    package_name = "WingHexExplorer2"
    architecture = detect_architecture()
    depends = r"${shlibs:Depends}, ${misc:Depends}"

    # start parsing build directory
    projectdeb = os.path.abspath(args.folder)

    buildinstaller = os.path.dirname(os.path.abspath(__file__))
    projectbase = os.path.abspath(os.path.join(buildinstaller, "../.."))

    if (os.path.exists(os.path.join(projectdeb, "CMakeCache.txt")) == False):
        print(
            Fore.RED + "[Error] This is not a CMake build directory!" + Style.RESET_ALL)
        exit(-1)

    version_file_src = os.path.join(projectdeb, "WINGHEX_VERSION")
    if (os.path.exists(version_file_src) == False):
        print(
            Fore.RED + "[Error] WINGHEX_VERSION file not found, maybe not a CMake build directory!" + Style.RESET_ALL)
        exit(-1)

    version_qt_src = os.path.join(projectdeb, "QT_VERSION")
    if (os.path.exists(version_qt_src) == False):
        print(
            Fore.RED + "[Error] QT_VERSION file not found, maybe not a CMake build directory!" + Style.RESET_ALL)
        exit(-1)

    with open(version_file_src, 'r') as version_file:
        version = version_file.read()

    with open(version_qt_src, 'r') as version_qt:
        qt_version = version_qt.read()

    debPath = os.path.join(projectdeb, "package")
    if (os.path.exists(debPath) == True):
        shutil.rmtree(debPath)

    os.mkdir(debPath)

    # ok, start copying files
    exeDebPath = os.path.join(debPath, package_name)
    os.mkdir(exeDebPath)

    # check
    print(Fore.GREEN + ">> Copying DEBIAN files..." + Style.RESET_ALL)
    debianfiles = ["control", "copyright", "postinst", "prerm"]
    ok = True
    for f in debianfiles:
        p = os.path.join(buildinstaller, "DEBIAN", f)
        if (os.path.exists(p) == False):
            ok = False
            break

    if (ok == False):
        print(
            Fore.RED + "[Error] DEBIAN files is not completed!" + Style.RESET_ALL)
        exit(-2)

    debianPath = os.path.join(exeDebPath, "DEBIAN")
    shutil.copytree(os.path.join(buildinstaller, "DEBIAN"), debianPath)

    exemain_src = os.path.join(projectdeb, package_name)
    if (os.path.exists(exemain_src) == False):
        print(
            Fore.RED + "[Error] WingHexExplorer2 is not found!" + Style.RESET_ALL)
        exit(-3)

    # calculate the md5 checksum
    with open(exemain_src, 'rb') as file_to_check:
        data = file_to_check.read()
        md5_returned = hashlib.md5(data).hexdigest().upper()

    print(Fore.GREEN + ">> Get MD5: " + md5_returned + Style.RESET_ALL)

    with open(os.path.join(debianPath, "md5sums"), 'w') as md5_file:
        md5_file.write(md5_returned)

    # Create control file
    control_content = f"""Source: {package_name}
Priority: optional
Maintainer: 寂静的羽夏(wingsummer)
Package: com.wingsummer.winghexexplorer2
Architecture: {architecture}
Version: {version}
Depends: {depends}
Section: utils
Description: a free and powerful opensource hexeditor based on QT
Homepage: https://www.cnblogs.com/wingsummer/
"""
    with open(os.path.join(debianPath, "control"), "w") as control_file:
        control_file.write(control_content)

    print(Fore.GREEN + ">> Constructing packages..." + Style.RESET_ALL)
    exeBasePath = os.path.join(exeDebPath, "opt", package_name)
    os.makedirs(exeBasePath)
    os.mkdir(os.path.join(exeBasePath, "plugin"))
    os.mkdir(os.path.join(exeBasePath, "scripts"))
    os.mkdir(os.path.join(exeBasePath, "aslib"))

    shutil.copy2(exemain_src, os.path.join(exeBasePath, package_name))

    desktopPath = os.path.join(exeDebPath, "usr", "share", "applications")
    os.makedirs(desktopPath)
    shutil.copyfile(os.path.join(buildinstaller, "com.wingsummer.winghexexplorer2.desktop"),
                    os.path.join(desktopPath, "com.wingsummer.winghexexplorer2.desktop"))

    shutil.copytree(os.path.join(buildinstaller, "share"),
                    os.path.join(exeBasePath, "share"))

    shutil.copytree(os.path.join(projectdeb, "lang"),
                    os.path.join(exeBasePath, "lang"))

    print(Fore.GREEN + ">> Copying dependencies..." + Style.RESET_ALL)

    ads_lib = os.path.join(projectdeb, "3rdparty",
                           "Qt-Advanced-Docking-System", "src", f"libqt{qt_version}advanceddocking.so")
    if (os.path.exists(ads_lib) == False):
        print(Fore.RED + f"[Error] libqt{qt_version}advanceddocking.so is not found! Maybe you are building it in debug mode which is not allowed!" + Style.RESET_ALL)
        exit(-4)
    shutil.copy2(ads_lib, os.path.join(
        exeBasePath, f"libqt{qt_version}advanceddocking.so"))

    print(Fore.GREEN + ">> Creating program starting entry..." + Style.RESET_ALL)
    shutil.copy2(os.path.join(buildinstaller, f"{package_name}.sh"),
                 os.path.join(exeBasePath, f"{package_name}.sh"))

    print(Fore.GREEN + ">> Copying License and other materials..." + Style.RESET_ALL)

    material_files = ["LICENSE", "authorband.svg",
                      "licenseband.svg", "screenshot.png", "README.md", "TODO.txt"]

    for f in material_files:
        shutil.copyfile(os.path.join(projectbase, f),
                        os.path.join(exeBasePath, f))

    shutil.copyfile(os.path.join(projectbase, "images", "author.jpg"),
                    os.path.join(exeBasePath, "author.jpg"))

    print(Fore.GREEN + ">> Copying finished, running dpkg building..." + Style.RESET_ALL)

    output_deb_dir = ""
    if args.output is not None:
        output_deb_dir =  os.path.join(args.output,f"com.wingsummer.winghexexplorer2_{version}_{architecture}.deb") 
    else:
        output_deb_dir =  f"com.wingsummer.winghexexplorer2_{version}_{architecture}.deb" 

    ret = run_command_interactive(["fakeroot", "dpkg", "-b", exeDebPath, output_deb_dir])
    
    exit(ret)


if __name__ == "__main__":
    main()
else:
    print(
        Fore.RED + "[Error] Please run this script in main mode" + Style.RESET_ALL)
