# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
collect (PROJECT_LIB_HEADERS Ecdsa.h)
collect (PROJECT_LIB_HEADERS xsecure_error.h)
collect (PROJECT_LIB_HEADERS xsecure_sha_hw.h)
collect (PROJECT_LIB_SOURCES xsecure_plat.c)
collect (PROJECT_LIB_HEADERS xsecure_plat.h)

if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_r5.a ${CMAKE_BINARY_DIR}/libxilsecure.a)
endif()

if (${NON_YOCTO})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_pmc.a ${CMAKE_BINARY_DIR}/libxilsecure_pmc.a)
    #execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libxilsecure_pmc.a) ${CMAKE_LIBRARY_PATH}/libxilsecure_pmc.a)
    file(COPY ${CMAKE_BINARY_DIR}/libxilsecure_pmc.a DESTINATION ${CMAKE_LIBRARY_PATH} FOLLOW_SYMLINK_CHAIN)

endif()

