# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

option(XILSECURE_secure_environment "Enables trusted execution environment" OFF)
if(XILSECURE_secure_environment)
	set(XSECURE_TRUSTED_ENVIRONMENT " ")
endif()
option(XILSECURE_tpm_support "Enables decryption of bitstream to memory and then writes it to PCAP, allows calculation of sha on decrypted bitstream in chunks valid only for ZynqMP FSBL BSP" OFF)
if(XILSECURE_tpm_support)
	set(XSECURE_TPM_ENABLE " ")
endif()
option(XILSECURE_Mode "Enables Client or Server mode for xilsecure versal" OFF)

configure_file(${CMAKE_SOURCE_DIR}/xsecure_config.h.in ${CMAKE_BINARY_DIR}/include/xsecure_config.h)
