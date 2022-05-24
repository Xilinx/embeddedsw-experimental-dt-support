# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
if (${XILSECURE_Mode} OR "${CMAKE_MACHINE}" STREQUAL "ZynqMP")
	collect (PROJECT_LIB_HEADERS xsecure_utils.h)
	collect (PROJECT_LIB_SOURCES xsecure_rsa.c)
	collect (PROJECT_LIB_HEADERS xsecure_rsa.h)
	collect (PROJECT_LIB_SOURCES xsecure_utils.c)
endif()
