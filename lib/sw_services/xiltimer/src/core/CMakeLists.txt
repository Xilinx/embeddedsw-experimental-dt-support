# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
if (("${XILTIMER_sleep_timer}" STREQUAL "Default") OR
    ("${XILTIMER_tick_timer}" STREQUAL "None"))
    add_subdirectory(default_timer)
endif()

if (${AXI_TIMER})
    add_subdirectory(axi_timer)
endif()

if (${TTCPS})
    add_subdirectory(ttcps)
endif()

if (${SCUTIMER})
    add_subdirectory(scutimer)
endif()
