# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
if (NOT(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")) OR NOT(${XILSECURE_Mode}))
	collect (PROJECT_LIB_HEADERS xsecure_defs.h)
endif()
