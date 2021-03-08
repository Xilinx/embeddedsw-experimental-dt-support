cmake_minimum_required(VERSION 3.5)
message("current PROJECT_MACHINE is ${PROJECT_MACHINE} ")
message("current PROJECT_PROCESSOR is ${PROJECT_PROCESSOR} ")
if (CMAKE_SYSTEM_PROCESSOR MATCHES "r5")
  set (PROJECT_PROCESSOR "arm" CACHE STRING "")
  set (CMAKE_SYSTEM_PROCESSOR "arm" CACHE STRING "")
  string(REPLACE "cortexr5" "arm" PROJECT_PROCESSOR ${PROJECT_PROCESSOR})
  string(REPLACE "cortexr5" "arm" CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})

endif()


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../data/xopenamp_config.h.in ${CMAKE_BINARY_DIR}/include/xopenamp_config.h)

