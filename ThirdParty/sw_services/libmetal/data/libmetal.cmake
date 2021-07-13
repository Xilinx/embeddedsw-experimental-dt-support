# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.5)
message("current PROJECT_MACHINE is ${PROJECT_MACHINE} ")
message("current PROJECT_PROCESSOR is ${PROJECT_PROCESSOR} ")
if (CMAKE_SYSTEM_PROCESSOR MATCHES "r5")
  set (PROJECT_PROCESSOR "arm" CACHE STRING "")
  set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "")
  string(REPLACE "cortexr5" "arm" PROJECT_PROCESSOR ${PROJECT_PROCESSOR})
  string(REPLACE "cortexr5" "arm" CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})

endif()

set (XPAR_SCUGIC_0_DIST_BASEADDR XPS_SCU_PERIPH_BASE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../data/xlibmetal_config.h.in ${CMAKE_BINARY_DIR}/include/xlibmetal_config.h)
