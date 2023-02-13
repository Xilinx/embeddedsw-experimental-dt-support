"""
This module configures the bsp as per the passed library and os related 
parameters.
"""

import argparse
import sys
import os
import utils
from library_utils import Library
from build_bsp import BSP


class Bsp_config(BSP, Library):
    """
    This class contains attributes and functions that help in configuring the
    created bsp. This makes use of BSP and Library class attirbutes and 
    functions to fetch the bsp confiration data and the supporting lib funcs.
    """

    def __init__(self, args):
        BSP.__init__(self, args)
        Library.__init__(
            self,
            self.domain_path,
            self.proc,
            self.os,
            self.sdt,
            self.cmake_paths_append,
            self.libsrc_folder,
            args['repo_info']
        )
        self.addlib = args["addlib"]
        self.rmlib = args["rmlib"]


def configure_bsp(args):
    """
    This function uses Bsp_config class and configures the bsp based on the 
    user input arguments.

    Args:
        args (dict): User inputs in a dictionary format
    """
    obj = Bsp_config(args)

    # If user wants to add a library to the bsp
    if obj.addlib:
        lib_name = obj.addlib[0]
        lib_version = ''
        if len(obj.addlib) > 1:
            lib_version = float(obj.addlib[1])
        if lib_name in obj.bsp_lib_config.keys():
            if lib_version and lib_version != obj.lib_info[lib_name]['version']:
                obj.remove_lib(lib_name)
            else:
                print(f"""\b
                    {lib_name} is already added in the bsp. Nothing to do.
                    Use config_bsp.py if you want to configure the library.
                """)
                sys.exit(1)
        obj.validate_lib_name(lib_name, lib_version)
        obj.gen_lib_cmake(lib_name, lib_version)
        lib_list, cmake_cmd_append = obj.add_lib(lib_name, is_app=False, version=lib_version)
        obj.config_lib(lib_name, lib_list, cmake_cmd_append, is_app=False, version=lib_version)

    # If user wants to remove a library from the bsp
    if obj.rmlib:
        obj.remove_lib(obj.rmlib)

    # If user wants to set library parameters
    if args.get("set_property"):
        prop_params = args["set_property"]

        # Validate the command line inputs provided
        usage_print = """
            [ERROR]: Please Pass Library Name followed by param:value.
            e.g. -st xilffs XILFFS_read_only:ON XILFFS_use_lfn:1
            Wrong inputs passed with set_property.
        """
        if len(prop_params) < 2:
            print(usage_print)
            sys.exit(1)
        lib_name = prop_params[0]
        # assert error if library is not added in bsp
        obj.validate_lib_in_bsp(lib_name)

        prop_dict = {lib_name: {}}
        for entries in prop_params[1:]:
            if ":" not in entries:
                print(usage_print)
                sys.exit(1)
            else:
                prop_data = entries.split(":")
                prop_dict[lib_name].update({prop_data[0]: prop_data[1]})
                obj.validate_lib_param(lib_name, prop_data[0])
                # Set the passed value in lib config dictionary
                obj.bsp_lib_config[lib_name][prop_data[0]]["value"] = prop_data[1]

        # set the cmake options to append lib param values.
        cmake_cmd_append = ""
        for key, value in prop_dict[lib_name].items():
            if key == "proc_extra_compiler_flags":
                utils.add_newline(
                    obj.toolchain_file,
                    f'set( CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} {value}")',
                )
                utils.add_newline(
                    obj.toolchain_file,
                    f'set( CMAKE_ASM_FLAGS "${{CMAKE_ASM_FLAGS}} {value}")',
                )
            else:
                cmake_cmd_append += f" -D{key}={value}"

        # configure the lib build area with new params
        build_metadata = os.path.join(obj.libsrc_folder, "build_configs/gen_bsp")
        utils.runcmd(f"cmake {obj.domain_path} {obj.cmake_paths_append} -DNON_YOCTO=ON {cmake_cmd_append}", cwd=build_metadata)

        # Update the lib config file
        if obj.proc in obj.bsp_lib_config.keys():
            proc_config = obj.bsp_lib_config.pop(obj.proc)
            proc_config = {obj.proc:proc_config}
            utils.update_yaml(obj.domain_config_file, "domain", "proc_config", proc_config, action="add")
        if obj.os in obj.bsp_lib_config.keys(): 
            os_config = obj.bsp_lib_config.pop(obj.os)
            os_config = {obj.os:os_config}
            utils.update_yaml(obj.domain_config_file, "domain", "os_config", os_config, action="add")
        utils.update_yaml(obj.domain_config_file, "domain", "lib_config", obj.bsp_lib_config)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Use this script to modify BSP Settings",
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

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "-al",
        "--addlib",
        nargs='+',
        action="store",
        default=[],
        help="Specify libaries that needs to be added if any",
    )
    group.add_argument(
        "-rl",
        "--rmlib",
        action="store",
        default="",
        help="Specify libaries that needs to be removed if any",
    )
    group.add_argument(
        "-st",
        "--set_property",
        nargs="*",
        action="store",
        help="Specify libaries with the params that need to be configured",
    )
    parser.add_argument(
        "-r",
        "--repo_info",
        action="store",
        help="Specify the .repo.yaml absolute path to use the set repo info",
        default='.repo.yaml',
    )
    args = vars(parser.parse_args())
    configure_bsp(args)
