from utils import *
import argparse, textwrap
import lopper
import os
import re
from collections import OrderedDict

class BSP:
    def __init__(self):
        pass

    def get_machine(self):
        with open(self.sdt, 'r') as file:
            content = file.read()
            if "cpus_a53" in content:
                return "zynqmp"
            elif "cpus_a72" in content:
                return "versal"
            else:
                return

    def apps_cmake_update(self):
        if self.name == "zynqmp_fsbl":
            if "a53" in self.proc:
                add_newline(self.toolchain_file, 'set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -flto -ffat-lto-objects -DARMA53_64")')
                add_newline(self.toolchain_file, 'set( CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -Os -flto -ffat-lto-objects -DARMA53_64")')
            if "r5" in self.proc:
                add_newline(self.toolchain_file, 'set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -flto -ffat-lto-objects -DARMR5")')
                add_newline(self.toolchain_file, 'set( CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -Os -flto -ffat-lto-objects -DARMR5")')
        if 'freertos' in self.os or 'freertos' in self.name:
            add_newline(self.toolchain_file, 'set( CMAKE_SYSTEM_NAME FreeRTOS)')
        if 'r5' in self.proc:
            replace_line(self.toolchain_file, 'ESW_MACHINE "cortexr5' ,f'set( ESW_MACHINE "cortexr5-{self.get_machine()}")')
            replace_line(self.toolchain_file, 'CMAKE_MACHINE "Versal' ,f'set( CMAKE_MACHINE "{self.get_machine().title()}")')



    def gen_xilstandalone(self):
        srcdir = os.path.join(self.repo,"lib/bsp/standalone/src")
        bspsrc = os.path.join(self.libsrc_folder,'standalone/src')
        copy_directory(srcdir, bspsrc)
        os.chdir(bspsrc)
        # Generate CPU specific Meta-data
        runcmd(f"lopper {self.sdt} -- baremetal_bspconfig_xlnx {self.machine} {srcdir}")

        build_xilstandalone = os.path.join(self.libsrc_folder,'standalone/build_xilstandalone')
        reset(build_xilstandalone)
        os.chdir(build_xilstandalone)
        runcmd(f"cmake {bspsrc} -DCMAKE_TOOLCHAIN_FILE={self.toolchain_file} -DOS_ESW=ON")
        runcmd("make -j22")
        runcmd("make install")

    def gen_libxil(self):
        print("Generating baremetal driver list\n\r");
        os.chdir(self.libsrc_folder)
        runcmd(f"lopper {self.sdt} -- baremetaldrvlist_xlnx {self.machine} {self.repo}")

        # 2. Copy the driver source code
        with open('distro.conf', 'r') as fd:
            drv_names = re.search('DISTRO_FEATURES = "(.*)"', fd.readline()).group(1)
            cmake_drvlist_format = re.sub(' +', ';', drv_names)
            drvlist = cmake_drvlist_format.split(';')
            with open('DRVLISTConfig.cmake', 'w') as cfd:
                cfd.write(f'set(DRIVER_LIST "{cmake_drvlist_format}")\n')

            for drv in drvlist:
                drv = drv.replace('-', '_')
                srcdir = os.path.join(self.esw_drivers_dir,drv,'src/')
                drvsrc = os.path.join(self.libsrc_folder,drv,'src')
                copy_directory(srcdir,drvsrc)

                # 3. Generate _g.c file
                try:
                    os.chdir(drvsrc)
                    drv_has_metadata = srcdir + str("../data/%s.yaml" % drv)
                    if os.path.isfile(drv_has_metadata):
                        print(f"Generating _g.c for the driver {drv}\n\r")
                        if re.match("uartps", drv):
                            runcmd(f"lopper {self.sdt} -- baremetalconfig_xlnx {self.machine} {srcdir} stdin")
                        else:
                            runcmd(f"lopper {self.sdt} -- baremetalconfig_xlnx {self.machine} {srcdir}")
                except:
                    pass

        os.chdir(self.libsrc_folder)
        # 4. Generate xparameters.h file
        print("Generating the xparameters.h file\n\r");
        runcmd(f"lopper -f {self.sdt} -- baremetal_xparameters_xlnx {self.machine} {self.repo}")

        # 5. Configure and compile and install libxil to <proc>/lib folder
        libxil_cmake = os.path.join(self.repo,"XilinxProcessorIPLib/drivers/CMakeLists.txt")
        copy_file(libxil_cmake, f'{self.libsrc_folder}/')

        build_libxil = os.path.join(self.libsrc_folder,'build_libxil')
        reset(build_libxil)
        os.chdir(build_libxil)
        runcmd(f"cmake {self.libsrc_folder} -DCMAKE_TOOLCHAIN_FILE={self.toolchain_file} -DOS_ESW=ON")
        runcmd("make -j22")
        runcmd("make install")

        os.chdir(self.libsrc_folder)
        remove('distro.conf')
        remove('libxil.conf')
        remove('DRVLISTConfig.cmake')

    def get_comp_dir(self,comp_name):
        if comp_name==self.name:
            comp_dir = os.path.join(self.esw_apps_dir, comp_name)
        elif comp_name.startswith('x'):
            comp_dir = os.path.join(self.esw_lib_dir, comp_name)
        elif 'freertos' in self.os or 'freertos' in self.name:
            comp_dir = os.path.join(self.esw_thirdparty_dir, f"bsp/freertos10_xilinx")
        elif 'lwip' in comp_name:
            comp_dir = os.path.join(self.esw_thirdparty_dir, f"sw_services/lwip211")
        else:
            comp_dir = os.path.join(self.esw_drivers_dir, comp_name)

        assert is_dir(comp_dir), f"{comp_dir} doesnt exist. Not able to add the lib"
        return comp_dir


    def fetch_dep_hierarch(self,comp_name,order=[]):
        comp_dir = self.get_comp_dir(comp_name)
        comp_base_name = get_base_name(comp_dir)
        yaml_file = os.path.join(comp_dir, f"data/{comp_base_name}.yaml")
        if comp_name in order:
            order.remove(comp_name)
        order+= [comp_name]
        lib_list = []
        if is_file(yaml_file):
            schema = load_yaml(yaml_file)
            lib_list = schema.get('depends_libs',[])

        for lib in lib_list:
            self.fetch_dep_hierarch(lib,order)

        return order

    def lib_dep_params(self,lib_dep_order):
        lib_params_dict = {}
        for lib in lib_dep_order:
            comp_dir = self.get_comp_dir(lib)
            comp_base_name = get_base_name(lib)
            yaml_file = os.path.join(comp_dir, f"data/{comp_base_name}.yaml")
            if is_file(yaml_file):
                schema = load_yaml(yaml_file)
                if schema.get('lib_config',{}):
                    for entry in schema['lib_config'].keys():
                        try:
                            lib_params_dict[entry] += schema['lib_config'][entry]
                        except KeyError:
                            lib_params_dict[entry] = schema['lib_config'][entry]
                if schema.get('required'):
                    try:
                        lib_params_dict[lib] += [{'hw_metadata':True}]
                    except KeyError:
                        lib_params_dict[lib] = [{'hw_metadata':True}]

        return lib_params_dict

    def add_lib(self, comp_name):
        # Handle dependnecies if any
        # Read yaml and find
        
        libs_to_build = list(reversed(self.fetch_dep_hierarch(comp_name)))
        lib_params_dict = self.lib_dep_params(libs_to_build)

        for lib in libs_to_build:
            build_hw_metadata = False
            if lib == self.name:
                continue
            print(f"lib is {lib}")
            cmake_cmd_append = ""
            if lib in lib_params_dict.keys():
                for entries in lib_params_dict[lib]:
                    for key, value in entries.items():
                        if key == 'hw_metadata':
                            build_hw_metadata = True
                        else:
                            cmake_cmd_append += f" -D{key}={value}"

            comp_dir = self.get_comp_dir(lib)
            srcdir = os.path.join(comp_dir, 'src/')
            dstdir = os.path.join(self.libsrc_folder, f"{lib}/src")
            copy_directory(srcdir, dstdir)
            os.chdir(dstdir)
            # Generate h/w meta-data
            if build_hw_metadata:
                runcmd(f"lopper {self.sdt} -- bmcmake_metadata_xlnx {self.machine} {srcdir} hwcmake_metadata {self.repo}")
            # Compile 
            build_lib = os.path.join(dstdir,f'build_{lib}')
            reset(build_lib)
            os.chdir(build_lib)
            runcmd(f"cmake {dstdir} -DCMAKE_TOOLCHAIN_FILE={self.toolchain_file} -DOS_ESW=ON {cmake_cmd_append}")
            runcmd("make -j22")
            runcmd("make install")

    def generate_bsp(self):
        self.apps_cmake_update()
        self.gen_xilstandalone()
        self.gen_libxil()

    def lib_addition(self):
        self.add_lib(self.name)
        if self.lib:
            for lib_name in lib:
                self.add_lib(lib_name)
