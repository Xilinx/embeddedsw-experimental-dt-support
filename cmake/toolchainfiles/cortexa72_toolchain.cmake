set( CMAKE_EXPORT_COMPILE_COMMANDS ON)
set( CMAKE_INSTALL_MESSAGE LAZY)
set( CMAKE_C_COMPILER aarch64-none-elf-gcc )
set( CMAKE_CXX_COMPILER aarch64-none-elf-g++ )
set( CMAKE_C_COMPILER_LAUNCHER  )
set( CMAKE_CXX_COMPILER_LAUNCHER  )
set( CMAKE_ASM_COMPILER aarch64-none-elf-gcc )
set( CMAKE_AR aarch64-none-elf-ar CACHE FILEPATH "Archiver" )
set( CMAKE_SYSTEM_PROCESSOR "cortexa72" )
set( CMAKE_MACHINE "Versal" )
set( CMAKE_SYSTEM_NAME "Generic" )
set( CMAKE_C_FLAGS " -O2 -pipe -g -feliminate-unused-debug-types -mcpu=cortex-a53 -march=armv8-a+crc -specs=$ENV{ESW_REPO}/scripts/specs/arm/Xilinx.spec -I${CMAKE_INCLUDE_PATH} " CACHE STRING "CFLAGS" )
set( CMAKE_CXX_FLAGS " -O2 -pipe -g -feliminate-unused-debug-types -mcpu=cortex-a53 -march=armv8-a+crc -specs=$ENV{ESW_REPO}/scripts/specs/arm/Xilinx.spec" CACHE STRING "CFLAGS" )
set( CMAKE_ASM_FLAGS " -O2 -pipe -g -feliminate-unused-debug-types -mcpu=cortex-a53 -march=armv8-a+crc -specs=$ENV{ESW_REPO}/scripts/specs/arm/Xilinx.spec -I${CMAKE_INCLUDE_PATH}" CACHE STRING "ASM FLAGS" )
set( CMAKE_C_LINK_FLAGS " -mcpu=cortex-a53 -march=armv8-a+crc  -pipe -g -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed -L${CMAKE_LIBRARY_PATH}" CACHE STRING "LDFLAGS" )
set( CMAKE_CXX_LINK_FLAGS " -mcpu=cortex-a53 -march=armv8-a+crc -pipe -g -feliminate-unused-debug-types -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed" CACHE STRING "LDFLAGS" )
set( CMAKE_C_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Additional CFLAGS for release" )
set( CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Additional CXXFLAGS for release" )
set( CMAKE_ASM_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Additional ASM FLAGS for release" )
