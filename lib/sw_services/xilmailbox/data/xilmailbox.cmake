cmake_minimum_required(VERSION 3.5)

list(LENGTH IPIPSU_NUM_DRIVER_INSTANCES _len)

if(${_len} EQUAL 0)
    message(FATAL_ERROR "This application requires an IPI instance in the hardware.")
else()
    list(GET IPIPSU0_PROP_LIST 0 XMAILBOX_IPI_BASEADDRESS)
    list(GET IPIPSU0_PROP_LIST 1 XMAILBOX_IPI_CHANNEL_ID)
endif()

configure_file(${CMAKE_SOURCE_DIR}/../data/xilmailbox_hwconfig.h.in ${CMAKE_BINARY_DIR}/include/xilmailbox_hwconfig.h)
