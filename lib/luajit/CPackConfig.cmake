# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


set(CPACK_BUILD_SOURCE_DIRS "/home/ivalentin/Workspace/Software/V-Gears/lib/luajit;/home/ivalentin/Workspace/Software/V-Gears/lib/luajit")
set(CPACK_CMAKE_GENERATOR "Unix Makefiles")
set(CPACK_COMPONENTS_ALL "Runtime;Library;Header;Data;Documentation;Example;Other")
set(CPACK_COMPONENTS_ALL_SET_BY_USER "TRUE")
set(CPACK_COMPONENT_DATA_DESCRIPTION "Application data. Installed into share/luajit.")
set(CPACK_COMPONENT_DATA_DISPLAY_NAME "luajit Data")
set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "Application documentation. Installed into share/luajit/doc.")
set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "luajit Documentation")
set(CPACK_COMPONENT_EXAMPLE_DESCRIPTION "Examples and their associated data. Installed into share/luajit/example.")
set(CPACK_COMPONENT_EXAMPLE_DISPLAY_NAME "luajit Examples")
set(CPACK_COMPONENT_HEADER_DESCRIPTION "Headers needed for development. Installed into include.")
set(CPACK_COMPONENT_HEADER_DISPLAY_NAME "luajit Development Headers")
set(CPACK_COMPONENT_LIBRARY_DESCRIPTION "Static and import libraries needed for development. Installed into lib or bin.")
set(CPACK_COMPONENT_LIBRARY_DISPLAY_NAME "luajit Development Libraries")
set(CPACK_COMPONENT_OTHER_DESCRIPTION "Other unspecified content. Installed into share/luajit/etc.")
set(CPACK_COMPONENT_OTHER_DISPLAY_NAME "luajit Unspecified Content")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Executables and runtime libraries. Installed into bin.")
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "luajit Runtime")
set(CPACK_COMPONENT_TEST_DESCRIPTION "Tests and associated data. Installed into share/luajit/test.")
set(CPACK_COMPONENT_TEST_DISPLAY_NAME "luajit Tests")
set(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
set(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_FILE "/usr/share/cmake-3.30/Templates/CPack.GenericDescription.txt")
set(CPACK_DEFAULT_PACKAGE_DESCRIPTION_SUMMARY "luajit built using CMake")
set(CPACK_GENERATOR "ZIP")
set(CPACK_INNOSETUP_ARCHITECTURE "x64")
set(CPACK_INSTALL_CMAKE_PROJECTS "/home/ivalentin/Workspace/Software/V-Gears/lib/luajit;luajit;ALL;/")
set(CPACK_INSTALL_PREFIX "/usr/local")
set(CPACK_MODULE_PATH "/home/ivalentin/Workspace/Software/V-Gears/lib/luajit/cmake")
set(CPACK_NSIS_DISPLAY_NAME "luajit 2.0.3")
set(CPACK_NSIS_INSTALLER_ICON_CODE "")
set(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
set(CPACK_NSIS_PACKAGE_NAME "luajit 2.0.3")
set(CPACK_NSIS_UNINSTALL_NAME "Uninstall")
set(CPACK_OBJCOPY_EXECUTABLE "/usr/bin/objcopy")
set(CPACK_OBJDUMP_EXECUTABLE "/usr/bin/objdump")
set(CPACK_OUTPUT_CONFIG_FILE "/home/ivalentin/Workspace/Software/V-Gears/lib/luajit/CPackConfig.cmake")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/")
set(CPACK_PACKAGE_DESCRIPTION_FILE "/usr/share/cmake-3.30/Templates/CPack.GenericDescription.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "luajit built using CMake")
set(CPACK_PACKAGE_FILE_NAME "luajit-2.0.3-Linux")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "luajit 2.0.3")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "luajit 2.0.3")
set(CPACK_PACKAGE_NAME "luajit")
set(CPACK_PACKAGE_RELOCATABLE "true")
set(CPACK_PACKAGE_VENDOR "LuaDist")
set(CPACK_PACKAGE_VERSION "2.0.3")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_READELF_EXECUTABLE "/usr/bin/readelf")
set(CPACK_RESOURCE_FILE_LICENSE "/usr/share/cmake-3.30/Templates/CPack.GenericLicense.txt")
set(CPACK_RESOURCE_FILE_README "/usr/share/cmake-3.30/Templates/CPack.GenericDescription.txt")
set(CPACK_RESOURCE_FILE_WELCOME "/usr/share/cmake-3.30/Templates/CPack.GenericWelcome.txt")
set(CPACK_SET_DESTDIR "OFF")
set(CPACK_SOURCE_GENERATOR "TBZ2;TGZ;TXZ;TZ")
set(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/home/ivalentin/Workspace/Software/V-Gears/lib/luajit/CPackSourceConfig.cmake")
set(CPACK_SOURCE_RPM "OFF")
set(CPACK_SOURCE_TBZ2 "ON")
set(CPACK_SOURCE_TGZ "ON")
set(CPACK_SOURCE_TXZ "ON")
set(CPACK_SOURCE_TZ "ON")
set(CPACK_SOURCE_ZIP "OFF")
set(CPACK_STRIP_FILES "TRUE")
set(CPACK_SYSTEM_NAME "Linux")
set(CPACK_THREADS "1")
set(CPACK_TOPLEVEL_TAG "Linux")
set(CPACK_WIX_SIZEOF_VOID_P "8")

if(NOT CPACK_PROPERTIES_FILE)
  set(CPACK_PROPERTIES_FILE "/home/ivalentin/Workspace/Software/V-Gears/lib/luajit/CPackProperties.cmake")
endif()

if(EXISTS ${CPACK_PROPERTIES_FILE})
  include(${CPACK_PROPERTIES_FILE})
endif()
