# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
project(axicdma)

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xaxicdma_sinit.c)
collect (PROJECT_LIB_SOURCES xaxicdma_bdring.c)
collect (PROJECT_LIB_SOURCES xaxicdma_g.c)
collect (PROJECT_LIB_HEADERS xaxicdma_i.h)
collect (PROJECT_LIB_HEADERS xaxicdma_bd.h)
collect (PROJECT_LIB_HEADERS xaxicdma.h)
collect (PROJECT_LIB_SOURCES xaxicdma.c)
collect (PROJECT_LIB_SOURCES xaxicdma_bd.c)
collect (PROJECT_LIB_SOURCES xaxicdma_intr.c)
collect (PROJECT_LIB_HEADERS xaxicdma_porting_guide.h)
collect (PROJECT_LIB_HEADERS xaxicdma_hw.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(axicdma STATIC ${_sources})
set_target_properties(axicdma PROPERTIES LINKER_LANGUAGE C)
install(TARGETS axicdma LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
