import argparse, textwrap
import sys
import os
import lopper
from utils import *
import re
from repo import Repo
import time

class Domain(Repo):
    def __init__(self,args,domain_create=True):
        super().__init__(args.get('repo'))
        self.wsdir = get_abs_path(args['wsdir'])
        self.proc = args.get('proc')
        self.domain_name = args.get('domain_name')
        self.os = args.get('os')
        self.app = args.get('app','')
        self.toolchain_file = None
        if args.get('sdt'):
            self.sdt = get_abs_path(args['sdt'])
            self.family = get_machine(self.sdt)
        self.domaindir = os.path.join(self.wsdir,self.proc,self.domain_name)
        self.bspdir = os.path.join(self.domaindir,'bsp')
        self.lops_dir = os.path.join(os.path.dirname(lopper.__file__),'lops')
        self.include_folder = os.path.join(self.bspdir,'include')
        self.lib_folder = os.path.join(self.bspdir,'lib')
        self.libsrc_folder = os.path.join(self.bspdir,'libsrc')
        self.domain_config_file = os.path.join(self.domaindir,'.domain.yaml')
        if domain_create:
            mkdir(self.bspdir)
            #validate_if_exist(self.domain_config_file,'domain',self.domain_name)

    def build_dir_struct(self):
        mkdir(self.include_folder)
        mkdir(self.lib_folder)
        mkdir(self.libsrc_folder)


    def toolchain_intr_mapping(self):
        proc_lops_map = {
                'a53' : ('cortexa53','lop-a53-imux'),
                'a72' : ('cortexa72','lop-a72-imux'),
                'r5'  : ('cortexr5','lop-r5-imux'),
                'pmu' : ('microblaze-pmu',''),
                'pmc' : ('microblaze-plm',''),
                'psm' : ('microblaze-psm','')
            }
        lops_file = ''
        out_dts_path = os.path.join(self.domaindir, f'{self.proc}_baremetal.dts')
        for val in proc_lops_map.keys():
            if val in self.proc:
                toolchain_file_name = f'{proc_lops_map[val][0]}_toolchain.cmake'
                toolchain_file_path = os.path.join(
                        self.repo, f'cmake/toolchainfiles/{toolchain_file_name}'
                    )
                lops_file = os.path.join(self.lops_dir, f'{proc_lops_map[val][1]}.dts')
                toolchain_file_copy = os.path.join(self.domaindir, toolchain_file_name)
                copy_file(toolchain_file_path, toolchain_file_copy)

        if 'r5' in self.proc:
            replace_line(toolchain_file_copy, 'CMAKE_MACHINE "Versal' ,f'set( CMAKE_MACHINE "{self.family.title()}")')

        if 'freertos' in self.os:
            add_newline(toolchain_file_copy, 'set( CMAKE_SYSTEM_NAME FreeRTOS)')

        if is_file(lops_file):
            runcmd(f"lopper -f --enhanced -O {self.domaindir} -i {lops_file} {self.sdt} {out_dts_path}")
        else:
            runcmd(f"lopper -f --enhanced -O {self.domaindir} {self.sdt} {out_dts_path}")

        return out_dts_path, toolchain_file_copy


    def lib_dependency(self, comp_name, is_app):
        comp_dir = self.get_comp_dir(comp_name, is_app)
        yaml_file = os.path.join(comp_dir, f"data/{comp_name}.yaml")
        lib_list = []
        lib_params_dict = {}
        hw_metadata = False
        if is_file(yaml_file):
            schema = load_yaml(yaml_file)
            lib_list = schema.get('depends_libs',[])
            if schema.get('lib_config',{}):
                for entry in schema['lib_config'].keys():
                    try:
                        lib_params_dict[entry] += schema['lib_config'][entry]
                    except KeyError:
                        lib_params_dict[entry] = schema['lib_config'][entry]
            if schema.get('required'):
                hw_metadata = True
                if not is_app:
                    lib_params_dict[comp_name] = [{'hw_metadata':True}]

        for lib in lib_list:
            if lib not in lib_params_dict.keys():
                lib_params_dict[lib] = []
            comp_dir = self.get_comp_dir(lib)
            yaml_file = os.path.join(comp_dir, f"data/{lib}.yaml")
            if is_file(yaml_file):
                schema = load_yaml(yaml_file)
                if schema.get('required'):
                    try:
                        lib_params_dict[lib] += [{'hw_metadata':True}]
                    except KeyError:
                        lib_params_dict[lib] = [{'hw_metadata':True}]

        return lib_list, lib_params_dict, hw_metadata



    def add_lib(self, comp, cmake_paths_append, is_app=False):
        lib_list, lib_params_dict, hw_metadata = self.lib_dependency(comp, is_app)
        cmake_lib_list = ''
        for entry in lib_list:
            if entry != self.app:
                cmake_lib_list += f"{entry};"
        app_config = {}
        cmake_cmd_append = ""

        if is_app:
            app_config[comp] = {'compiler_flags' : apps_cmake_update(self.toolchain_file, self.app, self.proc)}
            if hw_metadata:
                app_config[comp]['hw_metadata'] = hw_metadata

        for lib in lib_list:
            comp_dir = self.get_comp_dir(lib, is_app=False)
            srcdir = os.path.join(comp_dir, 'src/')
            dstdir = os.path.join(self.libsrc_folder, f"{lib}/src")
            copy_directory(srcdir, dstdir)
            build_hw_metadata = False
            if lib_params_dict[lib]:
                for entries in lib_params_dict[lib]:
                    for key, value in entries.items():
                        if key == 'hw_metadata':
                            build_hw_metadata = True
                        else:
                            cmake_cmd_append += f" -D{key}={value}"

            os.chdir(dstdir)

            if build_hw_metadata:
                startTime = time.time()
                runcmd(f"lopper {self.sdt} -- bmcmake_metadata_xlnx {self.proc} {srcdir} hwcmake_metadata {self.repo}")
                executionTime = (time.time() - startTime)
                print('Execution time in seconds for bmcmake metadata: ' + str(executionTime))
 
        build_lib = os.path.join(self.libsrc_folder,f'build_configs/xillib')
        mkdir(build_lib)
        copy_file(os.path.join(self.esw_lib_dir,'CMakeLists.txt'),f"{build_lib}{os.path.sep}")
        os.chdir(build_lib)
        startTime = time.time()
        runcmd(f'cmake . {cmake_paths_append} -DOS_ESW=ON -DLIB_LIST="{cmake_lib_list}" {cmake_cmd_append}')
        executionTime = (time.time() - startTime)
        print('Execution time in seconds for cmake: ' + str(executionTime))

        update_yaml(self.domain_config_file, 'domain', 'lib_config', lib_params_dict)
        update_yaml(self.domain_config_file, 'domain', 'app_config', app_config)


