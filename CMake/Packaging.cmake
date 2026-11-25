# Install rules for everything but the executables (those are in src/), and the CPack configuration.
#
# The directories are set in InstallDirs.cmake. To make the packages, from the build directory:
#   cmake --build . && cpack            Linux: .deb and .tar.gz. Windows and macOS: .zip.
#   cpack -G TGZ                        Only one of them.
# To install without a package:
#   cmake --install . --prefix /usr/local

# Data template. It's copied to the data directory of each user, where the data installer adds the game data.
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/data/
    DESTINATION ${VGEARS_INSTALL_TEMPLATEDIR}
    # Ogre plugins are found in the system or next to the executable, never in the template.
    PATTERN "*.so" EXCLUDE
    PATTERN "*.so.*" EXCLUDE
    PATTERN "*.dll" EXCLUDE
    PATTERN "*.dylib" EXCLUDE
)

# Documentation for players. The developer guides (build, style, code notes) are left out.
install(
    FILES
        ${CMAKE_SOURCE_DIR}/README.md
        ${CMAKE_SOURCE_DIR}/COPYING.txt
        ${CMAKE_SOURCE_DIR}/doc/AUTHORS.md
        ${CMAKE_SOURCE_DIR}/doc/HISTORY.md
        ${CMAKE_SOURCE_DIR}/doc/INSTALL_DATA.md
        ${CMAKE_SOURCE_DIR}/doc/RUN.md
    DESTINATION ${VGEARS_INSTALL_DOCDIR}
)

# Desktop integration. The icon names must match the Icon key of the desktop files, and the installer's desktop file
# name must match QApplication::setDesktopFileName so the shell shows the right icon for its window.
if(UNIX AND NOT APPLE)
    set(_vgears_desktop_files ${CMAKE_SOURCE_DIR}/v-gears.desktop)
    set(_vgears_icons ${CMAKE_SOURCE_DIR}/v-gears.png)
    if(BUILD_INSTALLER)
        list(APPEND _vgears_desktop_files ${CMAKE_SOURCE_DIR}/v-gears-installer.desktop)
        list(APPEND _vgears_icons ${CMAKE_SOURCE_DIR}/v-gears-installer.png)
    endif()
    install(FILES ${_vgears_desktop_files} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    # TODO: The icons are 300x300. Add 256x256 versions so they match their hicolor folder.
    install(FILES ${_vgears_icons} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps)
    install(FILES ${_vgears_icons} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pixmaps)
endif()

# Packages.
set(CPACK_PACKAGE_NAME "v-gears")
set(CPACK_PACKAGE_VENDOR "The V-Gears Team")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A free Final Fantasy VII engine")
set(CPACK_PACKAGE_DESCRIPTION
    "V-Gears is a free implementation of the Final Fantasy VII engine. It doesn't include any data of the original \
game: the V-Gears installer extracts it from your own copy of the PC version.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://v-gears.inigovalentin.com")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "V-Gears")
set(CPACK_PACKAGE_EXECUTABLES "v-gears" "V-Gears" "v-gears-installer" "V-Gears Installer")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGE_CHECKSUM SHA256)
set(CPACK_PACKAGE_FILE_NAME "v-gears-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    string(APPEND CPACK_PACKAGE_FILE_NAME "-debug")
endif()

if(WIN32 OR APPLE)
    set(CPACK_GENERATOR "ZIP")
    # The ZIP has a top-level folder, so it can be extracted anywhere.
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
else()
    set(CPACK_GENERATOR "DEB;TGZ")
    # The tarball has the prefix layout (bin/, share/...) in a top-level folder, so it can be extracted anywhere and
    # run from there. The .deb installs to /usr, the default of the DEB generator.
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
endif()

# Debian package.
set(VGEARS_PACKAGE_MAINTAINER "Iñigo Valentin <i@inigovalentin.com>" CACHE STRING "Maintainer of the packages")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${VGEARS_PACKAGE_MAINTAINER}")
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
# The dependencies are taken from the libraries the executables link. The Ogre plugins come in the same package as
# libOgreMain.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# The installer uses them to convert the music and the videos.
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "ffmpeg, timidity")

include(CPack)
