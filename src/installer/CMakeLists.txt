# Find packages required for the istaller.
find_package(Qt5Widgets REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)

# Source files for the installer: the ones its folders listed (see vgears_sources in src/CMakeLists.txt).
get_property(INSTALLER_SOURCE_FILES GLOBAL PROPERTY VGEARS_SOURCES)
list(FILTER INSTALLER_SOURCE_FILES INCLUDE REGEX "^installer/.*\\.cpp$")
list(TRANSFORM INSTALLER_SOURCE_FILES REPLACE "^installer/" "")
list(SORT INSTALLER_SOURCE_FILES)

# Included directories for the installer.
include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ZLIB_INCLUDE_DIRS}
    ${OGRE_INCLUDE_DIRS}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
)


# Generate QT UIs.
set(v-gears-installer_UIS gui/MainWindow.ui)
QT5_WRAP_UI(UIS ${v-gears-installer_UIS})
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
set(v-gears-installer_MOCS gui/MainWindow.h)
QT5_WRAP_CPP(MOCS ${v-gears-installer_MOCS})
set(v-gears-installer_RESOURCES gui/installer_resources.qrc)
qt5_add_resources(RCC_SOURCES ${v-gears-installer_RESOURCES})


# Compiler options.
if (UNIX) # For SUDM. TODO: Add elses?
    add_definitions(-DPOSIX)
endif()


# Generate v-gears-installer executable.
add_executable (v-gears-installer ${INSTALLER_SOURCE_FILES} ${UIS} ${MOCS} ${RCC_SOURCES})
target_compile_definitions(v-gears-installer PRIVATE VGEARS_INSTALLER_VERSION="${PROJECT_VERSION}")
SET_PROPERTY(TARGET v-gears-installer PROPERTY FOLDER "build/v-gears-installer")
if (APPLE)
    target_link_libraries(v-gears-installer "-framework CoreFoundation -framework Cocoa -framework IOKit")
endif()
if(WIN32)
    set_target_properties(v-gears-installer PROPERTIES WIN32_EXECUTABLE ON)
    if (MSVC)
        set_target_properties(v-gears-installer PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    endif()
endif()
target_link_libraries(v-gears-installer
    libvgears
    Qt5::Widgets
    Qt5::Core
    archive
    ${OIS_LIBRARIES}
    ${TinyXML_LIBRARIES}
    ${BOOST_LINK_LIBS}
    ${OGRE_LIBRARIES}
    ${ZLIB_LIBRARIES}
)

# If OgreBites was found by the top-level CMake as a deterministic fallback
# it may be present in OGRE_LIBRARIES already; ensure it's visible here too.
if(NOT OGRE_LIBRARIES MATCHES ".*OgreBites.*")
    if(EXISTS "/usr/lib/x86_64-linux-gnu/libOgreBites.so")
        list(APPEND OGRE_LIBRARIES "/usr/lib/x86_64-linux-gnu/libOgreBites.so")
        message(STATUS "Installer CMake: appended deterministic OgreBites path for linker: /usr/lib/x86_64-linux-gnu/libOgreBites.so")
    endif()
endif()

# Ensure any newly appended OGRE libraries are linked into the installer target
if(OGRE_LIBRARIES)
    message(STATUS "Installer linking additional OGRE libraries: ${OGRE_LIBRARIES}")
    target_link_libraries(v-gears-installer ${OGRE_LIBRARIES})
endif()


# Install v-gears-installer (see CMake/InstallDirs.cmake).
install(
    TARGETS v-gears-installer
    RUNTIME DESTINATION ${VGEARS_INSTALL_BINDIR} BUNDLE DESTINATION ${VGEARS_INSTALL_BINDIR}
)
