# Find packages required to compile tests. Imported targets are directory-scoped, so the Boost components that the
# engine links (found in src/) have to be looked up here too.
find_package(Boost REQUIRED COMPONENTS filesystem unit_test_framework program_options thread
    OPTIONAL_COMPONENTS system chrono)


# Compiler options.
if(UNIX OR APPLE)
    add_definitions(-DBOOST_TEST_DYN_LINK)
endif()
if (UNIX)
    add_definitions(-DPOSIX) # For SUDM. TODO: Add elses?
endif()


# Directories to include for test compilations.
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/installer
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${OGRE_INCLUDE_DIRS}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
)


# Test source files. Each folder lists its own in its CMakeLists.txt, with one of these functions: vgears_tests for
# the main test program, and vgears_installer_tests for the one of the installer logic.
function(vgears_collect_tests property)
    foreach(_source IN LISTS ARGN)
        file(RELATIVE_PATH _path "${PROJECT_SOURCE_DIR}/test" "${CMAKE_CURRENT_SOURCE_DIR}/${_source}")
        set_property(GLOBAL APPEND PROPERTY ${property} "${_path}")
    endforeach()
endfunction()
function(vgears_tests)
    vgears_collect_tests(VGEARS_TESTS ${ARGN})
endfunction()
function(vgears_installer_tests)
    vgears_collect_tests(VGEARS_INSTALLER_TESTS ${ARGN})
endfunction()

add_subdirectory(common)
add_subdirectory(engine)
add_subdirectory(formats)
add_subdirectory(installer)

get_property(TEST_SOURCE_FILES GLOBAL PROPERTY VGEARS_TESTS)
list(SORT TEST_SOURCE_FILES)
get_property(INSTALLER_TEST_SOURCE_FILES GLOBAL PROPERTY VGEARS_INSTALLER_TESTS)
list(SORT INSTALLER_TEST_SOURCE_FILES)


# Generate tests
add_executable(v-gears-tests main.cpp ${TEST_SOURCE_FILES})
SET_PROPERTY(TARGET v-gears-tests PROPERTY FOLDER "build/v-gears-test")

# Qt imported targets are directory-scoped: the ones created in src/ are not
# visible here. Look them up again in this scope, and fall back to plain
# linker flags when the config package is unavailable.
if(NOT TARGET Qt5::Widgets)
    find_package(Qt5 COMPONENTS Core Widgets QUIET)
endif()
if(TARGET Qt5::Widgets)
    set(_test_qt_libs Qt5::Widgets Qt5::Core)
else()
    set(_test_qt_libs -lQt5Widgets -lQt5Core)
endif()

# The tests use the same libraries as the engine (read from its target, as the variables set in src/ are not visible
# here), plus libarchive, which the installer code needs.
get_target_property(_engine_libs v-gears LINK_LIBRARIES)

target_link_libraries(v-gears-tests
    libvgears
    ${_test_qt_libs}
    ${_engine_libs}
    archive
    ${ZLIB_LIBRARIES}
    Boost::unit_test_framework
)
add_test(NAME v-gears-tests COMMAND v-gears-tests)


# Tests of the installer logic that doesn't need the rest of the engine (layout, option rules, verifier, log), and of
# the data directory lookup shared by the game and the installer.
add_executable(v-gears-installer-tests
    ${INSTALLER_TEST_SOURCE_FILES}
    # The code under test, which is built again here so these tests don't need the whole engine.
    ${CMAKE_SOURCE_DIR}/src/common/DataDirectory.cpp
    ${CMAKE_SOURCE_DIR}/src/common/InstallStamp.cpp
    ${CMAKE_SOURCE_DIR}/src/common/OgrePlugins.cpp
    ${CMAKE_SOURCE_DIR}/src/installer/extractors/MovieCamera.cpp
    ${CMAKE_SOURCE_DIR}/src/installer/steps/ExternalTools.cpp
    ${CMAKE_SOURCE_DIR}/src/installer/cli/InstallCommandLine.cpp
    ${CMAKE_SOURCE_DIR}/src/installer/steps/InstallLayout.cpp
    ${CMAKE_SOURCE_DIR}/src/installer/steps/InstallLog.cpp
    ${CMAKE_SOURCE_DIR}/src/installer/steps/InstallOptions.cpp
    ${CMAKE_SOURCE_DIR}/src/installer/steps/InstallVerifier.cpp
    ${CMAKE_SOURCE_DIR}/src/installer/steps/VideoFiles.cpp
)
SET_PROPERTY(TARGET v-gears-installer-tests PROPERTY FOLDER "build/v-gears-test")
target_link_libraries(v-gears-installer-tests ${_test_qt_libs} ${Boost_LIBRARIES})
target_compile_definitions(v-gears-installer-tests PRIVATE VGEARS_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME v-gears-installer-tests COMMAND v-gears-installer-tests)
