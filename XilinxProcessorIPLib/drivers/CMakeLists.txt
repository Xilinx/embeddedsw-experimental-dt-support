cmake_minimum_required(VERSION 3.15)
project(libxil)

if (${NON_YOCTO})
find_package(common)
include_directories(${CMAKE_BINARY_DIR}/include)
set(DRVLIST_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
find_package(DRVLIST)
FOREACH(DRIVER ${DRIVER_LIST})
    STRING(REGEX REPLACE "-" "_" DRIVER ${DRIVER})
    add_subdirectory(${DRIVER}/src)
    list(APPEND OBJS ${CMAKE_BINARY_DIR}/libsrc/${DRIVER}/src/CMakeFiles/${DRIVER}.dir/*.obj)
    subdirlist(SUBDIRS ${CMAKE_SOURCE_DIR}/${DRIVER}/src)
    foreach(subdir ${SUBDIRS})
	    list(APPEND OBJS ${CMAKE_BINARY_DIR}/libsrc/${DRIVER}/src/CMakeFiles/${DRIVER}.dir/${subdir}/*.obj)
    endforeach()
ENDFOREACH()

set(XIL_LIB ${CMAKE_BINARY_DIR}/libxil.a)
add_custom_target(combined ALL
	COMMAND ${CMAKE_AR} -qcs ${XIL_LIB} ${OBJS}
	COMMAND ${CMAKE_COMMAND} -E copy ${XIL_LIB} ${CMAKE_LIBRARY_PATH}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	DEPENDS ${DRIVER_LIST})
add_library(xil STATIC IMPORTED GLOBAL)
add_dependencies(xil combined)
set_target_properties(xil PROPERTIES IMPORTED_LOCATION ${XIL_LIB} LINKER_LANGUAGE C)
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INCLUDE_PATH}/..)
else()
include_directories(${CMAKE_BINARY_DIR}/include)
file(COPY xparameters.h DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(libxil INTERFACE)
endif()
