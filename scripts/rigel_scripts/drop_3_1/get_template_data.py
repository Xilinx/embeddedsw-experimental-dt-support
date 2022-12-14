"""
This module fetches all the template app related data from esw yamls and puts
it into a yaml file.
"""

import utils
from repo import Repo
import os, argparse

def fetch_template_data(esw_app_dir_path, dest_dir):
    """
    Collects all the template related data into a yaml file.
    
    Args:
        esw_app_dir_path : sw_apps directory path of embeddedsw
    """
    overall_data = {}
    for app in os.listdir(esw_app_dir_path):
        app_data_file = os.path.join(esw_app_dir_path, app, 'data', f'{app}.yaml')
        if utils.is_file(app_data_file):
            overall_data[app] = utils.load_yaml(app_data_file)

    if not utils.is_dir(dest_dir):
        utils.mkdir(dest_dir)
    utils.write_yaml(os.path.join(dest_dir,'template_data.yaml'), overall_data)

if __name__ == "__main__":
    repo_obj = Repo()
    parser = argparse.ArgumentParser(
        description="Fetches all the template app related data from esw yamls\
        and puts it into a yaml file",
        usage='use "python %(prog)s --help" for more information',
    )
    parser.add_argument(
        "-d",
        "--dir",
        action="store",
        help="Specify the directory path where yaml will be created (Default: Current working directory)",
        default="./"
    )

    args = vars(parser.parse_args())
    fetch_template_data(repo_obj.esw_apps_dir, args['dir'])
