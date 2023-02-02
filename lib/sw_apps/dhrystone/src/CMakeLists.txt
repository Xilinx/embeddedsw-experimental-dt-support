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

include(${CMAKE_CURRENT_SOURCE_DIR}/DhrystoneExample.cmake)
set(APP_NAME dhrystone)
project(${APP_NAME})

enable_language(C ASM)
find_package(common)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

collect (PROJECT_LIB_SOURCES platform.c)
collect (PROJECT_LIB_SOURCES dhry_1.c)
collect (PROJECT_LIB_SOURCES dhry_2.c)
collector_list (_sources PROJECT_LIB_SOURCES)
linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/linker_files/")
string(APPEND CMAKE_C_FLAGS ${USER_CC_FLAGS})
string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINKER_FLAGS})
add_executable(${APP_NAME}.elf ${_sources})
target_link_libraries(${APP_NAME}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
