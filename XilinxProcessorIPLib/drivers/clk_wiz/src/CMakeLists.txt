# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
project(clk_wiz)

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xclk_wiz.c)
collect (PROJECT_LIB_HEADERS xclk_wiz.h)
collect (PROJECT_LIB_SOURCES xclk_wiz_g.c)
collect (PROJECT_LIB_HEADERS xclk_wiz_hw.h)
collect (PROJECT_LIB_SOURCES xclk_wiz_intr.c)
collect (PROJECT_LIB_SOURCES xclk_wiz_sinit.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(clk_wiz STATIC ${_sources})
set_target_properties(clk_wiz PROPERTIES LINKER_LANGUAGE C)
install(TARGETS clk_wiz LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
