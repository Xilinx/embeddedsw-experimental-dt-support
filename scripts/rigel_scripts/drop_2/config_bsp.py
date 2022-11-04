import argparse, textwrap
import sys
import os
import lopper
from utils import *
import re
from create_bsp import Domain
from build_bsp import BSP

class Bsp_config(BSP, Domain):
    def __init__(self,args):
        Domain.__init__(self, args, domain_create=False)
        BSP.__init__(self, args)
        self.addlib = args['addlib']
        self.rmlib = args['rmlib']

def library_addition(args):
    obj = Bsp_config(args)
    if obj.addlib:
        #FIXME: Validate lib for addition as per proc and OS
        #runcmd(f"lopper {self.sdt} -- baremetal_getsupported_app.py {self.proc} {self.repo}")
        obj.add_lib(obj.addlib, obj.cmake_paths_append)
    # if obj.rmlib:
    #     lib_path = os.path.join(obj.libsrc_folder, obj.rmlib)
    #     remove(lib_path)
    #     update_yaml(obj.domain_config_file, 'domain', obj.rmlib, None, action="remove")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Use this script to modify BSP Settings',
                                     usage='use "python %(prog)s --help" for more information',
                                     formatter_class=argparse.RawTextHelpFormatter)
    required_argument = parser.add_argument_group('Required arguments')
    required_argument.add_argument('-domain', '--domain_path', action='store',help='Domain directory Path', required=True)
    parser.add_argument('-addlib', '--addlib', action='store', default='',
                          help='Specify libaries that needs to be added if any)')
    parser.add_argument('-rmlib', '--rmlib', action='store', default='',
                          help='Specify libaries that needs to be removed if any)')
    args = vars(parser.parse_args())
    library_addition(args)
