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
        if obj.addlib in obj.bsp_lib_config.keys():
            print(f"""\b
                {obj.addlib} is already added in the bsp. Nothing to do.
                Use config_bsp.py if you want to configure the library.
            """)
            sys.exit(1)
        obj.validate_lib_name(obj.addlib)
        cmake_file = os.path.join(obj.domain_path, "CMakeLists.txt")
        build_folder = os.path.join(obj.libsrc_folder, "build_configs")
        libcmake_file = os.path.join(build_folder, f"{obj.addlib}.cmake")
        srcdir = os.path.join(obj.get_comp_dir(obj.addlib, is_app=False), "src")
        dstdir = os.path.join(obj.libsrc_folder, f"{obj.addlib}/src")
        cmd = f"lopper -O {dstdir} -f {obj.sdt} --  bmcmake_metadata_xlnx {obj.proc} {srcdir} hwcmake_metadata {obj.repo}"
        with open(libcmake_file, 'w') as fd:
            fd.write(f"execute_process(COMMAND {cmd})")
            fd.write("\n"f"add_subdirectory({dstdir})\n")

        libsrc_exist = utils.check_if_line_in_file(cmake_file, f"add_subdirectory({obj.libsrc_folder})")
        if libsrc_exist:
            utils.remove_line(cmake_file, f"add_subdirectory({obj.libsrc_folder})")
        utils.add_newline(cmake_file, f"\ninclude (${{CMAKE_BINARY_DIR}}/../{obj.addlib}.cmake)")
        lib_list, cmake_cmd_append = obj.add_lib(obj.addlib)
        obj.config_lib(obj.addlib, lib_list, cmake_cmd_append)

    # If user wants to remove a library from the bsp
    if obj.rmlib:
        obj.validate_lib_in_bsp(obj.rmlib)
        lib_path = os.path.join(obj.libsrc_folder, obj.rmlib)
        base_lib_build_dir = os.path.join(obj.libsrc_folder, "build_configs", "gen_bsp", "libsrc")
        lib_build_dir = os.path.join(base_lib_build_dir, obj.rmlib)

        cmake_file = os.path.join(obj.domain_path, "CMakeLists.txt")
        utils.remove_line(cmake_file, obj.rmlib)

        # Run make clean to remove the respective headers and .a from lib and include folder.
        utils.runcmd(f"make -C {os.path.join(obj.rmlib, 'src')} clean", cwd=base_lib_build_dir)
        # Remove library src folder from libsrc
        utils.remove(lib_path)
        # Remove cmake build folder from cmake build area.
        utils.remove(lib_build_dir)
        # Update library config file
        utils.update_yaml(obj.domain_config_file, "domain", obj.rmlib, None, action="remove")
        obj.bsp_lib_config.pop(obj.rmlib)
        # If the library being removed was the only lib in bsp, remove the cmake build dir
        if not obj.bsp_lib_config:
            utils.remove(base_lib_build_dir)

    # If user wants to set library parameters
    if args.get("set_property"):
        prop_params = args["set_property"]

        # Validate the command line inputs provided
        usage_print = """
            [Usage]: Please Pass Library Name followed by param:value.
            e.g. -set_property xilffs XILFFS_read_only:ON XILFFS_use_lfn:1 
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
    parser.add_argument(
        "-al",
        "--addlib",
        action="store",
        default="",
        help="Specify libaries that needs to be added if any",
    )
    parser.add_argument(
        "-rl",
        "--rmlib",
        action="store",
        default="",
        help="Specify libaries that needs to be removed if any",
    )
    parser.add_argument(
        "-st",
        "--set_property",
        nargs="*",
        action="store",
        help="Specify libaries with the params that need to be configured",
    )
    args = vars(parser.parse_args())
    configure_bsp(args)
