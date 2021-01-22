import os
import sys
import glob
import operator
from optparse import OptionParser

parser = OptionParser()
parser.add_option('-d', '--drv-path', action='store',
                  help='Specify the driver relative source path')
parser.add_option('-n', '--name', action='store',
                  help='Specify the name of the driver')
parser.add_option('-l', '--lib-path', action='store',
                  help='Specify the library relative source path')
(options, args) = parser.parse_args()

def main():
    path = options.drv_path
    name = options.name
    lib = options.lib_path
    if path:
        os.chdir(path)
    if lib:
        os.chdir(lib)
    with open('CMakeLists.txt', "w") as f:
        f.write("cmake_minimum_required(VERSION 2.8.9)\n")
        f.write("include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../cmake/common.cmake NO_POLICY_SCOPE)\n")
        f.write("project(%s)\n\n" % name)
        tmp_str =  "${CMAKE_CURRENT_SOURCE_DIR}"
        tmp_str = '"{}"'.format(tmp_str)
        f.write("collector_create (PROJECT_LIB_HEADERS %s)\n" % tmp_str)
        if lib:
            f.write("collector_create (PROJECT_LIB_SOURCES %s)\n" % tmp_str)
        f.write("include_directories(${CMAKE_BINARY_DIR}/include)\n")
        for file in os.listdir(os.getcwd()):
            if file.endswith(".o"):
                f.write("collect (PROJECT_LIB_SOURCES %s)\n" % file)
            if file.endswith(".c"):
                f.write("collect (PROJECT_LIB_SOURCES %s)\n" % file)
            if file.endswith(".h"):
                f.write("collect (PROJECT_LIB_HEADERS %s)\n" % file)
        f.write("collector_list (_sources PROJECT_LIB_SOURCES)\n")
        f.write("collector_list (_headers PROJECT_LIB_HEADERS)\n")
        f.write("file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include)\n")
        print("lib is", lib)
        if lib:
            f.write("add_library(%s STATIC ${_sources})\n" % name)
            f.write("set_target_properties(%s PROPERTIES LINKER_LANGUAGE C)\n" % name)
            f.write("install(TARGETS %s LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})\n" % name)

if __name__ == '__main__':
    main()
