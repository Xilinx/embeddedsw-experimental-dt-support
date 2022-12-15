"""
This module builds archive files (.a) for the created bsp. These archive files
include os, driver and library related archives.
"""

import utils
import argparse
import os


class BSP:
    """
    This class contains attributes and functions to build the created bsp.
    It takes the domain path as input, reads the domain configuration file
    present in the path to get the required inputs, calls make command 
    inside all the cmake build area and builds the archive (.a) files for 
    baremetal.
    """

    def __init__(self, args):
        self.domain_path = utils.get_abs_path(args.get("domain_path"))
        self.domain_config_file = os.path.join(self.domain_path, "bsp.yaml")
        utils.validate_if_not_exist(
            self.domain_config_file, "domain", utils.get_base_name(self.domain_path)
        )
        domain_data = utils.fetch_yaml_data(self.domain_config_file, "domain")

        self.sdt = os.path.join(self.domain_path, domain_data["sdt"])
        self.proc = domain_data["proc"]
        self.os = domain_data["os"]
        self.include_folder = os.path.join(
            self.domain_path, domain_data["include_folder"]
        )
        self.lib_folder = os.path.join(self.domain_path, domain_data["lib_folder"])
        self.libsrc_folder = os.path.join(
            self.domain_path, domain_data["libsrc_folder"]
        )
        self.toolchain_file = os.path.join(
            self.domain_path, domain_data["toolchain_file"]
        )
        self.cmake_paths_append = f" -DCMAKE_LIBRARY_PATH={self.lib_folder} \
            -DCMAKE_INCLUDE_PATH={self.include_folder} \
            -DCMAKE_MODULE_PATH={self.domain_path} \
            -DCMAKE_TOOLCHAIN_FILE={self.toolchain_file}"

        self.drvlist = domain_data["drvlist"]
        self.lib_config = domain_data["lib_config"]

    def gen_xilstandalone(self):
        """
        Compiles the files of standalone bsp folder and generates xilstandalone.a
        """
        bspsrc = os.path.join(self.libsrc_folder, "standalone/src")
        build_xilstandalone = os.path.join(
            self.libsrc_folder, "build_configs/xilstandalone"
        )
        utils.mkdir(build_xilstandalone)
        utils.runcmd(f"cmake {bspsrc} -DNON_YOCTO=ON {self.cmake_paths_append}", cwd=build_xilstandalone)
        utils.runcmd("make -j22", cwd=build_xilstandalone)
        utils.runcmd("make install", cwd=build_xilstandalone)

    def gen_xilfreertos(self):
        """
        Compiles the files of freertos bsp folder and generates xilfreertos.a
        """
        bspsrc = os.path.join(self.libsrc_folder, "freertos10_xilinx/src")
        build_xilfreertos = os.path.join(
            self.libsrc_folder, "build_configs/xilfreertos"
        )
        utils.mkdir(build_xilfreertos)
        utils.runcmd(f"cmake {bspsrc} -DNON_YOCTO=ON {self.cmake_paths_append}", cwd=build_xilfreertos)
        utils.runcmd("make -j22", cwd=build_xilfreertos)
        utils.runcmd("make install", cwd=build_xilfreertos)

    def gen_libxil(self):
        """
        Compiles all the driver source files and generates libxil.a
        """
        build_libxil = os.path.join(self.libsrc_folder, "build_configs/libxil")
        utils.mkdir(build_libxil)
        utils.runcmd(f"cmake {self.libsrc_folder} -DNON_YOCTO=ON {self.cmake_paths_append}", cwd=build_libxil)
        utils.runcmd("make -j22", cwd=build_libxil)
        utils.runcmd("make install", cwd=build_libxil)

    def build_lib(self):
        """
        Compiles all the library source files added in the bsp and generates 
        corresponding .a file.
        """
        build_xillib = os.path.join(self.libsrc_folder, "build_configs/xillib")
        if utils.is_dir(build_xillib):
            utils.runcmd("make -j22", cwd=build_xillib)
            utils.runcmd("make install", cwd=build_xillib)


def generate_bsp(args):
    """
    Function to compile the created bsp for the user input domain path.
    """
    obj = BSP(args)
    obj.gen_xilstandalone()
    obj.gen_libxil()
    obj.build_lib()
    if obj.os == "freertos":
        obj.gen_xilfreertos()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Build the created bsp",
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    required_argument = parser.add_argument_group("Required arguments")
    required_argument.add_argument(
        "-d",
        "--domain_path",
        action="store",
        help="Domain directory Path",
        required=True,
    )
    args = vars(parser.parse_args())
    generate_bsp(args)
