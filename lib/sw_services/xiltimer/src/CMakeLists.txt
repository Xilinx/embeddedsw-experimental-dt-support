cmake_minimum_required(VERSION 2.8.9)
project(xiltimer)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
include(${CMAKE_CURRENT_SOURCE_DIR}/XiltimerExample.cmake NO_POLICY_SCOPE)
include(${CMAKE_CURRENT_SOURCE_DIR}/../data/xiltimer.cmake NO_POLICY_SCOPE)

collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/core)

collect (PROJECT_LIB_SOURCES xiltimer.c)
collect (PROJECT_LIB_HEADERS xiltimer.h)
collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(xiltimer STATIC ${_sources})
set_target_properties(xiltimer PROPERTIES LINKER_LANGUAGE C)
install(TARGETS xiltimer LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
