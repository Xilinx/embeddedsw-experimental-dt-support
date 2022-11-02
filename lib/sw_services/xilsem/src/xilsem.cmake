# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.3)

option(XILSEM_sem_cfrscan_en "Defines if XilSEM CRAM scan is enabled(1) or disabled (0)." OFF)
option(XILSEM_sem_npiscan_en "Defines if XilSEM NPI register scan is enabled(1) or disabled (0)." OFF)

if(${XILSEM_sem_cfrscan_en})
    set(XSEM_CFRSCAN_EN " ")
endif()
if(${XILSEM_sem_npiscan_en})
    set(XSEM_NPISCAN_EN " ")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilsem_config.h.in ${CMAKE_BINARY_DIR}/include/xilsem_config.h)
