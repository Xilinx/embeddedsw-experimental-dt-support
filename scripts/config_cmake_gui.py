import os
import sys
import glob
import operator
import subprocess
import argparse, textwrap
import shlex
import re
import yaml
import pathlib
import fileinput
from datetime import datetime, timezone
from distutils.dir_util import copy_tree
from distutils.file_util import copy_file

parser = argparse.ArgumentParser(description='Configure Software Configuration of a library using Cmake GUI',
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter)
required_argument = parser.add_argument_group('Required arguments')
required_argument.add_argument('-l', '--lib', action='store',
                  help='Library name)', required=True)
required_argument.add_argument('-w', '--workspace', action='store',
                  help='Top level workspace path', required=True)
args = parser.parse_args()

def update_timestamp(workspace):
    statinfo = os.stat(workspace)
    create_date = datetime.fromtimestamp(statinfo.st_ctime)
    modified_date = datetime.fromtimestamp(statinfo.st_mtime)
    time_stamp_file = os.getcwd() + str("/time_stamp.log")
    if not os.path.isfile(time_stamp_file):
        fd = open(time_stamp_file, 'w')
        fd.write("%s: %s\n" % (workspace, modified_date))
    else:
        fd = open(time_stamp_file, 'a')
        fd.write("%s: %s\n" % (workspace, modified_date))

def main():
    lib = args.lib
    workspace = args.workspace
    if lib == None or workspace == None:
        raise Exception('Missing required command line args')

    print("lib name", lib)
    workspace = os.path.abspath(workspace)
    lib_dir = workspace + str("/build_%s" % lib)
    cwd = os.getcwd()
    if not os.path.isdir(lib_dir):
        raise Exception('Given Library %s does not exist in the workspace' % lib)
    os.chdir(lib_dir)
    makeCmd = ["make", "edit_cache"]
    retCode = subprocess.check_call(makeCmd, stderr=subprocess.STDOUT, shell=False)
    makeCmd = ["make", "-j16"]
    retCode = subprocess.check_call(makeCmd, stderr=subprocess.STDOUT, shell=False)
    
    include_dir = workspace + str("/recipe-sysroot/usr/include") 
    libinclude_dir = lib_dir + str("/include")
    copy_tree(libinclude_dir, include_dir)

    os.chdir(workspace)
    update_timestamp(lib_dir)
    os.chdir(cwd)


if __name__ == '__main__':
    main()
