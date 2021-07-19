# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
if("${CMAKE_MACHINE}" STREQUAL "Versal")
add_subdirectory(versal)
else()
add_subdirectory(ZynqMP)
endif()
