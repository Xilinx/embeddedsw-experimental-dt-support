# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
project(xilfpga)

include(${CMAKE_CURRENT_SOURCE_DIR}/../data/xilfpga.cmake NO_POLICY_SCOPE)
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

if("${CMAKE_MACHINE}" STREQUAL "Versal")
   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/interface/versal/)
else()
   add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/interface/zynqmp/)
endif()
collect (PROJECT_LIB_SOURCES xilfpga.c)
collect (PROJECT_LIB_HEADERS xilfpga.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(xilfpga STATIC ${_sources})
set_target_properties(xilfpga PROPERTIES LINKER_LANGUAGE C)
install(TARGETS xilfpga LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
