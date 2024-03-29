# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
project(xilffs)

include(${CMAKE_CURRENT_SOURCE_DIR}/../data/xilffs.cmake NO_POLICY_SCOPE)
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

enable_language(C ASM)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include)
collect (PROJECT_LIB_SOURCES diskio.c)
collect (PROJECT_LIB_SOURCES ff.c)
collect (PROJECT_LIB_SOURCES ffsystem.c)
collect (PROJECT_LIB_SOURCES ffunicode.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(xilffs STATIC ${_sources})
set_target_properties(xilffs PROPERTIES LINKER_LANGUAGE C)
install(TARGETS xilffs LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
