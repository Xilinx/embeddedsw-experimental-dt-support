# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
project(xilpm)

include(${CMAKE_CURRENT_SOURCE_DIR}/../data/xilpm.cmake NO_POLICY_SCOPE)
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

if("${CMAKE_MACHINE}" STREQUAL "Versal")
add_subdirectory(versal)
else()
add_subdirectory(zynqmp/client)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(xilpm STATIC ${_sources})
set_target_properties(xilpm PROPERTIES LINKER_LANGUAGE C)
install(TARGETS xilpm LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
