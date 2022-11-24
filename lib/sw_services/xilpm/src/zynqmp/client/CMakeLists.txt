# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14.7)

add_subdirectory(common)
if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53-32")
	OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64"))
add_subdirectory(apu)
else()
add_subdirectory(rpu)
endif()
