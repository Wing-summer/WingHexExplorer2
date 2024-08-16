import argparse
import os
from colorama import Fore, Style

parser = argparse.ArgumentParser(
    prog="mkdeb.py", description="A deb installer maker for WingHexExplorer2")

parser.add_argument(
    "folder", help="A folder that has contained the binary build")
parser.add_argument("-o", "--output", help="where to put the deb file")
args = parser.parse_args()

projectdeb = os.path.abspath(args.folder)
buildinstaller = os.path.dirname(os.path.abspath(__file__))
print(buildinstaller)

if(os.path.exists(os.path.join(projectdeb,"CMakeCache.txt"))==False):
    print(Fore.RED + "This is not a CMake build directory!" + Style.RESET_ALL)
    exit(-1)

debPath = os.path.join(projectdeb,"package")
if(os.path.exists(debPath)==False): 
    os.mkdir(debPath)

# ok, copying files

# TODO

exit(0)
