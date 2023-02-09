"""
This module creates a domain and a bsp for the passed processor, os and system
device tree combination.
"""

import argparse, textwrap
import sys
import os
import lopper
import utils
import re
from library_utils import Library
from repo import Repo
from validate_bsp import Validation


class Domain(Repo):
    """
    This class helps in creating a software domain. This contains functions
    to create the domain's directory structure, validate the user inputs for
    the domain on demand and manipulate the cmake toolchain file as per the
    user inputs.
    """

    def __init__(self, args):
        super().__init__(repo_yaml_path=args["repo_info"])
        self.domain_dir = utils.get_abs_path(args["ws_dir"])
        self.proc = args["proc"]
        self.os = args["os"]
        self.app = args["template"]
        self.compiler_flags = ""
        self.toolchain_file = None
        self.sdt = utils.get_abs_path(args["sdt"])
        self.family = self._get_family()
        self.lops_dir = os.path.join(utils.get_dir_path(lopper.__file__), "lops")
        self.include_folder = os.path.join(self.domain_dir, "include")
        self.lib_folder = os.path.join(self.domain_dir, "lib")
        self.libsrc_folder = os.path.join(self.domain_dir, "libsrc")
        self.domain_config_file = os.path.join(self.domain_dir, "bsp.yaml")
        self.repo_paths_list = self.repo_schema['paths']
        self.drv_info = {}
        self.os_info = {}
        utils.mkdir(self.domain_dir)
        self._validate_inputs()

    def _get_family(self):
        """
        This function intends to fetch the platform/family name from the input
        sdt. Usage of this is under discussion and this may be removed later.

        Returns:
            family (str): versal/zynqmp
        """
        with open(self.sdt, "r") as file:
            content = file.read()
            if "cpus_a53" in content:
                return "ZynqMP"
            elif "cpus_a72" in content:
                return "Versal"

    def _validate_inputs(self):
        """
        If User wants to validate the inputs before creating the domain,
        'OSF' needs to be set over console. Once set, this function
        will come into action and validate if the processor, the os, the
        template app passed over command line are valid or not for the
        sdt input.
        """
        if os.environ.get("OSF"):
            cpu_list_file = os.path.join(self.domain_dir, "cpulist.yaml")
            app_list_file = os.path.join(self.domain_dir, "app_list.yaml")
            lib_list_file = os.path.join(self.domain_dir, "lib_list.yaml")

            if not utils.is_file(cpu_list_file):
                utils.runcmd(
                    f"lopper --werror -f -O {self.domain_dir} -i {self.lops_dir}/lop-cpulist.dts {self.sdt} >/dev/null",
                    cwd = self.domain_dir
                )
            avail_cpu_data = utils.fetch_yaml_data(cpu_list_file, "cpulist")
            if self.proc not in avail_cpu_data.keys():
                utils.remove(self.domain_dir)
                print(
                    f"[ERROR]: Please pass a valid processor name. Valid Processor Names for the given SDT are: {list(avail_cpu_data.keys())}"
                )
                sys.exit(1)
            if not utils.is_file(app_list_file) or not utils.is_file(lib_list_file):
                utils.runcmd(
                    f"lopper --werror -f -O {self.domain_dir} {self.sdt} -- baremetal_getsupported_comp_xlnx {self.proc} {self.repo_yaml_path}",
                    cwd = self.domain_dir
                )
            proc_data = utils.fetch_yaml_data(app_list_file, "app_list")[self.proc]
            Validation.validate_template_name(
                self.domain_dir, proc_data, self.os, self.app
            )

    def build_dir_struct(self):
        """
        Creates the include, lib and libsrc folder inside bsp directory.
        """
        utils.mkdir(self.include_folder)
        utils.mkdir(self.lib_folder)
        utils.mkdir(self.libsrc_folder)


    def toolchain_intr_mapping(self):
        """
        We have reference toolchain files in embeddedsw which contains default
        compiler related cmake inputs. This function copies the toolchain file
        according to user os and processor input in the domain directory. Once
        copied, it manipulates few entries in the file needed for specific proc
        /os/app scenario.

        In addition, this function also processes the sdt directory to create a
        a single system dts file that has interrupts correctly mapped as per 
        the input processor.

        Returns:
            sdt (str): 
                Processed system device tree file that would be used across the 
                created domain for further processing.
            toolchain_file (str): 
                Toolchain file for cmake infra that would be used across the 
                created domain for builds.
        """

        # no gic mapping is needed for procs other than APU and RPU
        proc_lops_specs_map = {
            "a53": ("cortexa53", "lop-a53-imux", "arm"),
            "a72": ("cortexa72", "lop-a72-imux", "arm"),
            "r5": ("cortexr5", "lop-r5-imux", "arm"),
            "pmu": ("microblaze-pmu", "", "microblaze"),
            "pmc": ("microblaze-plm", "", "microblaze"),
            "psm": ("microblaze-psm", "", "microblaze"),
        }
        lops_file = ""
        out_dts_path = os.path.join(self.domain_dir, f"{self.proc}_baremetal.dts")
        toolchain_file_copy = None
        for val in proc_lops_specs_map.keys():
            if val in self.proc:
                toolchain_file_name = f"{proc_lops_specs_map[val][0]}_toolchain.cmake"
                #FIXME: Dont use / in the paths
                toolchain_file_path = utils.get_high_precedence_path(
                    self.repo_paths_list, f"cmake/toolchainfiles/{toolchain_file_name}", "toolchain File"
                )
                lops_file = os.path.join(self.lops_dir, f"{proc_lops_specs_map[val][1]}.dts")
                toolchain_file_copy = os.path.join(self.domain_dir, toolchain_file_name)
                utils.copy_file(toolchain_file_path, toolchain_file_copy)
                specs_file = utils.get_high_precedence_path(
                    self.repo_paths_list, f"scripts/specs/{proc_lops_specs_map[val][2]}/Xilinx.spec", "Xilinx.spec File"
                )
                specs_copy_file = os.path.join(self.domain_dir, 'Xilinx.spec')
                utils.copy_file(specs_file, specs_copy_file)
                break

        if "r5" in self.proc:
            utils.replace_line(
                toolchain_file_copy,
                'CMAKE_MACHINE "Versal',
                f'set( CMAKE_MACHINE "{self.family}")',
            )

        # freertos needs a separate CMAKE_SYSTEM_NAME
        if "freertos" in self.os:
            utils.add_newline(toolchain_file_copy, "set( CMAKE_SYSTEM_NAME FreeRTOS)")

        # Do the gic pruning in the sdt for APU/RPU.
        if self.sdt != out_dts_path:
            if utils.is_file(lops_file):
                utils.runcmd(
                    f"lopper -f --enhanced -O {self.domain_dir} -i {lops_file} {self.sdt} {out_dts_path}"
                )
            else:
                utils.runcmd(
                    f"lopper -f --enhanced -O {self.domain_dir} {self.sdt} {out_dts_path}"
                )
        else:
            out_dts_path = self.sdt

        self.compiler_flags = self.apps_cflags_update(
            toolchain_file_copy, self.app, self.proc
        )

        return out_dts_path, toolchain_file_copy, specs_copy_file

    def apps_cflags_update(self, toolchain_file, app_name, proc):
        """
        This function acts as a helper for toolchain_intr_mapping. This adds 
        template application specific compiler entries in the cmake toolchain
        file of the domain.

        Args:
            | toolchain_file (str): The toolchain file that needs to be updated
            | app_name (str): Specific app name that needs new entries
            | proc (str): Proc specific data pertaining to the app.

        Returns:
            compiler_flags (str): returns the new compiler flags that were set.
        """
        compiler_flags = ""
        if app_name == "zynqmp_fsbl":
            if "a53" in proc:
                compiler_flags = "-Os -flto -ffat-lto-objects -DARMA53_64"
            if "r5" in proc:
                compiler_flags = "-Os -flto -ffat-lto-objects -DARMR5"

            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_C_FLAGS "${{CMAKE_C_FLAGS}} {compiler_flags}")',
            )
            utils.add_newline(
                toolchain_file,
                f'set( CMAKE_ASM_FLAGS "${{CMAKE_ASM_FLAGS}} {compiler_flags}")',
            )
        return compiler_flags


