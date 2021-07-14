# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
collect (PROJECT_LIB_SOURCES sys_arch_raw.c)
collect (PROJECT_LIB_SOURCES sys_arch.c)
add_subdirectory(netif)
