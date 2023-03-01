# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
add_subdirectory(common)
if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR 
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze") OR
   (${XILSECURE_Mode}))
   add_subdirectory(server)
else()
   if (${NON_YOCTO})
       get_property(src_files TARGET xilmailbox PROPERTY SOURCES)
       if (NOT src_files)
	   message(FATAL_ERROR "This library requires xilmailbox library in the Board Support Package.")
       endif()
   endif()
   add_subdirectory(client)
endif()
