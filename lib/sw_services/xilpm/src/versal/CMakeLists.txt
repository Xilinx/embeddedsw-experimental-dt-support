# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)

add_subdirectory(common)
if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
add_subdirectory(server)
else()
add_subdirectory(client)
endif()
