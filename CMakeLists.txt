cmake_minimum_required(VERSION 3.0)


# Build verbosity: on / off.
set(CMAKE_VERBOSE_MAKEFILE off)


# C++ Standard.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# Add cotire
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake")
include(cotire)

# Project Information.
project(V-Gears)
set(CMAKE_PACKAGE_ICON v-gears.png)
set(VGEARS_VERSION_MAJOR 0)
set(VGEARS_VERSION_MINOR 1)
set(VGEARS_VERSION_PATCH 18)
set(VGEARS_VERSION ${VGEARS_VERSION_MAJOR}.${VGEARS_VERSION_MINOR}.${VGEARS_VERSION_PATCH})


# Project options.
option(BUILD_INSTALLER "Build the V-Gears-Installer" TRUE)
option(BUILD_TESTS "Build the unit tests" FALSE)
option(MULTITHREADING "Enable multithreading" FALSE)


# Generate version header.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h) 


# Hanle build type
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
    # differentiation between debug and release builds.
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()


#Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin_debug/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin_debug/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin_debug)

# Enable Visual Studio solution "folders".
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)


# Global configurations.
set(CMAKE_DEBUG_POSTFIX "_d")
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_CURRENT_SOURCE_DIR
        "${VGEARS_SOURCE_DIR}/data" CACHE PATH "V-Gears install prefix" FORCE
    )
endif()

# First, build external libraries (luabind and luajit).
add_subdirectory(lib)

# Build v-gears (and, if requested, the installer).
add_subdirectory(src)

# If requested, build tests
if(BUILD_TESTS)
    add_subdirectory(test)
endif()


# TODO: Generate Installer (.msi, .deb, .appImage...)
