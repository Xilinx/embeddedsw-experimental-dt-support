# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
add_subdirectory(all)
if("${CMAKE_MACHINE}" STREQUAL "Versal")
	add_subdirectory(versal_common)
endif()
