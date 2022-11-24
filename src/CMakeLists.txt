# Define header and source files for V-Gears
set(HEADER_FILES
    modules/worldmap/WorldMapModule.h
    common/VGearsManualObject.h
    common/VGearsResource.h
    common/VGearsStringUtil.h
    common/TypeDefine.h
    common/LzsFile.h
    common/File.h
    common/FileSystem.h
    map/VGearsBackground2DFile.h
    map/VGearsBackground2DFileManager.h
    map/VGearsBackground2DFileXMLSerializer.h
    map/VGearsTile.h
    map/VGearsMapFile.h
    map/VGearsMapFileManager.h
    map/VGearsMapFileXMLSerializer.h
    map/VGearsWalkmeshFile.h
    map/VGearsWalkmeshFileManager.h
    map/VGearsWalkmeshFileXMLSerializer.h
    LuaIncludes.h
    VGearsGameState.h
    VGearsPrerequisites.h
    core/particles/emitters/PointEmitter.h
    core/particles/emitters/PointEmitterFactory.h
    core/particles/renderer/ParticleEntityAdditionalData.h
    core/particles/renderer/ParticleEntityRenderer.h
    core/particles/renderer/ParticleEntityRendererDictionary.h
    core/particles/renderer/ParticleEntityRendererFactory.h
    core/particles/Particle.h
    core/particles/ParticleAdditionalData.h
    core/particles/ParticleEmitter.h
    core/particles/ParticleEmitterDictionary.h
    core/particles/ParticleEmitterFactory.h
    core/particles/ParticleEmitterTranslator.h
    core/particles/ParticlePool.h
    core/particles/ParticlePoolMap.h
    core/particles/ParticleRenderer.h
    core/particles/ParticleRendererFactory.h
    core/particles/ParticleRendererTranslator.h
    core/particles/ParticleSystem.h
    core/particles/ParticleSystemFactory.h
    core/particles/ParticleSystemManager.h
    core/particles/ParticleSystemTranslator.h
    core/particles/ParticleSystemTranslatorManager.h
    core/particles/ParticleTechnique.h
    core/particles/ParticleTechniqueTranslator.h
    core/particles/ParticleVisual.h
    core/Assert.h
    core/AudioManager.h
    core/Background2D.h
    core/Background2DAnimation.h
    core/CameraManager.h
    core/CameraManagerCommands.h
    core/ConfigCmd.h
    core/ConfigCmdManager.h
    core/ConfigCmdManagerCommands.h
    core/ConfigFile.h
    core/ConfigVar.h
    core/ConfigVarManager.h
    core/Console.h
    core/DebugDraw.h
    core/Entity.h
    core/EntityCollision.h
    core/EntityDirection.h
    core/EntityManager.h
    core/EntityModel.h
    core/EntityPoint.h
    core/EntityTrigger.h
    core/Event.h
    core/GameFrameListener.h
    core/InputManager.h
    core/InputManagerCommands.h
    core/Logger.h
    core/Module.h
    core/Savemap.h
    core/SavemapManager.h
    core/ScriptManager.h
    core/ScriptManagerBinds.h
    core/ScriptManagerCommands.h
    core/Timer.h
    core/UiAnimation.h
    core/UiFont.h
    core/UiManager.h
    core/UiSprite.h
    core/UiTextArea.h
    core/UiWidget.h
    core/Utilites.h
    core/Walkmesh.h
    core/XmlBackground2DFile.h
    core/XmlFile.h
    core/XmlFontFile.h
    core/XmlFontsFile.h
    core/XmlMapFile.h
    core/XmlMusicsFile.h
    core/XmlPrototypesFile.h
    core/XmlScreenFile.h
    core/XmlScreensFile.h
    core/XmlScriptsFile.h
    core/XmlSoundsFile.h
    core/XmlTextFile.h
    core/XmlTextsFile.h
    core/DialogsManager.h
    core/ScriptManagerBinds.h
    core/TextManager.h
    core/TextManagerCommands.h
    data/VGearsXMLSerializer.h
    common/FinalFantasy7/FF7NameLookup.h
    map/FinalFantasy7/FF7WalkmeshFileSerializer.h
    FF7Character.h
    FF7Common.h
)

