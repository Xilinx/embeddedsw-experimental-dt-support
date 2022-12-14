"""
This module cretaes a domain and a bsp for the passed processor, os and system
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
        super().__init__()
        self.ws_dir = utils.get_abs_path(args["ws_dir"])
        self.proc = args["proc"]
        self.domain_name = args["name"]
        self.os = args["os"]
        self.app = args["template"]
        self.compiler_flags = ""
        self.toolchain_file = None
        self.sdt = utils.get_abs_path(args["sdt"])
        self.family = self._get_family()
        self.proc_dir = os.path.join(self.ws_dir, self.proc)
        self.domain_dir = os.path.join(self.proc_dir, self.domain_name)
        self.bsp_dir = os.path.join(self.domain_dir, "bsp")
        self.lops_dir = os.path.join(utils.get_dir_path(lopper.__file__), "lops")
        self.include_folder = os.path.join(self.bsp_dir, "include")
        self.lib_folder = os.path.join(self.bsp_dir, "lib")
        self.libsrc_folder = os.path.join(self.bsp_dir, "libsrc")
        self.domain_config_file = os.path.join(self.domain_dir, ".domain.yaml")
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
        'VAL_INPUTS' needs to be set over console. Once set, this function
        will come into action and validate if the processor, the os, the
        template app passed over command line are valid or not for the
        sdt input.
        """
        if os.environ.get("VAL_INPUTS"):
            utils.validate_if_exist(self.domain_config_file,'domain',self.domain_name)
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
                utils.remove(self.proc_dir)
                print(
                    f"[ERROR]: Please pass a valid processor name. Valid Processor Names for the given SDT are: {list(avail_cpu_data.keys())}"
                )
                sys.exit(1)
            if not utils.is_file(app_list_file) or not utils.is_file(lib_list_file):
                utils.runcmd(
                    f"lopper --werror -f -O {self.domain_dir} {self.sdt} -- baremetal_getsupported_comp_xlnx {self.proc} {self.repo}",
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
        proc_lops_map = {
            "a53": ("cortexa53", "lop-a53-imux"),
            "a72": ("cortexa72", "lop-a72-imux"),
            "r5": ("cortexr5", "lop-r5-imux"),
            "pmu": ("microblaze-pmu", ""),
            "pmc": ("microblaze-plm", ""),
            "psm": ("microblaze-psm", ""),
        }
        lops_file = ""
        out_dts_path = os.path.join(self.domain_dir, f"{self.proc}_baremetal.dts")
        toolchain_file_copy = None
        for val in proc_lops_map.keys():
            if val in self.proc:
                toolchain_file_name = f"{proc_lops_map[val][0]}_toolchain.cmake"
                toolchain_file_path = os.path.join(
                    self.repo, f"cmake/toolchainfiles/{toolchain_file_name}"
                )
                lops_file = os.path.join(self.lops_dir, f"{proc_lops_map[val][1]}.dts")
                toolchain_file_copy = os.path.join(self.domain_dir, toolchain_file_name)
                utils.copy_file(toolchain_file_path, toolchain_file_copy)

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
        if utils.is_file(lops_file):
            utils.runcmd(
                f"lopper -f --enhanced -O {self.domain_dir} -i {lops_file} {self.sdt} {out_dts_path}"
            )
        else:
            utils.runcmd(
                f"lopper -f --enhanced -O {self.domain_dir} {self.sdt} {out_dts_path}"
            )

        self.compiler_flags = self.apps_cflags_update(
            toolchain_file_copy, self.app, self.proc
        )

        return out_dts_path, toolchain_file_copy

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


def create_domain(args):
    """
    Function that uses the above Domain class to create the baremetal domain.
    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """

    # Initialize the Domain class
    obj = Domain(args)

    # Create the Domain specific sdt and the toolchain file.
    obj.sdt, obj.toolchain_file = obj.toolchain_intr_mapping()

    # Create the bsp directory structure.
    obj.build_dir_struct()

    # Common cmake variables to support cmake build infra.
    cmake_paths_append = f" -DCMAKE_LIBRARY_PATH={obj.lib_folder} \
            -DCMAKE_INCLUDE_PATH={obj.include_folder} \
            -DCMAKE_MODULE_PATH={obj.domain_dir} \
            -DCMAKE_TOOLCHAIN_FILE={obj.toolchain_file}"

    # Copy the standalone bsp src file.
    os_srcdir = os.path.join(obj.get_comp_dir("standalone"), "src")
    bspsrc = os.path.join(obj.libsrc_folder, "standalone/src")
    utils.copy_directory(os_srcdir, bspsrc)

    # Generate standalone bsp related metadata as per sdt.
    utils.runcmd(
        f"lopper -O {bspsrc} {obj.sdt} -- baremetal_bspconfig_xlnx {obj.proc} {os_srcdir}"
    )

    # Copy cmake file that contains cmake utility APIs to a common location.
    utils.copy_file(
        f"{obj.repo}/cmake/Findcommon.cmake",
        os.path.join(obj.domain_dir, "Findcommon.cmake"),
        silent_discard=False,
    )

    """ 
    Generate metadata for driver compilation. Metadata includes driver 
    list available in sdt and a cmake file that enables the generation of 
    _g.c files for available drivers in parallel.
    """
    utils.runcmd(
        f"lopper -O {obj.libsrc_folder} -f {obj.sdt} -- baremetaldrvlist_xlnx {obj.proc} {obj.repo}"
    )

    # Read the driver list available in libsrc folder
    drv_list_file = os.path.join(obj.libsrc_folder, "distro.conf")
    with open(drv_list_file, "r") as fd:
        drv_names = (
            re.search('DISTRO_FEATURES = "(.*)"', fd.readline())
            .group(1)
            .replace("-", "_")
        )
        drvlist = drv_names.split()
        obj.drvlist = drvlist

    # Create a build directory for cmake to generate all _g.c files.
    build_metadata = os.path.join(obj.libsrc_folder, "build_configs/metadata")
    utils.mkdir(build_metadata)

    # Run cmake configure and build to generate _g.c files.
    utils.runcmd(
        f"cmake {obj.libsrc_folder} -DCMAKE_TOOLCHAIN_FILE={obj.toolchain_file} -DSDT={obj.sdt} {cmake_paths_append}",
        cwd = build_metadata
    )
    utils.runcmd("make -f CMakeFiles/Makefile2 -j22 >/dev/null", cwd = build_metadata)

    # Copy the actual drivers cmake file in the libsrc folder.
    # This is to compile all the available driver sources.
    libxil_cmake = os.path.join(obj.esw_drivers_dir, "CMakeLists.txt")
    utils.copy_file(libxil_cmake, f"{obj.libsrc_folder}/")

    # Remove the metadata files that are no longer needed.
    utils.remove(drv_list_file)
    utils.remove(os.path.join(obj.libsrc_folder, "libxil.conf"))

    """ 
    Create a dictionary that will contain the current status of the domain.
    This data will later be used during bsp configuartion, bsp build and 
    app creation stages.
    """
    data = {
        "sdt": utils.get_rel_path(obj.sdt, obj.domain_dir),
        "os": obj.os,
        "toolchain_file": utils.get_rel_path(obj.toolchain_file, obj.domain_dir),
        "proc": obj.proc,
        "template": obj.app,
        "compiler_flags": obj.compiler_flags,
        "bsp_path": utils.get_rel_path(obj.bsp_dir, obj.domain_dir),
        "include_folder": utils.get_rel_path(obj.include_folder, obj.domain_dir),
        "lib_folder": utils.get_rel_path(obj.lib_folder, obj.domain_dir),
        "libsrc_folder": utils.get_rel_path(obj.libsrc_folder, obj.domain_dir),
        "drvlist": drvlist,
        "lib_config": {},
    }

    # Write the domain specific data as a configuration file in yaml format.
    utils.write_yaml(obj.domain_config_file, data)

    # If domain has to be created for a certain template, few libraries need
    # to be added in the bsp.
    lib_obj = Library(
        obj.domain_dir, obj.proc, obj.os, obj.sdt, cmake_paths_append, obj.libsrc_folder
    )
    if obj.os == "freertos":
        if obj.app:
            # If template app is passed, read the app's yaml file and add
            # lib accordingly.
            lib_obj.add_lib(obj.app, is_app=True)
        else:
            # If no app is passed and bsp is created for freertos os, add
            # xiltimer by default.
            lib_obj.add_lib("xiltimer", is_app=False)
        # Copy the freertos source code to libsrc folder
        os_srcdir = os.path.join(obj.get_comp_dir("freertos"), "src")
        bspsrc = os.path.join(obj.libsrc_folder, "freertos10_xilinx/src")
        utils.copy_directory(os_srcdir, bspsrc)
        # Generate metadata for freertos os.
        utils.runcmd(
            f"lopper -O {bspsrc} {obj.sdt} -- bmcmake_metadata_xlnx {obj.proc} {os_srcdir} hwcmake_metadata {obj.repo}"
        )

    elif obj.app:
        # If template app is passed, read the app's yaml file and add
        # lib accordingly.
        lib_obj.add_lib(obj.app, is_app=True)

    # Success prints if everything went well till this point
    if utils.is_file(obj.domain_config_file):
        print(f"Successfully created {obj.domain_name} Domain")


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
    required_argument.add_argument(
        "-n",
        "--name",
        action="store",
        help="Name of the Domain directory",
        default="standalone_domain",
    )
    required_argument.add_argument(
        "-w",
        "--ws_dir",
        action="store",
        help="Workspace directory where domain will be created",
        required=True,
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
            - hello_world
            - memory_tests
            - peripheral_tests
            - zynqmp_fsbl
            - zynqmp_pmufw
            - lwip_echo_server
            - freertos_hello_world
            - versal_plm
            - versal_psmfw
        """
        ),
    )

    args = vars(parser.parse_args())
    create_domain(args)