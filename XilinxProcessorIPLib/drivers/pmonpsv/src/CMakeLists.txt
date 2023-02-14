# Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(pmonpsv)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)
collect (PROJECT_LIB_SOURCES xpmonpsv.c)
collect (PROJECT_LIB_HEADERS xpmonpsv.h)
collect (PROJECT_LIB_SOURCES xpmonpsv_g.c)
collect (PROJECT_LIB_HEADERS xpmonpsv_hw.h)
collect (PROJECT_LIB_SOURCES xpmonpsv_sint.c)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(pmonpsv STATIC ${_sources})
set_target_properties(pmonpsv PROPERTIES LINKER_LANGUAGE C)
