"""
This module builds an already created app. It doesn't contain any members
other than main().
"""

import utils
import os
import argparse
from build_bsp import generate_bsp

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Use this script to build the cretaed app ",
        usage='use "python %(prog)s --help" for more information',
        formatter_class=argparse.RawTextHelpFormatter,
    )
    required_argument = parser.add_argument_group("Required arguments")

    # Get the app_path created by the user
    required_argument.add_argument(
        "-a",
        "--app_path",
        action="store",
        help="Specify the App Workspace Directory",
        required=True,
    )
    args = vars(parser.parse_args())
    app_path = utils.get_abs_path(args.get("app_path"))
    app_name = utils.get_base_name(app_path)

    # Read the config file that maintains app specific data
    app_config_file = os.path.join(app_path, ".app.yaml")
    utils.validate_if_not_exist(app_config_file, "app", app_name)
    args["domain_path"] = utils.fetch_yaml_data(app_config_file, "app")["domain_path"]

    # Build the bsp first before building application
    generate_bsp(args)

    # Run make inside cmake configured build area.
    app_build_dir = os.path.join(app_path, "build")
    os.chdir(app_build_dir)
    utils.runcmd("make -j22")
