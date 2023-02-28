"""
This module acts as a supporting module to get/set library related information
inside the bsp. It helps in validating the library input, generating the
library paramters database and adding/modifying the library when all the
criteria are met. It doesnt have any main() function and running this 
module independently is not intended.
"""


import sys
import os
import utils
import re
from repo import Repo


class Library(Repo):
    """
    This class contains attributes and functions that help in validating 
    library related inputs and adding a library to the created bsp.
    """

    def __init__(
        self, domain_path, proc, bsp_os, sdt, cmake_paths_append, libsrc_folder, repo_info
    ):
        super().__init__(repo_yaml_path= repo_info)
        self.domain_path = domain_path
        self.proc = proc
        self.os = bsp_os
        self.sdt = sdt
        self.domain_config_file = os.path.join(self.domain_path, "bsp.yaml")
        self.cmake_paths_append = cmake_paths_append
        self.libsrc_folder = libsrc_folder
        self.domain_data = utils.fetch_yaml_data(self.domain_config_file, "domain")
        self.bsp_lib_config = self.domain_data["lib_config"]
        self.bsp_lib_config.update(self.domain_data["os_config"])
        self.bsp_lib_config.update(self.domain_data["proc_config"])
        self.lib_list = list(self.domain_data["lib_config"].keys()) + [self.proc, self.os]
        self.lib_info = self.domain_data["lib_info"]

    def validate_lib_name(self, lib, version=''):
        """
        Checks if the passed library name from the user is valid for the sdt 
        proc and os combination. Exits with valid assertion if the user input 
        is wrong.
        
        Args:
            lib (str): Library name that needs to be validated
        """
        lib_list_yaml_path = os.path.join(self.domain_path, "lib_list.yaml")
        if os.environ.get("OSF"):
            if not utils.is_file(lib_list_yaml_path):
                utils.runcmd(
                    f"lopper --werror -f -O {self.domain_path} {self.sdt} -- baremetal_getsupported_comp_xlnx {self.proc} {self.repo_yaml_path}"
                )
            proc_os_data = utils.fetch_yaml_data(
                os.path.join(self.domain_path, "lib_list.yaml"), "lib_list"
            )
            lib_list_avail = list(proc_os_data[self.proc][self.os].keys())
            if lib not in lib_list_avail:
                print(
                    f"[ERROR]: {lib} is not a valid library for the given proc and os combination. Valid library names are {lib_list_avail}"
                )
                sys.exit(1)

    def validate_lib_in_bsp(self, lib):
        """
        Checks if the passed library name from the user exists in the bsp. This
        is a helper function to support remove library and set property usecases.

        Args:
            lib (str): Library name that needs to be validated
        """
        if os.environ.get("OSF") and lib not in self.bsp_lib_config.keys():
            print(f"{lib} is not added to the bsp. Add it first using -addlib")
            sys.exit(1)

    def validate_lib_param(self, lib, lib_param):
        """
        Checks if the passed library parameter that needs to be set in library
        configuration is valid. Exits with a valid assertion if parameter name
        is wrong. This acts as a helper in set property usecase.
        
        Args:
            | lib (str): Library name whose config needs to be changed
            | lib_param (str): Library parameter that needs to be changed
        """
        if (
            os.environ.get("OSF")
            and lib_param not in self.bsp_lib_config[lib].keys()
        ):
            print(
                f"{lib_param} is not a valid param for {lib}. Valid list of params are {self.bsp_lib_config[lib].keys()}"
            )
            sys.exit(1)

    def copy_lib_src(self, lib, version=''):
        """
        Copies the src directory of the passed library from the respective path
        of embeddedsw to the libsrc folder of bsp.

        Args:
            lib (str): library whose source code needs to be copied
        
        Returns:
            | libdir (str): Library path inside embeddedsw
            | srcdir (str): Path of src folder of library inside embeddedsw
            | dstdir (str): Path of src folder inside libsrc folder of bsp
        """
        libdir, lib_version = self.get_comp_dir(lib, version)
        srcdir = os.path.join(libdir, "src")
        dstdir = os.path.join(self.libsrc_folder, lib, "src")
        utils.copy_directory(srcdir, dstdir)
        self.lib_info[lib] = {'path': libdir, 'version': lib_version}
        return libdir, lib_version, srcdir, dstdir

    def get_default_lib_params(self, build_lib_dir, lib_list):
        """
        Creates a library configuration data that contains all the available 
        parameters and their values of each library added in the bsp.

        Args:
            build_lib_dir (str): 
                Cmake directory where the libraries are configured and compiled
            lib_list (str): List of libraries added in the bsp.
        
        Returns:
            default_lib_config (dict):
                A dictionary that contains all the available parameters and 
                their values of each library added in the bsp.
        """

        # Read all the cmake variables that got set during cmake config.
        with open(
            os.path.join(build_lib_dir, "cmake_lib_configs.txt"), "r"
        ) as cmake_confs:
            line_entries = cmake_confs.readlines()

        # Read all the cmake entries that got cached during cmake config. This
        # will help us in getting all the possible options for a given param.
        with open(os.path.join(build_lib_dir, "CMakeCache.txt"), "r") as cmake_cache:
            cmake_cache_entries = cmake_cache.readlines()

        default_lib_config = {}
        for line_index in range(0, len(line_entries)):
            lib_config_entry = {}
            permission = "read_write"
            for lib in lib_list:
                if lib not in default_lib_config.keys():
                    default_lib_config[lib] = {}
                # lwip param names are starting with lwip in cmake file but
                # lib name is lwip213.
                prefix = "lwip" if lib == "lwip213" else lib
                proc_prefix = "proc" if lib == self.proc else lib
                if (re.search(f"^{prefix}", line_entries[line_index], re.I) or
                   re.search(f"^{proc_prefix}", line_entries[line_index], re.I)):
                    param_name = line_entries[line_index].split(":")[0]
                    # In cmake there are just two types of params: option and string.
                    param_type = line_entries[line_index].split(":")[1].split("=")[0]
                    # cmake syntax is using 'ON/OFF' option, 'True/False' is lagacy entry.
                    bool_match = {"ON": "true", "OFF": "false"}
                    param_opts = []
                    try:
                        param_value = (
                            line_entries[line_index]
                            .split(":")[1]
                            .split("=")[1]
                            .rstrip("\n")
                        )
                        if param_type == "BOOL":
                            param_value = bool_match[param_value]
                            param_type = "boolean"
                            # Every entry from command line will come as string
                            param_opts = ["true", "false"]
                        elif param_type == "STRING":
                            # read only params
                            read_only_param = ["proc_archiver", "proc_assembler", "proc_compiler", "proc_compiler_flags"]
                            if param_name in read_only_param:
                                permission = "readonly"
                            # Showing it as integer (legacy entry)
                            if param_value.isdigit():
                                param_type = "integer"
                            else:
                                param_type = "string"
                            for line in cmake_cache_entries:
                                # Get the cached param options
                                if re.search(f"^{param_name}-STRINGS", line):
                                    try:
                                        param_opts = (
                                            line.rstrip("\n").split("=")[1].split(";")
                                        )
                                    except:
                                        param_opts = []
                    # For some entries there is no cache option (like start
                    # address which can be anything ), it's empty.
                    except:
                        param_value = ""
                        param_type = "integer"
                        param_opts = []

                    lib_config_entry = {
                        param_name: {
                            "name": param_name,
                            "permission": permission,
                            "type": param_type,
                            "value": param_value,
                            "default": param_value,
                            "options": param_opts,
                            "description": line_entries[line_index - 1]
                            .rstrip("\n")
                            .lstrip("// "),
                        }
                    }

                    default_lib_config[lib].update(lib_config_entry)

        return default_lib_config

    def validate_drv_for_lib(self, comp_name, drvlist, version=''):
        comp_dir, _ = self.get_comp_dir(comp_name, version)
        yaml_file = os.path.join(comp_dir, "data", f"{comp_name}.yaml")
        schema = utils.load_yaml(yaml_file)
        if schema.get("depends"):
            dep_drvlist = list(schema.get("depends").keys())
            valid_lib = [drv for drv in dep_drvlist if drv in drvlist]
            """
            Since sleep related implementation is part of xiltimer library
            it needs to be pulled irrespective of the hardware dependency.
            """
            if valid_lib or re.search("xiltimer", comp_name):
                return True
            else:
                return False
        return True

    def add_lib(self, comp_name, is_app=False, version=''):
        """
        Adds library to the bsp. Creates metadata if needed for the library, 
        runs cmake configure to prepare the build area for library 
        compilation and creates the library configuration of the bsp.

        Args:
            comp_name (str): 
                component name (either template or lib). If template depends on 
                certain libs, it fetches them otherwise it adds the passed library.
            is_app (bool): To distinguish between lib and template
        """
        cmake_cmd_append = ""
        cmake_lib_list = ""
        lib_list = []

        # Get a cmake list from available libs.
        for entries in self.bsp_lib_config.keys():
            if entries != comp_name:
                cmake_lib_list += f"{entries};"

        # If comp is not app, add them in the cmake list.
        if not is_app:
            cmake_lib_list += f"{comp_name};"
            lib_list = [comp_name]
            libdir, _, srcdir, dstdir = self.copy_lib_src(comp_name, version)

        # Read the yaml file of the passed component.
        comp_dir, _ = self.get_comp_dir(comp_name, version)
        yaml_file = os.path.join(comp_dir, "data", f"{comp_name}.yaml")
        schema = utils.load_yaml(yaml_file)
        lib_config = {}

        if schema:
            # If the passed template/lib has any lib dependency, add those dependencies.
            if schema.get("depends_libs", {}) and is_app:
                drvlist = list(utils.fetch_yaml_data(self.domain_config_file, "domain")["drv_info"].keys())
                for name, props in schema["depends_libs"].items():
                    if not self.validate_drv_for_lib(name, drvlist, version):
                        continue
                    cmake_lib_list += f"{name};"
                    lib_list += [name]
                    libdir, lib_version, srcdir, dstdir = self.copy_lib_src(name, version)
                    if props:
                        # If the template needs specific config param of the lib.
                        for key, value in props.items():
                            cmake_cmd_append += f" -D{key}={value}"

        return lib_list, cmake_cmd_append

    def config_lib(self, comp_name, lib_list, cmake_cmd_append, is_app=False, version=''):
        comp_dir, _ = self.get_comp_dir(comp_name, version)
        yaml_file = os.path.join(comp_dir, "data", f"{comp_name}.yaml")
        schema = utils.load_yaml(yaml_file)
        lib_config = {}
        if lib_list:
            # Run cmake configuration with all the default cache entries
            build_metadata = os.path.join(self.libsrc_folder, "build_configs/gen_bsp")
            self.cmake_paths_append = self.cmake_paths_append.replace('\\', '/')
            self.domain_path = self.domain_path.replace('\\', '/')
            build_metadata = build_metadata.replace('\\', '/')
            utils.runcmd(
                f'cmake {self.domain_path} {self.cmake_paths_append} -DNON_YOCTO=ON -LH > cmake_lib_configs.txt',
                cwd = build_metadata
            )

            # Get the default cmake entries into yaml configuration file
            lib_config = self.get_default_lib_params(build_metadata, lib_list)
            if is_app:
                cmake_cmd_append = cmake_cmd_append.replace('\\', '/')
                # Re-run cmake with modified lib entries
                utils.runcmd(f"cmake {self.domain_path} {self.cmake_paths_append} -DNON_YOCTO=ON {cmake_cmd_append}", cwd = build_metadata)
                # Add the modified lib param values in yaml configuration dict
                if schema.get("depends_libs", {}):
                    drvlist = list(utils.fetch_yaml_data(self.domain_config_file, "domain")["drv_info"].keys())
                    for name, props in schema["depends_libs"].items():
                        if not self.validate_drv_for_lib(name, drvlist, version):
                            continue
                        if props:
                            for key, value in props.items():
                                lib_config[name][key]["value"] = str(value)

            if lib_config.__contains__("freertos10_xilinx"):
                lib_config.pop("freertos10_xilinx")
            # Update the yaml config file with new entries.
            utils.update_yaml(self.domain_config_file, "domain", "lib_config", lib_config)
            utils.update_yaml(self.domain_config_file, "domain", "lib_info", self.lib_info)

    def gen_lib_cmake(self, lib, version=''):
        cmake_file = os.path.join(self.domain_path, "CMakeLists.txt")
        build_folder = os.path.join(self.libsrc_folder, "build_configs")
        libcmake_file = os.path.join(build_folder, f"{lib}.cmake")
        lib_dir_path, _ = self.get_comp_dir(lib, version)
        srcdir = os.path.join(lib_dir_path, "src")
        dstdir = os.path.join(self.libsrc_folder, f"{lib}/src")
        cmd = f"lopper -O {dstdir} -f {self.sdt} --  bmcmake_metadata_xlnx {self.proc} {srcdir} hwcmake_metadata {self.repo_yaml_path}"
        cmake_cmd = f"""
        execute_process(COMMAND {cmd})
        add_subdirectory({dstdir})
        """
        utils.write_into_file(libcmake_file,cmake_cmd)

        libsrc_exist = utils.check_if_line_in_file(cmake_file, f"add_subdirectory({self.libsrc_folder})")
        if libsrc_exist:
            utils.remove_line(cmake_file, f"add_subdirectory({self.libsrc_folder})")
        utils.add_newline(cmake_file, f"\ninclude (${{CMAKE_BINARY_DIR}}/../{lib}.cmake)")

    def remove_lib(self, lib):
        self.validate_lib_in_bsp(lib)
        lib_path = os.path.join(self.libsrc_folder, lib)
        base_lib_build_dir = os.path.join(self.libsrc_folder, "build_configs", "gen_bsp", "libsrc")
        lib_build_dir = os.path.join(base_lib_build_dir, lib)

        cmake_file = os.path.join(self.domain_path, "CMakeLists.txt")
        utils.remove_line(cmake_file, lib)

        # Run make clean to remove the respective headers and .a from lib and include folder.
        utils.runcmd(f"make -C {os.path.join(lib, 'src')} clean", cwd=base_lib_build_dir)
        # Remove library src folder from libsrc
        utils.remove(lib_path)
        # Remove cmake build folder from cmake build area.
        utils.remove(lib_build_dir)
        # Update library config file
        utils.update_yaml(self.domain_config_file, "domain", lib, None, action="remove")
        self.bsp_lib_config.pop(lib)
        # If the library being removed was the only lib in bsp, remove the cmake build dir
        if not self.bsp_lib_config:
            utils.remove(base_lib_build_dir)
