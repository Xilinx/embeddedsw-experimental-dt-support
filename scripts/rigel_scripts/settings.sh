export XILINX_TOOLCHAIN=/proj/xbuilds/2023.1_daily_latest/installs/lin64/Vitis/2023.1
export PYTHON_VER="python-3.8.3"
export CMAKE_VER="cmake-3.24.2"

export XBUILDS_LOPPER_PATH=${XILINX_TOOLCHAIN}/data/lopper
export XBUILDS_CMAKE_PATH=${XILINX_TOOLCHAIN}/tps/lnx64/${CMAKE_VER}
export XBUILDS_PYTHON_PATH=${XILINX_TOOLCHAIN}/tps/lnx64/${PYTHON_VER}

# LD_LIBRARY_PATH is needed to add the linker libraries (e.g. libffi) to the path
export LD_LIBRARY_PATH=${XBUILDS_PYTHON_PATH}/lib:${XBUILDS_CMAKE_PATH}/libs/Ubuntu:${LD_LIBRARY_PATH}

# Source Paths of different toolchains, python, cmake and lopper binary
export PATH=${XILINX_TOOLCHAIN}/bin:${XILINX_TOOLCHAIN}/gnu/microblaze/lin/bin:${XILINX_TOOLCHAIN}/gnu/arm/lin/bin:${XILINX_TOOLCHAIN}/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:${XILINX_TOOLCHAIN}/gnu/aarch32/lin/gcc-arm-none-eabi/bin:${XILINX_TOOLCHAIN}/gnu/aarch64/lin/aarch64-none/bin:${XILINX_TOOLCHAIN}/gnu/armr5/lin/gcc-arm-none-eabi/bin:${XBUILDS_PYTHON_PATH}/bin:${XBUILDS_LOPPER_PATH}/bin:${XBUILDS_CMAKE_PATH}/bin:$PATH

# Adds Installed Lopper directory to the PYTHONPATH
export PYTHONPATH=${XBUILDS_LOPPER_PATH}:$PYTHONPATH

# OSF flag is needed to enable the validation of inputs. Set this flag to "" when
# using the GUI flow.
export OSF="True"

# Below is a fallback mechanism to set an esw repo when no user repo is set
export ESW_REPO="${XILINX_TOOLCHAIN}/data/embeddedsw-sdt"

# LOPPER_DTC_FLAGS is needed to generate device tree with symbols in it.
export LOPPER_DTC_FLAGS="-b 0 -@"
