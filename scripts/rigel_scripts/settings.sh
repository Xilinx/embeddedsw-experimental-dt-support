export LOPPER_PATH=/proj/xhdsswstaff/onkarh/demo/lopper_env_chk/lopper_deleted_pycache
export CMAKE_PATH=/proj/xhdsswstaff/onkarh/demo/cmake_install
export XILINX_TOOLCHAIN=/proj/xbuilds/2023.1_daily_latest/installs/lin64/Vitis/2023.1
export PYTHON_VER="python-3.8.3"

export LD_LIBRARY_PATH=${XILINX_TOOLCHAIN}/tps/lnx64/${PYTHON_VER}/lib:${LD_LIBRARY_PATH}

export PATH=${XILINX_TOOLCHAIN}/gnu/microblaze/lin/bin:${XILINX_TOOLCHAIN}/gnu/arm/lin/bin:${XILINX_TOOLCHAIN}/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:${XILINX_TOOLCHAIN}/gnu/aarch32/lin/gcc-arm-none-eabi/bin:${XILINX_TOOLCHAIN}/gnu/aarch64/lin/aarch64-none/bin:${XILINX_TOOLCHAIN}/gnu/armr5/lin/gcc-arm-none-eabi/bin:${XILINX_TOOLCHAIN}/tps/lnx64/${PYTHON_VER}/bin:${LOPPER_PATH}/bin:${CMAKE_PATH}/bin:$PATH

export PYTHONPATH=${LOPPER_PATH}:$PYTHONPATH
export OSF=""
#"True"
export ESW_REPO="/proj/xhdsswstaff/onkarh/demo/embeddedsw-experimental-dt-support"
export LOPPER_DTC_FLAGS="-b 0 -@"
