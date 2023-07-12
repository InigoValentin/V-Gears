# Find Ogre module.
set(CMAKE_FRAMEWORK_PATH
    ${CMAKE_FRAMEWORK_PATH}
    ${CMAKE_SOURCE_DIR}/OgreSDK/lib/macosx/Release
    ${CMAKE_SOURCE_DIR}/OgreSDK/lib/macosx/Debug
)
set(OGRE_FRAMEWORK_INCLUDES ${CMAKE_SOURCE_DIR}/OgreSDK/include)
find_path(OGRE_CMAKE_MODULE_PATH FindOGRE.cmake
    HINTS
    "$ENV{OGRE_HOME}/CMake/"
    "/usr/local/lib/OGRE/cmake"
    "/usr/lib/OGRE/cmake"
    "/usr/share/OGRE/cmake/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/OgreSDK/CMake"
)
if(OGRE_CMAKE_MODULE_PATH-NOTFOUND)
    message(SEND_ERROR "Failed to find OGRE module path.")
else()
    set(CMAKE_MODULE_PATH "${OGRE_CMAKE_MODULE_PATH};${CMAKE_MODULE_PATH}")
endif()
set(CMAKE_MODULE_PATH "${OGRE_CMAKE_MODULE_PATH};${CMAKE_MODULE_PATH} ")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/CMake")


# Find Boost module.
if (WIN32)
    set(Boost_USE_STATIC_LIBS TRUE)
else()
    # Statically linking boost to a dynamic Ogre build doesn't work on Linux 64bit.
    set(Boost_USE_STATIC_LIBS ${OGRE_STATIC})
endif ()
if (MINGW)
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
    message(SEND_ERROR "Boost_LIBRARIES is not set")
endif()


# Find packacges required for V-Gears.
find_package(OGRE REQUIRED QUIET)
find_package(OIS REQUIRED)
find_package(OpenAL REQUIRED)
find_package(OggVorbis REQUIRED)
find_package(TinyXML REQUIRED)
find_package(Boost COMPONENTS program_options filesystem thread REQUIRED QUIET)


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


# Directories for compiling libvgears and v-gears.
include_directories(
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
)


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
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC") # Position Independent Code, required if QT is compiled with -reduce-relocations
#if(CMAKE_BUILD_TYPE MATCHES "Debug") # Debug build
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic -O0")
#endif()
add_definitions(-DTIXML_USE_STL)


# Generate libvgears.
add_library(libvgears STATIC ${VGEARS_SOURCE_FILES})
cotire(libvgears)
SET_PROPERTY(TARGET libvgears PROPERTY FOLDER "build/libvgears")


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
    ${Boost_LIBRARIES}
    Boost::thread
    ${OIS_LIBRARIES}
    ${TinyXML_LIBRARIES}
)
if(MULTITHREADING)
    find_package (Threads REQUIRED) # Find Threads
    set(VGEARS_LIBRARIES
        ${VGEARS_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )
endif()
target_link_libraries(v-gears ${VGEARS_LIBRARIES})


# Install v-gears.
if(WIN32 OR APPLE)
    install(TARGETS v-gears DESTINATION .)
else()
    install(TARGETS v-gears RUNTIME DESTINATION ./games)
endif()


# If requested, build the installer.
if(BUILD_INSTALLER)
    add_subdirectory(installer)
endif()
