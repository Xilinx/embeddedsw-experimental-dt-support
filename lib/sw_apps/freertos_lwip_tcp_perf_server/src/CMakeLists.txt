# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
include(${CMAKE_CURRENT_SOURCE_DIR}/Freertos_lwip_tcp_perf_serverExample.cmake)
project(freertos_lwip_tcp_perf_server)

include(${CMAKE_SOURCE_DIR}/../data/freertos_lwip_tcp_perf_server.cmake NO_POLICY_SCOPE)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS lwip211)
collect(PROJECT_LIB_DEPS freertos)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(SOURCES main.c;iic_phyreset.c;freertos_tcp_perf_server.c;)
linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/")
add_executable(freertos_lwip_tcp_perf_server.elf ${SOURCES})
target_link_libraries(freertos_lwip_tcp_perf_server.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
target_include_directories(freertos_lwip_tcp_perf_server.elf PUBLIC "${CMAKE_BINARY_DIR}/include")
