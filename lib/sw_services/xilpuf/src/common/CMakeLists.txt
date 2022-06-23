if (NOT(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72") OR ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")) OR NOT(${XILPUF_Mode}))
	collect (PROJECT_LIB_HEADERS xpuf_defs.h)
endif()
