## ProjectCommon.cmake
# Centralized CMake settings used across the V-Gears project.

# Build verbosity default (can be overridden by caller)
if(NOT DEFINED CMAKE_VERBOSE_MAKEFILE)
  set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "Enable verbose makefile output" FORCE)
endif()

# C++ standard and basic compile settings
if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Produce position independent code by default where relevant
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Debug postfix for libraries
if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "_d")
endif()

# Multi-config generators should use generator expressions; for single-config
# generators provide per-configuration output folders as a convenience.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin_debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin_debug/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin_debug/lib)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Make sure our custom CMake modules are discoverable (already appended by top-level)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

# Create directory for generated headers
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")

## Helper: install default GNUInstallDirs if not available
include(GNUInstallDirs OPTIONAL)

# Policy: prefer modern targets if project transitions to target-based linking.
