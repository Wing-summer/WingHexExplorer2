# -*- coding:utf-8 -*-
# @Time    : 2024/12/27
# @Author  : 寂静的羽夏(wingsummer)
# @FileName: qt-template-installer.py

import argparse
import sys
import os
import shutil
import pathlib
from colorama import Fore, Style

def install():
    installer_path = os.path.dirname(os.path.abspath(__file__))
    install_path = os.path.join(getQtTemplateDir(), "winghexplugin")
    shutil.rmtree(install_path, ignore_errors=True)   # uninstall first
    shutil.copytree(os.path.join(installer_path, "winghexplugin"),
                    install_path, dirs_exist_ok=True)
    print(Fore.GREEN + "WingHexExplorer2 plugin Template was installed under: " + install_path + Style.RESET_ALL)

def uninstall():
    install_path = os.path.join(getQtTemplateDir(), "winghexplugin")
    shutil.rmtree(install_path)
    print(Fore.RED + "WingHexExplorer2 plugin Template was removed: " + install_path + Style.RESET_ALL)    
    

def getQtTemplateDir() -> pathlib.Path:
    # https://docs.huihoo.com/qt/qtcreator/4.2/creator-project-wizards.html#locating-wizards
    home = pathlib.Path.home()
    if sys.platform == "win32":
        return home / "AppData/Roaming/QtProject/qtcreator/templates/wizards"
    else:
        return home / ".config/QtProject/qtcreator/templates/wizards"
    
def main():
    parser = argparse.ArgumentParser(
        prog="qt-template-installer.py", description=f"A WingHexExplorer2 Plugin tempalte installer for QT.")
    parser.add_argument(
        "action",
        choices=["install", "uninstall"],
        nargs="?",
        help="Action to perform: install or uninstall."
    )
    
    args = parser.parse_args()
    
    # checking build toolkits
    if args.action == "install":
        install()
    elif args.action == "uninstall":
        uninstall()

    exit(0)



if __name__ == "__main__":
    main()
else:
    print(
        Fore.RED + "[Error] Please run this script in main mode" + Style.RESET_ALL)