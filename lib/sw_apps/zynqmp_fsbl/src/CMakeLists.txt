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
set(APP_NAME zynqmp_fsbl)
project(${APP_NAME})

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilpm)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilffs)
collect(PROJECT_LIB_DEPS xilsecure)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

set(_lib "xilpm")
list (APPEND _lib "xil")
list (APPEND _lib "xilffs")
list (APPEND _lib "xilsecure")
list (APPEND _lib "xilstandalone")
list (APPEND _lib "gcc")
list (APPEND _lib "c")
find_library(_lib_path ${_lib})

collector_list (_deps PROJECT_LIB_DEPS)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/psu_init.h defs)
string(FIND "${defs}" "xfsbl_hw.h"_index)
if (${_index} EQUAL -1)
list(PREPEND defs "#include \"xfsbl_hw.h\"\n\n")
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/psu_init.h "${defs}")
endif()
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${D}${libdir})
file(GLOB SOURCES "*.c")
if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53-32")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64"))
    set(S_SOURCES xfsbl_exit.S xfsbl_translation_table_a53_64.S)
else()
    set(S_SOURCES xfsbl_exit.S xfsbl_translation_table_a53_32.S)
endif()
string(APPEND CMAKE_C_FLAGS ${USER_CC_FLAGS})
string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINKER_FLAGS})
add_executable(${APP_NAME}.elf ${S_SOURCES} ${SOURCES})

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
    OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64"))
target_link_libraries(${APP_NAME}.elf -Os -flto -ffat-lto-objects -Wl,--gc-sections -n -T\"${CMAKE_SOURCE_DIR}/lscript_a53.ld\" -Wl,--start-group ${_deps} -Wl,--end-group)
else()
target_link_libraries(${APP_NAME}.elf -Os -flto -ffat-lto-objects -Wl,--gc-sections -n -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -Wl,--start-group ${_deps} -Wl,--end-group)
endif()
