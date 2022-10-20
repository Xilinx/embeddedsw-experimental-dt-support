# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.3)

SET(XILSKEY_device_series XSK_FPGA_SERIES_ZYNQ CACHE STRING "Device series FPGA SERIES ZYNQ:XSK_FPGA_SERIES_ZYNQ FPGA SERIES ULTRA:XSK_FPGA_SERIES_ULTRA FPGA SERIES ULTRA PLUS:XSK_FPGA_SERIES_ULTRA_PLUS")
SET_PROPERTY(CACHE XILSKEY_device_series PROPERTY STRINGS XSK_FPGA_SERIES_ZYNQ XSK_FPGA_SERIES_ULTRA XSK_FPGA_SERIES_ULTRA_PLUS)
SET(XILSKEY_device_id 0 CACHE STRING "IDCODE")
SET(XILSKEY_device_irlen 0 CACHE STRING "IR length")
SET(XILSKEY_device_numslr 1 CACHE STRING "Number of SLRs")

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
        OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
        OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53-32")
        OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64"))
	set(XPAR_XSK_ARM_PLATFORM " ")
endif()

# How to differentiate b/w pmu microblaze and soft microblaze?? 
# how to find soc family kintex ultscale or ultscale+?? (Does MACHINE variable work??)
if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel")
	set(XPAR_XSK_MICROBLAZE_PLATFORM " ")
endif()

if (${XILSKEY_device_id})
	if((NOT ${XILSKEY_device_id} MATCHES 0x0ba00477) AND
	   (NOT ${XILSKEY_device_id} MATCHES 0x03822093) AND
	   (NOT ${XILSKEY_device_id} MATCHES 0x03842093) AND
	   (NOT ${XILSKEY_device_id} MATCHES 0x04A62093) AND
	   (NOT ${XILSKEY_device_id} MATCHES 0x04B51093))
		set(XSK_USER_DEVICE_SERIES ${XILSKEY_device_series})
		set(XSK_USER_DEVICE_ID ${XILSKEY_device_id})
		set(XSK_USER_DEVICE_IRLEN ${XILSKEY_device_irlen})
		set(XSK_USER_DEVICE_NUMSLR ${XILSKEY_device_numslr})
	else()
		message("ERROR: Device IDCODE already exist by Default.")
		# Using cmakedefine we can't assign value of zero and we can't use cmakedefine01 here
		# need to fix source code?? 
		set(XSK_USER_DEVICE_SERIES 0)
		set(XSK_USER_DEVICE_ID 0)
		set(XSK_USER_DEVICE_IRLEN 0)
		set(XSK_USER_DEVICE_NUMSLR 0)
	endif()
else()	
	set(XSK_USER_DEVICE_SERIES ${XILSKEY_device_series})
	set(XSK_USER_DEVICE_ID ${XILSKEY_device_id})
	set(XSK_USER_DEVICE_IRLEN ${XILSKEY_device_irlen})
	set(XSK_USER_DEVICE_NUMSLR ${XILSKEY_device_numslr})
endif()

configure_file(${CMAKE_SOURCE_DIR}/xilskey_config.h.in ${CMAKE_BINARY_DIR}/include/xilskey_config.h)
