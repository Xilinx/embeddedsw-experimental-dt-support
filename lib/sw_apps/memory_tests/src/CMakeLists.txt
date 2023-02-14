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
include(${CMAKE_CURRENT_SOURCE_DIR}/Memory_testsExample.cmake)
set(APP_NAME memory_tests)
project(${APP_NAME})

find_package(common)
enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

MACRO(memory_section index ddrip start_address size)
     set(MEMNODE${index}
         "{
        \t\"${ddrip}\",
        \t\"${ddrip}\",
        \t${start_address},
        \t${size},
          },\n\t ")
endmacro()

list(LENGTH TOTAL_MEM_CONTROLLERS _len)
MATH(EXPR _len "${_len}-1")
set(NUM_MEM_RANGES ${_len})
set(index 0)
foreach(MEM_CONTROLLER ${TOTAL_MEM_CONTROLLERS})
    set(val ${${MEM_CONTROLLER}})
    list(GET val 0 start_address)
    list(GET val 1 size)
    memory_section(${index} ${MEM_CONTROLLER} ${start_address} ${size}) 
    if (${index} EQUAL 0)
        set(linker_mem ${MEM_CONTROLLER})
    endif()
    MATH(EXPR index "${index}+1") 
endforeach()
foreach(i RANGE ${index})
    if (${i} EQUAL 0)
        string(APPEND MEMNODES "/* ")
        string(APPEND MEMNODES ${linker_mem})
        string(APPEND MEMNODES " memory will not be tested since application resides in the same memory */\n\t")
    else()
        string(APPEND MEMNODES ${MEMNODE${i}})
    endif()
endforeach()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/memory_config_g.c.in ${CMAKE_CURRENT_SOURCE_DIR}/memory_config_g.c)
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
    set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})
endif()
set(SOURCES memorytest.c;platform.c;memory_config_g.c)
set(CMAKE_INFILE_PATH "${CMAKE_SOURCE_DIR}/linker_files/")
linker_gen(${CMAKE_INFILE_PATH})
string(APPEND CMAKE_C_FLAGS ${USER_CC_FLAGS})
string(APPEND CMAKE_C_LINK_FLAGS ${USER_LINKER_FLAGS})
add_executable(${APP_NAME}.elf ${SOURCES})
target_link_libraries(${APP_NAME}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
