"""
This module enables utilities for OpenAMP and Libmetal. This includes
enabling these libraries in BSPs, application configure and application
build steps.
"""

import os
import utils


def open_amp_copy_lib_src(lib, libdir, dstdir):
    """
    Copies the src directory of the passed library from the respective path
    of OpenAMP or Libmetal to the libsrc folder of bsp.

    Args:
        lib (str): library whose source code needs to be copied
        libdir (str): location for library in embeddedsw for cmake info.
        dstdir (str): destionation to copy lib sources to

    Returns:
        None
    """
    # copy specific cmake files
    srcdir = os.path.join(libdir, "src")
    top_srcdir = os.path.join(srcdir, 'top-CMakeLists.txt')

    top_dstdir = os.path.join(dstdir, 'CMakeLists.txt')
    utils.copy_file(top_srcdir, top_dstdir)

    lib_cmakelist = os.path.join(dstdir, 'lib')
    lib_cmakelist = os.path.join(lib_cmakelist, 'CMakeLists.txt')
    with open(lib_cmakelist, 'r', encoding='utf-8') as file:
        content = file.read()
        # Tell CMake build to install headers in BSP include area
        content = content.replace("DESTINATION include RENAME ${PROJECT_NAME}/${f})",
                                  "DESTINATION ${CMAKE_INCLUDE_PATH}/ RENAME ${PROJECT_NAME}/${f})")
        # Tell CMake build to install library in BSP library area

        # libmetal specific logic. Only picked up in case of libmetal
        orig = "install (TARGETS ${_lib} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})"
        new = "install (TARGETS ${_lib} ARCHIVE DESTINATION ${CMAKE_LIBRARY_PATH})"
        content = content.replace(orig, new)
        # open-amp specific logic. Only picked up in case of open-amp
        orig = "install (DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/include/openamp\" "
        orig += "DESTINATION include)"
        new = "install (DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/include/openamp\" "
        new += "DESTINATION ${CMAKE_INCLUDE_PATH}/)"
        content = content.replace(orig, new)
        orig = "install (DIRECTORY \"${PROJECT_BINARY_DIR}/include/generated/openamp\" "
        orig += "DESTINATION include)"
        new = "install (DIRECTORY \"${PROJECT_BINARY_DIR}/include/generated/openamp\" "
        new += "DESTINATION ${CMAKE_INCLUDE_PATH}/)"
        content = content.replace(orig, new)

    lib_dstdir = os.path.join(dstdir, 'lib')
    lib_dstdir = os.path.join(lib_dstdir, 'CMakeLists.txt')
    with open(lib_dstdir, 'w', encoding='utf-8') as file:
        file.write(content)

