# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)
include(${CMAKE_CURRENT_SOURCE_DIR}/Freertos_lwip_udp_perf_clientExample.cmake)
set(APP_NAME freertos_lwip_udp_perf_client)
project(${APP_NAME})

find_package(common)
include(${CMAKE_SOURCE_DIR}/freertos_lwip_udp_perf_client.cmake NO_POLICY_SCOPE)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS lwip211)
collect(PROJECT_LIB_DEPS freertos)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(SOURCES udp_perf_client.c;main.c;iic_phyreset.c;)
linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/linker_files/")
add_executable(${APP_NAME}.elf ${SOURCES})
target_link_libraries(${APP_NAME}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
target_include_directories(${APP_NAME}.elf PUBLIC "${CMAKE_BINARY_DIR}/include")
if (${NON_YOCTO})
   set (INCLUDE_DIRS
       "${CMAKE_INCLUDE_PATH}/include/"
       "${CMAKE_BINARY_DIR}/include")
  target_include_directories(${APP_NAME}.elf PUBLIC ${INCLUDE_DIRS})
endif()
