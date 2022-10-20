set( ESW_MACHINE "cortexr5-versal")
set( CMAKE_C_COMPILER armr5-none-eabi-gcc )
set( CMAKE_CXX_COMPILER armr5-none-eabi-g++ )
set( CMAKE_C_COMPILER_LAUNCHER  )
set( CMAKE_CXX_COMPILER_LAUNCHER  )
set( CMAKE_ASM_COMPILER armr5-none-eabi-gcc )
set( CMAKE_AR armr5-none-eabi-ar CACHE FILEPATH "Archiver" )
set( CMAKE_SYSTEM_PROCESSOR "cortexr5" )
set( CMAKE_MACHINE "Versal" )
set( CMAKE_SYSTEM_NAME "Generic" )
set( CMAKE_C_FLAGS " -mfpu=vfpv3-d16 -mfloat-abi=hard -mcpu=cortex-r5 -DARMR5 -mcpu=cortex-r5  -O2 -pipe -g -feliminate-unused-debug-types -specs=$ENV{ESW_REPO}/scripts/specs/arm/Xilinx.spec -I${CMAKE_INCLUDE_PATH} " CACHE STRING "CFLAGS" )
set( CMAKE_CXX_FLAGS " -mfpu=vfpv3-d16 -mfloat-abi=hard -mcpu=cortex-r5 -DARMR5 -mcpu=cortex-r5 -O2 -pipe -g -feliminate-unused-debug-types -specs=$ENV{ESW_REPO}/scripts/specs/arm/Xilinx.spec" CACHE STRING "CFLAGS" )
set( CMAKE_ASM_FLAGS " -mfpu=vfpv3-d16 -mfloat-abi=hard -mcpu=cortex-r5 -DARMR5 -mcpu=cortex-r5 -O2 -pipe -g -feliminate-unused-debug-types -specs=$ENV{ESW_REPO}/scripts/specs/arm/Xilinx.spec -I${CMAKE_INCLUDE_PATH}" CACHE STRING "ASM FLAGS" )
set( CMAKE_C_LINK_FLAGS " -mfpu=vfpv3-d16 -mfloat-abi=hard -mcpu=cortex-r5 -DARMR5 -mcpu=cortex-r5 -O2  -pipe -g -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed -L${CMAKE_LIBRARY_PATH}" CACHE STRING "LDFLAGS" )
set( CMAKE_CXX_LINK_FLAGS " -mfpu=vfpv3-d16 -mfloat-abi=hard -mcpu=cortex-r5 -DARMR5 -mcpu=cortex-r5 -O2 -pipe -g -feliminate-unused-debug-types -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed" CACHE STRING "LDFLAGS" )
set( CMAKE_C_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Additional CFLAGS for release" )
set( CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Additional CXXFLAGS for release" )
set( CMAKE_ASM_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Additional ASM FLAGS for release" )
