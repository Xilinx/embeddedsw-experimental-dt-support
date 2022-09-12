# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
set(BSPCONFIG_description CACHE STRING "Below are software config parameters for the standalone library")
SET_PROPERTY(CACHE BSPCONFIG_description PROPERTY STRINGS "Below are the software config for the standalone bsp")
if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
        OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
        OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53-32")
        OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64"))
    option(BSPCONFIG_hypervisor_guest "Enable hypervisor guest for EL1 Nonsecure" OFF)
    set(XPAR_PS_INCLUDE "#include \"xparameters_ps.h\"")
    if(BSPCONFIG_hypervisor_guest)
        set(EL1_NONSECURE " ")
        set(HYP_GUEST " ")
    else()
        set(EL3 " ")
    endif()
endif()

if("${CMAKE_MACHINE}" STREQUAL "Versal")
    set(versal " ")
elseif("${CMAKE_MACHINE}" STREQUAL "ZynqMP")
    set(PLATFORM_ZYNQMP " ")
elseif("${CMAKE_MACHINE}" STREQUAL "Zynq")
    set(PLATFORM_ZYNQ " ")
endif()

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5"))
    set(XPAR_PS_INCLUDE "#include \"xparameters_ps.h\"")
    set(EL3 " ")
endif()

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
    set(PSU_PMU 1)
elseif(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze"))
    set(VERSAL_PLM " ")
elseif(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze"))
    set(PLATFORM_MB " ")
endif()
	
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/bspconfig.h.in ${CMAKE_BINARY_DIR}/include/bspconfig.h)
