# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
project(spips)

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xspips.c)
collect (PROJECT_LIB_HEADERS xspips.h)
collect (PROJECT_LIB_SOURCES xspips_g.c)
collect (PROJECT_LIB_SOURCES xspips_hw.c)
collect (PROJECT_LIB_HEADERS xspips_hw.h)
collect (PROJECT_LIB_SOURCES xspips_options.c)
collect (PROJECT_LIB_SOURCES xspips_selftest.c)
collect (PROJECT_LIB_SOURCES xspips_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(spips STATIC ${_sources})
set_target_properties(spips PROPERTIES LINKER_LANGUAGE C)
install(TARGETS spips LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
