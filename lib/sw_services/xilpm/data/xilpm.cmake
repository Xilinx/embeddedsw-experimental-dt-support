# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.5)

set(XPAR_XILPM_ENABLED " ")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../data/xpm_config.h.in ${CMAKE_BINARY_DIR}/include/xpm_config.h)