def apps_cmake_update(toolchain_file, app_name, proc):
    compiler_flags = ""
    if app_name == "zynqmp_fsbl":
        if "a53" in proc:
            compiler_flags = "-Os -flto -ffat-lto-objects -DARMA53_64"
        if "r5" in proc:
            compiler_flags = "-Os -flto -ffat-lto-objects -DARMR5"

        add_newline(toolchain_file, f'set( CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} {compiler_flags}")')
        add_newline(toolchain_file, f'set( CMAKE_ASM_FLAGS "${{CMAKE_ASM_FLAGS}} {compiler_flags}")')
    return compiler_flags

def get_machine(sdt):
    with open(sdt, 'r') as file:
        content = file.read()
        if "cpus_a53" in content:
            return "zynqmp"
        elif "cpus_a72" in content:
            return "versal"
        else:
            return

def create_domain(args):
    obj = Domain(args)
    obj.sdt , obj.toolchain_file = obj.toolchain_intr_mapping()
    obj.build_dir_struct()

    cmake_paths_append = f" -DCMAKE_LIBRARY_PATH={obj.lib_folder} \
            -DCMAKE_INCLUDE_PATH={obj.include_folder} \
            -DCMAKE_MODULE_PATH={obj.domaindir} \
            -DCMAKE_TOOLCHAIN_FILE={obj.toolchain_file}"

    os_srcdir = os.path.join(obj.get_comp_dir('standalone'),"src")
    bspsrc = os.path.join(obj.libsrc_folder,'standalone/src')
    copy_directory(os_srcdir, bspsrc)
    copy_file(f"{bspsrc}/common.cmake",os.path.join(obj.domaindir,'Findcommon.cmake'), silent_discard=False)
    os.chdir(obj.libsrc_folder)

    runcmd(f"lopper {obj.sdt} -- baremetaldrvlist_xlnx {obj.proc} {obj.repo}")

    with open('distro.conf', 'r') as fd:
        drv_names = re.search('DISTRO_FEATURES = "(.*)"', fd.readline()).group(1).replace('-','_')
        drvlist = drv_names.split()
        obj.drvlist = drvlist

    build_metadata = os.path.join(obj.libsrc_folder,'build_configs/metadata')
    mkdir(build_metadata)
    os.chdir(build_metadata)

    runcmd(f"cmake {obj.libsrc_folder} -DCMAKE_TOOLCHAIN_FILE={obj.toolchain_file} -DSDT={obj.sdt} {cmake_paths_append}")

    runcmd(f"make -f CMakeFiles/Makefile2 -j22 >/dev/null")

    os.chdir(obj.libsrc_folder)
    remove('distro.conf')
    remove('libxil.conf')
    remove(build_metadata)

    data = {
        "sdt" : os.path.relpath(obj.sdt, obj.domaindir),
        "os" : obj.os,
        "toolchain_file" : os.path.relpath(obj.toolchain_file, obj.domaindir),
        "proc" : obj.proc,
        "bsp_path" : os.path.relpath(obj.bspdir, obj.domaindir),
        "include_folder" : os.path.relpath(obj.include_folder, obj.domaindir),
        "lib_folder" : os.path.relpath(obj.lib_folder, obj.domaindir),
        "libsrc_folder" : os.path.relpath(obj.libsrc_folder, obj.domaindir),
        "drvlist" : drvlist,
        "lib_config" : {},
        "app_config" : {}
        }

    write_yaml(obj.domain_config_file, data)
    if is_file(obj.domain_config_file):
        print(f"Successfully Generated Domain {obj.domain_name}")

    if obj.app:
        obj.add_lib(obj.app, cmake_paths_append, is_app=True)


