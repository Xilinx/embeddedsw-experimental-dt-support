from utils import *
import argparse, textwrap
import lopper
import os
import re
from build_bsp import BSP

class App(BSP):
    def __init__(self, args):
        self.wsdir = get_abs_path(args['wsdir'])
        self.name = args.get('name')
        self.appdir = os.path.join(self.wsdir,self.name)
        #assert not is_dir(self.appdir), f'App Directory with name "{self.name}" already exists.'
        mkdir(self.appdir)
        self.domain_path = args.get('domain_path')
        self.template = args.get('template')
        super().__init__(args)
        assert is_file(os.path.join(self.lib_folder, 'libxil.a')), f"Couldn't find libxil.a in the bsp, compile the BSP first"
        assert is_file(os.path.join(self.lib_folder, 'libxilstandalone.a')), \
            f"Couldn't find libxilstandalone.a in the bsp, compile the BSP first"

    #FIXME Validate BSP for the supported libs of the template app

def create_app(args):
    obj = App(args)
    srcdir = os.path.join(obj.get_comp_dir(obj.template, is_app=True), 'src/')
    app_src_dir = os.path.join(obj.appdir,"src")
    mkdir(app_src_dir)
    os.chdir(obj.appdir)
    app_config = obj.app_config.get(obj.template,{})
    for entry in app_config.keys():
        if 'hw_metadata' in entry:
            runcmd(f"lopper -O {app_src_dir} {obj.sdt} -- bmcmake_metadata_xlnx {obj.proc} {srcdir} hwcmake_metadata {obj.repo}")
            break
    linker_cmd = f"lopper -O {app_src_dir} {obj.sdt} -- baremetallinker_xlnx {obj.proc} {srcdir}"
    if obj.name == "memory_tests":
        runcmd(f"{linker_cmd} memtest")
    else:
        runcmd(linker_cmd)
    linker_dir = os.path.join(app_src_dir,'linker_files')
    linker_src = os.path.join(obj.repo,'scripts/linker_files')
    copy_directory(linker_src,linker_dir)
    if obj.template == "peripheral_tests":
        runcmd(f"lopper {obj.sdt} -- baremetal_gentestapp_xlnx {obj.machine} {obj.repo}")
    app_work_dir = os.path.join(obj.appdir,f'Debug')
    mkdir(app_work_dir)
    os.chdir(app_work_dir)
    copy_directory(srcdir, app_src_dir)
    runcmd(f"cmake {app_src_dir} {obj.cmake_paths_append} -DOS_ESW=ON")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Use this script to create a template App using the built BSP',
                                     usage='use "python %(prog)s --help" for more information',
                                     formatter_class=argparse.RawTextHelpFormatter)
    required_argument = parser.add_argument_group('Required arguments')
    required_argument.add_argument('-domain', '--domain_path', action='store',help='Specify the built BSP Path', required=True)
    required_argument.add_argument('-w', '--wsdir', action='store',help='Workspace directory', required=True)
    required_argument.add_argument('-name', '--name', action='store',help='Provide a name to your app directory', required=True)
    required_argument.add_argument('-template', '--template', action='store', required=True, help=textwrap.dedent('''\
                            'Specify template app name. Available names are:
                                - hello_world
                                - memory_tests
                                - peripheral_tests
                                - zynqmp_fsbl
                                - zynqmp_pmufw
                                - lwip_echo_server
                                - freertos_hello_world
                                - versal_plm
                                - versal_psmfw
                            '''))
    
    args = vars(parser.parse_args())
    create_app(args)