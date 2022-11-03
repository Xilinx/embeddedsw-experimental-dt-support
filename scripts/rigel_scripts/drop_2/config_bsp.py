import argparse, textwrap
import sys
import os
import lopper
from utils import *
import re
from create_bsp import Domain

class Bsp_config(Domain):
    def __init__(self,args):
        super().__init__(args, domain_create=False)
        self.lib = args['lib']

def library_addition(args):
    obj = Bsp_config(args)
    if obj.lib:
        obj.add_lib(obj.lib)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate bsp for different template applications',
                                     usage='use "python %(prog)s --help" for more information',
                                     formatter_class=argparse.RawTextHelpFormatter)
    required_argument = parser.add_argument_group('Required arguments')
    required_argument.add_argument('-domain', '--domain_name', action='store',help='Platform Workspace directory', default='standalone_domain')
    required_argument.add_argument('-w', '--wsdir', action='store',help='Workspace directory', required=True)
    required_argument.add_argument('-proc', '--proc', action='store',
                              help=textwrap.dedent('''\
                                Specify the processor name
                                '''), required=True)
    parser.add_argument('-o', '--os', action='store', default='standalone', help='Specify OS (Default: standalone)')
    parser.add_argument('-r', '--repo', action='store', default='',
                          help='Specify repo path')
    parser.add_argument('-addlib', '--lib', action='store', default='',
                          help='Specify libaries needs to be added if any)')
    args = vars(parser.parse_args())
    print(args)
    library_addition(args)