set(HEADER_FILES_DATA
    common/VGearsApplication.h
    data/worldmap/MapFile.h
    data/worldmap/MapFileManager.h
    data/worldmap/MapFileSerializer.h
    data/worldmap/TxzFile.h
    data/worldmap/TxzFileManager.h
    data/worldmap/TxzFileSerializer.h
    data/VGearsAFile.h
    data/VGearsAFileManager.h
    data/VGearsAFileSerializer.h
    data/VGearsBackgroundFile.h
    data/VGearsBackgroundFileManager.h
    data/VGearsBackgroundFileSerializer.h
    data/VGearsBackgroundTextureLoader.h
    data/VGearsCameraMatrixFile.h
    data/VGearsCameraMatrixFileManager.h
    data/VGearsCameraMatrixFileSerializer.h
    data/VGearsFLevelBackground2DLoader.h
    data/VGearsFLevelFile.h
    data/VGearsFLevelFileManager.h
    data/VGearsFLevelFileSerializer.h
    data/VGearsFLevelTextureLoader.h
    data/VGearsHRCFile.h
    data/VGearsHRCFileManager.h
    data/VGearsHRCFileSerializer.h
    data/VGearsHRCMeshLoader.h
    data/VGearsHRCSkeletonLoader.h
    data/VGearsLGPArchive.h
    data/VGearsLGPArchiveFactory.h
    data/VGearsLGPArchiveSerializer.h
    data/VGearsLZSDataStream.h
    data/VGearsLZSFLevelFile.h
    data/VGearsLZSFLevelFileManager.h
    data/VGearsPaletteFile.h
    data/VGearsPaletteFileManager.h
    data/VGearsPaletteFileSerializer.h
    data/VGearsPFile.h
    data/VGearsPFileManager.h
    data/VGearsPFileSerializer.h
    data/VGearsRSDFile.h
    data/VGearsRSDFileManager.h
    data/VGearsRSDFileSerializer.h
    data/VGearsSerializer.h
    data/VGearsTexCodec.h
    data/VGearsTexFile.h
    data/VGearsTriggersFile.h
    data/VGearsMapListFile.h
    data/FinalFantasy7/FF7ModelListFile.h
    data/FinalFantasy7/FF7ModelListFileManager.h
    data/FinalFantasy7/FF7ModelListFileSerializer.h
)

set(SOURCE_FILES_DATA
    common/VGearsApplication.cpp
    data/VGearsAFile.cpp
    data/VGearsAFileManager.cpp
    data/VGearsAFileSerializer.cpp
    data/worldmap/MapFile.cpp
    data/worldmap/MapFileManager.cpp
    data/worldmap/MapFileSerializer.cpp
    data/worldmap/TxzFile.cpp
    data/worldmap/TxzFileManager.cpp
    data/worldmap/TxzFileSerializer.cpp
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
    data/VGearsMapListFile.cpp
    data/FinalFantasy7/FF7ModelListFile.cpp
    data/FinalFantasy7/FF7ModelListFileManager.cpp
    data/FinalFantasy7/FF7ModelListFileSerializer.cpp
)

set(SOURCE_FILES
    modules/worldmap/WorldMapModule.cpp
    common/VGearsManualObject.cpp
    common/VGearsResource.cpp
    common/VGearsStringUtil.cpp
    common/TypeDefine.cpp
    common/LzsFile.cpp
    common/File.cpp
    common/FileSystem.cpp
    core/particles/emitters/PointEmitter.cpp
    core/particles/emitters/PointEmitterFactory.cpp
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
    core/particles/renderer/ParticleEntityRenderer.cpp
    core/particles/renderer/ParticleEntityRendererDictionary.cpp
    core/AudioManager.cpp
    core/Background2D.cpp
    core/Background2DAnimation.cpp
    core/CameraManager.cpp
    core/ConfigCmd.cpp
    core/ConfigCmdManager.cpp
    core/ConfigFile.cpp
    core/ConfigVar.cpp
    core/ConfigVarManager.cpp
    core/Console.cpp
    core/DebugDraw.cpp
    core/Entity.cpp
    core/EntityCollision.cpp
    core/EntityDirection.cpp
    core/EntityManager.cpp
    core/EntityModel.cpp
    core/EntityPoint.cpp
    core/EntityTrigger.cpp
    core/GameFrameListener.cpp
    core/InputManager.cpp
    core/Savemap.cpp
    core/SavemapManager.cpp
    core/ScriptManager.cpp
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
    core/XmlFile.cpp
    core/XmlFontFile.cpp
    core/XmlFontsFile.cpp
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
    core/DialogsManager.cpp
    core/TextManager.cpp
    map/VGearsBackground2DFile.cpp
    map/VGearsBackground2DFileManager.cpp
    map/VGearsBackground2DFileXMLSerializer.cpp
    map/VGearsMapFile.cpp
    map/VGearsMapFileManager.cpp
    map/VGearsMapFileXMLSerializer.cpp
    map/VGearsWalkmeshFile.cpp
    map/VGearsWalkmeshFileManager.cpp
    map/VGearsWalkmeshFileXMLSerializer.cpp
    VGearsGameState.cpp
    data/VGearsXMLSerializer.cpp
    common/FinalFantasy7/FF7NameLookup.cpp
    map/FinalFantasy7/FF7WalkmeshFileSerializer.cpp
    FF7Character.cpp
    FF7Common.cpp
)

