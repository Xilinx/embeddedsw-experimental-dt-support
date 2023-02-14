# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
####    START OF USER SETTINGS    ####
# Below settings can be customized
set(USER_CC_FLAGS
	""
)
set(USER_LINKER_FLAGS
	""
)
####    END OF USER SETTINGS    ####
include_directories(${CMAKE_BINARY_DIR}/include)
set(APP_NAME zynqmp_pmufw)
project(${APP_NAME})

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
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()
file(GLOB SOURCES "*.c")
string(APPEND CMAKE_C_FLAGS ${USER_CC_FLAGS})
string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINKER_FLAGS})
add_executable(${APP_NAME}.elf xpfw_start.S ${SOURCES})
target_link_libraries(${APP_NAME}.elf -Wl,--no-relax -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v9.2 -mxl-soft-mul -Os -flto -ffat-lto-objects -Wl,--gc-sections -nostartfiles -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -Wl,--start-group ${_deps} -Wl,--end-group)

