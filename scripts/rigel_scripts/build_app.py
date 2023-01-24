"""
This module builds an already created app. It doesn't contain any members
other than main().
"""

import utils
import os
import argparse
from build_bsp import generate_bsp
from build_bsp import BSP
from repo import Repo
from validate_bsp import Validation

class Build_App(BSP, Repo):
    """
    This class helps in building a template application.
    """

    def __init__(self, args):
        self._build_dir_struct(args)
        BSP.__init__(self, args)
        Repo.__init__(self)

    def _build_dir_struct(self, args):
        if args["ws_dir"] == '.' and not args.get('build_dir'):
            print("[WARNING]: The app Workspace is taken as current working directory. To avoid this, please use -w option")
        self.app_dir = utils.get_abs_path(args.get("ws_dir"))
        if args.get('src_dir'):
            self.app_src_dir = utils.get_abs_path(args["src_dir"])
        else:
            self.app_src_dir = os.path.join(self.app_dir, "src")
        if args.get('build_dir'):
            self.app_build_dir = utils.get_abs_path(args["build_dir"])
        else:
            self.app_build_dir = os.path.join(self.app_dir, "build")
        utils.mkdir(self.app_build_dir)
        self.app_config_file = os.path.join(self.app_src_dir, "app.yaml")
        self.domain_path = utils.fetch_yaml_data(self.app_config_file, "domain_path")["domain_path"]
        args["domain_path"] = self.domain_path

def build_app(args):
    obj = Build_App(args)

    # Build the bsp first before building application
    libxil_a_path = os.path.join(obj.domain_path, 'lib', 'libxil.a')
    libxilstandalone_a_path = os.path.join(obj.domain_path, 'lib', 'libxilstandalone.a')

    if not utils.is_file(libxil_a_path) or not utils.is_file(libxilstandalone_a_path):
        generate_bsp(args)

    # Run make inside cmake configured build area
    obj.app_src_dir = obj.app_src_dir.replace('\\', '/')
    obj.cmake_paths_append = obj.cmake_paths_append.replace('\\', '/')
    obj.app_build_dir = obj.app_build_dir.replace('\\', '/')
    utils.runcmd(f"cmake {obj.app_src_dir} {obj.cmake_paths_append} -DNON_YOCTO=ON", cwd=obj.app_build_dir)
    utils.runcmd("make -j22", cwd=obj.app_build_dir)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Use this script to build the cretaed app ",
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )

    # Get the app_path created by the user
    parser.add_argument(
        "-w",
        "--ws_dir",
        action="store",
        help="Specify the App Workspace Directory",
        default='.',
    )

    parser.add_argument(
        "-b",
        "--build_dir",
        action="store",
        help="Specify the App Build Directory",
    )

    parser.add_argument(
        "-s",
        "--src_dir",
        action="store",
        help="Specify the App source directory "
    )

    args = vars(parser.parse_args())
    build_app(args)
