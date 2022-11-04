from utils import *
import argparse, textwrap
import lopper
import os
import re
from repo import Repo

class BSP(Repo):
    def __init__(self,args):
        Repo.__init__(self, args.get('repo'))
        self.domain_path = get_abs_path(args.get('domain_path'))
        self.domain_config_file = os.path.join(self.domain_path,'.domain.yaml')
        validate_if_not_exist(self.domain_config_file,'domain',get_base_name(self.domain_path))
        domain_data = fetch_yaml_data(self.domain_config_file, 'domain')

        self.sdt = os.path.join(self.domain_path, domain_data['sdt'])
        self.proc = domain_data['proc']
        self.os = domain_data['os']
        self.include_folder = os.path.join(self.domain_path, domain_data['include_folder'])
        self.lib_folder = os.path.join(self.domain_path, domain_data['lib_folder'])
        self.libsrc_folder = os.path.join(self.domain_path, domain_data['libsrc_folder'])
        self.toolchain_file = os.path.join(self.domain_path, domain_data['toolchain_file'])
        self.cmake_paths_append = f" -DCMAKE_LIBRARY_PATH={self.lib_folder} \
            -DCMAKE_INCLUDE_PATH={self.include_folder} \
            -DCMAKE_MODULE_PATH={self.domain_path} \
            -DCMAKE_TOOLCHAIN_FILE={self.toolchain_file}"

        self.drvlist = domain_data['drvlist']
        self.lib_config = domain_data['lib_config']
        self.app_config = domain_data['app_config']


    def gen_xilstandalone(self):
        os_srcdir = os.path.join(self.get_comp_dir('standalone'),"src")
        bspsrc = os.path.join(self.libsrc_folder,'standalone/src')
        runcmd(f"lopper -O {bspsrc} {self.sdt} -- baremetal_bspconfig_xlnx {self.proc} {os_srcdir}")
        build_xilstandalone = os.path.join(self.libsrc_folder,'build_configs/xilstandalone')
        mkdir(build_xilstandalone)
        os.chdir(build_xilstandalone)
        runcmd(f"cmake {bspsrc} -DOS_ESW=ON {self.cmake_paths_append}")
        runcmd("make -j22")
        runcmd("make install")

    def gen_libxil(self):
        build_libxil = os.path.join(self.libsrc_folder,'build_configs/libxil')
        mkdir(build_libxil)
        os.chdir(build_libxil)
        libxil_cmake = os.path.join(self.esw_drivers_dir, "CMakeLists.txt")
        copy_file(libxil_cmake, f'{self.libsrc_folder}/')
        runcmd(f"cmake {self.libsrc_folder} -DOS_ESW=ON {self.cmake_paths_append}")
        runcmd("make -j22")
        runcmd("make install")

    def build_lib(self):
        build_xillib = os.path.join(self.libsrc_folder,'build_configs/xillib')
        if is_dir(build_xillib):
            os.chdir(build_xillib)
            runcmd("make -j22")
            runcmd("make install")

def generate_bsp(args):
    obj = BSP(args)
    obj.gen_xilstandalone()
    obj.gen_libxil()
    obj.build_lib()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Build the created bsp',
                                     usage='use "python %(prog)s --help" for more information',
                                     formatter_class=argparse.RawTextHelpFormatter)
    required_argument = parser.add_argument_group('Required arguments')
    required_argument.add_argument('-domain', '--domain_path', action='store',help='Domain directory Path', required=True)
    args = vars(parser.parse_args())
    generate_bsp(args)