def cmake_add_target(comp_name, output_dir, sdt, cmd, output):
    cmake_cmd = f"""
add_custom_target(
	{comp_name} ALL
	COMMAND lopper -O {output_dir} {sdt} -- {cmd}
	BYPRODUCTS {output}
)
"""
    return cmake_cmd

def cmake_drv_custom_target(proc, libsrc_folder, sdt, cmake_drv_name_list, cmake_drv_path_list):
    cmake_cmd = f'''
set(DRIVER_TARGETS {cmake_drv_name_list})
set(DRIVER_LOCATIONS {cmake_drv_path_list})

list(LENGTH DRIVER_TARGETS no_of_drivers)
set(index 0)

while(${{index}} LESS ${{no_of_drivers}})
    list(GET DRIVER_TARGETS ${{index}} drv)
    list(GET DRIVER_LOCATIONS ${{index}} drv_dir)
    set(src_dir "${{drv_dir}}/src")
    execute_process(COMMAND ${{CMAKE_COMMAND}} -E copy_directory ${{src_dir}} ${{CMAKE_LIBRARY_PATH}}/../libsrc/${{drv}}/src)
    add_custom_target(
        ${{drv}} ALL
        COMMAND lopper -O {libsrc_folder}/${{drv}}/src {sdt} -- baremetalconfig_xlnx {proc} ${{src_dir}}
        BYPRODUCTS x${{drv}}_g.c)
    MATH(EXPR index "${{index}}+1")
endwhile()
'''
    return cmake_cmd

