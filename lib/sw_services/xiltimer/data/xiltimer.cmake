cmake_minimum_required(VERSION 3.5)

list(APPEND TOTAL_TIMER_INSTANCES ${TMRCTR_NUM_DRIVER_INSTANCES})
list(APPEND TOTAL_TIMER_INSTANCES ${TTCPS_NUM_DRIVER_INSTANCES})
list(APPEND TOTAL_TIMER_INSTANCES ${SCUTIMER_NUM_DRIVER_INSTANCES})

set(XILTIMER_sleep_timer "Default;${TOTAL_TIMER_INSTANCES}" CACHE STRING "This parameter is used to select specific timer for sleep functionality")
SET_PROPERTY(CACHE XILTIMER_sleep_timer PROPERTY STRINGS "Default;${TOTAL_TIMER_INSTANCES}")

set(XILTIMER_tick_timer "None;${TOTAL_TIMER_INSTANCES}" CACHE STRING "This parameter is used to select specific timer for tick functionality")
SET_PROPERTY(CACHE XILTIMER_tick_timer PROPERTY STRINGS "None;${TOTAL_TIMER_INSTANCES}")

list(LENGTH TMRCTR_NUM_DRIVER_INSTANCES CONFIG_AXI_TIMER)
list(LENGTH TTCPS_NUM_DRIVER_INSTANCES CONFIG_TTCPS)
list(LENGTH SCUTIMER_NUM_DRIVER_INSTANCES CONFIG_SCUTIMER)
list(LENGTH XILTIMER_sleep_timer _len)

if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze")
    set(XTIMER_NO_TICK_TIMER 1)
    set(XTIMER_IS_DEFAULT_TIMER " ")
    set(XILTIMER_sleep_timer Default)
