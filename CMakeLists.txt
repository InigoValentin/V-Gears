# Minimum required CMake and project declaration
cmake_minimum_required(VERSION 3.0)
# Enable both C and CXX languages so C and C++ toolchains are configured.
project(V-Gears VERSION 0.1.18 LANGUAGES C CXX)

# Provide a central common include for project-wide policies and defaults.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
include(ProjectCommon)

# Configure CMake options
option(USE_COTIRE "Enable cotire precompiled header support" ON)
option(BUILD_INSTALLER "Build the V-Gears-Installer" TRUE)
option(BUILD_TESTS "Build the unit tests" FALSE)
option(MULTITHREADING "Enable multithreading" TRUE)

# Optional hint for Qt configuration dirs. Setting this early helps subprojects
# (like `src`) locate Qt via config-mode packages when installs are in
# distro-specific locations. Example: -DQT_DIR_HINT=/usr/lib/x86_64-linux-gnu/cmake
set(QT_DIR_HINT "" CACHE PATH "Optional hint for Qt CMake config directory (parent of Qt5/Qt6)")
if(QT_DIR_HINT)
    list(APPEND CMAKE_PREFIX_PATH "${QT_DIR_HINT}")
    if(EXISTS "${QT_DIR_HINT}/Qt5")
        set(Qt5_DIR "${QT_DIR_HINT}/Qt5" CACHE PATH "Qt5 dir from hint" FORCE)
    elseif(EXISTS "${QT_DIR_HINT}/Qt5Core")
        set(Qt5_DIR "${QT_DIR_HINT}/Qt5Core" CACHE PATH "Qt5 dir from hint" FORCE)
    endif()
    if(EXISTS "${QT_DIR_HINT}/Qt6")
        set(Qt6_DIR "${QT_DIR_HINT}/Qt6" CACHE PATH "Qt6 dir from hint" FORCE)
    endif()
endif()

# Optionally enable cotire (precompiled header/speedup).
if(USE_COTIRE)
    include(cotire OPTIONAL RESULT_VARIABLE COTIRE_INCLUDE_RESULT)
    if(NOT COTIRE_INCLUDE_RESULT)
        message(WARNING "cotire requested but cotire.cmake not found in CMake modules; skipping")
    endif()
endif()
set(CMAKE_PACKAGE_ICON v-gears.png)

# Generate version header into the build tree so source tree is not modified.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h.in ${CMAKE_BINARY_DIR}/generated/Version.h @ONLY)
include_directories(${CMAKE_BINARY_DIR}/generated)

# Default build type when none is supplied by the caller.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
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
