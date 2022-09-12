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
from distutils.dir_util import copy_tree
from distutils.file_util import copy_file
from shutil import move
import time
import fileinput
from datetime import datetime, timezone
from glob import glob

def to_cmakelist(pylist):
    cmake_list = ';'.join(pylist)
    cmake_list = '"{}"'.format(cmake_list)

    return cmake_list

def check_type(var):
    if not var:
       var = '""'
    return var

def main():
    machine = "cortexa53-zynqmp"
    repo_path = os.environ['REPO']
    workspace = os.environ['WORKSPACE']
    sdt = os.environ['SYSTEM_DT']

    srcdir = repo_path + str("/lib/bsp/standalone/src/")
    tool_chain_file = repo_path + str("/cmake/toolchainfiles/cortexa53_toolchain.cmake")
    cwd = os.getcwd()
    os.chdir(workspace)
    
    # Create bsp structure
    pathlib.Path('psu_cortexa53_0/include').mkdir(parents=True, exist_ok=True)
    pathlib.Path('psu_cortexa53_0/lib').mkdir(parents=True, exist_ok=True)
    pathlib.Path('psu_cortexa53_0/libsrc').mkdir(parents=True, exist_ok=True)
    pathlib.Path('psu_cortexa53_0/code').mkdir(parents=True, exist_ok=True)

    # Copy the bsp source code
    bspsrc = workspace + str('/psu_cortexa53_0/libsrc/standalone/src')
    copy_tree(srcdir, bspsrc)

    os.chdir(bspsrc)
    # Generate CPU specific Meta-data
    os.system("lopper %s -- %s %s %s" % (sdt, "baremetal_bspconfig_xlnx", machine, srcdir))

    # Create build folder and compile
    build_xilstandalone = workspace + str('/psu_cortexa53_0/libsrc/standalone/build_xilstandalone')
    pathlib.Path(build_xilstandalone).mkdir(parents=True, exist_ok=True)
    os.chdir(build_xilstandalone)
    os.system("cmake %s -DCMAKE_TOOLCHAIN_FILE=%s -DOS_ESW=ON" % (bspsrc, tool_chain_file))
    os.system("make -j22")
    os.system("make install")
    os.chdir(workspace)    

    # Generate libxil
    # 1. Get driver list
    print("Generating baremetal driver list\n\r");
    os.system("lopper %s -- %s %s %s" % (sdt, "baremetaldrvlist_xlnx", machine, repo_path))

    # 2. Copy the driver source code
    with open('distro.conf', 'r') as fd:
        drvlist = fd.readline()
        drvlist = drvlist.split(" = ")[1]
        drvlist = drvlist.replace('"', '')
        drvlist = drvlist.split(" ")
        with open('DRVLISTConfig.cmake', 'w') as cfd:
            cfd.write("set(DRIVER_LIST %s)\n" % to_cmakelist(drvlist))
        for drv in drvlist:
            drv = drv.replace('-', '_')
            srcdir = repo_path + str("/XilinxProcessorIPLib/drivers/") + drv + str("/src/")
            drvsrc = workspace + str("/psu_cortexa53_0/libsrc/") + drv + str("/src")
            copy_tree(srcdir, drvsrc)
            # 3. Generate _g.c file
            cwd = os.getcwd()
            try:
                os.chdir(drvsrc)
                print("Generating _g.c for the driver %s\n\r" % drv)
                if re.match("uartps", drv):
                    os.system("lopper %s -- %s %s %s stdin" % (sdt, "baremetalconfig_xlnx", machine, srcdir))
                else:
                    os.system("lopper %s -- %s %s %s" % (sdt, "baremetalconfig_xlnx", machine, srcdir))
            except:
                pass
            os.chdir(cwd)

    # 4. Generate xparameters.h file
    os.chdir(workspace)    
    libsrc_path = workspace + str("/psu_cortexa53_0/libsrc")
    move('DRVLISTConfig.cmake', libsrc_path)
    os.remove('distro.conf')
    os.remove('libxil.conf')
    os.chdir(libsrc_path)
    print("Generating the xparameters.h file\n\r");
    os.system("lopper %s -- %s %s %s" % (sdt, "baremetal_xparameters_xlnx", machine, repo_path))

    # 5. Configure and compile and install libxil to <proc>/lib folder
    libxil_cmake = repo_path + str("/XilinxProcessorIPLib/drivers/CMakeLists.txt")
    copy_file(libxil_cmake, libsrc_path)

    build_libxil = workspace + str('/psu_cortexa53_0/libsrc/build_libxil')
    pathlib.Path(build_libxil).mkdir(parents=True, exist_ok=True)
    os.chdir(build_libxil)
    os.system("cmake %s -DCMAKE_TOOLCHAIN_FILE=%s -DOS_ESW=ON" % (libsrc_path, tool_chain_file))
    os.system("make -j22")
    os.system("make install")
    os.chdir(workspace)
    
    # 6. Hello World compilation
    srcdir = repo_path + str("/lib/sw_apps/hello_world/src/")
    appsrc = workspace + str("/../")
    copy_tree(srcdir, appsrc)
    
    os.chdir(appsrc)
    # Generate linker
    print("Generating Linker script\n\r");
    os.system("lopper %s -- %s %s %s" % (sdt, "baremetallinker_xlnx", machine, srcdir))
    linker_files = repo_path + str("/scripts/linker_files/")
    linker_f = appsrc + str("/linker_files/")
    copy_tree(linker_files, linker_f)
    build_hello = workspace + str('/../build_hello')
    pathlib.Path(build_hello).mkdir(parents=True, exist_ok=True)
    os.chdir(build_hello)
    os.system("cmake %s -DCMAKE_TOOLCHAIN_FILE=%s -DOS_ESW=ON" % (appsrc, tool_chain_file))
    os.system("make -j22")
    os.chdir(workspace)
    os.chdir(cwd)

if __name__ == '__main__':
    main()

