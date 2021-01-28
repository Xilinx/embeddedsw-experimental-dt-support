cmake_minimum_required(VERSION 2.8.9)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)
project(xilsecure)

include(${CMAKE_CURRENT_SOURCE_DIR}/../data/xilsecure.cmake NO_POLICY_SCOPE)
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include)

enable_language(C ASM)
add_subdirectory(common)
if("${CMAKE_MACHINE}" STREQUAL "Versal")
add_subdirectory(versal)
else()
add_subdirectory(zynqmp)
endif()

foreach(lang ASM C CXX)
  # GNU ar always updates the index: no need for CMAKE_RANLIB
  SET(CMAKE_${lang}_CREATE_STATIC_LIBRARY
      "<CMAKE_AR> rc <TARGET> <LINK_FLAGS> <OBJECTS>")
endforeach()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)
#execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/versal/libxilsecure_pmc.a ${CMAKE_BINARY_DIR}/libxilsecure.a)
add_library(xilsecure STATIC ${_sources})
set_target_properties(xilsecure PROPERTIES LINKER_LANGUAGE C)
install(TARGETS xilsecure LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
