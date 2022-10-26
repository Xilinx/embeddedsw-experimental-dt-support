set( CMAKE_EXPORT_COMPILE_COMMANDS ON)
set( ESW_MACHINE "microblaze-plm")
set( CMAKE_C_COMPILER mb-gcc )
set( CMAKE_CXX_COMPILER mb-g++ )
set( CMAKE_C_COMPILER_LAUNCHER  )
set( CMAKE_CXX_COMPILER_LAUNCHER  )
set( CMAKE_ASM_COMPILER mb-gcc )
set( CMAKE_AR mb-ar CACHE FILEPATH "Archiver" )
set( CMAKE_SYSTEM_PROCESSOR "plm_microblaze" )
set( CMAKE_MACHINE "Versal" )
set( CMAKE_SYSTEM_NAME "Generic" )
set( CMAKE_C_FLAGS " -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mxl-reorder -mcpu=v10.0 -mno-xl-soft-mul -mxl-multiply-high -mno-xl-soft-div -Os -flto -ffat-lto-objects  -DVERSAL_PLM=1 -specs=$ENV{ESW_REPO}/scripts/specs/microblaze/Xilinx.spec -I${CMAKE_INCLUDE_PATH}" CACHE STRING "CFLAGS" )
set( CMAKE_CXX_FLAGS " -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mxl-reorder -mcpu=v10.0 -mno-xl-soft-mul -mxl-multiply-high -mno-xl-soft-div -Os -flto -ffat-lto-objects -DVERSAL_PLM=1 -specs=$ENV{ESW_REPO}/scripts/specs/microblaze/Xilinx.spec" CACHE STRING "CFLAGS" )
set( CMAKE_ASM_FLAGS " -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mxl-reorder -mcpu=v10.0 -mno-xl-soft-mul -mxl-multiply-high -mno-xl-soft-div -Os -flto -ffat-lto-objects -DVERSAL_PLM=1 -specs=$ENV{ESW_REPO}/scripts/specs/microblaze/Xilinx.spec -I${CMAKE_INCLUDE_PATH}" CACHE STRING "ASM FLAGS" )
set( CMAKE_C_LINK_FLAGS " -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mxl-reorder -mcpu=v10.0 -mno-xl-soft-mul -mxl-multiply-high -mno-xl-soft-div -Wl,-O1  -Wl,--as-needed -DVERSAL_PLM=1 -L${CMAKE_LIBRARY_PATH}" CACHE STRING "LDFLAGS" )
set( CMAKE_CXX_LINK_FLAGS " -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mxl-reorder -mcpu=v10.0 -mno-xl-soft-mul -mxl-multiply-high -mno-xl-soft-div -Wl,-O1  -Wl,--as-needed -DVERSAL_PLM=1 -L${CMAKE_LIBRARY_PATH}" CACHE STRING "LDFLAGS" )
set( CMAKE_C_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Additional CFLAGS for release" )
set( CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Additional CXXFLAGS for release" )
set( CMAKE_ASM_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "Additional ASM FLAGS for release" )
