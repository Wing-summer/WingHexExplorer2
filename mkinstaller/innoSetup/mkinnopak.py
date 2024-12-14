# -*- coding:utf-8 -*-
# @Time    : 2024/08/28
# @Author  : 寂静的羽夏(wingsummer)
# @FileName: makeinnopak.py

import argparse
import os
import shutil
import hashlib
import codecs
import subprocess
import platform

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

def main():
    parser = argparse.ArgumentParser(
        prog="mkdeb.py", description="A deb installer maker for WingHexExplorer2")

    parser.add_argument(
        "folder", help="A folder that has contained the binary build")
    parser.add_argument("-c", "--cc", help="where ISCC.exe locates", default="C:\Program Files (x86)\Inno Setup 6\ISCC.exe")
    parser.add_argument("-o", "--output", help="where to put the installer")
    
    args = parser.parse_args()

    # checking build toolkits
    if (os.path.exists(args.cc) == False):
        print(Fore.RED +
              "[Error] InnoSetup is not installed on your system." + Style.RESET_ALL)
        exit(-5)

    package_name = "WingHexExplorer2"
    exe_name = package_name + ".exe"

    # start parsing build directory
    projectdeb = os.path.abspath(args.folder)

    buildinstaller = os.path.dirname(os.path.abspath(__file__))
    projectbase = os.path.abspath(os.path.join(buildinstaller, "../.."))

    cmake_cache = os.path.join(projectdeb, "CMakeCache.txt")
    if (os.path.exists(cmake_cache) == False):
        print(
            Fore.RED + "[Error] This is not a CMake build directory!" + Style.RESET_ALL)
        exit(-1)
    
    deploy_exec = ""
    
    with open(cmake_cache, 'r') as cmake_config:
        while(True):
            line = cmake_config.readline()
            if not line:
                break
            if(line.startswith("WINDEPLOYQT_EXECUTABLE:FILEPATH")):
                set = line.split('=', 1)
                deploy_exec = set[1].strip('\n')
                pass

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
    exemain_src = os.path.join(projectdeb, exe_name)
    if (os.path.exists(exemain_src) == False):
        print(
            Fore.RED + "[Error] WingHexExplorer2.exe is not found!" + Style.RESET_ALL)
        exit(-3)

    # calculate the md5 checksum
    with open(exemain_src, 'rb') as file_to_check:
        data = file_to_check.read()
        md5_returned = hashlib.md5(data).hexdigest().upper()

    print(Fore.GREEN + ">> Get MD5: " + md5_returned + Style.RESET_ALL)

    with open(os.path.join(exeDebPath, "Md5.txt"), 'w') as md5_file:
        md5_file.write(md5_returned)

    print(Fore.GREEN + ">> Constructing packages..." + Style.RESET_ALL)
    os.mkdir(os.path.join(exeDebPath, "plugin"))
    os.mkdir(os.path.join(exeDebPath, "scripts"))
    os.mkdir(os.path.join(exeDebPath, "aslib"))

    shutil.copy2(exemain_src, os.path.join(exeDebPath, exe_name))

    shutil.copytree(os.path.join(buildinstaller, "share"),
                    os.path.join(exeDebPath, "share"))

    shutil.copytree(os.path.join(projectdeb, "lang"),
                    os.path.join(exeDebPath, "lang"))

    print(Fore.GREEN + ">> Copying License and other materials..." + Style.RESET_ALL)

    material_files = ["LICENSE", "authorband.svg",
                      "licenseband.svg", "screenshot.png", "README.md"]

    for f in material_files:
        shutil.copyfile(os.path.join(projectbase, f),
                        os.path.join(exeDebPath, f))

    shutil.copyfile(os.path.join(projectbase, "images", "author.jpg"),
                    os.path.join(exeDebPath, "author.jpg"))
    
    # deploy the software 

    print(Fore.GREEN + ">> Copying finished, deploying the software..." + Style.RESET_ALL)

    try:
        subprocess.run([deploy_exec, os.path.join(exeDebPath, exe_name) ], check=True,
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except subprocess.CalledProcessError as e:
        print(Fore.RED + f"[Error] deploy package error: \n{e.stderr.decode('utf-8')}" + Style.RESET_ALL)
        exit(-4)
    except FileNotFoundError:
        exit(-4)        

    # generate iss file
    print(Fore.GREEN + ">> Copying finished, generate ISCC script..." + Style.RESET_ALL)
    
    iss_content = fr"""
; Script generated by the mkinnopak.py by wingsummer.

#define MyAppName "{package_name}"
#define MyAppVersion "{version}"
#define MyAppPublisher "WingCloud"
#define MyAppURL "https://github.com/Wing-summer/WingHexExplorer2"
#define MyAppExeName "{exe_name}"
#define MyAppLicenseFile "{os.path.join(exeDebPath, "LICENSE")}"
#define MyAppExePath "{os.path.join(exeDebPath, exe_name)}"
#define MyOutputBaseFilename "{package_name}_Setup_{version}_{platform.machine()}"

"""
    iss_content += r"""
[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{2D42FCEE-E47D-4BF5-B77F-CCEF0A1C669B}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={commonpf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
LicenseFile={#MyAppLicenseFile}
OutputBaseFilename={#MyOutputBaseFilename}
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"

[CustomMessages]                                
english.OpenWithWingHexExplorer=Open with WingHexExplorer2
chinesesimplified.OpenWithWingHexExplorer=使用羽云十六进制打开

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: {#MyAppExePath}; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

"""
    iss_content += fr'Source: "{exeDebPath}\*"; ' + r'DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "*.iss,{#MyOutputBaseFilename}.exe,README.md"' + '\n'
    iss_content += fr'Source: "{exeDebPath}\README.md"; ' + r'DestDir: "{app}"; Flags: isreadme'

    iss_content += """
[Registry]
; Register the application as a handler for all file types

"""

    iss_content += r'Root: HKCR; Subkey: "*\shell\OpenWithWingHexExplorer"; ValueType: expandsz; ValueName: ""; ValueData: {cm:OpenWithWingHexExplorer}; Flags: uninsdeletekey' + '\n'
    iss_content += r'Root: HKCR; Subkey: "*\shell\OpenWithWingHexExplorer"; ValueType: expandsz; ValueName: "Icon"; ValueData: {app}\{#MyAppExeName}; Flags: uninsdeletekey' + '\n'
    iss_content += r'Root: HKCR; Subkey: "*\shell\OpenWithWingHexExplorer\command"; ValueType: expandsz; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Flags: uninsdeletekey' + '\n'

    iss_content += """
[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
"""

    script_src = os.path.join(exeDebPath, "mkiss.iss")
    with codecs.open(script_src,'w', "utf-8-sig") as iss:
        iss.write(iss_content)


    print(Fore.GREEN + ">> Copying finished, running ISCC building..." + Style.RESET_ALL)
    
    pak_out = ""
    if args.output is None:
        pak_out = exeDebPath
    else:
        pak_out = args.output
    
    if len(args.cc) == 0:
        exit(0)
    
    ret = run_command_interactive([args.cc, f'/O{pak_out}', script_src]) 
    exit(ret)


if __name__ == "__main__":
    main()
else:
    print(
        Fore.RED + "[Error] Please run this script in main mode" + Style.RESET_ALL)
