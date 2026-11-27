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


# Test source files.
set(SOURCE_FILES
    main.cpp
    formats/model/VGearsAFile.cpp
    formats/worldmap/MapFileSerializer.cpp
    engine/graphics/MovieCameraFrame.cpp
    engine/graphics/MovieState.cpp
    engine/graphics/ScreenFadeState.cpp
    engine/entity/EntityManager.cpp
    engine/entity/EntityPoint.cpp
    engine/ui/TextHandler.cpp
    installer/extractors/FieldDataInstaller.cpp
    installer/extractors/SfxDump.cpp
    installer/extractors/WindowFonts.cpp
    installer/decompiler/field/FieldCodeGenerator.cpp
    installer/decompiler/field/FieldDecompiler.cpp
    installer/decompiler/world/WorldDisassembler.cpp
    installer/decompiler/world/instruction/WorldSubStackInstruction.cpp
)


# Generate tests
add_executable(v-gears-tests ${SOURCE_FILES})
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
    common/DataDirectory.cpp
    common/OgrePlugins.cpp
    ${CMAKE_SOURCE_DIR}/src/common/DataDirectory.cpp
    ${CMAKE_SOURCE_DIR}/src/common/InstallStamp.cpp
    ${CMAKE_SOURCE_DIR}/src/common/OgrePlugins.cpp
    installer/steps/InstallLogic.cpp
    installer/steps/ExternalTools.cpp
    installer/extractors/MovieCamera.cpp
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