# Maximum number of parameters in functions bind to Lua.
# Some of them have quite many of them, so keep it high.
#add_definitions(-DLUABIND_MAX_ARITY=32)

# Find LibOIS. TODO: Find it.
set(ois_lib "/usr/lib/x86_64-linux-gnu/libOIS.so")

set(luabind_lib "/usr/lib/x86_64-linux-gnu/libluabind.so")
#set(luabind_lib "/usr/lib/x86_64-linux-gnu/liblua5.1.so")
set(luajit_lib "/usr/lib/x86_64-linux-gnu/libluajit-5.1.a")
set(lua_lib "/usr/lib/x86_64-linux-gnu/liblua5.2.so")

# TODO: Find OIS
include_directories(
    ${VGears_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /usr/include/OIS/
    /usr/include/luabind/
    /usr/include/luajit-2.1/
)

# Compiler options
if (MSVC)
else()
    if(WIN32)
        add_definitions(
            -W
            -mthreads
            -fmessage-length=0
            -fexceptions
            -fident
        )
    
        # Linker options
        add_definitions(
            -Wl,--enable-runtime-pseudo-reloc
            -Wl,--enable-auto-image-base
            -Wl,--add-stdcall-alias
        )
    endif()
endif()

# Defines.
if(WIN32)
    # Check usage and remove if uneccessary
    add_definitions(-DWIN32 -D__WIN32__ -D_WINDOWS)
endif()

# TODO: Find OIS
add_library(V-Gears STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
    /usr/include/OIS/
)

# Coverage uses more memory, combined with PCH can cause OOM/ICE
#if ($ENV{COVERAGE}!=1)
cotire(V-Gears)
#endif()
SET_PROPERTY(TARGET V-Gears PROPERTY FOLDER "v-gears")

add_library(v-gears-data STATIC
    ${HEADER_FILES_DATA}
    ${SOURCE_FILES_DATA}
)
add_dependencies(v-gears-data configure_vgears_version)
# Coverage uses more memory, combined with PCH can cause OOM/ICE
if ($ENV{COVERAGE}!=1)
    cotire(v-gears-data)
endif()
SET_PROPERTY(TARGET v-gears-data PROPERTY FOLDER "v-gears")

add_executable(v-gears Main.cpp)
SET_PROPERTY(TARGET v-gears PROPERTY FOLDER "v-gears")
set(CPACK_PACKAGE_EXECUTABLES v-gears "v-gears")
set(CPACK_WIX_PROGRAM_MENU_FOLDER "VGears")

if (APPLE)
    target_link_libraries(v-gears "-framework CoreFoundation -framework Cocoa -framework IOKit" ${ois_lib})
endif()

if( WIN32 AND NOT (CMAKE_BUILD_TYPE MATCHES "Debug") )
    set_target_properties( v-gears PROPERTIES WIN32_EXECUTABLE ON )
    if (MSVC)
        set_target_properties(v-gears PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    endif()
endif()

target_link_libraries(V-Gears 
    ${VGears_LIBRARIES}
    #${lua_lib}
    luabind
    #lua
    tinyxml
    ${ois_lib}
    ${luabind_lib}
    ${luajit_lib}
)

target_link_libraries(v-gears 
    V-Gears
    ${Boost_LIBRARIES}
    v-gears-data
    ${ois_lib}
    ${luabind_lib}
)

target_link_libraries(v-gears-data
    V-Gears
    ${ois_lib}
)

if(WIN32 OR APPLE)
    install(TARGETS v-gears DESTINATION .)
else()
    install(TARGETS v-gears RUNTIME DESTINATION ./games)
endif()
