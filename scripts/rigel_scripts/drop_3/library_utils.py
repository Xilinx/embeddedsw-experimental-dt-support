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
        self, domain_path, proc, bsp_os, sdt, cmake_paths_append, libsrc_folder
    ):
        super().__init__()
        self.domain_path = domain_path
        self.proc = proc
        self.os = bsp_os
        self.sdt = sdt
        self.domain_config_file = os.path.join(self.domain_path, ".domain.yaml")
        self.cmake_paths_append = cmake_paths_append
        self.libsrc_folder = libsrc_folder
        self.bsp_lib_config = utils.fetch_yaml_data(self.domain_config_file, "domain")[
            "lib_config"
        ]

    def validate_lib_name(self, lib):
        """
        Checks if the passed library name from the user is valid for the sdt 
        proc and os combination. Exits with valid assertion if the user input 
        is wrong.
        
        Args:
            lib (str): Library name that needs to be validated
        """
        lib_list_yaml_path = os.path.join(self.domain_path, "lib_list.yaml")
        if os.environ.get("VAL_INPUTS"):
            if not utils.is_file(lib_list_yaml_path):
                utils.runcmd(
                    f"lopper --werror -f -O {self.domain_path} {self.sdt} -- baremetal_getsupported_comp_xlnx {self.proc} {self.repo}"
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
        if os.environ.get("VAL_INPUTS") and lib not in self.bsp_lib_config.keys():
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
            os.environ.get("VAL_INPUTS")
            and lib_param not in self.bsp_lib_config[lib].keys()
        ):
            print(
                f"{lib_param} is not a valid param for {lib}. Valid list of params are {self.bsp_lib_config[lib].keys()}"
            )
            sys.exit(1)

    def copy_lib_src(self, lib):
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
        libdir = self.get_comp_dir(lib, is_app=False)
        srcdir = os.path.join(libdir, "src/")
        dstdir = os.path.join(self.libsrc_folder, f"{lib}/src")
        utils.copy_directory(srcdir, dstdir)
        return libdir, srcdir, dstdir

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
            for lib in lib_list:
                if lib not in default_lib_config.keys():
                    default_lib_config[lib] = {}
                # lwip param names are starting with lwip in cmake file but
                # lib name is lwip211.
                prefix = "lwip" if lib == "lwip211" else lib
                if re.search(f"^{prefix}", line_entries[line_index], re.I):
                    param_name = line_entries[line_index].split(":")[0]
                    # In cmake there are just two types of params: option and string.
                    param_type = line_entries[line_index].split(":")[1].split("=")[0]
                    # cmake syntax is using 'ON/OFF' option, 'True/False' is lagacy entry.
                    bool_match = {"ON": "True", "OFF": "False"}
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
                            param_opts = ["True", "False"]
                        elif param_type == "STRING":
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

    def add_lib(self, comp_name, is_app=False):
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
            libdir, srcdir, dstdir = self.copy_lib_src(comp_name)

        # Read the yaml file of the passed component.
        comp_dir = self.get_comp_dir(comp_name, is_app)
        yaml_file = os.path.join(comp_dir, f"data/{comp_name}.yaml")
        schema = utils.load_yaml(yaml_file)
        lib_config = {}
        if schema:
            # If the passed library has some driver dependency, generate that metadata
            if schema.get("depends") and not is_app:
                utils.runcmd(
                    f"lopper -O {dstdir} {self.sdt} -- bmcmake_metadata_xlnx {self.proc} {srcdir} hwcmake_metadata {self.repo}"
                )
            # If the passed template/lib has any lib dependency, add those dependencies.
            if schema.get("depends_libs", {}):
                for name, props in schema["depends_libs"].items():
                    cmake_lib_list += f"{name};"
                    lib_list += [name]
                    libdir, srcdir, dstdir = self.copy_lib_src(name)
                    if props:
                        # If the template needs specific config param of the lib.
                        for key, value in props.items():
                            cmake_cmd_append += f" -D{key}={value}"
                    lib_yaml_file = os.path.join(libdir, "data", f"{name}.yaml")
                    if utils.is_file(lib_yaml_file):
                        lib_schema = utils.load_yaml(lib_yaml_file)
                        # If the library (on which comp is depending) has some
                        # driver dependency, generate that metadata.
                        if lib_schema.get("depends"):
                            utils.runcmd(
                                f"lopper -O {dstdir} {self.sdt} -- bmcmake_metadata_xlnx {self.proc} {srcdir} hwcmake_metadata {self.repo}"
                            )

            if cmake_lib_list:
                # Create a cmake build directory for library compilation.
                build_lib = os.path.join(self.libsrc_folder, f"build_configs/xillib")
                utils.mkdir(build_lib)
                utils.copy_file(
                    os.path.join(self.esw_lib_dir, "CMakeLists.txt"),
                    f"{build_lib}{os.path.sep}",
                )

                # Run cmake configuration with all the default cache entries
                utils.runcmd(
                    f'cmake . {self.cmake_paths_append} -DOS_ESW=ON -DLIB_LIST="{cmake_lib_list}" -LH > cmake_lib_configs.txt',
                    cwd = build_lib
                )
                # Get the default cmake entries into yaml configuration file
                lib_config = self.get_default_lib_params(build_lib, lib_list)
                # Re-run cmake with modified lib entries
                utils.runcmd(f"cmake . {self.cmake_paths_append} -DOS_ESW=ON {cmake_cmd_append}", cwd = build_lib)

                # Add the modified lib param values in yaml configuration dict
                if schema.get("depends_libs", {}):
                    for name, props in schema["depends_libs"].items():
                        if props:
                            for key, value in props.items():
                                lib_config[name][key]["value"] = str(value)

                # Update the yaml config file with new entries.
                utils.update_yaml(self.domain_config_file, "domain", "lib_config", lib_config)
