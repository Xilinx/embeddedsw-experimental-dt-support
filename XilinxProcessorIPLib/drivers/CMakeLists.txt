cmake_minimum_required(VERSION 2.8.9)
project(libxil)

if (${OS_ESW})
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/common.cmake)
include_directories(${CMAKE_BINARY_DIR}/include)
set(DRVLIST_DIR "${CMAKE_SOURCE_DIR}")
find_package(DRVLIST)
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_list (_sources PROJECT_LIB_SOURCES)
FOREACH(DRIVER ${DRIVER_LIST})
    STRING(REGEX REPLACE "-" "_" DRIVER ${DRIVER})
    if(EXISTS "${CMAKE_SOURCE_DIR}/${DRIVER}/data/${DRIVER}.yaml")
        gen_drvconfig(${DRIVER})
    endif()
    add_subdirectory(${DRIVER}/src)
    collector_list (_sources PROJECT_LIB_SOURCES)
    foreach (src ${_sources})
        list (APPEND sources ${DRIVER}/src/${src})
    endforeach()
ENDFOREACH()
gen_xparams()
file(COPY xparameters.h DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(xil STATIC ${sources})
set_target_properties(xil PROPERTIES LINKER_LANGUAGE C)
else()
include_directories(${CMAKE_BINARY_DIR}/include)
file(COPY xparameters.h DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(libxil INTERFACE)
endif()
