cmake_minimum_required (VERSION 2.6)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../cmake/common.cmake NO_POLICY_SCOPE)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../data/libmetal.cmake NO_POLICY_SCOPE)

list (APPEND CMAKE_MODULE_PATH
      "${CMAKE_SOURCE_DIR}/cmake"
      "${CMAKE_SOURCE_DIR}/cmake/modules"
      "${CMAKE_SOURCE_DIR}/cmake/platforms")

set(METAL_INCLUDE_DIRS
    "${CMAKE_BINARY_DIR}/include"
	"${CMAKE_BINARY_DIR}/lib/include"
)
include (syscheck)

project (metal C)

include (CheckIncludeFiles)
include (CheckCSourceCompiles)
include (collect)
include (options)
include (depends)

foreach(_inc_path ${CMAKE_INCLUDE_PATH})
  collect (PROJECT_INC_DIRS "${_inc_path}")
endforeach()

enable_testing ()

add_subdirectory (lib)

if (WITH_TESTS)
  add_subdirectory (test)
endif (WITH_TESTS)

if (WITH_DOC)
  add_subdirectory (doc)
endif (WITH_DOC)

if (WITH_EXAMPLES)
  add_subdirectory (examples)
endif (WITH_EXAMPLES)

# vim: expandtab:ts=2:sw=2:smartindent
