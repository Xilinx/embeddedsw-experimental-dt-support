cmake_minimum_required(VERSION 2.8.9)
message(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake)
project(xilstandalone)

set(Mem_DIR "${CMAKE_SOURCE_DIR}")

find_package(Mem)
list(APPEND defs "#ifndef XMEM_CONFIG_H_\n")
list(APPEND defs "#define XMEM_CONFIG_H_\n\n")

list(LENGTH MEM_DEF_NAMES len)
math(EXPR mem_len "${len} - 1")
foreach(val RANGE ${mem_len})
   list(GET MEM_DEF_NAMES ${val} MEM)
   list(GET MEM_RANGES ${val} VAL)
   list(APPEND defs "#define ${MEM} ${VAL}\n")
endforeach()
list(APPEND defs "\n#endif")
file(WRITE ${CMAKE_SOURCE_DIR}/common/xmem_config.h ${defs})
include(${CMAKE_CURRENT_SOURCE_DIR}/../data/xilstandalone.cmake NO_POLICY_SCOPE)
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")

#Bring the headers into the project
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

enable_language(C ASM)
add_subdirectory(common)
if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
add_subdirectory(microblaze)
else()
add_subdirectory(arm)
endif()

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)
file(COPY ${_headers} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include)

#Compile it as a static library
add_library(xilstandalone STATIC ${_sources})
set_target_properties(xilstandalone PROPERTIES LINKER_LANGUAGE C)
install(TARGETS xilstandalone LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
