cmake_minimum_required(VERSION 2.8.9)
project(libxil)

include_directories(${CMAKE_BINARY_DIR}/include)
file(COPY xparameters.h DESTINATION ${CMAKE_BINARY_DIR}/include)
add_library(libxil INTERFACE)
