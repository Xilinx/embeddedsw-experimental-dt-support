# SPDX-License-Identifier: MIT
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
include(${CMAKE_CURRENT_SOURCE_DIR}/XilsecureExample.cmake)
project(xilsecure)

enable_language(C ASM)
collect(PROJECT_LIB_DEPS xilstandalone)
collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS xiltimer)
collect(PROJECT_LIB_DEPS xilsecure)
collect(PROJECT_LIB_DEPS gcc)
collect(PROJECT_LIB_DEPS c)
collector_list (_deps PROJECT_LIB_DEPS)

SET(VERSAL_EXAMPLES xilsecure_versal_aes_example.c;xilsecure_rsa_generic_example.c;xilsecure_sha_example.c;)

SET(ZYNQMP_EXAMPLES xilsecure_aes_example.c;xilsecure_simple_aes_example.c;xilsecure_rsa_example.c;xilsecure_rsa_generic_example.c;xilsecure_sha_example.c;)

linker_gen("${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/")

if("${CMAKE_MACHINE}" STREQUAL "Versal")
	foreach(EXAMPLE ${VERSAL_EXAMPLES})
		string(REGEX REPLACE "\\.[^.]*$" "" EXAMPLE ${EXAMPLE})
        	add_executable(${EXAMPLE}.elf ${EXAMPLE})
		target_link_libraries(${EXAMPLE}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
	endforeach()
else()
	foreach(EXAMPLE ${ZYNQMP_EXAMPLES})
		string(REGEX REPLACE "\\.[^.]*$" "" EXAMPLE ${EXAMPLE})
                add_executable(${EXAMPLE}.elf ${EXAMPLE})
		target_link_libraries(${EXAMPLE}.elf -Wl,--gc-sections -T\"${CMAKE_SOURCE_DIR}/lscript.ld\" -L\"${CMAKE_SOURCE_DIR}/\" -Wl,--start-group ${_deps} -Wl,--end-group)
        endforeach()
endif()
