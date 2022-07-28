# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
# Author: Appana Durga Kedareswara Rao <appanad.durga.rao@xilinx.com>
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
import time
import fileinput
from datetime import datetime, timezone
from glob import glob

parser = argparse.ArgumentParser(description='Generating different ESW template application using System device-tree flow',
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter)
required_argument = parser.add_argument_group('Required arguments')
required_argument.add_argument('-s', '--sdt-path', action='store',
                  help='Specify the System device-tree path (till system-top.dts file)', required=True)
required_argument.add_argument('-p', '--proc', action='store',
                  help=textwrap.dedent('''\
                        Specify the processor name supported processors are
                        - cortexa72
                        - cortexa53
                        - cortexr5
                        - microblaze-pmu
                        - microblaze-plm
                        - microblaze-psm
                        '''), required=True)
required_argument.add_argument('-n', '--name', action='store',
                  help=textwrap.dedent('''\
                        Specify the name of the application supported apps are 
                        - hello_world 
                        - empty_application
                        - memory_tests
                        - peripheral_tests
                        - zynqmp_fsbl
                        - zynqmp_pmufw
                        - lwip_echo_server
                        - freertos_hello_world
                        - versal_plm
                        - versal_psmfw
                        - <Driver Name for compiling driver examples>
                        - <Library Name for compiling library examples>
                        '''), required=True)
required_argument.add_argument('-r', '--repo', action='store',
                  help='Specify repo path', required=True)
parser.add_argument('-w', '--workdir', action='store',
                  help='Workspace directory')
parser.add_argument('-o', '--os', action='store',
                  help='Specify OS (Default: standalone)')
parser.add_argument('-l', '--lib', action='append', nargs='*',
                  help='Specify libaries needs to be added if any)')
parser.add_argument('--compiler', action='store',
                  help=textwrap.dedent('''\
                        Specify compiler name supported compilers are
                        - armcc 
                        - iar
                        - armclang
                        - gcc (default)
                         '''))
parser.add_argument('-c', '--compiler-flags', action='store',
                  help='Specify compiler flags if any')
parser.add_argument('-f', '--linker-flags', action='store',
                  help='Specify linker flags if any')
parser.add_argument('-i', '--include-path', action='store',
                  help='Specify the include path')
parser.add_argument('--lib-path', action='store',
                  help='The additional library path which should be added to the'
                        'application linker settings ')
parser.add_argument('--lang', action='store',
                  help='Specify the language c or c++')
args = parser.parse_args()

def to_cmakelist(pylist):
    cmake_list = ';'.join(pylist)
    cmake_list = '"{}"'.format(cmake_list)

    return cmake_list

def check_type(var):
    if not var:
       var = '""'
    return var

def handle_dep(yaml_file, repo, build_dir, tool_chain_file):
    dep_drvlist = []
    dep_liblist = []
    cmake_configlist = []
    if os.path.isfile(yaml_file):
        with open(yaml_file, 'r') as stream:
            schema = yaml.safe_load(stream)
            try:
                dep_drvlist = schema['depends']
            except:
                dep_drvlist = []

            try:
                dep_liblist = schema['depends_libs']
            except:
                dep_liblist = []

            try:
                cmake_configlist = schema['lib_config']
            except:
                cmake_configlist = []

    if dep_drvlist:
        for drv in dep_drvlist:
            srcdir = repo + str("/XilinxProcessorIPLib/drivers/%s/" % drv)
            drvsrc = build_dir + str("/src/XilinxProcessorIPLib/drivers/%s/" % drv)
            if not os.path.isdir(drvsrc):
                copy_tree(srcdir, drvsrc)

    if dep_liblist:
        for libname in dep_liblist:
            os.chdir(build_dir)
            app_build_dir = build_dir + str("/build_%s" % libname)
            if not os.path.isdir(app_build_dir):
                pathlib.Path('build_%s' % libname).mkdir(parents=True, exist_ok=True)

            lib_config = 0
            if cmake_configlist:
                try:
                    lib_config = cmake_configlist[libname]
                except:
                    pass

            if libname.startswith('x'):
                srcdir = repo + str("/lib/sw_services/%s/" % libname)
                yaml_file = repo + str("/lib/sw_services/%s/" % libname) + str("/data/%s.yaml" % libname)
                libsrc = build_dir + str("/src/lib/sw_services/%s/" % libname)
            elif re.search("freertos", os_type) or re.search("freertos", name):
                srcdir = repo + str("/ThirdParty/bsp/freertos10_xilinx/")
                libsrc = build_dir + str("/src/ThirdParty/bsp/freertos10_xilinx/")
                yaml_file = repo + str("/ThirdParty/bsp/freertos10_xilinx/data/freertos10_xilinx.yaml")
            else:
                srcdir = repo + str("/ThirdParty/sw_services/%s/" % libname)
                yaml_file = repo + str("/ThirdParty/sw_services/%s/" % libname) + str("/data/%s.yaml" % libname)
                libsrc = build_dir + str("/src/ThirdParty/sw_services/%s/" % libname)
            copy_tree(srcdir, libsrc)
            build_lib(build_dir, libname, tool_chain_file, yaml_file, lib_config)

