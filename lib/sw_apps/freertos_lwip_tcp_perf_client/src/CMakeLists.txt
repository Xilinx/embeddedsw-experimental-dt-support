# SPDX-License-Identifier: MIT
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
include(${CMAKE_CURRENT_SOURCE_DIR}/Freertos_lwip_tcp_perf_clientExample.cmake)
project(freertos_lwip_tcp_perf_client)

include(${CMAKE_SOURCE_DIR}/../data/freertos_lwip_tcp_perf_client.cmake NO_POLICY_SCOPE)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS lwip211)
collect(PROJECT_LIB_DEPS freertos)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(SOURCES main.c;iic_phyreset.c;freertos_tcp_perf_client.c;)
linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/")
add_executable(freertos_lwip_tcp_perf_client.elf ${SOURCES})
target_link_libraries(freertos_lwip_tcp_perf_client.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
target_include_directories(freertos_lwip_tcp_perf_client.elf PUBLIC "${CMAKE_BINARY_DIR}/include")
