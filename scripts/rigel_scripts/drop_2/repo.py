import os
from utils import *

class Repo:
    def __init__(self, repo=None):
        if repo:
            self.repo = get_abs_path(repo)
        else:
            self.repo = os.environ['ESW_REPO']
        self.esw_apps_dir = os.path.join(self.repo, "lib/sw_apps")
        self.esw_lib_dir = os.path.join(self.repo, "lib/sw_services")
        self.esw_drivers_dir = os.path.join(self.repo,"XilinxProcessorIPLib/drivers")
        self.esw_thirdparty_dir = os.path.join(self.repo,"ThirdParty")

    def get_comp_dir(self, comp_name, is_app=False):
        if is_app:
            comp_dir = os.path.join(self.esw_apps_dir, comp_name)
        elif comp_name.startswith('x'):
            comp_dir = os.path.join(self.esw_lib_dir, comp_name)
        elif 'freertos' in comp_name:
            comp_dir = os.path.join(self.esw_thirdparty_dir, f"bsp/freertos10_xilinx")
        elif 'lwip' in comp_name:
            comp_dir = os.path.join(self.esw_thirdparty_dir, f"sw_services/lwip211")
        elif 'standalone' in comp_name:
            comp_dir = os.path.join(self.repo, f"lib/bsp/standalone")
        else:
            comp_dir = os.path.join(self.esw_drivers_dir, comp_name)

        assert is_dir(comp_dir), f"{comp_dir} doesnt exist. Not able to add the lib"
        return comp_dir

    
