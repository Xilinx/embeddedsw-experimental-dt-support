"""
This module re creates the bsp for a given domain and system device-tree.
"""

import utils
import argparse
import os
from create_bsp import Domain, create_domain 
from library_utils import Library
from build_bsp import BSP
import inspect

class RegenBSP(BSP, Library):
    """
    This class contains attributes and functions to regenerate the bsp.
    It takes the domain path and sdt as inputs, reads the domain configuration
    file present in the path to get the required inputs.
    """

    def __init__(self, args):
        self.domain_path = utils.get_abs_path(args.get("domain_path"))
        BSP.__init__(self, args)
        if args.get('sdt'):
            self.sdt = utils.get_abs_path(args["sdt"])
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

    def modify_bsp(self, args):
        args.update({
            'ws_dir':self.domain_path,
            'proc':self.proc,
            'os':self.os,
            'template':self.template,
            'sdt':self.sdt
        })

        # Remove existing folder structure
        utils.remove(self.libsrc_folder)
        utils.remove(self.include_folder)
        utils.remove(self.lib_folder)

        # Create BSP
        create_domain(args)

        # Get the library list in the new bsp
        domain_data = utils.fetch_yaml_data(self.domain_config_file, "domain")
        lib_list = list(domain_data["lib_config"].keys()) + [self.proc, self.os]
        drvlist = list(domain_data["drv_info"].keys())
        add_lib_list = [lib for lib in self.lib_list if lib not in lib_list]

        # Check library list against existing lib_list in the previous bsp.yaml 
        # if lib is missing add the library
        cmake_file = os.path.join(self.domain_path, "CMakeLists.txt")
        build_folder = os.path.join(self.libsrc_folder, "build_configs")
        ignored_lib_list = []
        changed_version_lib_list = []
        for lib in add_lib_list:
            cur_lib_version = self.lib_info[lib]['version']
            lib_version_fetched = ''
            if cur_lib_version in self.repo_schema.get('library').get(lib,{}).keys():
                lib_version_fetched = cur_lib_version
            else:
                changed_version_lib_list += [lib]
            if not self.validate_drv_for_lib(lib, drvlist, lib_version_fetched):
                ignored_lib_list.append(lib)
                continue
            self.gen_lib_cmake(lib, lib_version_fetched)
            lib_list, cmake_cmd_append = self.add_lib(lib, is_app=False, version=lib_version_fetched)
            self.config_lib(lib, lib_list, cmake_cmd_append, is_app=False, version=lib_version_fetched)

        # Apply the Old software configuraiton
        cmake_cmd_append = ""
        # cmake syntax is using 'ON/OFF' option, 'True/False' is lagacy entry.
        bool_match = {"true": "ON", "false": "OFF"}
        for lib in self.lib_list:
            for key, value in self.bsp_lib_config[lib].items():
                val = value['value']
                if val in bool_match:
                    val = bool_match[val]
                cmake_cmd_append += f" -D{key}='{val}'"

        build_metadata = os.path.join(self.libsrc_folder, "build_configs", "gen_bsp")
        utils.runcmd(f"cmake {self.domain_path} {self.cmake_paths_append} -DNON_YOCTO=ON {cmake_cmd_append}", cwd=build_metadata)

        # Run cmake configuration to get cache entries update bsp yaml with this data
        utils.runcmd(
                f'cmake {self.domain_path} {self.cmake_paths_append} -DNON_YOCTO=ON -LH > cmake_lib_configs.txt',
                cwd = build_metadata
        )
        lib_config = self.get_default_lib_params(build_metadata, add_lib_list)
        utils.update_yaml(self.domain_config_file, "domain", "lib_config", lib_config)
        proc_config = self.get_default_lib_params(build_metadata, [self.proc])
        utils.update_yaml(self.domain_config_file, "domain", "proc_config", proc_config)
        os_config = self.get_default_lib_params(build_metadata, [self.os])
        utils.update_yaml(self.domain_config_file, "domain", "os_config", os_config)

        # In case of Re generate BSP with different SDT print differences
        add_drv_list = [drv for drv in drvlist if drv not in self.drvlist]
        del_drv_list = [drv for drv in self.drvlist if drv not in drvlist]
        if add_drv_list or del_drv_list or ignored_lib_list or changed_version_lib_list:
            print(f"During Regeneration of BSP")
            if add_drv_list:
                print(f"Drivers {*add_drv_list,} got added")
            if del_drv_list:
                print(f"Drivers {*del_drv_list,} got deleted")
            if ignored_lib_list:
                print(f"Libraries {*ignored_lib_list,} ignored due to incompatible with new system device-tree")
            if changed_version_lib_list:
                print(f"Libraries {*changed_version_lib_list,} changed their versions")


def regenerate_bsp(args):
    """
    Function to re generate the bsp for the user input domain path and system device-tree.
    """
    obj = RegenBSP(args)
    obj.modify_bsp(args)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Regenerate the BSP",
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
        "-s",
        "--sdt",
        action="store",
        help="Specify the System device-tree path (till system-top.dts file)"
    )
    parser.add_argument(
        "-r",
        "--repo_info",
        action="store",
        help="Specify the .repo.yaml absolute path to use the set repo info",
        default='.repo.yaml',
    )
    args = vars(parser.parse_args())
    regenerate_bsp(args)
