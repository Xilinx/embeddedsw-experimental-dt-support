# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
project(xilsem)

find_package(common)
include(${CMAKE_CURRENT_SOURCE_DIR}/xilsem.cmake NO_POLICY_SCOPE)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

collect (PROJECT_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/server/xilsem.h)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(xilsem INTERFACE)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/server/libxilsem.a ${CMAKE_BINARY_DIR}/libxilsem.a)
#execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libxilsem.a) ${CMAKE_LIBRARY_PATH}/libxilsem.a)
#install(FILES ${CMAKE_BINARY_DIR}/libxilsem.a LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
#install(TARGETS xilsem LIBRARY DESTINATION ${CMAKE_LIBRARY_PATH} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})
file(COPY ${CMAKE_BINARY_DIR}/libxilsem.a DESTINATION ${CMAKE_LIBRARY_PATH} FOLLOW_SYMLINK_CHAIN)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..) 