if __name__ == '__main__':
    import time
    startTime = time.time()
    parser = argparse.ArgumentParser(description='Generate bsp for different template applications',
                                     usage='use "python %(prog)s --help" for more information',
                                     formatter_class=argparse.RawTextHelpFormatter)
    required_argument = parser.add_argument_group('Required arguments')
    required_argument.add_argument('-proc', '--proc', action='store',
                              help=textwrap.dedent('''\
                                Specify the processor name
                                '''), required=True)
    required_argument.add_argument('-s', '--sdt', action='store',
                                    help='Specify the System device-tree path (till system-top.dts file)', required=True)
    required_argument.add_argument('-name', '--domain_name', action='store',help='Platform Workspace directory', default='standalone_domain')
    required_argument.add_argument('-w', '--wsdir', action='store',help='Workspace directory', required=True)
    parser.add_argument('-os', '--os', action='store', default='standalone', help='Specify OS (Default: standalone)', choices=['standalone','freertos'])
    parser.add_argument('-r', '--repo', action='store', default='',
                          help='Specify repo path')
    parser.add_argument('-template', '--app', action='store', default='', help='Specify template app name')
    
    args = vars(parser.parse_args())
    print(args)
    create_domain(args)
    executionTime = (time.time() - startTime)
    print('Execution time in seconds: ' + str(executionTime))
