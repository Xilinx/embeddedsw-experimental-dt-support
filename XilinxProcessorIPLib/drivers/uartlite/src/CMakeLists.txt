# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
project(uartlite)

set(UARTLITE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(UARTLITE)
list(APPEND defs "#ifndef XUARTLITE_CONFIG_H_\n")
list(APPEND defs "#define XUARTLITE_CONFIG_H_\n\n")
if (DEFINED STDIN_INSTANCE)
    set(index 0)
    LIST_INDEX(${index} ${STDIN_INSTANCE} "${DRIVER_INSTANCES}")
    list(GET TOTAL_DRIVER_PROP_LIST ${index} prop_list)
    list(GET ${prop_list} 1 base_addr)
    list(APPEND defs "#define XPAR_STDIN_IS_UARTLITE\n")
    list(APPEND defs "#define STDIN_BASEADDRESS ${base_addr}U\n")
    list(APPEND defs "#define STDOUT_BASEADDRESS ${base_addr}U\n")
endif()
list(APPEND defs "\n#endif")
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/xuartlite_config.h ${defs})

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xuartlite.c)
collect (PROJECT_LIB_HEADERS xuartlite.h)
collect (PROJECT_LIB_SOURCES xuartlite_g.c)
collect (PROJECT_LIB_HEADERS xuartlite_i.h)
collect (PROJECT_LIB_SOURCES xuartlite_intr.c)
collect (PROJECT_LIB_SOURCES xuartlite_l.c)
collect (PROJECT_LIB_HEADERS xuartlite_l.h)
collect (PROJECT_LIB_SOURCES xuartlite_selftest.c)
collect (PROJECT_LIB_SOURCES xuartlite_sinit.c)
collect (PROJECT_LIB_SOURCES xuartlite_stats.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(uartlite STATIC ${_sources})
set_target_properties(uartlite PROPERTIES LINKER_LANGUAGE C)
install(TARGETS uartlite LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
