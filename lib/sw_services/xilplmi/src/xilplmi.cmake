# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT

option(XILPLMI_plm_uart_dbg_en "Enables (if enabled in hardware design too) or Disables Debug prints from UART (log to memory done irrespectively" ON)
if(NOT XILPLMI_plm_uart_dbg_en)
	set(PLM_PRINT_NO_UART " ")
endif()
set(XILPLMI_plm_dbg_lvl "level1" CACHE STRING "Selects the debug logs level")
set_property(CACHE XILPLMI_plm_dbg_lvl PROPERTY STRINGS "level0" "level1" "level2" "level3")
if("${XILPLMI_plm_dbg_lvl}" STREQUAL "level0")
	set(PLM_PRINT " ")
elseif("${XILPLMI_plm_dbg_lvl}" STREQUAL "level1")
	set(PLM_DEBUG " ")
elseif("${XILPLMI_plm_dbg_lvl}" STREQUAL "level2")
	set(PLM_DEBUG_INFO " ")
elseif("${XILPLMI_plm_dbg_lvl}" STREQUAL "level3")
	set(PLM_DEBUG_DETAILED	" ")
endif()

option(XILPLMI_plm_perf_en "Enables or Disables Boot time measurement" ON)
if (XILPLMI_plm_perf_en)
	set(PLM_PRINT_PERF " ")
endif()

option(XILPLMI_plm_qspi_en "Enables (if enabled in hardware design too) or Disables QSPI boot mode" ON)
if (NOT XILPLMI_plm_qspi_en)
	set(PLM_QSPI_EXCLUDE " ")
endif()

option(XILPLMI_plm_sd_en "Enables (if enabled in hardware design too) or Disables SD boot mode" ON)
if (NOT XILPLMI_plm_sd_en)
	set(PLM_SD_EXCLUDE " ")
endif()

option(XILPLMI_plm_ospi_en "Enables (if enabled in hardware design too) or Disables OSPI boot mode" ON)
if (NOT XILPLMI_plm_ospi_en)
	set(PLM_OSPI_EXCLUDE " ")
endif()

option(XILPLMI_plm_sem_en "Enables (if enabled in hardware design too) or Disables SEM feature" ON)
if (NOT XILPLMI_plm_sem_en)
	set(PLM_SEM_EXCLUDE " ")
endif()

option(XILPLMI_plm_secure_en "Enables or Disbales Secure features" ON)
if (NOT XILPLMI_plm_secure_en)
	set(PLM_SECURE_EXCLUDE " ")
endif()

option(XILPLMI_plm_usb_en "Enables (if enabled in hardware design too) or disables USB boot mode" OFF)
if (NOT XILPLMI_plm_usb_en)
	set(PLM_USB_EXCLUDE " ")
endif()

option(XILPLMI_plm_nvm_en "Enables or Disables NVM handlers" OFF)
if (NOT XILPLMI_plm_nvm_en)
	set(PLM_NVM_EXCLUDE " ")
endif()

option(XILPLMI_plm_puf_en "Enables or Disables PUF handlers" OFF)
if (NOT XILPLMI_plm_puf_en)
	set(PLM_PUF_EXCLUDE " ")
endif()

option(XILPLMI_plm_stl_en "Enables or Disables STL" OFF)
if (XILPLMI_plm_stl_en)
	set(PLM_ENABLE_STL " ")
endif()

option(XILPLMI_ssit_plm_to_plm_comm_en "Enables or Disables SSIT PLM to PLM communication (valid only for Versal)" ON)
if (XILPLMI_ssit_plm_to_plm_comm_en)
	set(PLM_ENABLE_PLM_TO_PLM_COMM " ")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/xilplmi_config.h.in ${CMAKE_BINARY_DIR}/include/xilplmi_config.h)
