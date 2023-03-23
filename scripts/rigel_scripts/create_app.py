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
        BSP.__init__(self, args)
        Repo.__init__(self, repo_yaml_path=args['repo_info'])
        self._build_dir_struct(args)
        self.app_name = args.get("name")
        self.template = args.get("template")
        self.repo_paths_list = self.repo_schema['paths']

    def _build_dir_struct(self, args):
        """
        Creates the directory structure for Apps.
        """
        self.app_dir = utils.get_abs_path(args["ws_dir"])
        if args.get('src_dir'):
            self.app_src_dir = utils.get_abs_path(args["src_dir"])
        else:
            self.app_src_dir = os.path.join(self.app_dir, "src")

        # Validate the passed template w.r.t the passed domain path
        if os.environ.get("OSF"):
            obj = Validation(args)
            obj.validate_template_for_bsp()
        utils.mkdir(self.app_src_dir)
        # App directory needs to have its own yaml configuration
        # (for compiler flags, linker flags etc.)
        self.app_config_file = os.path.join(self.app_src_dir, "app.yaml")


def create_app(args):
    """
    Function that uses the above App class to create the template application.
    
    Args:
        args (dict): Takes all the user inputs in a dictionary.
    """
    obj = App(args)

    # Copy the application src directory from embeddedsw to app src folder.
    esw_app_dir, _ = obj.get_comp_dir(obj.template)
    srcdir = os.path.join(esw_app_dir, "src")
    utils.copy_directory(srcdir, obj.app_src_dir)

    if obj.app_name:
        src_cmake = os.path.join(obj.app_src_dir, "CMakeLists.txt")
        utils.replace_line(
            src_cmake,
            f'APP_NAME {obj.template}',
            f'set(APP_NAME {obj.app_name})',
        )

    # Checks if the app depends on any driver, if yest, generates the corresponding metadata
    app_yaml_file = os.path.join(esw_app_dir, "data", f"{obj.template}.yaml")
    if utils.is_file(app_yaml_file):
        app_schema = utils.load_yaml(app_yaml_file)
        if app_schema.get("depends"):
            utils.runcmd(
                f"lopper -O {obj.app_src_dir} {obj.sdt} -- bmcmake_metadata_xlnx {obj.proc} {srcdir} hwcmake_metadata {obj.repo_yaml_path}"
            )

    # Generates the metadata for linker script
    linker_cmd = (
        f"lopper -O {obj.app_src_dir} {obj.sdt} -- baremetallinker_xlnx {obj.proc} {srcdir}"
    )
    if obj.template == "memory_tests":
        utils.runcmd(f"{linker_cmd} memtest")
    else:
        utils.runcmd(linker_cmd)

    # Copy the static linker files from embeddedsw to the app src dir
    linker_dir = os.path.join(obj.app_src_dir, "linker_files")
    linker_src = utils.get_high_precedence_path(
            obj.repo_paths_list, "scripts/linker_files", "Linker file directory"
        )
    utils.copy_directory(linker_src, linker_dir)
    # Copy the User Configuration cmake file to the app src dir
    user_config_cmake = utils.get_high_precedence_path(
            obj.repo_paths_list, "cmake/UserConfig.cmake", "UserConfig.cmake file"
        )
    utils.copy_file(user_config_cmake, obj.app_src_dir)

    # Generate the CMake file specifically for peripheral app
    if obj.template == "peripheral_tests":
        utils.runcmd(
            f"lopper -O {obj.app_src_dir} {obj.sdt} -- baremetal_gentestapp_xlnx {obj.proc} {obj.repo_yaml_path}"
        )

    # Add domain path entry in the app configuration file.
    data = {"domain_path": obj.domain_path,
            "app_src_dir": esw_app_dir,
            "template": obj.template
        }
    utils.write_yaml(obj.app_config_file, data)

    # Create a dummy folder to get compile_commands.json
    compile_commands_dir = os.path.join(obj.app_src_dir, ".compile_commands")
    utils.mkdir(compile_commands_dir)
    utils.runcmd(f"cmake {obj.app_src_dir} {obj.cmake_paths_append} -DNON_YOCTO=ON > nul", cwd=compile_commands_dir)

    '''
    compile_commands.json file needs to be kept inside src directory.
    Silent_discard needs to be true as for Empty Application, this file
    is not created.
    '''
    utils.copy_file(f"{compile_commands_dir}/compile_commands.json", obj.app_src_dir, silent_discard=True)

    '''
    There are few GCC flags (e.g. -fno-tree-loop-distribute-patterns) that
    clang server doesnt recognise for Code Intellisense. To get over this
    "Unknown Argument" Error of clang, a .clangd file with below content is
    to be kept in parallel to compile_commands.json file.
    '''
    clangd_ignore_content = f'''
CompileFlags:
    Add: -Wno-unknown-warning-option
    Remove: [-m*, -f*]
'''
    clangd_ignore_file = os.path.join(obj.app_src_dir, ".clangd")
    utils.write_into_file(clangd_ignore_file, clangd_ignore_content)

    '''
    The generated compile_commands.json file has the directory path (where it
    was created originally) in it. That directory needs to be maintained to
    avoid clang error.
    '''
    utils.remove(os.path.join(compile_commands_dir, "*"), pattern=True)

    # Success prints if everything went well till this point.
    if utils.is_file(obj.app_config_file):
        print(f"Successfully Created Application sources at {obj.app_src_dir}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Use this script to create a template App using the BSP path",
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
    parser.add_argument(
        "-w", "--ws_dir", action="store", help="Workspace directory (Default: Current Work Directory)", default='.'
    )
    parser.add_argument(
        "-s", "--src_dir", action="store", help="App source directory (Default: <Current Work Directory>/src)"
    )
    parser.add_argument(
        "-n", "--name", action="store", help="App name"
    )
    required_argument.add_argument(
        "-t",
        "--template",
        action="store",
        required=True,
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
    create_app(args)