def add_lib(lib_name, build_dir, repo, tool_chain_file):
    os.chdir(build_dir)
    app_build_dir = build_dir + str("/build_%s" % lib_name)

    if not os.path.isdir(app_build_dir):
        pathlib.Path('build_%s' % lib_name).mkdir(parents=True, exist_ok=True)

    if lib_name.startswith('x'):
        srcdir = repo + str("/lib/sw_services/%s/" % lib_name)
        helsrc = build_dir + str("/src/lib/sw_services/%s/" % lib_name)
        yaml_file = repo + str("/lib/sw_services/%s/" % lib_name) + str("/data/%s.yaml" % lib_name)
    else:
        if re.search("freertos", os_type) or re.search("freertos", name):
            srcdir = repo + str("/ThirdParty/bsp/freertos10_xilinx/")
            helsrc = build_dir + str("/src/ThirdParty/bsp/freertos10_xilinx/")
            yaml_file = repo + str("/ThirdParty/bsp/freertos10_xilinx/data/freertos10_xilinx.yaml")
        else:
            srcdir = repo + str("/ThirdParty/sw_services/%s/" % lib_name)
            helsrc = build_dir + str("/src/ThirdParty/sw_services/%s/" % lib_name)
            yaml_file = repo + str("/ThirdParty/sw_services/%s/" % lib_name) + str("/data/%s.yaml" % lib_name)
 
    copy_tree(srcdir, helsrc)

    # Get the library dependencies
    handle_dep(yaml_file, repo, build_dir, tool_chain_file)
    build_lib(build_dir, lib_name, tool_chain_file, yaml_file, 0)

def build_lib(build_dir, lib_name, tool_chain_file, yaml_file, lib_config):
    handle_dep(yaml_file, repo, build_dir, tool_chain_file)

    hello_build_dir = build_dir + str("/build_%s" % lib_name)
    os.chdir(hello_build_dir)
    srcdir = build_dir + str("/src/")
    if lib_name.startswith('x'):
        cmake_src = build_dir + str("/src/lib/sw_services/%s/src/" % lib_name)
    else:
        if re.search("freertos", os_type) or re.search("freertos", name):
            cmake_src = build_dir + str("/src/ThirdParty/bsp/freertos10_xilinx/src/")
        else:
            cmake_src = build_dir + str("/src/ThirdParty/sw_services/%s/src/" % lib_name)

    if os.path.isfile(yaml_file):
        with open(yaml_file, 'r') as stream:
            schema = yaml.safe_load(stream)
            gen_libconfig = ""
            try:
                gen_libconfig = schema['depends']
            except:
                gen_libconfig = ""

            if gen_libconfig:
                cwd = os.getcwd()
                os.chdir(cmake_src)
                os.system("lopper %s -- %s %s %s %s %s" % (sdt, "bmcmake_metadata_xlnx", machine, cmake_src, "hwcmake_metadata", srcdir))
                os.chdir(cwd)

    cmake_config = ""
    if lib_config:
        for config in lib_config:
            for key, value in config.items():
                val = str(" -D") + str(key) + str("=") + str(value)
                cmake_config += val
    
    #cmakeCmd = ["cmake", cmake_src, "-DCMAKE_TOOLCHAIN_FILE=%s"%tool_chain_file, "-DOSL_ESW=ON", cmake_config]
    #retCode = subprocess.check_call(cmakeCmd, stderr=subprocess.STDOUT, shell=False)
    os.chdir(build_dir)
    ret = get_timestamp(hello_build_dir)
    os.chdir(hello_build_dir)
    if ret:
        os.system("cmake %s -DCMAKE_TOOLCHAIN_FILE=%s -DOS_ESW=ON %s" % (cmake_src, tool_chain_file, cmake_config))
        makeCmd = ["make", "-j16"]
        retCode = subprocess.check_call(makeCmd, stderr=subprocess.STDOUT, shell=False)
        os.chdir(build_dir)
        update_timestamp(hello_build_dir)

    libinclude_dir = hello_build_dir + str("/include")
    copy_tree(libinclude_dir, include_dir)

    os.chdir(hello_build_dir)
    for image in os.listdir(hello_build_dir):
        if image.endswith(".a"):
            copy_file(image, lib_dir)
    #lib_archive = hello_build_dir + str("/lib%s.a" % lib_name)
    #copy_file(lib_archive, lib_dir)


