# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(lib)

foreach(lib ${LIB_LIST})
	add_subdirectory("${CMAKE_LIBRARY_PATH}/../libsrc/${lib}/src" ${CMAKE_CURRENT_BINARY_DIR}/${lib})
endforeach()

