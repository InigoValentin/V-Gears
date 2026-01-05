# Find Ogre module.
# Auto-detect common OGRE install locations if the user didn't provide one.
# TODO: Add candidates for other platforms.
if(NOT DEFINED OGRE_ROOT)
    set(_OGRE_CANDIDATES
        "/usr/lib/x86_64-linux-gnu/OGRE"
        "/usr/lib/OGRE"
        "/usr/local/lib/OGRE"
        "/usr/share/OGRE"
    )
    foreach(_cand IN LISTS _OGRE_CANDIDATES)
        if(EXISTS "${_cand}")
            # If the candidate points to an include directory like /usr/include/OGRE,
            # set OGRE_ROOT to the system root (two directories up) so later
            # searches for modules/libraries can succeed.
            if(_cand MATCHES "/include/OGRE$" OR _cand MATCHES ".*/OGRE$")
                get_filename_component(_p "${_cand}" DIRECTORY)
                get_filename_component(_p2 "${_p}" DIRECTORY)
                set(OGRE_ROOT "${_p2}")
            else()
                set(OGRE_ROOT "${_cand}")
            endif()
            message(STATUS "Auto-detected OGRE_ROOT=${OGRE_ROOT}")
            break()
        endif()
    endforeach()
endif()



# Explicitly search for other common libraries if their CMake find modules failed
if(NOT OIS_LIBRARIES)
    find_library(_ois_lib NAMES OIS OIS-1.3.0 libOIS-1.3.0 libOIS HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(_ois_lib)
        set(OIS_LIBRARIES ${_ois_lib})
        message(STATUS "Fallback: Found OIS library: ${_ois_lib}")
    endif()
endif()

# Option: control whether deterministic filesystem fallbacks are allowed.
# When ON (default), CMake will try SONAME/`/usr/lib` fallbacks and inject
# generic `-lboost_*` flags as a last resort to improve out-of-tree builds on
# systems without proper dev packages. Set to OFF to require proper find_package
# results or user-supplied hints (preferred for packaging/CI).
option(USE_DETERMINISTIC_FALLBACKS "Enable deterministic filesystem fallbacks for library detection (SONAME and /usr/lib path appends). Toggle off to require proper dev packages or OGRE_ROOT." ON)


if(NOT TinyXML_LIBRARIES)
    find_library(_tinyxml_lib NAMES tinyxml libtinyxml TinyXML TiXml libtinyxml2 tinyxml2 libtinyxml2 HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(_tinyxml_lib)
        set(TinyXML_LIBRARIES ${_tinyxml_lib})
        message(STATUS "Fallback: Found TinyXML library: ${_tinyxml_lib}")
    endif()
endif()

if(NOT OPENAL_LIBRARY)
    find_library(_openal_lib NAMES openal libopenal OpenAL openal32 libopenal.so.1 HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(_openal_lib)
        set(OPENAL_LIBRARY ${_openal_lib})
        message(STATUS "Fallback: Found OpenAL library: ${_openal_lib}")
    endif()
endif()

if(NOT OGGVORBIS_LIBRARIES)
    set(_ov)
    find_library(_vorbisfile_lib NAMES vorbisfile libvorbisfile libvorbisfile.so vorbisfile.so HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    find_library(_vorbis_lib NAMES vorbis libvorbis libvorbis.so vorbis.so HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    find_library(_ogg_lib NAMES ogg libogg libogg.so ogg.so HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(_vorbisfile_lib)
        list(APPEND _ov ${_vorbisfile_lib})
    endif()
    if(_vorbis_lib)
        list(APPEND _ov ${_vorbis_lib})
    endif()
    if(_ogg_lib)
        list(APPEND _ov ${_ogg_lib})
    endif()
    if(_ov)
        set(OGGVORBIS_LIBRARIES ${_ov})
        message(STATUS "Fallback: Found Ogg/Vorbis libraries: ${_ov}")
    endif()
endif()

# Try to find additional OGRE components if they are present as system libs
foreach(_comp IN ITEMS OgreOverlay OgreBites OgreRTShaderSystem OgreProperty OgrePaging OgreVolume)
    find_library(_comp_lib NAMES ${_comp} lib${_comp} ${_comp}.so HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib ${OGRE_ROOT}/lib)
    if(_comp_lib)
        list(APPEND OGRE_LIBRARIES ${_comp_lib})
        message(STATUS "Fallback: Found OGRE component ${_comp}: ${_comp_lib}")
    endif()
endforeach()

# Remove any duplicate entries that may have been appended by fallbacks
list(REMOVE_DUPLICATES OGRE_LIBRARIES)

# Try pkg-config to find OGRE (if available on the system)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(_ogre_pkgs QUIET OGRE)
    if(_ogre_pkgs_FOUND)
        if(_ogre_pkgs.prefix)
            if(NOT DEFINED OGRE_ROOT)
                set(OGRE_ROOT "${_ogre_pkgs.prefix}")
            endif()
            message(STATUS "Found OGRE via pkg-config, OGRE_ROOT=${OGRE_ROOT}")
            # If pkg-config gives us a prefix, try to set CMake module path accordingly
            set(_candidate_cm_path "${_ogre_pkgs.prefix}/CMake")
            if(EXISTS "${_candidate_cm_path}")
                set(OGRE_CMAKE_MODULE_PATH "${_candidate_cm_path}")
            else()
                set(_candidate_cm_path "${_ogre_pkgs.prefix}/cmake")
                if(EXISTS "${_candidate_cm_path}")
                    set(OGRE_CMAKE_MODULE_PATH "${_candidate_cm_path}")
                endif()
            endif()
        endif()
        # Export include dirs and libraries from pkg-config for later target usage
            set(OGRE_INCLUDE_DIRS ${_ogre_pkgs_INCLUDE_DIRS})
            # Convert pkg-config library flags (e.g. "-L.. -lOgreMain -lOgreBites")
            # into a CMake list so target_link_libraries handles them correctly.
            string(REPLACE " " ";" _pkg_libs_list "${_ogre_pkgs_LIBRARIES}")
            list(APPEND OGRE_LIBRARIES ${_pkg_libs_list})
        set(OGRE_PKG_FOUND TRUE)
    endif()
endif()

# If we still don't have OGRE_ROOT, try locating Ogre headers or the OgreMain library directly
if(NOT DEFINED OGRE_ROOT)
    find_path(_ogre_header Ogre.h HINTS /usr/include/OGRE /usr/include /usr/local/include)
    if(_ogre_header)
        # If header was found in /usr/include/OGRE, set OGRE_ROOT to /usr
        get_filename_component(_hdr_dir "${_ogre_header}" DIRECTORY)
        if(_hdr_dir MATCHES "/include/OGRE$" OR _hdr_dir MATCHES ".*/OGRE$")
            get_filename_component(_p "${_hdr_dir}" DIRECTORY)
            get_filename_component(_p2 "${_p}" DIRECTORY)
            set(OGRE_ROOT "${_p2}")
        else()
            # header found in include root; set OGRE_ROOT to parent
            get_filename_component(_p "${_hdr_dir}" DIRECTORY)
            set(OGRE_ROOT "${_p}")
        endif()
        message(STATUS "Found Ogre header at ${_ogre_header}; setting OGRE_ROOT=${OGRE_ROOT}")
    endif()
endif()

if(NOT DEFINED OGRE_ROOT)
    find_library(_ogre_lib OgreMain HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(_ogre_lib)
        get_filename_component(_lib_dir "${_ogre_lib}" DIRECTORY)
        # set OGRE_ROOT two levels up if libs are in /usr/lib/x86_64-linux-gnu
        get_filename_component(_p "${_lib_dir}" DIRECTORY)
        get_filename_component(_p2 "${_p}" DIRECTORY)
        set(OGRE_ROOT "${_p2}")
        message(STATUS "Found Ogre library ${_ogre_lib}; setting OGRE_ROOT=${OGRE_ROOT}")
    endif()
endif()
set(CMAKE_FRAMEWORK_PATH
    ${CMAKE_FRAMEWORK_PATH}
    ${CMAKE_SOURCE_DIR}/OgreSDK/lib/macosx/Release
    ${CMAKE_SOURCE_DIR}/OgreSDK/lib/macosx/Debug
)
set(OGRE_FRAMEWORK_INCLUDES ${CMAKE_SOURCE_DIR}/OgreSDK/include)
find_path(OGRE_CMAKE_MODULE_PATH FindOGRE.cmake
    HINTS
    "$ENV{OGRE_HOME}/CMake/"
    "/usr/lib/x86_64-linux-gnu/OGRE/cmake"
    "/usr/lib/x86_64-linux-gnu/OGRE"
    "/usr/local/lib/OGRE/cmake"
    "/usr/lib/OGRE/cmake"
    "/usr/share/OGRE/cmake/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/OgreSDK/CMake"
)
if(NOT OGRE_CMAKE_MODULE_PATH)
    # Try additional conventional locations using OGRE_ROOT if set by user or auto-detection
    if(DEFINED OGRE_ROOT)
        find_path(OGRE_CMAKE_MODULE_PATH FindOGRE.cmake HINTS "${OGRE_ROOT}/CMake" "${OGRE_ROOT}/cmake")
    endif()
endif()
if(NOT OGRE_CMAKE_MODULE_PATH)
    message(WARNING "FindOGRE.cmake not found in expected locations; will try pkg-config or config-mode find_package(OGRE)")
else()
    list(APPEND CMAKE_MODULE_PATH "${OGRE_CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/CMake")
endif()


# Find Boost module.
if (WIN32)
    set(Boost_USE_STATIC_LIBS TRUE)
else()
    # Statically linking boost to a dynamic Ogre build doesn't work on Linux 64bit.
    set(Boost_USE_STATIC_LIBS ${OGRE_STATIC})
endif ()
if (MINGW)
    # Ensure OgreBites is linked (provides WindowEventUtilities and related symbols)
    find_library(_ogrebites_lib OgreBites HINTS ${OGRE_ROOT} /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(_ogrebites_lib)
    # This is probably a bug in CMake: the boost find module tries to look for boost libraries with
    # name libboost_*, but CMake already prefixes library search names with "lib". This is the
    # workaround.
    set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_FIND_LIBRARY_PREFIXES} "")
endif ()
# Components that need linking (NB does not include header-only components like bind).
set(OGRE_BOOST_COMPONENTS thread date_time)
find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} QUIET)
if (NOT Boost_FOUND)
    set(Boost_USE_STATIC_LIBS NOT ${Boost_USE_STATIC_LIBS})
    find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} QUIET)
endif()
if(Boost_FOUND AND Boost_VERSION GREATER 104900)
    set(OGRE_BOOST_COMPONENTS thread date_time system chrono)
    find_package(Boost COMPONENTS ${OGRE_BOOST_COMPONENTS} QUIET)
endif()
# Set up referencing of Boost.
if (NOT APPLE)
    add_definitions(-DBOOST_ALL_NO_LIB)
endif()
if (Boost_LIBRARIES STREQUAL "")
    message(STATUS "Boost_LIBRARIES is not set yet; will attempt robust fallbacks later")
endif()

message(STATUS "DEBUG: entering Boost detection; BOOST_COMPONENTS=${BOOST_COMPONENTS} BOOST_LINK_LIBS=${BOOST_LINK_LIBS}")


# Find packages required for V-Gears.
# Prefer pkg-config discovery for OGRE (works with system packages). If
# pkg-config finds OGRE, populate OGRE_INCLUDE_DIRS and OGRE_LIBRARIES and
# skip find_package(OGRE). Otherwise fall back to the existing FindOGRE logic.
find_package(PkgConfig QUIET)
if(NOT DEFINED OGRE_PKG_FOUND)
    if(PKG_CONFIG_FOUND)
        # we already attempted pkg-config above; if that set OGRE_PKG_FOUND use it
        if(DEFINED _ogre_pkgs_FOUND AND _ogre_pkgs_FOUND)
            set(OGRE_PKG_FOUND TRUE)
            # OGRE_INCLUDE_DIRS/OGRE_LIBRARIES were set earlier when _ogre_pkgs was found
        endif()
    endif()
endif()
if(NOT OGRE_PKG_FOUND)
    find_package(OGRE REQUIRED QUIET)
else()
    message(STATUS "Using OGRE from pkg-config: includes=${OGRE_INCLUDE_DIRS}")
endif()

# If pkg-config was used, try to locate common OGRE component libraries
if(OGRE_PKG_FOUND)
    set(_extra_ogre_libs)
    foreach(_comp IN ITEMS OgreOverlay OgreBites OgreRTShaderSystem)
        find_library(_lib ${_comp} HINTS ${OGRE_ROOT} /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib PATHS ${OGRE_ROOT}/lib)
        if(_lib)
            list(APPEND _extra_ogre_libs ${_lib})
            message(STATUS "Found OGRE component ${_comp}: ${_lib}")
        endif()
    endforeach()
    if(_extra_ogre_libs)
        list(APPEND OGRE_LIBRARIES ${_extra_ogre_libs})
    endif()
    # Debug: show final OGRE libraries that will be linked
    message(STATUS "Final OGRE_LIBRARIES: ${OGRE_LIBRARIES}")
endif()
find_package(OIS QUIET)
if(NOT OIS_FOUND)
    find_library(_ois_lib NAMES OIS libOIS ois HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(_ois_lib)
        set(OIS_LIBRARIES ${_ois_lib})
        message(STATUS "Found OIS library: ${_ois_lib}")
    else()
        message(STATUS "OIS not found by find_package; will attempt to continue and emit clearer diagnostics later.")
    endif()
endif()

find_package(OpenAL QUIET)
if(NOT OPENAL_FOUND AND PKG_CONFIG_FOUND)
    pkg_check_modules(_openal_pkgs QUIET openal)
    if(_openal_pkgs_FOUND)
        list(APPEND OPENAL_LIBRARY ${_openal_pkgs_LIBRARIES})
        list(APPEND OPENAL_INCLUDE_DIR ${_openal_pkgs_INCLUDE_DIRS})
        message(STATUS "Found OpenAL via pkg-config: ${_openal_pkgs_LIBRARIES}")
    endif()
endif()
if(NOT OPENAL_FOUND AND NOT OPENAL_LIBRARY)
    find_library(_openal_lib NAMES openal libopenal OpenAL openal32 HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(_openal_lib)
        set(OPENAL_LIBRARY ${_openal_lib})
        message(STATUS "Found OpenAL library: ${_openal_lib}")
    endif()
endif()

find_package(OggVorbis QUIET)
if(NOT OGGVORBIS_FOUND)
    # Try to find vorbisfile/ogg libs directly
    find_library(_vorbisfile_lib NAMES vorbisfile libvorbisfile HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    find_library(_vorbis_lib NAMES vorbis libvorbis HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    find_library(_ogg_lib NAMES ogg libogg HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    set(_ovlibs)
    if(_vorbisfile_lib)
        list(APPEND _ovlibs ${_vorbisfile_lib})
    endif()
    if(_vorbis_lib)
        list(APPEND _ovlibs ${_vorbis_lib})
    endif()
    if(_ogg_lib)
        list(APPEND _ovlibs ${_ogg_lib})
    endif()
    if(_ovlibs)
        set(OGGVORBIS_LIBRARIES ${_ovlibs})
        message(STATUS "Found Ogg/Vorbis libraries: ${_ovlibs}")
    else()
        message(STATUS "Ogg/Vorbis not found by find_package; will attempt to continue and emit clearer diagnostics later.")
    endif()
endif()

# If the installer is requested, ensure Qt is available. If Qt is missing,
# disable the installer to avoid compilation failures (useful for developers
# who don't want to install Qt dev packages).
if(BUILD_INSTALLER)
    # Optional user-provided hint: allow passing a single path that contains
    # the Qt CMake config directories (e.g. '/usr/lib/x86_64-linux-gnu/cmake').
    set(QT_DIR_HINT "" CACHE PATH "Optional hint for Qt CMake config directory (parent of Qt5/Qt6)")
    if(QT_DIR_HINT)
        # If user passed a hint, set Qt5_DIR/Qt6_DIR to assist find_package.
        if(EXISTS "${QT_DIR_HINT}/Qt5")
            set(Qt5_DIR "${QT_DIR_HINT}/Qt5" CACHE PATH "Qt5 dir from hint" FORCE)
        elseif(EXISTS "${QT_DIR_HINT}/Qt5Core")
            set(Qt5_DIR "${QT_DIR_HINT}/Qt5Core" CACHE PATH "Qt5 dir from hint" FORCE)
        endif()
        if(EXISTS "${QT_DIR_HINT}/Qt6")
            set(Qt6_DIR "${QT_DIR_HINT}/Qt6" CACHE PATH "Qt6 dir from hint" FORCE)
        endif()
    endif()
    # Try Qt5 first, then fall back to Qt6. Prefer imported targets for linking.
    find_package(Qt5 COMPONENTS Core Widgets Gui QUIET)
    # Debugging: print detected Qt5 variables/targets when present
    if(TARGET Qt5::Core)
        message(STATUS "DEBUG: Qt5 imported target Qt5::Core is available")
    endif()
    message(STATUS "DEBUG: Qt5Core_FOUND=${Qt5Core_FOUND} Qt5Widgets_FOUND=${Qt5Widgets_FOUND} Qt5Gui_FOUND=${Qt5Gui_FOUND}")
    # Some distro setups place Qt config files in nonstandard cmake dirs that
    # CMake may not probe. If the normal find_package didn't create imported
    # targets or set FOUND variables, probe common Qt config locations and
    # retry find_package with explicit hints.
    if(NOT (TARGET Qt5::Core OR Qt5Core_FOUND))
        set(_qt_candidate_dirs
            /usr/lib/x86_64-linux-gnu/cmake/Qt5
            /usr/lib/x86_64-linux-gnu/cmake/Qt5Core
            /usr/lib/cmake/Qt5
            /usr/lib64/cmake/Qt5
            /usr/local/lib/cmake/Qt5
            $ENV{HOME}/.local/lib/cmake/Qt5
        )
        foreach(_qd IN LISTS _qt_candidate_dirs)
            if(EXISTS "${_qd}/Qt5Config.cmake" OR EXISTS "${_qd}/Qt5CoreConfig.cmake")
                message(STATUS "Found Qt5 config file under ${_qd}; retrying find_package(Qt5) with PATHS hint")
                set(Qt5_DIR "${_qd}" CACHE PATH "Detected Qt5 dir" FORCE)
                find_package(Qt5 COMPONENTS Core Widgets Gui QUIET PATHS "${_qd}")
                break()
            endif()
        endforeach()
    endif()

    if(TARGET Qt5::Core OR Qt5Core_FOUND)
        set(QT_LINK_LIBS Qt5::Core Qt5::Widgets Qt5::Gui)
        if(DEFINED Qt5Core_INCLUDE_DIRS)
            set(QT_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS})
        endif()
        message(STATUS "Found Qt5; installer will be built using Qt5.")
    else()
        find_package(Qt6 COMPONENTS Core Widgets Gui QUIET)
        if(NOT (TARGET Qt6::Core OR Qt6Core_FOUND))
            # Retry probing common Qt6 cmake locations as well
            set(_qt6_candidate_dirs
                /usr/lib/x86_64-linux-gnu/cmake/Qt6
                /usr/lib/cmake/Qt6
                /usr/lib64/cmake/Qt6
                /usr/local/lib/cmake/Qt6
                $ENV{HOME}/.local/lib/cmake/Qt6
            )
            foreach(_qd IN LISTS _qt6_candidate_dirs)
                if(EXISTS "${_qd}/Qt6Config.cmake" OR EXISTS "${_qd}/Qt6CoreConfig.cmake")
                    message(STATUS "Found Qt6 config file under ${_qd}; retrying find_package(Qt6) with PATHS hint")
                    set(Qt6_DIR "${_qd}" CACHE PATH "Detected Qt6 dir" FORCE)
                    find_package(Qt6 COMPONENTS Core Widgets Gui QUIET PATHS "${_qd}")
                    break()
                endif()
            endforeach()
        endif()
        if(TARGET Qt6::Core OR Qt6Core_FOUND)
            set(QT_LINK_LIBS Qt6::Core Qt6::Widgets Qt6::Gui)
            set(QT_INCLUDE_DIRS)
            message(STATUS "Found Qt6; installer will be built using Qt6.")
        else()
            # Attempt a pragmatic fallback: search for Qt headers/libs in common
            # distro include/lib locations and link manually if found. This is
            # less featureful than using the Qt CMake config (imported targets),
            # but it helps the installer build on systems where Qt dev packages
            # are installed but config-mode find_package didn't succeed.
            set(_qt_fallback_includes)
            set(_qt_fallback_libs)
            set(_qt_inc_candidates /usr/include/qt5 /usr/include/x86_64-linux-gnu/qt5 /usr/include)
            foreach(_inc IN LISTS _qt_inc_candidates)
                if(EXISTS "${_inc}/QtCore/QtGlobal")
                    list(APPEND _qt_fallback_includes ${_inc})
                    message(STATUS "Pragmatic Qt fallback: found headers under ${_inc}")
                    break()
                endif()
            endforeach()
            # Try to find Qt5 libraries by SONAME/name
            foreach(_libname IN ITEMS Qt5Core Qt5Widgets Qt5Gui)
                find_library(_qtl NAMES ${_libname} ${_libname}.so lib${_libname}.so HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
                if(_qtl)
                    list(APPEND _qt_fallback_libs ${_qtl})
                    message(STATUS "Pragmatic Qt fallback: found ${_libname}: ${_qtl}")
                endif()
            endforeach()
            if(_qt_fallback_libs AND _qt_fallback_includes)
                set(QT_LINK_LIBS ${_qt_fallback_libs})
                set(QT_INCLUDE_DIRS ${_qt_fallback_includes})
                message(STATUS "Using pragmatic Qt fallback (manual include/libs). Installer will be built.")
            else()
                # As a last resort, and only when deterministic fallbacks are
                # enabled, inject generic Qt5 linker flags and common include
                # paths so the installer can still be built on Debian/Ubuntu-like
                # systems where the config-mode package lookup didn't succeed.
                if(USE_DETERMINISTIC_FALLBACKS)
                    message(STATUS "Pragmatic Qt fallback failed; using generic Qt -l flags and include paths as last resort")
                    set(QT_LINK_LIBS -lQt5Core -lQt5Widgets -lQt5Gui)
                    set(QT_INCLUDE_DIRS /usr/include/qt5 /usr/include/x86_64-linux-gnu/qt5)
                else()
                    message(WARNING "BUILD_INSTALLER requested but Qt5/Qt6 not found and pragmatic fallback failed. Disabling installer build. Install Qt dev packages or set BUILD_INSTALLER=OFF.")
                    set(BUILD_INSTALLER OFF CACHE BOOL "Build the V-Gears-Installer" FORCE)
                endif()
            endif()
        endif()
    endif()
endif()

find_package(TinyXML QUIET)
if(NOT TinyXML_FOUND)
    # try to locate tinyxml library directly
    find_library(_tinyxml_lib NAMES tinyxml libtinyxml TiXml TiXmlDocument HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(_tinyxml_lib)
        set(TinyXML_LIBRARIES ${_tinyxml_lib})
        message(STATUS "Found TinyXML library: ${_tinyxml_lib}")
    else()
        message(STATUS "TinyXML not found by find_package; will attempt to continue and emit clearer diagnostics later.")
    endif()
endif()

# Find Boost module with common components and fall back to robust per-component search
set(BOOST_COMPONENTS program_options filesystem thread system chrono)
# First try to pick up modern imported targets (Boost::component)
set(BOOST_LINK_LIBS)
foreach(_comp IN LISTS BOOST_COMPONENTS)
    if(TARGET Boost::${_comp})
        list(APPEND BOOST_LINK_LIBS Boost::${_comp})
    endif()
endforeach()

# Next try the classic find_package that populates Boost_LIBRARIES
if(NOT BOOST_LINK_LIBS)
    find_package(Boost COMPONENTS ${BOOST_COMPONENTS} QUIET)
    if(NOT Boost_FOUND)
        # Try toggling static preference once
        set(Boost_USE_STATIC_LIBS NOT ${Boost_USE_STATIC_LIBS})
        find_package(Boost COMPONENTS ${BOOST_COMPONENTS} QUIET)
    endif()
    if(Boost_FOUND AND Boost_LIBRARIES)
        list(APPEND BOOST_LINK_LIBS ${Boost_LIBRARIES})
        message(STATUS "Found Boost (find_package): ${Boost_VERSION}; libraries: ${Boost_LIBRARIES}")
    endif()
endif()

# Final fallback: search for per-component shared objects in common lib dirs
if(NOT BOOST_LINK_LIBS)
    foreach(_comp IN LISTS BOOST_COMPONENTS)
        # Look for any versioned libboost_${comp}*.so* file
        file(GLOB _found_libs
            /usr/lib*/libboost_${_comp}*.so*
            /usr/local/lib*/libboost_${_comp}*.so*
        )
        if(_found_libs)
            # Prefer the first (usually the SONAME symlink), but append all found to be safe
            list(APPEND BOOST_LINK_LIBS ${_found_libs})
            message(STATUS "Fallback: found boost ${_comp}: ${_found_libs}")
        else()
            message(STATUS "Fallback: boost ${_comp} not found in standard lib dirs")
        endif()
    endforeach()
    if(NOT BOOST_LINK_LIBS)
        message(STATUS "No Boost libs found; linking will fail. Install libboost-*-dev packages.")
    endif()
endif()

# Try a direct SONAME-style find_library pass for each component (preferred:
# yields absolute paths like /usr/lib/x86_64-linux-gnu/libboost_program_options.so.1.83.0).
if(NOT BOOST_LINK_LIBS)
    foreach(_comp IN LISTS BOOST_COMPONENTS)
        # try common SONAME/filename variants so find_library returns full paths
        find_library(_boost_soname
            NAMES libboost_${_comp}.so libboost_${_comp} boost_${_comp} boost_${_comp}.so
            HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
            PATH_SUFFIXES . lib lib64
        )
        if(_boost_soname)
            list(APPEND BOOST_LINK_LIBS ${_boost_soname})
            message(STATUS "Found Boost SONAME for ${_comp}: ${_boost_soname}")
        endif()
    endforeach()
    if(NOT BOOST_LINK_LIBS)
        message(STATUS "SONAME pass did not find Boost libs; falling back to -l flags next.")
    endif()
endif()

# If previous attempts didn't produce usable lib paths, fall back to linker flags
# using common Boost library names so the system linker searches standard paths.
if(NOT BOOST_LINK_LIBS)
    if(USE_DETERMINISTIC_FALLBACKS)
        set(BOOST_LINK_LIBS -lboost_program_options -lboost_filesystem -lboost_thread -lboost_system -lboost_chrono)
        message(STATUS "Fallback: using generic -l flags for Boost: ${BOOST_LINK_LIBS}")
    else()
        message(WARNING "Boost libraries not found and USE_DETERMINISTIC_FALLBACKS is OFF. Install libboost-*-dev packages or provide Boost hints (Boost_DIR/Boost_INCLUDE_DIR).")
    endif()
endif()

# Deduplicate and export a stable list for later linking
list(REMOVE_DUPLICATES BOOST_LINK_LIBS)
message(STATUS "Final BOOST_LINK_LIBS: ${BOOST_LINK_LIBS}")

# Root source dir for V-Gears sources (used for include paths).
if(NOT DEFINED VGEARS_SOURCE_DIR)
    set(VGEARS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Ensure OgreBites (WindowEventUtilities) is explicitly found and linked when available
if(NOT OGRE_LIBRARIES)
    set(OGRE_LIBRARIES)
endif()
find_library(_ogrebites_lib NAMES OgreBites libOgreBites OgreBites.so HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib ${OGRE_ROOT}/lib)
if(_ogrebites_lib)
    list(FIND OGRE_LIBRARIES ${_ogrebites_lib} _idx)
    if(_idx EQUAL -1)
        list(APPEND OGRE_LIBRARIES ${_ogrebites_lib})
        message(STATUS "Explicitly found OgreBites: ${_ogrebites_lib}")
    endif()
else()
    message(STATUS "OgreBites not found by explicit search; OgreBites symbols may be unresolved.")
endif()

# As a last-resort deterministic fallback, append the system path if the file exists.
if(NOT OGRE_LIBRARIES MATCHES ".*OgreBites.*")
    if(USE_DETERMINISTIC_FALLBACKS)
        if(EXISTS "/usr/lib/x86_64-linux-gnu/libOgreBites.so")
            list(APPEND OGRE_LIBRARIES "/usr/lib/x86_64-linux-gnu/libOgreBites.so")
            message(STATUS "Appended deterministic OgreBites path: /usr/lib/x86_64-linux-gnu/libOgreBites.so")
        endif()
    else()
        message(WARNING "OgreBites not found and USE_DETERMINISTIC_FALLBACKS is OFF. Set OGRE_ROOT or install OGRE dev packages.")
    endif()
endif()


# Source files for libvgears and v-gears.
set(VGEARS_SOURCE_FILES
    main.cpp
    FF7Character.cpp
    FF7Common.cpp
    VGearsGameState.cpp
    common/File.cpp
    common/FileSystem.cpp
    common/FinalFantasy7/FF7NameLookup.cpp
    common/LzsFile.cpp
    common/TypeDefine.cpp
    common/VGearsApplication.cpp
    common/VGearsManualObject.cpp
    common/VGearsResource.cpp
    common/VGearsStringUtil.cpp
    core/AudioManager.cpp
    core/Background2DAnimation.cpp
    core/Background2D.cpp
    core/BattleManager.cpp
    core/CameraManager.cpp
    core/ConfigCmd.cpp
    core/ConfigCmdHandler.cpp
    core/ConfigFile.cpp
    core/ConfigVar.cpp
    core/ConfigVarHandler.cpp
    core/Console.cpp
    core/DebugDraw.cpp
    core/DialogsManager.cpp
    core/Enemy.cpp
    core/EntityCollision.cpp
    core/Entity.cpp
    core/EntityDirection.cpp
    core/EntityManager.cpp
    core/EntityModel.cpp
    core/EntityPoint.cpp
    core/EntityTrigger.cpp
    core/GameFrameListener.cpp
    core/InputManager.cpp
    core/Manager.cpp
    core/particles/Particle.cpp
    core/particles/ParticleEmitter.cpp
    core/particles/ParticleEmitterDictionary.cpp
    core/particles/ParticleEmitterTranslator.cpp
    core/particles/ParticleRenderer.cpp
    core/particles/ParticleRendererTranslator.cpp
    core/particles/ParticleSystem.cpp
    core/particles/ParticleSystemFactory.cpp
    core/particles/ParticleSystemManager.cpp
    core/particles/ParticleSystemTranslator.cpp
    core/particles/ParticleSystemTranslatorManager.cpp
    core/particles/ParticleTechnique.cpp
    core/particles/ParticleTechniqueTranslator.cpp
    core/particles/ParticleVisual.cpp
    core/particles/emitters/PointEmitter.cpp
    core/particles/emitters/PointEmitterFactory.cpp
    core/particles/renderer/ParticleEntityRenderer.cpp
    core/particles/renderer/ParticleEntityRendererDictionary.cpp
    core/Savemap.cpp
    core/SavemapHandler.cpp
    core/ScriptManager.cpp
    core/TextHandler.cpp
    core/Timer.cpp
    core/UiAnimation.cpp
    core/UiFont.cpp
    core/UiManager.cpp
    core/UiSprite.cpp
    core/UiTextArea.cpp
    core/UiWidget.cpp
    core/Utilites.cpp
    core/Walkmesh.cpp
    core/WorldMapManager.cpp
    core/XmlBackground2DFile.cpp
    core/XmlBattleCharactersFile.cpp
    core/XmlBattleScenesFile.cpp
    core/XmlEnemyFile.cpp
    core/XmlFile.cpp
    core/XmlFontFile.cpp
    core/XmlFontsFile.cpp
    core/XmlFormationFile.cpp
    core/XmlMapFile.cpp
    core/XmlMapsFile.cpp
    core/XmlMusicsFile.cpp
    core/XmlPrototypesFile.cpp
    core/XmlScreenFile.cpp
    core/XmlScreensFile.cpp
    core/XmlScriptsFile.cpp
    core/XmlSoundsFile.cpp
    core/XmlTextFile.cpp
    core/XmlTextsFile.cpp
    core/XmlWalkmeshFile.cpp
    core/XmlWorldMapFile.cpp
    data/VGearsAFile.cpp
    data/VGearsAFileManager.cpp
    data/VGearsAFileSerializer.cpp
    data/VGearsBackgroundFile.cpp
    data/VGearsBackgroundFileManager.cpp
    data/VGearsBackgroundFileSerializer.cpp
    data/VGearsBackgroundTextureLoader.cpp
    data/VGearsCameraMatrixFile.cpp
    data/VGearsCameraMatrixFileManager.cpp
    data/VGearsCameraMatrixFileSerializer.cpp
    data/VGearsFLevelBackground2DLoader.cpp
    data/VGearsFLevelFile.cpp
    data/VGearsFLevelFileManager.cpp
    data/VGearsFLevelFileSerializer.cpp
    data/VGearsFLevelTextureLoader.cpp
    data/VGearsHRCFile.cpp
    data/VGearsHRCFileManager.cpp
    data/VGearsHRCFileSerializer.cpp
    data/VGearsHRCMeshLoader.cpp
    data/VGearsHRCSkeletonLoader.cpp
    data/VGearsLGPArchive.cpp
    data/VGearsLGPArchiveFactory.cpp
    data/VGearsLGPArchiveSerializer.cpp
    data/VGearsLZSDataStream.cpp
    data/VGearsLZSFLevelFile.cpp
    data/VGearsLZSFLevelFileManager.cpp
    data/VGearsMapListFile.cpp
    data/VGearsPaletteFile.cpp
    data/VGearsPaletteFileManager.cpp
    data/VGearsPaletteFileSerializer.cpp
    data/VGearsPFile.cpp
    data/VGearsPFileManager.cpp
    data/VGearsPFileSerializer.cpp
    data/VGearsRSDFile.cpp
    data/VGearsRSDFileManager.cpp
    data/VGearsRSDFileSerializer.cpp
    data/VGearsSerializer.cpp
    data/VGearsTexCodec.cpp
    data/VGearsTexFile.cpp
    data/VGearsTriggersFile.cpp
    data/VGearsXMLSerializer.cpp
    data/FinalFantasy7/FF7ModelListFile.cpp
    data/FinalFantasy7/FF7ModelListFileManager.cpp
    data/FinalFantasy7/FF7ModelListFileSerializer.cpp
    data/worldmap/WorldmapFile.cpp
    data/worldmap/WorldmapFileManager.cpp
    data/worldmap/WorldmapFileSerializer.cpp
    data/worldmap/TxzFile.cpp
    data/worldmap/TxzFileManager.cpp
    data/worldmap/TxzFileSerializer.cpp
    map/VGearsBackground2DFile.cpp
    map/VGearsBackground2DFileManager.cpp
    map/VGearsBackground2DFileXMLSerializer.cpp
    map/VGearsMapFile.cpp
    map/VGearsMapFileManager.cpp
    map/VGearsMapFileXMLSerializer.cpp
    map/VGearsWalkmeshFile.cpp
    map/VGearsWalkmeshFileManager.cpp
    map/VGearsWalkmeshFileXMLSerializer.cpp
    map/FinalFantasy7/FF7WalkmeshFileSerializer.cpp
    modules/worldmap/WorldmapModule.cpp
    #viewer/ViewerModule.cpp
)


    message(STATUS "Final OGRE_LIBRARIES: ${OGRE_LIBRARIES}")
    endif()
# NOTE: include directories and compile definitions are attached to targets
# later with `target_include_directories` and `target_compile_definitions`.


# Compiler options.
if(APPLE) # Apple specific options
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
endif()
if(WIN32) # Check usage and remove if uneccessary
    add_definitions(-DWIN32 -D__WIN32__ -D_WINDOWS)
    add_definitions(-W -mthreads -fmessage-length=0 -fexceptions -fident)
    # Linker options
    add_definitions(
    -Wl,--enable-runtime-pseudo-reloc
    -Wl,--enable-auto-image-base
    -Wl,--add-stdcall-alias
    )
endif()
if (MSVC) # Build cpp files on all cores, up virtual mem for PCH's as cotire really makes the compiler scream
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Zm193")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
if (UNIX AND $ENV{COVERAGE}==1) # Coverage options (disables optimization)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0") # debug, no optimisation
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # enabling coverage
endif()
# Prefer target-level compile definitions and options below (kept conservative).


# Generate libvgears.
if(NOT VGEARS_SOURCE_FILES)
    file(GLOB_RECURSE VGEARS_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
    list(LENGTH VGEARS_SOURCE_FILES _vgears_auto_count)
    message(STATUS "Auto-collected VGEARS_SOURCE_FILES count: ${_vgears_auto_count}")
endif()

# If installer is disabled, remove installer sources from the collected list
if(NOT BUILD_INSTALLER)
    set(_removed_count 0)
    foreach(_f IN LISTS VGEARS_SOURCE_FILES)
        if(_f MATCHES "installer/")
            list(REMOVE_ITEM VGEARS_SOURCE_FILES ${_f})
            math(EXPR _removed_count "${_removed_count} + 1")
        endif()
    endforeach()
    if(_removed_count GREATER 0)
        message(STATUS "Removed ${_removed_count} installer source(s) because BUILD_INSTALLER=OFF")
    endif()
endif()

# Conservative global Qt include probe (ensure headers visible when
# generating libvgears target). Prefer distro qt5 include locations.
set(_qt_global_candidates /usr/include/x86_64-linux-gnu/qt5 /usr/include/qt5 /usr/include)
foreach(_qinc IN LISTS _qt_global_candidates)
    if(EXISTS "${_qinc}/QtCore/QtGlobal")
        message(STATUS "Global probe: adding Qt include directory ${_qinc}")
        include_directories(BEFORE PRIVATE ${_qinc})
        break()
    endif()
endforeach()
        # Ensure UI headers generated by uic are available before compiling
        # libvgears. Some build order setups (unity/PCH/cotire) may attempt to
        # compile sources that include generated `ui_*.h` before uic runs, so add
        # an explicit custom command/target to generate the header.
        set(_ui_src "${CMAKE_CURRENT_SOURCE_DIR}/installer/MainWindow.ui")
        # Place generated headers under the build tree parallel to the source dir
        # CMAKE_CURRENT_BINARY_DIR is <build>/src so append 'installer' (not 'src/installer')
        set(_ui_out "${CMAKE_CURRENT_BINARY_DIR}/installer/ui_MainWindow.h")
        find_program(QT_UIC_EXECUTABLE NAMES uic uic-qt5 PATHS /usr/lib/qt5/bin /usr/bin /usr/local/bin)
        if(QT_UIC_EXECUTABLE AND EXISTS "${_ui_src}")
            add_custom_command(
                OUTPUT "${_ui_out}"
                COMMAND ${QT_UIC_EXECUTABLE} -o "${_ui_out}" "${_ui_src}"
                DEPENDS "${_ui_src}"
                COMMENT "Generating ui_MainWindow.h via uic"
                VERBATIM
            )
            add_custom_target(generate_ui_files DEPENDS "${_ui_out}")
            set(_GENERATED_UI_TARGET generate_ui_files)
        endif()

        # Exclude unit tests under installer/decompiler/test from the main
        # library to avoid pulling test-only dependencies (gtest/gmock).
        set(_test_removed_count 0)
        foreach(_f IN LISTS VGEARS_SOURCE_FILES)
            if(_f MATCHES "installer/decompiler/test/")
                list(REMOVE_ITEM VGEARS_SOURCE_FILES ${_f})
                math(EXPR _test_removed_count "${_test_removed_count} + 1")
            endif()
        endforeach()
        if(_test_removed_count GREATER 0)
            message(STATUS "Removed ${_test_removed_count} decompiler test source(s) from libvgears")
        endif()

        add_library(libvgears STATIC ${VGEARS_SOURCE_FILES})
            # Ensure generated ui headers are created before compiling libvgears
        if(DEFINED _GENERATED_UI_TARGET)
            add_dependencies(libvgears ${_GENERATED_UI_TARGET})
            # Generated ui headers live in <build>/src/installer
            target_include_directories(libvgears PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/installer")
        endif()
        # Ensure platform macro for scummsys.h selection on Unix
        if(UNIX)
            target_compile_definitions(libvgears PRIVATE POSIX)
        endif()
if(USE_COTIRE)
    cotire(libvgears)
endif()
SET_PROPERTY(TARGET libvgears PROPERTY FOLDER "build/libvgears")

# Attach include directories and compile definitions to `libvgears` target.
target_include_directories(libvgears
    PUBLIC
        ${VGEARS_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/installer
        ${CMAKE_CURRENT_BINARY_DIR}/installer
        ${Boost_INCLUDE_DIR}
        ${OIS_INCLUDE_DIRS}
        ${OGRE_INCLUDE_DIRS}
        ${OGRE_INCLUDE_DIRS}/Overlay
        ${OGRE_INCLUDE_DIRS}/Bites
        ${CMAKE_SOURCE_DIR}/lib/luajit
        ${CMAKE_SOURCE_DIR}/lib/luajit/src
        ${CMAKE_SOURCE_DIR}/lib/luabind
        ${CMAKE_SOURCE_DIR}/lib/luabind/luabind/detail
        ${QT_INCLUDE_DIRS}
)
target_compile_definitions(libvgears PUBLIC TIXML_USE_STL)
if(NOT MSVC)
    target_compile_options(libvgears PRIVATE -fPIC)
endif()

# If Qt link libraries were detected (imported targets or fallback libs),
# link them to libvgears so imported-target include directories are
# propagated to installer/source compilation. This ensures Qt headers are
# available when compiling installer sources into the static lib.
if(QT_LINK_LIBS)
    message(STATUS "Linking libvgears with Qt libraries: ${QT_LINK_LIBS}")
    target_link_libraries(libvgears PUBLIC ${QT_LINK_LIBS})
endif()

# If QT_LINK_LIBS were provided but QT_INCLUDE_DIRS ended up empty (for example
# when generic -l flags were injected), try a conservative probe for common Qt
# include locations and attach them to the target so compiler `-I` flags are
# present when building installer sources.
if(QT_LINK_LIBS AND (NOT QT_INCLUDE_DIRS))
    set(_qt_probe_candidates /usr/include/qt5 /usr/include/x86_64-linux-gnu/qt5 /usr/include)
    foreach(_qi IN LISTS _qt_probe_candidates)
        if(EXISTS "${_qi}/QtCore/QtGlobal")
            message(STATUS "Detected Qt headers under ${_qi}; setting QT_INCLUDE_DIRS")
            set(QT_INCLUDE_DIRS ${_qi} CACHE PATH "Detected Qt include dir" FORCE)
            break()
        endif()
    endforeach()
endif()

if(QT_INCLUDE_DIRS)
    message(STATUS "Adding Qt include dirs to libvgears: ${QT_INCLUDE_DIRS}")
    target_include_directories(libvgears PUBLIC ${QT_INCLUDE_DIRS})
endif()


# Generate v-gears executable
add_executable(v-gears ${VGEARS_SOURCE_FILES})
SET_PROPERTY(TARGET v-gears PROPERTY FOLDER "build/v-gears")
set(CPACK_PACKAGE_EXECUTABLES v-gears "v-gears")
set(CPACK_WIX_PROGRAM_MENU_FOLDER "VGears")
if(APPLE)
    target_link_libraries(v-gears "-framework CoreFoundation -framework Cocoa -framework IOKit" ${ois_lib})
endif()

if(WIN32 AND NOT (CMAKE_BUILD_TYPE MATCHES "Debug"))
    set_target_properties( v-gears PROPERTIES WIN32_EXECUTABLE ON )
    if (MSVC)
        set_target_properties(v-gears PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    endif()
endif()
set(VGEARS_LIBRARIES
    libluabind
    liblua
    ${OPENAL_LIBRARY}
    ${OGGVORBIS_LIBRARIES}
    ${OGRE_LIBRARIES}
)

# Ensure we have something for Boost; if BOOST_LINK_LIBS is empty, inject generic
# `-lboost_*` flags directly so the system linker will attempt to resolve them from
# standard library search paths.
if(NOT BOOST_LINK_LIBS)
    if(USE_DETERMINISTIC_FALLBACKS)
        set(_generic_boost_flags -lboost_program_options -lboost_filesystem -lboost_thread -lboost_system -lboost_chrono)
        message(STATUS "Injecting generic Boost -l flags into link list: ${_generic_boost_flags}")
        list(APPEND BOOST_LINK_LIBS ${_generic_boost_flags})
    else()
        message(WARNING "Boost not found and deterministic fallbacks disabled; build may fail. Install libboost-*-dev or provide Boost hints.")
    endif()
endif()

# `BOOST_LINK_LIBS` should have been computed earlier (imported targets, find_package,
# or the robust per-component fallback). Append whatever was found into the final
# link list for the executable.
list(APPEND VGEARS_LIBRARIES ${BOOST_LINK_LIBS} ${OIS_LIBRARIES} ${TinyXML_LIBRARIES})
if(QT_LINK_LIBS)
    list(APPEND VGEARS_LIBRARIES ${QT_LINK_LIBS})
endif()
message(STATUS "Resolved library variables:")
message(STATUS "  OPENAL_LIBRARY=${OPENAL_LIBRARY}")
message(STATUS "  OGGVORBIS_LIBRARIES=${OGGVORBIS_LIBRARIES}")
message(STATUS "  OGRE_LIBRARIES=${OGRE_LIBRARIES}")
message(STATUS "  Boost_LIBRARIES=${Boost_LIBRARIES}")
message(STATUS "  OIS_LIBRARIES=${OIS_LIBRARIES}")
message(STATUS "  TinyXML_LIBRARIES=${TinyXML_LIBRARIES}")
if(MULTITHREADING)
    find_package (Threads REQUIRED) # Find Threads
    set(VGEARS_LIBRARIES
        ${VGEARS_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )
endif()
message(STATUS "Final VGEARS_LIBRARIES list: ${VGEARS_LIBRARIES}")
# If OgreBites SONAME exists on the system but wasn't picked up earlier,
# append it directly to the final link list so WindowEventUtilities symbols
# are resolved at link time.
if(USE_DETERMINISTIC_FALLBACKS)
    if(EXISTS "/usr/lib/x86_64-linux-gnu/libOgreBites.so")
        list(FIND VGEARS_LIBRARIES "/usr/lib/x86_64-linux-gnu/libOgreBites.so" _found_bites)
        if(_found_bites EQUAL -1)
            list(APPEND VGEARS_LIBRARIES "/usr/lib/x86_64-linux-gnu/libOgreBites.so")
            message(STATUS "Appending system OgreBites to VGEARS_LIBRARIES: /usr/lib/x86_64-linux-gnu/libOgreBites.so")
        endif()
    endif()
else()
    # When deterministic fallbacks are disabled, warn if OgreBites is likely missing.
    if(NOT OGRE_LIBRARIES MATCHES ".*OgreBites.*")
        message(WARNING "OgreBites not present in OGRE_LIBRARIES and deterministic fallbacks are disabled. Link errors may occur if OGRE dev packages are not installed.")
    endif()
endif()

target_link_libraries(v-gears ${VGEARS_LIBRARIES})

# Attach include dirs/defs/options to executable as well (uses same sources).
target_include_directories(v-gears
    PRIVATE
        ${VGEARS_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Boost_INCLUDE_DIR}
        ${OIS_INCLUDE_DIRS}
        ${OGRE_INCLUDE_DIRS}
        ${OGRE_INCLUDE_DIRS}/Overlay
        ${OGRE_INCLUDE_DIRS}/Bites
        ${CMAKE_SOURCE_DIR}/lib/luajit
        ${CMAKE_SOURCE_DIR}/lib/luajit/src
        ${CMAKE_SOURCE_DIR}/lib/luabind
        ${CMAKE_SOURCE_DIR}/lib/luabind/luabind/detail
    ${QT_INCLUDE_DIRS}
)
target_compile_definitions(v-gears PRIVATE TIXML_USE_STL)
if(NOT MSVC)
    target_compile_options(v-gears PRIVATE -fPIC)
endif()


# Install v-gears.
if(WIN32 OR APPLE)
    install(TARGETS v-gears DESTINATION .)
else()
    install(TARGETS v-gears RUNTIME DESTINATION ./games)
endif()



# If installer build is enabled, ensure Qt includes/links are available for
# the `installer` subdirectory. Do this before adding the subdirectory so
# its targets inherit the settings.
if(BUILD_INSTALLER)
    # If pragmatic detection earlier set `QT_INCLUDE_DIRS`, expose them as
    # global include dirs so installer targets find headers.
    if(QT_INCLUDE_DIRS)
        include_directories(${QT_INCLUDE_DIRS})
    else()
        # Last-resort add common system Qt include paths.
        if(EXISTS "/usr/include/qt5/QtCore/QDir")
            include_directories(/usr/include/qt5 /usr/include/x86_64-linux-gnu/qt5)
            message(STATUS "Added generic Qt include paths: /usr/include/qt5")
        endif()
    endif()
    # Make sure any QT_LINK_LIBS are visible to subdirectories by appending
    # them to the global link list variable used when creating targets.
    if(QT_LINK_LIBS)
        list(APPEND VGEARS_LIBRARIES ${QT_LINK_LIBS})
    endif()

    message(STATUS "Building installer as requested (BUILD_INSTALLER=ON)")
    add_subdirectory(installer)
endif()
