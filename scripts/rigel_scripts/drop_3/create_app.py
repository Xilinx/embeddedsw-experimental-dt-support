"""
This module creates the template application using the domain information
provided to it. It generates the directory structure and the metadata
required to build a particular template application.
"""

import utils
import argparse, textwrap
import os
from build_bsp import BSP
from repo import Repo
from validate_bsp import Validation


class App(BSP, Repo):
    """
    This class helps in creating a template application. It contains attributes
    and functions that take domain path, and template name as an input, create 
    the directory structure and the metadata(if needed) for the app.
    """

    def __init__(self, args):
        self.template = args.get("template")
        BSP.__init__(self, args)
        Repo.__init__(self)
        self._build_dir_struct()

    def _build_dir_struct(self):
        """
        Creates the directory structure for Apps.
        """
        self.wsdir = utils.get_abs_path(args["wsdir"])
        self.name = args.get("name")
        self.appdir = os.path.join(self.wsdir, self.name)
        # Validate the passed template w.r.t the passed domain path
        if os.environ.get("VAL_INPUTS"):
            obj = Validation(args)
            obj.validate_template_for_bsp()
        utils.mkdir(self.appdir)
        # App directory needs to have its own yaml configuration
        # (for compiler flags, linker flags etc.)
        self.app_config_file = os.path.join(self.appdir, ".app.yaml")


def create_app(args):
    """
    Function that uses the above App class to create the template application.
    
    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """
    obj = App(args)

    # Copy the application src directory from embeddedsw to app src folder.
    esw_app_dir = obj.get_comp_dir(obj.template, is_app=True)
    srcdir = os.path.join(esw_app_dir, "src")
    app_src_dir = os.path.join(obj.appdir, "src")
    utils.copy_directory(srcdir, app_src_dir)

    # Checks if the app depends on any driver, if yest, generates the corresponding metadata
    app_yaml_file = os.path.join(esw_app_dir, "data", f"{obj.template}.yaml")
    if utils.is_file(app_yaml_file):
        app_schema = utils.load_yaml(app_yaml_file)
        if app_schema.get("depends"):
            utils.runcmd(
                f"lopper -O {app_src_dir} {obj.sdt} -- bmcmake_metadata_xlnx {obj.proc} {srcdir} hwcmake_metadata {obj.repo}"
            )

    # Generates the metadata for linker script
    linker_cmd = (
        f"lopper -O {app_src_dir} {obj.sdt} -- baremetallinker_xlnx {obj.proc} {srcdir}"
    )
    if obj.name == "memory_tests":
        utils.runcmd(f"{linker_cmd} memtest")
    else:
        utils.runcmd(linker_cmd)

    # Copy the static linker files from embeddedsw to the app src dir
    linker_dir = os.path.join(app_src_dir, "linker_files")
    linker_src = os.path.join(obj.repo, "scripts/linker_files")
    utils.copy_directory(linker_src, linker_dir)

    # Generate the CMake file specifically for peripheral app
    # FIXME: Fix the lopper assist to use sdt.outdir
    if obj.template == "peripheral_tests":
        utils.runcmd(
            f"lopper -O {app_src_dir} {obj.sdt} -- baremetal_gentestapp_xlnx {obj.proc} {obj.repo}"
        )

    # Create the cmake build dir for app and run cmake inside it.
    app_work_dir = os.path.join(obj.appdir, f"build")
    utils.mkdir(app_work_dir)
    utils.runcmd(f"cmake {app_src_dir} {obj.cmake_paths_append} -DOS_ESW=ON", cwd=app_work_dir)

    # Add domain path entry in the app configuration file.
    data = {"domain_path": obj.domain_path}
    utils.write_yaml(obj.app_config_file, data)

    # Success prints if everything went well till this point.
    if utils.is_file(obj.app_config_file):
        print(f"Successfully Created {obj.name} Application")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Use this script to create a template App using the built BSP",
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    required_argument = parser.add_argument_group("Required arguments")
    required_argument.add_argument(
        "-d",
        "--domain_path",
        action="store",
        help="Specify the built BSP Path",
        required=True,
    )
    required_argument.add_argument(
        "-w", "--wsdir", action="store", help="Workspace directory", required=True
    )
    required_argument.add_argument(
        "-n",
        "--name",
        action="store",
        help="Provide a name to your app directory",
        required=True,
    )
    required_argument.add_argument(
        "-t",
        "--template",
        action="store",
        required=True,
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
    create_app(args)
