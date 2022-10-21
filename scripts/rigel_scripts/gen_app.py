from gen_bsp import BSP
from utils import *
import argparse, textwrap
import lopper
import os
import re
from collections import OrderedDict

class App_gen(BSP):
    def __init__(self, args):
        #FIXME: Validation of the args
        self.sdt = get_abs_path(args['sdt'])
        self.proc = args['proc'] 
        self.name = args['name'] 
        self.wsdir = get_abs_path(args['wsdir'])
        mkdir(self.wsdir)
        self.lib = args['lib']
        if args['repo']:
            self.repo = get_abs_path(args['repo'])
        else:
            self.repo = os.environ['ESW_REPO']
        self.os = args['os']
        self.ext_bsp = args['ext_bsp']
        self.build_till_bsp = args['build_till_bsp']
        self.lops_dir = os.path.join(os.path.dirname(lopper.__file__),'lops')
        if self.name:
            self.bsp_folder = os.path.join(self.wsdir, f"{self.name}_bsp", self.proc)
        else:
            self.bsp_folder = os.path.join(self.wsdir, self.proc)

        self.include_folder = os.path.join(self.bsp_folder,'include')
        self.lib_folder = os.path.join(self.bsp_folder,'lib')
        self.libsrc_folder = os.path.join(self.bsp_folder,'libsrc')

        if self.ext_bsp:
            self.ext_bsp = get_abs_path(args['ext_bsp'])
            assert is_dir(self.ext_bsp), "Provided External BSP path doesnt exist"
            self.toolchain_file = find_file("*_toolchain.cmake", self.ext_bsp)
            assert is_file(self.toolchain_file), "Tool Chain File couldnt be found in the BSP."
            self.sdt =  find_file("*.dts", self.ext_bsp)
            assert is_file(self.sdt), "Baremetal SDT couldnt be found in the BSP."
            self.bsp_folder = os.path.join(get_dir_path(self.toolchain_file),self.proc)
            self.include_folder = os.path.join(self.bsp_folder,'include')
            self.lib_folder = os.path.join(self.bsp_folder,'lib')
            self.libsrc_folder = os.path.join(self.bsp_folder,'libsrc')
        else:
            self.build_dir_struct()
            self.sdt , self.toolchain_file = self.toolchain_intr_mapping()
        
        self.machine = self.fetch_machine()
        self.esw_apps_dir = os.path.join(self.repo, "lib/sw_apps")
        self.esw_lib_dir = os.path.join(self.repo, "lib/sw_services")
        self.esw_drivers_dir = os.path.join(self.repo,"XilinxProcessorIPLib/drivers")
        self.esw_thirdparty_dir = os.path.join(self.repo,"ThirdParty")


    def build_dir_struct(self):
        mkdir(self.include_folder)
        mkdir(self.lib_folder)
        mkdir(self.libsrc_folder)


    def toolchain_intr_mapping(self):
        bsp_work_folder = get_dir_path(self.bsp_folder)
        os.chdir(bsp_work_folder)
        proc_lops_map = {
                'a53' : ('cortexa53','lop-a53-imux'),
                'a72' : ('cortexa72','lop-a72-imux'),
                'r5'  : ('cortexr5','lop-r5-imux'),
                'pmu' : ('microblaze-pmu',''),
                'pmc' : ('microblaze-plm',''),
                'psm' : ('microblaze-psm','')
            }
        lops_file = ''
        out_dts_path = os.path.join(bsp_work_folder, f'{self.proc}_baremetal.dts')
        for val in proc_lops_map.keys():
            if val in self.proc:
                toolchain_file_name = f'{proc_lops_map[val][0]}_toolchain.cmake'
                toolchain_file_path = os.path.join(
                        self.repo, f'cmake/toolchainfiles/{toolchain_file_name}'
                    )
                lops_file = os.path.join(self.lops_dir, f'{proc_lops_map[val][1]}.dts')
                toolchain_file_copy = os.path.join(bsp_work_folder, toolchain_file_name)
                copy_file(toolchain_file_path, toolchain_file_copy)
                with open(toolchain_file_copy, 'r+') as fd:
                    content = fd.readlines()
                    content.insert(0, f"set( CMAKE_INCLUDE_PATH {self.include_folder})\n")
                    content.insert(0, f"set( CMAKE_LIBRARY_PATH {self.lib_folder})\n")
                    content.insert(0, f"list(APPEND CMAKE_MODULE_PATH {bsp_work_folder})\n")
                    fd.seek(0, 0)
                    fd.writelines(content)
                break

        if not is_file(out_dts_path) and is_file(lops_file):
            runcmd(f"lopper -f --enhanced -i {lops_file} {self.sdt} {out_dts_path}")
        elif not is_file(out_dts_path):
            runcmd(f"lopper -f --enhanced {self.sdt} {out_dts_path}")

        return out_dts_path, toolchain_file_copy

    def fetch_machine(self):
        with open(self.toolchain_file, 'r') as fd:
            file_lines = fd.readlines()
            for line in file_lines:
                if "ESW_MACHINE" in line:
                    machine = re.search('ESW_MACHINE(.*)\)', line).group(1)
                    return machine

    def generate_app(self):
        srcdir = os.path.join(self.esw_apps_dir, f"{self.name}/src/")
        os.chdir(self.wsdir)
        linker_cmd = f"lopper {self.sdt} -- baremetallinker_xlnx {self.machine} {srcdir}"
        if self.name == "memory_tests":
            runcmd(f"{linker_cmd} memtest")
        else:
            runcmd(linker_cmd)
        linker_dir = os.path.join(self.wsdir,'linker_files')
        linker_src = os.path.join(self.repo,'scripts/linker_files')
        copy_directory(linker_src,linker_dir)
        copy_directory(srcdir, self.wsdir)
        if self.name == "peripheral_tests":
            runcmd(f"lopper {self.sdt} -- baremetal_gentestapp_xlnx {self.machine} {self.repo}")
        app_work_dir = os.path.join(self.wsdir,f'build_{self.name}')
        mkdir(app_work_dir)
        os.chdir(app_work_dir)
        runcmd(f"cmake {self.wsdir} -DCMAKE_TOOLCHAIN_FILE={self.toolchain_file} -DOS_ESW=ON")
        runcmd("make -j22")
        remove(linker_dir)
        elf_list = find_files("*.elf",app_work_dir)
        for elfs in elf_list:
            copy_file(elfs, f"{self.wsdir}/")

    def build_requirement(self):
        if not self.ext_bsp:
            self.generate_bsp()
        if not self.build_till_bsp and self.name:
            self.lib_addition()
            self.generate_app()


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("You haven't specified any arguments. Use -h to get more details on how to use this command.")
        sys.exit(1)

    parser = argparse.ArgumentParser(description='Generate bsp for different template applications',
                                     usage='use "python %(prog)s --help" for more information',
                                     formatter_class=argparse.RawTextHelpFormatter)
    required_argument = parser.add_argument_group('Required arguments')
    required_argument.add_argument('-s', '--sdt', action='store',
                                    help='Specify the System device-tree path (till system-top.dts file)', required=True)
    required_argument.add_argument('-p', '--proc', action='store',
                              help=textwrap.dedent('''\
                                Specify the processor name
                                '''), required=True)
    parser.add_argument('-n', '--name', action='store', default='',
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
                                '''))
    parser.add_argument('-r', '--repo', action='store', default='',
                          help='Specify repo path')
    parser.add_argument('-w', '--wsdir', action='store', default='workspace',
                          help='Workspace directory')
    parser.add_argument('-o', '--os', action='store', default='standalone',
                          help='Specify OS (Default: standalone)')
    parser.add_argument('-l', '--lib', action='append', nargs='*', default=[],
                          help='Specify libaries needs to be added if any)')
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
    parser.add_argument('-e', '--ext_bsp', action='store', default='',
                          help='Specify the language c or c++')
    parser.add_argument('-bsp', '--build_till_bsp', action='store', default=False,
                          help='Mark it True value if you want to build everything')
    args = parser.parse_args()

    obj = App_gen(vars(args))
    obj.build_requirement()
    sys.exit(0)
