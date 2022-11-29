export XILINX_TOOLCHAIN=/proj/xbuilds/2023.1_daily_latest/installs/lin64/Vitis/2023.1
if [ -n "${PATH}" ]; then
  export PATH=${XILINX_TOOLCHAIN}/gnu/microblaze/lin/bin:${XILINX_TOOLCHAIN}/gnu/arm/lin/bin:${XILINX_TOOLCHAIN}/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:${XILINX_TOOLCHAIN}/gnu/aarch32/lin/gcc-arm-none-eabi/bin:${XILINX_TOOLCHAIN}/gnu/aarch64/lin/aarch64-none/bin:${XILINX_TOOLCHAIN}/gnu/armr5/lin/gcc-arm-none-eabi/bin:$PATH
else
  export PATH=${XILINX_TOOLCHAIN}/gnu/microblaze/lin/bin:${XILINX_TOOLCHAIN}/gnu/arm/lin/bin:${XILINX_TOOLCHAIN}/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:${XILINX_TOOLCHAIN}/gnu/aarch32/lin/gcc-arm-none-eabi/bin:${XILINX_TOOLCHAIN}/gnu/aarch64/lin/aarch64-none/bin:${XILINX_TOOLCHAIN}/gnu/armr5/lin/gcc-arm-none-eabi/bin
fi
export ESW_REPO="/proj/xhdsswstaff/onkarh/rigel_test/new_try"
export VAL_INPUTS="True"
export LOPPER_DTC_FLAGS="-b 0 -@"
