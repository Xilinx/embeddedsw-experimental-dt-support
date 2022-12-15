"""
This module acts as a supporting module for all the other modules. It helps
in validating the embeddedsw repo set in the environment and sets up the
correct path for different components within embeddedsw. It doesnt have 
any main() function and running this module independently is not intended.
"""

import os, sys
from utils import is_dir, get_abs_path


class Repo:
    """
    This is the base class to get the embeddedsw repo path. This checks if the
    embeddedsw path is set correctly. If set, this helps in retrieving the 
    corresponding directory path of the component inside embeddedsw.
    """

    def __init__(self):
        self.repo = self.validate_repo(os.environ.get("ESW_REPO", ""))
        self.esw_apps_dir = os.path.join(self.repo, "lib/sw_apps")
        self.esw_lib_dir = os.path.join(self.repo, "lib/sw_services")
        self.esw_drivers_dir = os.path.join(self.repo, "XilinxProcessorIPLib/drivers")
        self.esw_thirdparty_dir = os.path.join(self.repo, "ThirdParty")

    def validate_repo(self, repo):
        """
        Returns the set absolute path of embeddedsw repo.

        Args:
            repo (str): The user input for the embeddedsw repo path
        Returns:
            repo (str): 
                If user entry is correct, returns the absolute path 
                of that entry
        """
        if not repo:
            print(
                "Please set a ESW repo path in shell. Usage in BASH: export ESW_REPO='<esw path>'"
            )
            sys.exit(1)
        else:
            repo = get_abs_path(repo)
            if not is_dir(repo):
                print(f"ESW repo path {repo} doesn't exist")
                sys.exit(1)

        return repo

    def get_comp_dir(self, comp_name, is_app=False):
        """
        Returns the absolute path of components in embeddedsw repo.

        Args:
            | comp_name (str): component name whose path is to be retrieved
            | is_app (bool): if the component passed is a template application.
        Returns:
            comp_dir (str): Absolute path of the component in embeddedsw
        """
        if is_app:
            comp_dir = os.path.join(self.esw_apps_dir, comp_name)
        elif comp_name.startswith("x"):
            comp_dir = os.path.join(self.esw_lib_dir, comp_name)
        elif "freertos" in comp_name:
            comp_dir = os.path.join(self.esw_thirdparty_dir, f"bsp/freertos10_xilinx")
        elif "lwip" in comp_name:
            comp_dir = os.path.join(self.esw_thirdparty_dir, f"sw_services/lwip211")
        elif "standalone" in comp_name:
            comp_dir = os.path.join(self.repo, f"lib/bsp/standalone")
        else:
            comp_dir = os.path.join(self.esw_drivers_dir, comp_name)

        assert is_dir(
            comp_dir
        ), f"{comp_dir} doesnt exist. Not able to fetch the dir for {comp_name}"
        return comp_dir
