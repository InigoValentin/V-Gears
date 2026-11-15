# Where `make install` (and the packages made with CPack) put each part of V-Gears.
#
# Linux and other Unix systems use the usual layout of the prefix:
#   <prefix>/bin/v-gears, <prefix>/bin/v-gears-installer
#   <prefix>/share/v-gears/          The data template: plugins.cfg, resources.cfg, data/...
#   <prefix>/share/doc/v-gears/      Documentation and license
#   <prefix>/share/applications/     Desktop files, and the icons in share/icons and share/pixmaps
#
# Windows and macOS use a single portable folder:
#   <prefix>/v-gears(.exe), <prefix>/v-gears-installer(.exe)
#   <prefix>/data/                   The data template
#   <prefix>/                        Documentation and license
#
# The game finds the template in both layouts (see src/common/DataDirectory.h), and copies it to the data directory
# of the user the first time it runs.

include(GNUInstallDirs)

if(WIN32 OR APPLE)
    set(VGEARS_INSTALL_BINDIR ".")
    set(VGEARS_INSTALL_TEMPLATEDIR "data")
    set(VGEARS_INSTALL_DOCDIR ".")
else()
    set(VGEARS_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
    set(VGEARS_INSTALL_TEMPLATEDIR "${CMAKE_INSTALL_DATADIR}/v-gears")
    set(VGEARS_INSTALL_DOCDIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/v-gears")
endif()
