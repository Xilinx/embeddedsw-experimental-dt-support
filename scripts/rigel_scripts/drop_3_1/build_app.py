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

    # Get the app_path created by the user
    parser.add_argument(
        "-w",
        "--ws_dir",
        action="store",
        help="Specify the App Workspace Directory",
        default='.',
    )

    parser.add_argument(
        "-b",
        "--build_dir",
        action="store",
        help="Specify the App Build Directory",
    )

    args = vars(parser.parse_args())
    app_dir = utils.get_abs_path(args.get("ws_dir"))
    if args.get('build_dir'):
        app_build_dir = utils.get_abs_path(args["build_dir"])
    else:
        app_build_dir = os.path.join(app_dir, "build")

    # Read the config file that maintains app specific data
    app_config_file = os.path.join(app_build_dir, ".app.yaml")
    utils.validate_if_not_exist(app_config_file, "app", app_build_dir)
    args["domain_path"] = utils.fetch_yaml_data(app_config_file, "app")["domain_path"]

    # Build the bsp first before building application
    generate_bsp(args)

    # Run make inside cmake configured build area.
    utils.runcmd("make -j22", cwd=app_build_dir)
