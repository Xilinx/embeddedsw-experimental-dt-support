# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
include(${CMAKE_CURRENT_SOURCE_DIR}/Lwip211Example.cmake NO_POLICY_SCOPE)
project(lwip211)

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

set(LWIP_DIR ${CMAKE_SOURCE_DIR}/lwip-2.1.1) 
set(LWIP_INCLUDE_DIRS
    "${LWIP_DIR}/src"
    "${LWIP_DIR}/src/include"
    "${CMAKE_SOURCE_DIR}/contrib/ports/xilinx/include"
    "${CMAKE_SOURCE_DIR}/contrib/ports/xilinx/include/arch"
    "${CMAKE_SOURCE_DIR}/contrib/ports/xilinx/include/netif"
)
include(${CMAKE_SOURCE_DIR}/Filelists.cmake NO_POLICY_SCOPE)
collect (PROJECT_LIB_SOURCES ${lwipnoapps_SRCS})

list(LENGTH EMACLITE_NUM_DRIVER_INSTANCES CONFIG_EMACLITE)
list(LENGTH AXIETHERNET_NUM_DRIVER_INSTANCES CONFIG_AXIETHERNET)
list(LENGTH EMACPS_NUM_DRIVER_INSTANCES CONFIG_EMACPS)
foreach(x IN LISTS TOTAL_AXIETHERNET_PROP_LIST)
    set(y ${${x}})
    list(GET y -1 connected_ip)
    list(GET y -2 axieth_rxcsum)
    list(GET y -3 axieth_txcsum)
    if (${connected_ip} EQUAL 0x1)
       set(CONFIG_AXIFIFO 1)
    elseif (${connected_ip} EQUAL 0x2)
       set(CONFIG_AXIDMA 1)
    elseif (${connected_ip} EQUAL 0x3)
       set(CONFIG_AXIMCDMA 1)
    endif()
endforeach()
add_subdirectory(contrib/ports/xilinx/)
collect (PROJECT_LIB_SOURCES xtopology_g.c)

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

list(APPEND TOTAL_MAC_INSTANCES ${EMACPS_NUM_DRIVER_INSTANCES})
list(APPEND TOTAL_MAC_INSTANCES ${AXIETHERNET_NUM_DRIVER_INSTANCES})
list(APPEND TOTAL_MAC_INSTANCES ${EMACLITE_NUM_DRIVER_INSTANCES})
SET(MAC_INSTANCES "${TOTAL_MAC_INSTANCES}" CACHE STRING "MAC Instances")
SET_PROPERTY(CACHE MAC_INSTANCES PROPERTY STRINGS "${TOTAL_MAC_INSTANCES}")

include(${CMAKE_SOURCE_DIR}/../data/lwip211.cmake NO_POLICY_SCOPE)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
file(COPY ${LWIP_INCLUDE_DIRS} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(lwip211 STATIC ${_sources})
target_include_directories(lwip211 PUBLIC ${LWIP_INCLUDE_DIRS})
set_target_properties(lwip211 PROPERTIES LINKER_LANGUAGE C)
install(TARGETS lwip211 LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