sdt = 0
lopper_path = 0
machine = 0
include_dir = 0
lib_dir = 0
my_event_handler = 0
os_type = 0
repo = 0
name = 0

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

def get_timestamp(workspace):
    statinfo = os.stat(workspace)
    create_date = datetime.fromtimestamp(statinfo.st_ctime)
    modified_date = datetime.fromtimestamp(statinfo.st_mtime)
    time_stamp_file = os.getcwd() + str("/time_stamp.log")
    if os.path.isfile(time_stamp_file):
        fd = open(time_stamp_file, 'r+')
        content = fd.readlines()
        match = [line for line in content if re.search(workspace, line)]
        if match:
            old_timestamp = match[-1].split(": ")[1]
            old_timestamp = old_timestamp.split(" ")[1]
            new_timestamp = str(modified_date).split(" ")[1]
            if re.search(new_timestamp, old_timestamp):
                return False
    return True

def get_machine(sdt):
    with open(sdt, 'r') as fd:
        file_lines = fd.readlines()
        match = [line for line in file_lines if re.search("cpus_a53", line)]
        if match:
            machine = "ZynqMP"
        else:
            machine = "Versal"

    return machine

def main():
    global sdt
    global lopper_path
    global machine
    global include_dir
    global lib_dir
    global os_type
    global repo
    global name

    sdt = args.sdt_path
    proc = args.proc
    name = args.name
    workspace = args.workdir
    repo = check_type(args.repo)
    repo = os.path.abspath(repo)
    os_type = check_type(args.os)
    lib = args.lib
    abspath = False
    c_flags = check_type(args.compiler_flags)
    l_flags = check_type(args.linker_flags)
    include_path = check_type(args.include_path)
    lib_path = check_type(args.lib_path)
    lang = check_type(args.lang)

    if name == None or sdt == None or proc == None:
        raise Exception('Missing required command line args')
 
    if workspace == None:
        work = os.getcwd() + str("/workspace")
        if not os.path.isdir(work):
            pathlib.Path('workspace').mkdir(parents=True, exist_ok=True)
        workspace = "/workspace"
    else:
        if os.path.isabs(workspace):
            abspath = True
            if not os.path.isdir(workspace):
                pathlib.Path(workspace).mkdir(parents=True, exist_ok=True)
        else:
            work = os.getcwd() + str("/") + workspace
            if not os.path.isdir(work):
                pathlib.Path(workspace).mkdir(parents=True, exist_ok=True)
                workspace = str("/") + workspace

    cwd = os.getcwd()
    sdt = pathlib.Path(sdt).resolve()
    sdt_dir = os.path.dirname(sdt)
    build_dir = cwd + str(workspace)
    if abspath:
        build_dir = str(workspace)

    os.chdir(build_dir)

    try:
        lopper_path = subprocess.check_output('which lopper', shell=True)
        lopper_path = lopper_path.decode("utf-8")
    except:
        pass

    if not lopper_path:
        print("ERROR: couln't find lopper please install it\n\r")
        return
    else:
        lops_path = os.path.dirname(lopper_path)
        lops_dir = os.path.dirname(lops_path)  + str("/lib/python*/site-packages/lopper/lops")

    if re.search("cortexa53", proc):
        a53_lops = lops_dir + str("/lop-a53-imux.dts")
        bm_sdt = os.getcwd() + str("/") + proc + str("_baremetal.dts")
        if not os.path.isfile(bm_sdt):
            os.system("lopper --enhanced -i %s %s %s" % (a53_lops, sdt, bm_sdt))
        sdt = bm_sdt
    elif re.search("cortexa72", proc):
        a72_lops = lops_dir + str("/lop-a72-imux.dts")
        bm_sdt = os.getcwd() + str("/") + proc + str("_baremetal.dts")
        if not os.path.isfile(bm_sdt):
            os.system("lopper --enhanced -i %s %s %s" % (a72_lops, sdt, bm_sdt))
        sdt = bm_sdt
    elif re.search("cortexr5", proc):
        r5_lops = lops_dir + str("/lop-r5-imux.dts")
        bm_sdt = os.getcwd() + str("/") + proc + str("_baremetal.dts")
        if not os.path.isfile(bm_sdt):
            os.system("lopper --enhanced -i %s %s %s" % (r5_lops, sdt, bm_sdt))
        sdt = bm_sdt
    else:
        sdt = str(sdt)

    os.environ["SYSTEM_DTFILE"] = sdt
    os.environ["LOPPER_DTC_FLAGS"] = "-b 0 -@"

    cmakeCmd = ['export', 'SYSTEM_DTFILE', '=', sdt]
    retCode = subprocess.check_call(cmakeCmd, stderr=subprocess.STDOUT, shell=True)
    cmakeCmd = ['export', 'LOPPER_DTC_FLAGS', '=', "-b 0 -@"]
    retCode = subprocess.check_call(cmakeCmd, stderr=subprocess.STDOUT, shell=True)
    tool_chain_file = ""
    machine = ""
    
    tool_chain_file = repo + str("/cmake/toolchainfiles/%s_toolchain.cmake" % proc)
    with open(tool_chain_file, 'r') as fd:
        file_lines = fd.readlines()
        machine = [line for line in file_lines if re.search("ESW_MACHINE", line)]
        if machine:
            machine = machine[0].split("ESW_MACHINE")
            machine = machine[1].split(")")[0]

    pathlib.Path(build_dir + str('/recipe-sysroot/usr/')).mkdir(parents=True, exist_ok=True)
    include_dir = build_dir + str("/recipe-sysroot/usr/include")
    lib_dir = build_dir + str("/recipe-sysroot/usr/lib")
    bin_dir = build_dir + str("/recipe-sysroot/usr/bin")
    pathlib.Path(lib_dir).mkdir(parents=True, exist_ok=True)
    pathlib.Path(bin_dir).mkdir(parents=True, exist_ok=True)
    
    # Create a Source struct
    pathlib.Path('src').mkdir(parents=True, exist_ok=True)
    os.chdir(cwd)

    # COPY Specs file
    srcdir = repo + str("/scripts/specs/")
    bspsrc = build_dir + str("/src/scripts/specs")
    copy_tree(srcdir, bspsrc)

    srcdir = repo + str("/lib/bsp/standalone")
    bspsrc = build_dir + str("/src/lib/bsp/standalone")
    copy_tree(srcdir, bspsrc)

    srcdir = repo + str("/cmake/common.cmake")
    
    pathlib.Path(build_dir + str('/src/cmake')).mkdir(parents=True, exist_ok=True)
    dstdir = build_dir + str("/src/cmake/common.cmake")
    copy_file(srcdir, dstdir)

    dstdir = build_dir + str("/src/cmake/%s_toolchain" % proc)
    copy_file(tool_chain_file, dstdir)
    tool_chain_file = dstdir
    
    with open(tool_chain_file, 'r+') as fd:
         content = fd.readlines()
         tmp_str = '"{}"'.format(include_dir)
         str1 = "include path"
         tmp_str1 = '"{}"'.format(str1)
         content.insert(0, "set( CMAKE_INCLUDE_PATH %s CACHE STRING %s)\n" % (tmp_str, tmp_str1))
         sysroot_path = build_dir + str("/recipe-sysroot/usr/lib/")
         ### FIXME: Workaround for MB
         if re.search("microblaze", proc):
             tool_chain_path = subprocess.check_output(['which', 'mb-gcc'])
             tool_chain_path = tool_chain_path.decode('utf-8')
             dir_path = os.path.dirname(os.path.realpath(tool_chain_path))
             gloss_path = dir_path + str("/../")
             result = [y for x in os.walk(gloss_path) for y in glob(os.path.join(x[0], 'libgloss.a'))]
             if result:
                 lib_xil_ar = sysroot_path + str("/libxil.a")
                 copy_file(result[0], lib_xil_ar)

         tmp_str = '"{}"'.format(sysroot_path)
         content.insert(0, "set( CMAKE_LIBRARY_PATH %s)\n" % tmp_str)
         fd.seek(0, 0)
         fd.writelines(content)
         if re.search("zynqmp_fsbl", name):
             if re.search("cortexa53", proc):
                 tmp_str = '"{}"'.format("${CMAKE_C_FLAGS} -Os -flto -ffat-lto-objects -DARMA53_64")
                 fd.write("\n set( CMAKE_C_FLAGS %s)\n" % tmp_str)
                 tmp_str = '"{}"'.format("${CMAKE_ASM_FLAGS} -Os -flto -ffat-lto-objects -DARMA53_64")
                 fd.write("\n set( CMAKE_ASM_FLAGS %s)\n" % tmp_str)
             elif re.search("cortexr5", proc):
                 tmp_str = '"{}"'.format("${CMAKE_C_FLAGS} -Os -flto -ffat-lto-objects -DARMR5")
                 fd.write("\n set( CMAKE_C_FLAGS %s)\n" % tmp_str)
                 tmp_str = '"{}"'.format("${CMAKE_ASM_FLAGS} -Os -flto -ffat-lto-objects -DARMR5")
                 fd.write("\n set( CMAKE_ASM_FLAGS %s)\n" % tmp_str)

         if re.search("cortexr5", proc):
             machine_type = get_machine(sdt)
             if re.search("ZynqMP", machine_type):
                 str1 = "cortexr5-zynqmp"
                 tmp_str1 = '"{}"'.format(str1)
                 fd.write("\n set( ESW_MACHINE %s)\n" % tmp_str1)
                 tmp_str1 = '"{}"'.format(machine_type)
                 fd.write("\n set( CMAKE_MACHINE %s)\n" % tmp_str1)

         if re.search("freertos", os_type) or re.search("freertos", name):
             fd.write("\n set( CMAKE_SYSTEM_NAME FreeRTOS)\n")
    
    # Build xilstandalone
    os.chdir(build_dir)
    srcdir = bspsrc + str("/src/")

    # Lopper Assist Handling
    src_cmndir = bspsrc + str("/src/common")
    os.chdir(src_cmndir)
    os.system("lopper %s -- %s %s %s" % (sdt, "baremetal_bspconfig_xlnx", machine, srcdir))
    cmd = ["cp", "%s/MemConfig.cmake"%src_cmndir, srcdir]
    retCode = subprocess.check_call(cmd, stderr=subprocess.STDOUT, shell=False)

    os.chdir(build_dir)
    bspbuild_dir = build_dir + str("/build_bsp")
    if not os.path.isdir(bspbuild_dir):
        pathlib.Path('build_bsp').mkdir(parents=True, exist_ok=True)

    ret = get_timestamp(bspbuild_dir)
    os.chdir(bspbuild_dir)
    if ret:
        cmakeCmd = ["cmake", srcdir, "-DCMAKE_TOOLCHAIN_FILE=%s"%tool_chain_file, "-DOS_ESW=ON"]
        retCode = subprocess.check_call(cmakeCmd, stderr=subprocess.STDOUT, shell=False)
        makeCmd = ["make", "-j16"]
        retCode = subprocess.check_call(makeCmd, stderr=subprocess.STDOUT, shell=False)
        os.chdir(build_dir)
        update_timestamp(bspbuild_dir)

    bsp_includedir = bspbuild_dir + str("/include")
    copy_tree(bsp_includedir, include_dir)
    bsp_archive = bspbuild_dir + str("/libxilstandalone.a")
    copy_file(bsp_archive, lib_dir)

    # Build libxil
    # Get list of drivers
    os.chdir(build_dir)
    os.system("lopper %s -- %s %s %s" % (sdt, "baremetaldrvlist_xlnx", machine, repo))
    drvlist = 0
    with open('distro.conf', 'r') as fd:
        drvlist = fd.readline()
        drvlist = drvlist.split(" = ")[1]
        drvlist = drvlist.replace('"', '')
        drvlist = drvlist.split(" ")
        with open('DRVLISTConfig.cmake', 'w') as cfd:
            cfd.write("set(DRIVER_LIST %s)\n" % to_cmakelist(drvlist))
        for drv in drvlist:
            drv = drv.replace('-', '_')
            srcdir = repo + str("/XilinxProcessorIPLib/drivers/") + drv
            drvsrc = build_dir + str("/src/XilinxProcessorIPLib/drivers/") + drv
            copy_tree(srcdir, drvsrc)

        src_file = repo + str("/XilinxProcessorIPLib/drivers/CMakeLists.txt")
        dst_file = build_dir + str("/src/XilinxProcessorIPLib/drivers/CMakeLists.txt")
        copy_file(src_file, dst_file)

    for drv in drvlist:
        if re.search("common", drv):
            common_srcdir = build_dir + str("/src/XilinxProcessorIPLib/drivers/common/src/")
            os.chdir(common_srcdir)
            intc_src = repo + str("/XilinxProcessorIPLib/drivers/intc/src/") 
            gic_src = repo + str("/XilinxProcessorIPLib/drivers/scugic/src/")
            os.system("lopper %s -- %s %s %s" % (sdt, "baremetalconfig_xlnx", machine, intc_src))
            os.system("lopper %s -- %s %s %s" % (sdt, "baremetalconfig_xlnx", machine, gic_src))

    os.chdir(build_dir)
    libxilbuild_dir = build_dir + str("/build_libxil")
    if not os.path.isdir(libxilbuild_dir):
        pathlib.Path('build_libxil').mkdir(parents=True, exist_ok=True)
    srcdir = build_dir + str("/src/XilinxProcessorIPLib/drivers/")
    dst_file = srcdir + str("/DRVLISTConfig.cmake")
    copy_file("DRVLISTConfig.cmake", dst_file) 
    libxilbuild_dir = build_dir + str("/build_libxil")
    
    os.chdir(build_dir)
    ret = get_timestamp(libxilbuild_dir)
    os.chdir(libxilbuild_dir)
    if ret:
        cmakeCmd = ["cmake", srcdir, "-DCMAKE_TOOLCHAIN_FILE=%s"%tool_chain_file, "-DOS_ESW=ON"]
        retCode = subprocess.check_call(cmakeCmd, stderr=subprocess.STDOUT, shell=False)
        makeCmd = ["make", "-j16"]
        retCode = subprocess.check_call(makeCmd, stderr=subprocess.STDOUT, shell=False)
        os.chdir(build_dir)
        update_timestamp(libxilbuild_dir)
    
    libxil_includedir = libxilbuild_dir + str("/include")
    copy_tree(libxil_includedir, include_dir)
    libxil_archive = libxilbuild_dir + str("/libxil.a")
    copy_file(libxil_archive, lib_dir)

    # compile libraries if specified using -l option
    # In case of invalid library how to through error/warning? 
    if lib:
        for lib_name in lib:
            add_lib(lib_name[0], build_dir, repo, tool_chain_file)
    elif re.search("freertos", os_type):
        add_lib("freertos", build_dir, repo, tool_chain_file)
    else:
        add_lib("xiltimer", build_dir, repo, tool_chain_file)
   
    # Build application
    os.chdir(build_dir)
    srcdir = repo + str("/scripts/linker_files/")
    linker_files = build_dir + str("/src/scripts/linker_files/")
    copy_tree(srcdir, linker_files)
   
    # Handle APP dependencies
    yaml_file = repo + str("/lib/sw_apps/%s/" % name) + str("/data/%s.yaml" % name)
    handle_dep(yaml_file, repo, build_dir, tool_chain_file)

    os.chdir(build_dir)
    app_build_dir = build_dir + str("/build_%s" % name)
    if not os.path.isdir(app_build_dir):
        pathlib.Path('build_%s' % name).mkdir(parents=True, exist_ok=True)

    is_drv_ex = [drv for drv in drvlist if re.search(name, drv)]
    lib_list = ["xilffs", "xilflash", "xilfpga", "xilisf", "xilmailbox", "xilnvm", "xilpm", "xilpuf", "xilsecure", "xilsem", "xilskey"]
    is_lib_ex = [lib for lib in lib_list if re.search(name, lib)]
    if is_lib_ex:
        add_lib(name, build_dir, repo, tool_chain_file)
        cmakesrc = build_dir + str("/src/lib/sw_services/%s/examples/" % name)
        app_src = build_dir + str("/src/lib/sw_services/%s/" % name)
        app_build_dir = build_dir + str("/build_%s-example" % name)
        os.chdir(build_dir)
        if not os.path.isdir(app_build_dir):
            pathlib.Path('build_%s-example' % name).mkdir(parents=True, exist_ok=True)
    elif is_drv_ex:
        cmakesrc = build_dir + str("/src/XilinxProcessorIPLib/drivers/%s/examples/" % name)
        app_src = build_dir + str("/src/XilinxProcessorIPLib/drivers/%s/" % name)
        os.chdir(cmakesrc)
        os.system("lopper %s -- %s %s %s %s" % (sdt, "bmcmake_metadata_xlnx", machine, cmakesrc, "drvcmake_metadata"))
    else:
        srcdir = repo + str("/lib/sw_apps/%s/" % name)
        app_src = build_dir + str("/src/lib/sw_apps/%s/" % name)
        cmakesrc = build_dir + str("/src/lib/sw_apps/%s/src/" % name)
        copy_tree(srcdir, app_src)
        if re.search("zynqmp_fsbl", name):
            psu_init_c = sdt_dir + str("/psu_init.c")
            psu_init_h = sdt_dir + str("/psu_init.h")
            if os.path.isfile(psu_init_c) and os.path.isfile(psu_init_h):
                copy_file(psu_init_c, cmakesrc)
                copy_file(psu_init_h, cmakesrc)

    os.chdir(cmakesrc)
    if re.search("memory_tests", name):
        os.system("lopper %s -- %s %s %s memtest" % (sdt, "baremetallinker_xlnx", machine, cmakesrc))
    else:
        os.system("lopper %s -- %s %s %s" % (sdt, "baremetallinker_xlnx", machine, cmakesrc))
    srcdir = build_dir + str("/src/")
    yamlfile = app_src + str("/data/%s.yaml" % name)
    if os.path.isfile(yaml_file):
        with open(yaml_file, 'r') as stream:
            schema = yaml.safe_load(stream)
            gen_libconfig = ""
            try:
                gen_libconfig = schema['depends']
            except:
                gen_libconfig = ""

            if gen_libconfig:
                os.system("lopper %s -- %s %s %s %s %s" % (sdt, "bmcmake_metadata_xlnx", machine, cmakesrc, "hwcmake_metadata", srcdir))

    if re.search("empty_application", name):
        empty_app_src = os.environ["CUSTOM_SRC"]
        copy_tree(empty_app_src, cmakesrc)


    if re.search("peripheral_tests", name):
        os.system("lopper %s -- %s %s %s" % (sdt, "baremetal_gentestapp_xlnx", machine, srcdir))

    os.chdir(app_build_dir)
    cmakeCmd = ["cmake", cmakesrc, "-DCMAKE_TOOLCHAIN_FILE=%s"%tool_chain_file, "-DOS_ESW=ON"]
    retCode = subprocess.check_call(cmakeCmd, stderr=subprocess.STDOUT, shell=False)
    makeCmd = ["make", "-j16"]
    retCode = subprocess.check_call(makeCmd, stderr=subprocess.STDOUT, shell=False)

    for image in os.listdir(app_build_dir):
        if image.endswith(".elf"):
            copy_file(image, bin_dir)

if __name__ == '__main__':
    main()
