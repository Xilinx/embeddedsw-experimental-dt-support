import os
import sys
import glob
import re
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
        tmp_str = name.capitalize() + str("Example.cmake")
        f.write("include(${CMAKE_CURRENT_SOURCE_DIR}/%s)\n" % tmp_str)
        f.write("project(%s)\n\n" % name)
        f.write("enable_language(C ASM)\n")
        f.write("collect(PROJECT_LIB_DEPS xilstandalone)\n")
        f.write("collect(PROJECT_LIB_DEPS xilmem)\n")
        f.write("collect(PROJECT_LIB_DEPS xil)\n")
        f.write("collect(PROJECT_LIB_DEPS xiltimer)\n")
        if lib:
            f.write("collect(PROJECT_LIB_DEPS %s)\n" % name)
        f.write("collect(PROJECT_LIB_DEPS gcc)\n")
        f.write("collect(PROJECT_LIB_DEPS c)\n")
        f.write("collector_list (_deps PROJECT_LIB_DEPS)\n\n")

        if path:
            tmp_str =  "Driver Instances"
            tmp_str = '"{}"'.format(tmp_str)
            tmp_str1 =  "${NUM_DRIVER_INSTANCES}"
            tmp_str1 = '"{}"'.format(tmp_str1)
            f.write("SET(DRIVER_INSTANCES %s CACHE STRING %s)\n" % (tmp_str1, tmp_str))
            f.write("SET_PROPERTY(CACHE DRIVER_INSTANCES PROPERTY STRINGS %s)\n" % tmp_str1)
            f.write("set(index 0)\n")
            f.write("LIST_INDEX(${index} ${DRIVER_INSTANCES} %s)\n" % tmp_str1)
            f.write("list(GET TOTAL_EXAMPLE_LIST ${index} ex_list)\n")
            f.write("list(GET REG_LIST ${index} reg)\n")
            tmp_str =  "${reg}"
            tmp_str = '"{}"'.format(tmp_str)
            f.write("SET(X%s_BASEADDRESS %s)\n" % (name.upper(), tmp_str))

        example_list = ""
        example_parse_list = ""
        common_file = ""
        for file in os.listdir(os.getcwd()):
            if file.endswith(".c"):
                match = re.search("intr", file)
                util = re.search("util", file)
                if not match:
                    if util:
                        common_file += str(file)
                    else:
                        example_list += str(file)
                        example_list += ";"
                else:
                    example_parse_list += str(file)
                    example_parse_list += ";"

        print("example_list", example_list)
        tmp_str =  "Driver Example List"
        tmp_str = '"{}"'.format(tmp_str)
        tmp_str2 =  "${${ex_list}}"
        tmp_str2 = '"{}"'.format(tmp_str2)
        if common_file:
            f.write("SET(COMMON_FILE %s)\n" % common_file)
        f.write("SET(COMMON_EXAMPLES %s)\n" % example_list)

        if path:
            f.write("SET(EXAMPLE_LIST %s CACHE STRING %s)\n" % (tmp_str2 , tmp_str))
            f.write("SET_PROPERTY(CACHE EXAMPLE_LIST PROPERTY STRINGS %s)\n\n" % tmp_str2)
            f.write("set(valid_ex 0)\n")
            f.write("foreach(LIST1 ${TOTAL_EXAMPLE_LIST})\n")
            tmp_str =  "${${LIST1}}"
            tmp_str = '"{}"'.format(tmp_str)
            tmp_str1 =  "${EXAMPLE_LIST}"
            tmp_str1 = '"{}"'.format(tmp_str1)
            f.write("    if (%s STREQUAL %s)\n" % (tmp_str, tmp_str1))
            f.write("\tset(valid_ex 1)\n")
            f.write("\tbreak()\n")
            f.write("    endif()\n")
            f.write("endforeach()\n")
        tmp_str =  "${CMAKE_CURRENT_SOURCE_DIR}/../../../../scripts/linker_files/"
        tmp_str = '"{}"'.format(tmp_str)
        if lib:
            f.write("\nlinker_gen(%s)\n" % tmp_str)
        if path:
            f.write("\nset(CMAKE_INFILE_PATH %s)\n" % tmp_str)
            f.write("linker_gen(${CMAKE_INFILE_PATH})\n")
            f.write("gen_exheader(${CMAKE_INFILE_PATH} ${CMAKE_PROJECT_NAME} ${X%s_BASEADDRESS} x)\n" % name.upper())
            f.write("\nif (${valid_ex})\n")
            f.write("    list(APPEND COMMON_EXAMPLES %s)\n" % tmp_str2)
            f.write("else()\n")
            f.write("    list(APPEND COMMON_EXAMPLES ${EXAMPLE_LIST})\n")
            f.write("endif()\n\n")
        f.write("foreach(EXAMPLE ${COMMON_EXAMPLES})\n")
        tmp_str =  "\\\.[^.]*$"
        tmp_str = '"{}"'.format(tmp_str)
        tmp_str1 = ""
        tmp_str1 = '"{}"'.format(tmp_str1)
        f.write("    string(REGEX REPLACE %s %s EXAMPLE ${EXAMPLE})\n" % (tmp_str, tmp_str1))
        if common_file:
            f.write("    add_executable(${EXAMPLE}.elf ${EXAMPLE} ${COMMON_FILE})\n")
        else:
            f.write("    add_executable(${EXAMPLE}.elf ${EXAMPLE})\n")
        tmp_str = "${CMAKE_SOURCE_DIR}/lscript.ld\\" 
        tmp_str = '"{}"'.format(tmp_str)
        tmp_str1 = "${CMAKE_SOURCE_DIR}/\\"
        tmp_str1 = '"{}"'.format(tmp_str1)
        f.write("    target_link_libraries(${EXAMPLE}.elf -Wl,--gc-sections -T\%s -L\%s -Wl,--start-group ${_deps} -Wl,--end-group)\n" % (tmp_str, tmp_str1))
        f.write("endforeach()\n")

if __name__ == '__main__':
    main()