def create_domain(args):
    """
    Function that uses the above Domain class to create the baremetal domain.
    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """

    # Initialize the Domain class
    obj = Domain(args)

    # Create the Domain specific sdt and the toolchain file.
    obj.sdt, obj.toolchain_file, obj.specs_file = obj.toolchain_intr_mapping()

    # Create the bsp directory structure.
    obj.build_dir_struct()

    # Common cmake variables to support cmake build infra.
    cmake_paths_append = f" -DCMAKE_LIBRARY_PATH={obj.lib_folder} \
            -DCMAKE_INCLUDE_PATH={obj.include_folder} \
            -DCMAKE_MODULE_PATH={obj.domain_dir} \
            -DCMAKE_TOOLCHAIN_FILE={obj.toolchain_file} \
            -DCMAKE_SPECS_FILE={obj.specs_file} \
            -DCMAKE_VERBOSE_MAKEFILE=ON"

    # Create top level CMakeLists.txt inside domain dir
    cmake_file = os.path.join(obj.domain_dir, "CMakeLists.txt")

    # Copy the standalone bsp src file.
    os_dir_path, os_dir_version = obj.get_comp_dir("standalone")
    os_srcdir = os.path.join(os_dir_path, "src")
    bspsrc = os.path.join(obj.libsrc_folder, "standalone", "src")
    utils.copy_directory(os_srcdir, bspsrc)
    obj.os_info['standalone'] = {'path': os_dir_path, 'version':os_dir_version}
    
    cmake_header = """
cmake_minimum_required(VERSION 3.15)
project(bsp)
find_package(common)
    """
    cmake_file_cmds = cmake_header

    cmd = f"baremetal_bspconfig_xlnx {obj.proc} {os_srcdir}"
    cmake_file_cmds += cmake_add_target("xilstandalone_config", bspsrc, obj.sdt, cmd, "MemConfig.cmake")
    bspcomsrc = os.path.join(obj.libsrc_folder, "standalone", "src", "common")
    cmd = f"bmcmake_metadata_xlnx {obj.proc} {os_srcdir} hwcmake_metadata {obj.repo_yaml_path}"
    cmake_file_cmds += cmake_add_target("xilstandalone_meta", bspcomsrc, obj.sdt, cmd, "StandaloneExample.cmake")

    # Copy cmake file that contains cmake utility APIs to a common location.
    # FIXME: Dont use / in the paths
    find_common_cmake_path = utils.get_high_precedence_path(
            obj.repo_paths_list, "cmake/Findcommon.cmake", "Findcommon.cmake"
        )
    utils.copy_file(
        find_common_cmake_path,
        os.path.join(obj.domain_dir, "Findcommon.cmake"),
        silent_discard=False,
    )

    """ 
    Generate metadata for driver compilation. Metadata includes driver 
    list available in sdt and a cmake file that enables the generation of 
    _g.c files for available drivers in parallel.
    """
    utils.runcmd(
        f"lopper -O {obj.libsrc_folder} -f {obj.sdt} -- baremetaldrvlist_xlnx {obj.proc} {obj.repo_yaml_path}"
    )

    # Read the driver list available in libsrc folder
    drv_list_file = os.path.join(obj.libsrc_folder, "distro.conf")
    cmake_drv_path_list = ""
    cmake_drv_name_list = ""

    with open(drv_list_file, "r") as fd:
        drv_names = (
            re.search('DISTRO_FEATURES = "(.*)"', fd.readline())
            .group(1)
            .replace("-", "_")
        )
        drv_list = drv_names.split()

    for drv in drv_list:
        drv_path, _ = obj.get_comp_dir(drv)
        if not drv_path:
            print(f"Couldnt find the src directory for {drv}.")
            sys.exit(1)
        obj.drv_info[drv] = {'path' : drv_path}
        cmake_drv_path_list += f"{drv_path};"

    cmake_drv_name_list += ';'.join(list(obj.drv_info.keys()))
    ip_drv_map_file = os.path.join(obj.libsrc_folder, "ip_drv_map.yaml")


    cmake_file_cmds += cmake_drv_custom_target(obj.proc, obj.libsrc_folder, obj.sdt, cmake_drv_name_list, cmake_drv_path_list)
    cmd = f"baremetal_xparameters_xlnx {obj.proc} {obj.repo_yaml_path}"
    cmake_file_cmds += cmake_add_target("xparam", obj.include_folder, obj.sdt, cmd, "xparameters.h")

    """ 
    Create a dictionary that will contain the current status of the domain.
    This data will later be used during bsp configuartion, bsp build and 
    app creation stages.
    """
    data = {
        "sdt": utils.get_rel_path(obj.sdt, obj.domain_dir),
        "os": obj.os,
        "os_info": obj.os_info,
        "os_config": {},
        "toolchain_file": utils.get_rel_path(obj.toolchain_file, obj.domain_dir),
        "specs_file": utils.get_rel_path(obj.specs_file, obj.domain_dir),
        "proc": obj.proc,
        "proc_config": {},
        "template": obj.app,
        "compiler_flags": obj.compiler_flags,
        "include_folder": utils.get_rel_path(obj.include_folder, obj.domain_dir),
        "lib_folder": utils.get_rel_path(obj.lib_folder, obj.domain_dir),
        "libsrc_folder": utils.get_rel_path(obj.libsrc_folder, obj.domain_dir),
        "ip_drv_map": utils.load_yaml(ip_drv_map_file),
        "drv_info": obj.drv_info,
        "lib_info": {},
        "lib_config": {}
    }

    # Write the domain specific data as a configuration file in yaml format.
    utils.write_yaml(obj.domain_config_file, data)

    # If domain has to be created for a certain template, few libraries need
    # to be added in the bsp.
    lib_list = []
    cmake_cmd_append = ""
    lib_obj = Library(
        obj.domain_dir, obj.proc, obj.os, obj.sdt, cmake_paths_append, obj.libsrc_folder, obj.repo_yaml_path
    )

    if obj.os == "freertos":
        if obj.app:
            # If template app is passed, read the app's yaml file and add
            # lib accordingly.
            lib_list, cmake_cmd_append = lib_obj.add_lib(obj.app, is_app=True)
        else:
            # If no app is passed and bsp is created for freertos os, add
            # xiltimer by default.
            lib_list, cmake_cmd_append = lib_obj.add_lib("xiltimer", is_app=False)
        # Copy the freertos source code to libsrc folder
        os_dir_path, os_dir_version = obj.get_comp_dir("freertos10_xilinx")
        os_srcdir = os.path.join(os_dir_path, "src")
        bspsrc = os.path.join(obj.libsrc_folder, "freertos10_xilinx/src")
        utils.copy_directory(os_srcdir, bspsrc)
        obj.os_info['freertos10_xilinx'] = {'path': os_dir_path, 'version':os_dir_version}

    elif obj.app:
        # If template app is passed, read the app's yaml file and add
        # lib accordingly.
        lib_list, cmake_cmd_append = lib_obj.add_lib(obj.app, is_app=True)
    else:
        # If no app is passed and bsp is created add xiltimer by default.
        lib_list, cmake_cmd_append = lib_obj.add_lib("xiltimer", is_app=False)


    cmake_lib_list = ""
    if lib_list:
        for lib in lib_list:
            cmake_lib_list += f"{lib};"

    cmake_file_cmds += f"\nset(lib_list {cmake_lib_list})\n"
    if obj.os == "freertos":
        lib_list.append("freertos10_xilinx")

    if lib_list:
        for lib in lib_list:
            lib_dir_path, lib_dir_version = obj.get_comp_dir(lib)
            srcdir = os.path.join(lib_dir_path, "src")
            dstdir = os.path.join(obj.libsrc_folder, f"{lib}/src")
            cmd = f"bmcmake_metadata_xlnx {obj.proc} {srcdir} hwcmake_metadata {obj.repo_yaml_path}"
            outfile = f"{lib}Example.cmake"
            cmake_file_cmds += cmake_add_target(lib, dstdir, obj.sdt, cmd, outfile)

    cmake_file_cmds += f"\nadd_library(bsp INTERFACE)"
    cmake_file_cmds = cmake_file_cmds.replace('\\', '/')
    cmake_file_cmds += f"\nadd_dependencies(bsp xilstandalone_config xilstandalone_meta xparam {cmake_lib_list} {cmake_drv_name_list})"
    utils.write_into_file(cmake_file, cmake_file_cmds)

    # Create a build directory for cmake to generate all _g.c files.
    build_metadata = os.path.join(obj.libsrc_folder, "build_configs/metadata")
    utils.mkdir(build_metadata)

    # Run cmake configure and build to generate _g.c files
    obj.domain_dir = obj.domain_dir.replace('\\', '/')
    obj.toolchain_file = obj.toolchain_file.replace('\\', '/')
    cmake_paths_append = cmake_paths_append.replace('\\', '/')
    build_metadata = build_metadata.replace('\\', '/')
    utils.runcmd(
        f"cmake {obj.domain_dir} {cmake_paths_append}",
        cwd = build_metadata
    )

    utils.runcmd("make -f CMakeFiles/Makefile2 -j22 > nul", cwd = build_metadata)

    # Create new CMakeLists.txt
    cmake_file_cmds = cmake_header
    cmake_file_cmds += """ 
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()
    """
    cmake_file_cmds += f"\nadd_subdirectory({obj.libsrc_folder}/standalone/src)\n"
    if lib_list:
        for lib in lib_list:
            dstdir = os.path.join(obj.libsrc_folder, f"{lib}/src")
            cmake_file_cmds += f"\nadd_subdirectory({dstdir})\n"
    cmake_file_cmds = cmake_file_cmds.replace('\\', '/')
    utils.write_into_file(cmake_file, cmake_file_cmds)

    build_metadata = os.path.join(obj.libsrc_folder, "build_configs", "gen_bsp")
    utils.mkdir(build_metadata)
    if obj.app:
        lib_obj.config_lib(obj.app, lib_list, cmake_cmd_append, is_app=True)
    else:
        # If no app is passed and bsp is created xiltimer got added default
        # Update config entries for the same.
        lib_obj.config_lib("xiltimer", lib_list, cmake_cmd_append, is_app=False)

    # Run cmake configuration with all the default cache entries
    cmake_paths_append = cmake_paths_append.replace('\\', '/')
    build_metadata = build_metadata.replace('\\', '/')

    utils.runcmd(
            f'cmake {obj.domain_dir} {cmake_paths_append} -DNON_YOCTO=ON -LH > cmake_lib_configs.txt',
            cwd = build_metadata
    )
    if obj.os == "freertos":
        os_config = lib_obj.get_default_lib_params(build_metadata, ["freertos"])
    else:
        os_config = lib_obj.get_default_lib_params(build_metadata, ["standalone"])
    proc_config = lib_obj.get_default_lib_params(build_metadata,[obj.proc])

    utils.update_yaml(obj.domain_config_file, "domain", "os_config", os_config)
    utils.update_yaml(obj.domain_config_file, "domain", "proc_config", proc_config)
    utils.update_yaml(obj.domain_config_file, "domain", "os_info", obj.os_info)
    utils.update_yaml(obj.domain_config_file, "domain", "lib_info", lib_obj.lib_info)

    # Copy the actual drivers cmake file in the libsrc folder.
    # This is to compile all the available driver sources.
    libxil_cmake = utils.get_high_precedence_path(
        obj.repo_paths_list, "XilinxProcessorIPLib/drivers/CMakeLists.txt", "Driver compilation CMake File"
    )
    utils.copy_file(libxil_cmake, f"{obj.libsrc_folder}/")

    # Remove the metadata files that are no longer needed.
    utils.remove(drv_list_file)
    utils.remove(ip_drv_map_file)
    utils.remove(os.path.join(obj.libsrc_folder, "libxil.conf"))

    utils.remove(os.path.join(obj.domain_dir, "*.dtb"), pattern=True)
    utils.remove(os.path.join(obj.domain_dir, "*.pp"), pattern=True)

    # Success prints if everything went well till this point
    if utils.is_file(obj.domain_config_file):
        print(f"Successfully created Domain at {obj.domain_dir}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Create bsp for the given sdt, os, processor and template app",
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    required_argument = parser.add_argument_group("Required arguments")
    required_argument.add_argument(
        "-p",
        "--proc",
        action="store",
        help="Specify the processor name",
        required=True,
    )
    required_argument.add_argument(
        "-s",
        "--sdt",
        action="store",
        help="Specify the System device-tree path (till system-top.dts file)",
        required=True,
    )
    parser.add_argument(
        "-w",
        "--ws_dir",
        action="store",
        help="Workspace directory where domain will be created (Default: Current Work Directory)",
        default='.',
    )
    parser.add_argument(
        "-o",
        "--os",
        action="store",
        default="standalone",
        help="Specify OS (Default: standalone)",
        choices=["standalone", "freertos"],
    )
    parser.add_argument(
        "-t",
        "--template",
        action="store",
        default="",
        help=textwrap.dedent(
            """\
        Specify template app name. Available names are:
            - empty_application
            - hello_world
            - memory_tests
            - peripheral_tests
            - zynqmp_fsbl
            - zynqmp_pmufw
            - lwip_echo_server
            - freertos_hello_world
            - versal_plm
            - versal_psmfw
            - freertos_lwip_echo_server
            - freertos_lwip_tcp_perf_client
            - freertos_lwip_tcp_perf_server
            - freertos_lwip_udp_perf_client
            - freertos_lwip_udp_perf_server
            - lwip_tcp_perf_client
            - lwip_tcp_perf_server
            - lwip_udp_perf_client
            - lwip_udp_perf_server
            - dhrystone
            - zynqmp_dram_test
        """
        ),
    )
    parser.add_argument(
        "-r",
        "--repo_info",
        action="store",
        help="Specify the .repo.yaml absolute path to use the set repo info",
        default='.repo.yaml',
    )


    args = vars(parser.parse_args())
    create_domain(args)
