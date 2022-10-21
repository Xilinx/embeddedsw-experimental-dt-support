# Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
collect (PROJECT_LIB_HEADERS Ecdsa.h)
collect (PROJECT_LIB_SOURCES xsecure_aes.c)
collect (PROJECT_LIB_HEADERS xsecure_aes.h)
collect (PROJECT_LIB_HEADERS xsecure_aes_core_hw.h)
collect (PROJECT_LIB_HEADERS xsecure_ecdsa_rsa_hw.h)
collect (PROJECT_LIB_SOURCES xsecure_elliptic.c)
collect (PROJECT_LIB_HEADERS xsecure_elliptic.h)
collect (PROJECT_LIB_SOURCES xsecure_ellipticcrvs.c)
collect (PROJECT_LIB_HEADERS xsecure_ellipticcrvs.h)
collect (PROJECT_LIB_HEADERS xsecure_error.h)
collect (PROJECT_LIB_SOURCES xsecure_cryptochk.c)
collect (PROJECT_LIB_HEADERS xsecure_cryptochk.h)
if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
	collect (PROJECT_LIB_SOURCES xsecure_aes_ipihandler.c)
	collect (PROJECT_LIB_HEADERS xsecure_aes_ipihandler.h)
	collect (PROJECT_LIB_SOURCES xsecure_rsa_ipihandler.c)
	collect (PROJECT_LIB_HEADERS xsecure_rsa_ipihandler.h)
	collect (PROJECT_LIB_SOURCES xsecure_sha_ipihandler.c)
	collect (PROJECT_LIB_HEADERS xsecure_sha_ipihandler.h)
	collect (PROJECT_LIB_SOURCES xsecure_elliptic_ipihandler.c)
	collect (PROJECT_LIB_HEADERS xsecure_elliptic_ipihandler.h)
	collect (PROJECT_LIB_SOURCES xsecure_cmd.c)
	collect (PROJECT_LIB_HEADERS xsecure_cmd.h)
	collect (PROJECT_LIB_SOURCES xsecure_tamper.c)
	collect (PROJECT_LIB_HEADERS xsecure_tamper.h)
	collect (PROJECT_LIB_SOURCES xsecure_init.c)
	collect (PROJECT_LIB_HEADERS xsecure_init.h)
endif()
collect (PROJECT_LIB_SOURCES xsecure_rsa_core.c)
collect (PROJECT_LIB_HEADERS xsecure_rsa_core.h)
collect (PROJECT_LIB_SOURCES xsecure_sha.c)
collect (PROJECT_LIB_HEADERS xsecure_sha.h)
collect (PROJECT_LIB_HEADERS xsecure_sha_hw.h)
collect (PROJECT_LIB_SOURCES xsecure_sss.c)
collect (PROJECT_LIB_HEADERS xsecure_sss.h)

if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze"))
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_pmc.a ${CMAKE_BINARY_DIR}/libxilsecure.a)
endif()

if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexa72")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_a72_64.a ${CMAKE_BINARY_DIR}/libxilsecure.a)
endif()

if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortexr5")
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_r5.a ${CMAKE_BINARY_DIR}/libxilsecure.a)
endif()

if (${OS_ESW})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/libxilsecure_pmc.a ${CMAKE_BINARY_DIR}/libxilsecure_pmc.a)
    #execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libxilsecure_pmc.a) ${CMAKE_LIBRARY_PATH}/libxilsecure_pmc.a)
    file(COPY ${CMAKE_BINARY_DIR}/libxilsecure_pmc.a DESTINATION ${CMAKE_LIBRARY_PATH} FOLLOW_SYMLINK_CHAIN)

endif()

