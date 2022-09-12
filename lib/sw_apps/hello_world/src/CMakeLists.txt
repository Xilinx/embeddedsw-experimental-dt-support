# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
#include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
include(${CMAKE_CURRENT_SOURCE_DIR}/Hello_worldExample.cmake)
project(hello_world)

enable_language(C ASM)
set(common_DIR "${CMAKE_INCLUDE_PATH}/../../")
find_package(common)
collect(PROJECT_LIB_DEPS xilstandalone)
#collect(PROJECT_LIB_DEPS xilmem)
collect(PROJECT_LIB_DEPS xil)
#collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

LOAD_CACHE("${CMAKE_CURRENT_SOURCE_DIR}")

collect (PROJECT_LIB_SOURCES platform.c)
collect (PROJECT_LIB_SOURCES helloworld.c)
collector_list (_sources PROJECT_LIB_SOURCES)
linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/linker_files/")
add_executable(hello_world.elf ${_sources})
target_link_libraries(hello_world.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
