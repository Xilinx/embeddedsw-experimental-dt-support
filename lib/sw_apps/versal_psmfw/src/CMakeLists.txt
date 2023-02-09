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
set(APP_NAME versal_psmfw)
project(${APP_NAME})

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)


collector_list (_deps PROJECT_LIB_DEPS)
message(STATUS "deps is:" ${_deps})

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(common)
add_subdirectory(versal)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
string(APPEND CMAKE_C_FLAGS ${USER_CC_FLAGS})
string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINKER_FLAGS})
add_executable(${APP_NAME}.elf ${_sources})
target_link_libraries(${APP_NAME}.elf -Wl,-Map=versal_plm.map -Wl,--build-id=none -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/versal/lscript.ld\" -Wl,--start-group ${_deps} -Wl,--end-group)
