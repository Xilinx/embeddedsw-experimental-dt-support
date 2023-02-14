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

include(${CMAKE_CURRENT_SOURCE_DIR}/Hello_worldExample.cmake)
set(APP_NAME hello_world)
project(${APP_NAME})

enable_language(C ASM CXX)
find_package(common)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

collect (PROJECT_LIB_SOURCES platform.c)
collect (PROJECT_LIB_SOURCES helloworld.c)
collector_list (_sources PROJECT_LIB_SOURCES)
foreach (source ${_sources})
    get_filename_component(ext ${source} EXT)
    list(APPEND src_ext ${ext})
endforeach()

if (".cpp" IN_LIST src_ext)
    set(PROJECT_TYPE "c++")
else()
    set(PROJECT_TYPE "c")
endif()

if("${PROJECT_TYPE}" STREQUAL "c++")
collect(PROJECT_LIB_DEPS stdc++)
endif()
collector_list (_deps PROJECT_LIB_DEPS)

if("${PROJECT_TYPE}" STREQUAL "c++")
string (REPLACE ";" ",-l" _deps "${_deps}")
endif()
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()
linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/linker_files/")
string(APPEND CMAKE_C_FLAGS ${USER_CC_FLAGS})
string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINKER_FLAGS})
add_executable(${APP_NAME}.elf ${_sources})
target_link_libraries(${APP_NAME}.elf -Wl,-T -Wl,\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -L\"${CMAKE_LIBRARY_PATH}/\" -Wl,--start-group,-l${_deps} -Wl,--end-group)
