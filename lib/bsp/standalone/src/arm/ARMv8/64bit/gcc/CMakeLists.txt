# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
collect (PROJECT_LIB_SOURCES xil-crt0.S)
collect (PROJECT_LIB_SOURCES boot.S)
collect (PROJECT_LIB_SOURCES initialise_monitor_handles.c)
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "FreeRTOS")
collect (PROJECT_LIB_SOURCES  asm_vectors.S)
endif()