elseif (("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
        OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53")
        OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa53-32"))
        if ((${_len} GREATER 2)
            OR (${_len} GREATER 1))
            list(GET XILTIMER_sleep_timer 0 XILTIMER_sleep_timer)
            list(GET XILTIMER_tick_timer 1 XILTIMER_tick_timer)
            set(XTIMER_IS_DEFAULT_TIMER "")
            if ("${XILTIMER_sleep_timer}" STREQUAL "Default")
                set(XTIMER_IS_DEFAULT_TIMER 1)
            endif()
            if (${_len} GREATER 1)
                if ("${XILTIMER_tick_timer}" STREQUAL "")
                    set(XTIMER_NO_TICK_TIMER 1)
                endif()
            endif()
        elseif (${_len} EQUAL 1)
            list(GET XILTIMER_sleep_timer 0 XILTIMER_sleep_timer)
	    list(LENGTH XILTIMER_tick_timer _ticktmrlen)
	    if (${_ticktmrlen} EQUAL 1)
                list(GET XILTIMER_tick_timer 0 XILTIMER_tick_timer)
	    else()
                list(GET XILTIMER_tick_timer 1 XILTIMER_tick_timer)
	    endif()
            set(XTIMER_IS_DEFAULT_TIMER "")
            if ("${XILTIMER_sleep_timer}" STREQUAL "Default")
                set(XTIMER_IS_DEFAULT_TIMER 1)
            endif()
            if ("${XILTIMER_tick_timer}" STREQUAL "None")
                set(XTIMER_NO_TICK_TIMER 1)
            endif()
        else(${_len} EQUAL 0)
            set(XTIMER_NO_TICK_TIMER 1)
            set(XTIMER_IS_DEFAULT_TIMER " ")
        endif()
elseif (${_len} EQUAL 1)
    if (XILTIMER_sleep_timer IN_LIST TOTAL_TIMER_INSTANCES)
        if ("${XILTIMER_sleep_timer}" STREQUAL "${XILTIMER_tick_timer}")
            message(FATAL_ERROR "For sleep and tick functionality select different timers")
        endif()
    else()
        list(GET XILTIMER_tick_timer 0 XILTIMER_tick_timer)
        set(XTIMER_IS_DEFAULT_TIMER " ")
        if ("${XILTIMER_tick_timer}" STREQUAL "None")
            set(XTIMER_NO_TICK_TIMER 1)
        endif()
    endif()
elseif (${_len} GREATER 2)
    list(GET XILTIMER_sleep_timer 2 XILTIMER_sleep_timer)
    list(GET XILTIMER_tick_timer 3 XILTIMER_tick_timer)
elseif (${_len} GREATER 1)
    if ("${TOTAL_TIMER_INSTANCES}" STREQUAL "")
       list(GET XILTIMER_sleep_timer 1 XILTIMER_sleep_timer)
       list(GET XILTIMER_tick_timer 1 XILTIMER_tick_timer)
    else()
       list(GET XILTIMER_sleep_timer 1 XILTIMER_sleep_timer)
       list(GET XILTIMER_tick_timer 2 XILTIMER_tick_timer)
    endif()
    set(XTIMER_IS_DEFAULT_TIMER " ")
    if ("${XILTIMER_tick_timer}" STREQUAL "")
       set(XTIMER_NO_TICK_TIMER 1)
    endif()
else(${_len} EQUAL 0)
    set(XTIMER_NO_TICK_TIMER 1)
    set(XTIMER_IS_DEFAULT_TIMER " ")
endif()

if (${CONFIG_AXI_TIMER})
    if(XILTIMER_sleep_timer IN_LIST TMRCTR_NUM_DRIVER_INSTANCES)
        set(XSLEEPTIMER_IS_AXITIMER " ")
        set(index 0)
        LIST_INDEX(${index} ${XILTIMER_sleep_timer} "${TMRCTR_NUM_DRIVER_INSTANCES}")
        list(GET TOTAL_TMRCTR_PROP_LIST ${index} reg)
        set(XSLEEPTIMER_BASEADDRESS ${${reg}})
        set(AXI_TIMER 1)
    endif()
    if(XILTIMER_tick_timer IN_LIST TMRCTR_NUM_DRIVER_INSTANCES)
        set(XTICKTIMER_IS_AXITIMER " ")
        set(index 0)
        LIST_INDEX(${index} ${XILTIMER_tick_timer} "${TMRCTR_NUM_DRIVER_INSTANCES}")
        list(GET TOTAL_TMRCTR_PROP_LIST ${index} reg)
        set(XTICKTIMER_BASEADDRESS ${${reg}})
        set(AXI_TIMER 1)
    endif()
endif()

if(${CONFIG_TTCPS})
    if(XILTIMER_sleep_timer IN_LIST TTCPS_NUM_DRIVER_INSTANCES)
        set(XSLEEPTIMER_IS_TTCPS " ")
        set(index 0)
        LIST_INDEX(${index} ${XILTIMER_sleep_timer} "${TTCPS_NUM_DRIVER_INSTANCES}")
        list(GET TOTAL_TTCPS_PROP_LIST ${index} reg)
        set(XSLEEPTIMER_BASEADDRESS ${${reg}})
        set(TTCPS 1)
    endif()
    if(XILTIMER_tick_timer IN_LIST TTCPS_NUM_DRIVER_INSTANCES)
        set(XTICKTIMER_IS_TTCPS " ")
        set(index 0)
        LIST_INDEX(${index} ${XILTIMER_tick_timer} "${TTCPS_NUM_DRIVER_INSTANCES}")
        list(GET TOTAL_TTCPS_PROP_LIST ${index} reg)
        set(XTICKTIMER_BASEADDRESS ${${reg}})
        set(TTCPS 1)
    endif()
endif()

if(${CONFIG_SCUTIMER})
    if(XILTIMER_sleep_timer IN_LIST SCUTIMER_NUM_DRIVER_INSTANCES)
        set(XSLEEPTIMER_IS_SCUTIMER " ")
        set(index 0)
        LIST_INDEX(${index} ${XILTIMER_sleep_timer} "${SCUTIMER_NUM_DRIVER_INSTANCES}")
        list(GET TOTAL_SCUTIMER_PROP_LIST ${index} reg)
        set(XSLEEPTIMER_BASEADDRESS ${${reg}})
        set(SCUTIMER 1)
    endif()
    if(XILTIMER_tick_timer IN_LIST SCUTIMER_NUM_DRIVER_INSTANCES)
        set(XTICKTIMER_IS_SCUTIMER " ")
        set(index 0)
        LIST_INDEX(${index} ${XILTIMER_tick_timer} "${SCUTIMER_NUM_DRIVER_INSTANCES}")
        list(GET TOTAL_SCUTIMER_PROP_LIST ${index} reg)
        set(XTICKTIMER_BASEADDRESS ${${reg}})
        set(SCUTIMER 1)
    endif()
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../data/xtimer_config.h.in ${CMAKE_BINARY_DIR}/include/xtimer_config.h)
