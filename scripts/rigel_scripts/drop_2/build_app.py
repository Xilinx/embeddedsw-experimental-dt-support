from utils import get_abs_path, get_base_name, runcmd
import os
import argparse

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Use this script to build the cretaed app ',
                                     usage='use "python %(prog)s --help" for more information',
                                     formatter_class=argparse.RawTextHelpFormatter)
    required_argument = parser.add_argument_group('Required arguments')
    required_argument.add_argument('-app', '--app_path', action='store', help='Specify the App Workspace Directory', required=True)
    args = vars(parser.parse_args())
    app_path = get_abs_path(args.get('app_path'))
    app_name = get_base_name(app_path)
    app_build_dir = os.path.join(app_path, "Debug")
    os.chdir(app_build_dir)
    runcmd("make -j22")