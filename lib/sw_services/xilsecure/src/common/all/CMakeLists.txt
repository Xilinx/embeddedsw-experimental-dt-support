if(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "microblazeel") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "plm_microblaze") OR
   ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "pmu_microblaze") OR
   ${XILSECURE_Mode} OR "${CMAKE_MACHINE}" STREQUAL "ZynqMP")
collect (PROJECT_LIB_SOURCES xsecure_rsa.c)
collect (PROJECT_LIB_HEADERS xsecure_rsa.h)
endif()
