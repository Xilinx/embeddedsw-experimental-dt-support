# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/Freertos_lwip_echo_serverExample.cmake)
project(freertos_lwip_echo_server)

find_package(common)
include(${CMAKE_SOURCE_DIR}/freertos_lwip_echo_server.cmake NO_POLICY_SCOPE)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS lwip211)
collect(PROJECT_LIB_DEPS freertos)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(SOURCES echo.c;main.c;iic_phyreset.c;)
linker_gen("${CMAKE_SOURCE_DIR}/linker_files/")
add_executable(freertos_lwip_echo_server.elf ${SOURCES})
target_link_libraries(freertos_lwip_echo_server.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
if (${NON_YOCTO})
    set(INCLUDE_DIRS
        "${CMAKE_INCLUDE_PATH}/"
        "${CMAKE_INCLUDE_PATH}/include/"
        "${CMAKE_BINARY_DIR}/include")
    message("include dirs")
    message("${INCLUDE_DIRS}")
    target_include_directories(freertos_lwip_echo_server.elf PUBLIC ${INCLUDE_DIRS})
else()
    target_include_directories(freertos_lwip_echo_server.elf PUBLIC "${CMAKE_BINARY_DIR}/include")
endif()
