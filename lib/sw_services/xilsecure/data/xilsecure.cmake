# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

option(XILSECURE_secure_environment "Enables trusted execution environment" OFF)
if(XILSECURE_secure_environment)
	set(XSECURE_TRUSTED_ENVIRONMENT " ")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../data/xsecure_config.h.in ${CMAKE_BINARY_DIR}/include/xsecure_config.h)
