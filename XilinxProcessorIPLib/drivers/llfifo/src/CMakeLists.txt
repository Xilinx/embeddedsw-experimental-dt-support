# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
project(llfifo)

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xllfifo.c)
collect (PROJECT_LIB_HEADERS xllfifo.h)
collect (PROJECT_LIB_SOURCES xllfifo_g.c)
collect (PROJECT_LIB_HEADERS xllfifo_hw.h)
collect (PROJECT_LIB_SOURCES xllfifo_sinit.c)
collect (PROJECT_LIB_SOURCES xstreamer.c)
collect (PROJECT_LIB_HEADERS xstreamer.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(llfifo STATIC ${_sources})
set_target_properties(llfifo PROPERTIES LINKER_LANGUAGE C)
install(TARGETS llfifo LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
