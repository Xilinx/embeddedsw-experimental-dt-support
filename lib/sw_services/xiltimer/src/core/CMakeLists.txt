if ("${XILTIMER_sleep_timer}" STREQUAL "Default")
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
