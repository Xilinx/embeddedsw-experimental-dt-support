# SPDX-License-Identifier: MIT
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
include(${CMAKE_CURRENT_SOURCE_DIR}/V_frmbuf_rdExample.cmake)
project(v_frmbuf_rd)

enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(DRIVER_INSTANCES "${NUM_DRIVER_INSTANCES}" CACHE STRING "Driver Instances")
SET_PROPERTY(CACHE DRIVER_INSTANCES PROPERTY STRINGS "${NUM_DRIVER_INSTANCES}")
set(index 0)
LIST_INDEX(${index} ${DRIVER_INSTANCES} "${NUM_DRIVER_INSTANCES}")
list(GET TOTAL_EXAMPLE_LIST ${index} ex_list)
list(GET REG_LIST ${index} reg)
SET(XV_FRMBUF_RD_BASEADDRESS "${reg}")
SET(EXAMPLE ${CMAKE_CURRENT_SOURCE_DIR}/src/xv_frmbufrd_example.c;)
SET(COMMON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/platform.c;)
SET(EXAMPLE_LIST "${${ex_list}}" CACHE STRING "Driver Example List")
SET_PROPERTY(CACHE EXAMPLE_LIST PROPERTY STRINGS "${${ex_list}}")

set(CMAKE_INFILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/")
linker_gen(${CMAKE_INFILE_PATH})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/platform.h DESTINATION ${CMAKE_BINARY_DIR}/include)

add_executable(xv_frmbufrd_example.elf ${EXAMPLE} ${COMMON_FILE})
target_link_libraries(xv_frmbufrd_example.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
