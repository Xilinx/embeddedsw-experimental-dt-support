# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
include_directories(${CMAKE_BINARY_DIR}/include)
project(pmufw)

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilsecure)
collect(PROJECT_LIB_DEPS xilfpga)
collect(PROJECT_LIB_DEPS xilskey)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

set(_lib "xilstandalone")
list (APPEND _lib "xil")
list (APPEND _lib "xilsecure")
list (APPEND _lib "xilfpga")
list (APPEND _lib "xilskey")
list (APPEND _lib "gcc")
list (APPEND _lib "c")
find_library(_lib_path ${_lib})

collector_list (_deps PROJECT_LIB_DEPS)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${D}${libdir})
file(GLOB SOURCES "*.c")
#set(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")
#set_source_files_properties(xpfw_start.S PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
add_executable(pmufw.elf xpfw_start.S ${SOURCES})
target_link_libraries(pmufw.elf -Wl,--no-relax -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v9.2 -mxl-soft-mul -Os -flto -ffat-lto-objects -Wl,--gc-sections -nostartfiles -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -Wl,--start-group ${_deps} -Wl,--end-group)

