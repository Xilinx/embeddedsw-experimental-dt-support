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
include_directories(${CMAKE_BINARY_DIR}/include)
set(APP_NAME versal_plm)
project(${APP_NAME})

find_package(common)
enable_language(C ASM)
#collect(PROJECT_LIB_DEPS gloss)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilsecure)
if (${NON_YOCTO})
    collect(PROJECT_LIB_DEPS xilsecure_pmc)
endif()
collect(PROJECT_LIB_DEPS xilffs)
collect(PROJECT_LIB_DEPS xilpuf)
collect(PROJECT_LIB_DEPS xilpdi)
collect(PROJECT_LIB_DEPS xilplmi)
collect(PROJECT_LIB_DEPS xilloader)
collect(PROJECT_LIB_DEPS xilpm)
collect(PROJECT_LIB_DEPS xilnvm)
collect(PROJECT_LIB_DEPS xilsem)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)

set(_lib "xilstandalone")
list (APPEND _lib "xilmem")
list (APPEND _lib "xil")
list (APPEND _lib "xilsecure")
list (APPEND _lib "xilffs")
list (APPEND _lib "xilpdi")
list (APPEND _lib "gcc")
list (APPEND _lib "c")
find_library(_lib_path ${_lib})

collector_list (_deps PROJECT_LIB_DEPS)
message(STATUS "deps is:" ${_deps})

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(common)
if (("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze"))
	add_subdirectory(versal)
else()
	add_subdirectory(versal_net)
endif()

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
#target_link_libraries(${APP_NAME}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -Wl,--start-group ${_deps} -Wl,--end-group)
target_link_libraries(${APP_NAME}.elf -Wl,-Map=versal_plm.map -Wl,--build-id=none -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/versal/lscript.ld\" -Wl,--start-group ${_deps} -Wl,--end-group)
