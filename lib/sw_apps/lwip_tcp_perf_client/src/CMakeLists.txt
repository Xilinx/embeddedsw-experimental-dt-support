# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/Lwip_tcp_perf_clientExample.cmake)
project(lwip_tcp_perf_client)

find_package(common)
include(${CMAKE_SOURCE_DIR}/lwip_tcp_perf_client.cmake NO_POLICY_SCOPE)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS lwip211)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(SOURCES si5324.c;sfp.c;main.c;i2c_access.c;platform.c;iic_phyreset.c;tcp_perf_client.c)
linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/linker_files/")
add_executable(lwip_tcp_perf_client.elf ${SOURCES})
target_link_libraries(lwip_tcp_perf_client.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
target_include_directories(lwip_tcp_perf_client.elf PUBLIC "${CMAKE_BINARY_DIR}/include")
if (${NON_YOCTO})
   set (INCLUDE_DIRS
       "${CMAKE_INCLUDE_PATH}/include/"
       "${CMAKE_BINARY_DIR}/include")
  target_include_directories(lwip_tcp_perf_client.elf PUBLIC ${INCLUDE_DIRS})
endif()
