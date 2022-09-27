# Define header and source files for V-Gears
set(HEADER_FILES
    include/modules/worldmap/WorldMapModule.h
    include/common/VGearsManualObject.h
    include/common/VGearsResource.h
    include/common/VGearsStringUtil.h
    include/common/TypeDefine.h
    include/common/LzsFile.h
    include/common/File.h
    include/common/FileSystem.h
    include/common/make_unique.h
    include/map/VGearsBackground2DFile.h
    include/map/VGearsBackground2DFileManager.h
    include/map/VGearsBackground2DFileXMLSerializer.h
    include/map/VGearsTile.h
    include/map/VGearsMapFile.h
    include/map/VGearsMapFileManager.h
    include/map/VGearsMapFileXMLSerializer.h
    include/map/VGearsWalkmeshFile.h
    include/map/VGearsWalkmeshFileManager.h
    include/map/VGearsWalkmeshFileXMLSerializer.h
    include/LuaIncludes.h
    include/VGearsGameState.h
    include/VGearsPrerequisites.h
    include/core/particles/emitters/ParticlePointEmitter.h
    include/core/particles/emitters/ParticlePointEmitterFactory.h
    include/core/particles/renderer/ParticleEntityAdditionalData.h
    include/core/particles/renderer/ParticleEntityRenderer.h
    include/core/particles/renderer/ParticleEntityRendererDictionary.h
    include/core/particles/renderer/ParticleEntityRendererFactory.h
    include/core/particles/Particle.h
    include/core/particles/ParticleAdditionalData.h
    include/core/particles/ParticleEmitter.h
    include/core/particles/ParticleEmitterDictionary.h
    include/core/particles/ParticleEmitterFactory.h
    include/core/particles/ParticleEmitterTranslator.h
    include/core/particles/ParticlePool.h
    include/core/particles/ParticlePoolMap.h
    include/core/particles/ParticleRenderer.h
    include/core/particles/ParticleRendererFactory.h
    include/core/particles/ParticleRendererTranslator.h
    include/core/particles/ParticleSystem.h
    include/core/particles/ParticleSystemFactory.h
    include/core/particles/ParticleSystemManager.h
    include/core/particles/ParticleSystemTranslator.h
    include/core/particles/ParticleSystemTranslatorManager.h
    include/core/particles/ParticleTechnique.h
    include/core/particles/ParticleTechniqueTranslator.h
    include/core/particles/ParticleVisual.h
    include/core/Assert.h
    include/core/Background2D.h
    include/core/Background2DAnimation.h
    include/core/CameraManager.h
    include/core/CameraManagerCommands.h
    include/core/ConfigCmd.h
    include/core/ConfigCmdManager.h
    include/core/ConfigCmdManagerCommands.h
    include/core/ConfigFile.h
    include/core/ConfigVar.h
    include/core/ConfigVarManager.h
    include/core/Console.h
    include/core/DebugDraw.h
    include/core/Entity.h
    include/core/EntityCollision.h
    include/core/EntityDirection.h
    include/core/EntityManager.h
    include/core/EntityModel.h
    include/core/EntityPoint.h
    include/core/EntityTrigger.h
    include/core/Event.h
    include/core/GameFrameListener.h
    include/core/InputManager.h
    include/core/InputManagerCommands.h
    include/core/Logger.h
    include/core/Module.h
    include/core/ScriptManager.h
    include/core/ScriptManagerBinds.h
    include/core/ScriptManagerCommands.h
    include/core/Timer.h
    include/core/UiAnimation.h
    include/core/UiFont.h
    include/core/UiManager.h
    include/core/UiSprite.h
    include/core/UiTextArea.h
    include/core/UiWidget.h
    include/core/Utilites.h
    include/core/Walkmesh.h
    include/core/XmlBackground2DFile.h
    include/core/XmlFile.h
    include/core/XmlFontFile.h
    include/core/XmlFontsFile.h
    include/core/XmlMapFile.h
    include/core/XmlPrototypesFile.h
    include/core/XmlScreenFile.h
    include/core/XmlScreensFile.h
    include/core/XmlScriptsFile.h
    include/core/XmlTextFile.h
    include/core/XmlTextsFile.h
    include/core/DialogsManager.h
    include/core/ScriptManagerBinds.h
    include/core/TextManager.h
    include/core/TextManagerCommands.h
    include/data/VGearsXMLSerializer.h
    include/common/FinalFantasy7/FF7NameLookup.h
    include/map/FinalFantasy7/FF7WalkmeshFileSerializer.h
    include/FF7Character.h
    include/FF7Common.h
)

set(HEADER_FILES_DATA
    include/common/VGearsApplication.h
    include/data/worldmap/MapFile.h
    include/data/worldmap/MapFileManager.h
    include/data/worldmap/MapFileSerializer.h
    include/data/worldmap/TxzFile.h
    include/data/worldmap/TxzFileManager.h
    include/data/worldmap/TxzFileSerializer.h
    include/data/VGearsAFile.h
    include/data/VGearsAFileManager.h
    include/data/VGearsAFileSerializer.h
    include/data/VGearsBackgroundFile.h
    include/data/VGearsBackgroundFileManager.h
    include/data/VGearsBackgroundFileSerializer.h
    include/data/VGearsBackgroundTextureLoader.h
    include/data/VGearsCameraMatrixFile.h
    include/data/VGearsCameraMatrixFileManager.h
    include/data/VGearsCameraMatrixFileSerializer.h
    include/data/VGearsFLevelBackground2DLoader.h
    include/data/VGearsFLevelFile.h
    include/data/VGearsFLevelFileManager.h
    include/data/VGearsFLevelFileSerializer.h
    include/data/VGearsFLevelTextureLoader.h
    include/data/VGearsHRCFile.h
    include/data/VGearsHRCFileManager.h
    include/data/VGearsHRCFileSerializer.h
    include/data/VGearsHRCMeshLoader.h
    include/data/VGearsHRCSkeletonLoader.h
    include/data/VGearsLGPArchive.h
    include/data/VGearsLGPArchiveFactory.h
    include/data/VGearsLGPArchiveSerializer.h
    include/data/VGearsLZSDataStream.h
    include/data/VGearsLZSFLevelFile.h
    include/data/VGearsLZSFLevelFileManager.h
    include/data/VGearsPaletteFile.h
    include/data/VGearsPaletteFileManager.h
    include/data/VGearsPaletteFileSerializer.h
    include/data/VGearsPFile.h
    include/data/VGearsPFileManager.h
    include/data/VGearsPFileSerializer.h
    include/data/VGearsRSDFile.h
    include/data/VGearsRSDFileManager.h
    include/data/VGearsRSDFileSerializer.h
    include/data/VGearsSerializer.h
    include/data/VGearsTexCodec.h
    include/data/VGearsTexFile.h
    include/data/VGearsTriggersFile.h
    include/data/VGearsMapListFile.h
    include/data/FinalFantasy7/FF7ModelListFile.h
    include/data/FinalFantasy7/FF7ModelListFileManager.h
    include/data/FinalFantasy7/FF7ModelListFileSerializer.h
)

set(SOURCE_FILES_DATA
    src/common/VGearsApplication.cpp
    src/data/VGearsAFile.cpp
    src/data/VGearsAFileManager.cpp
    src/data/VGearsAFileSerializer.cpp
    src/data/worldmap/MapFile.cpp
    src/data/worldmap/MapFileManager.cpp
    src/data/worldmap/MapFileSerializer.cpp
    src/data/worldmap/TxzFile.cpp
    src/data/worldmap/TxzFileManager.cpp
    src/data/worldmap/TxzFileSerializer.cpp
    src/data/VGearsBackgroundFile.cpp
    src/data/VGearsBackgroundFileManager.cpp
    src/data/VGearsBackgroundFileSerializer.cpp
    src/data/VGearsBackgroundTextureLoader.cpp
    src/data/VGearsCameraMatrixFile.cpp
    src/data/VGearsCameraMatrixFileManager.cpp
    src/data/VGearsCameraMatrixFileSerializer.cpp
    src/data/VGearsFLevelBackground2DLoader.cpp
    src/data/VGearsFLevelFile.cpp
    src/data/VGearsFLevelFileManager.cpp
    src/data/VGearsFLevelFileSerializer.cpp
    src/data/VGearsFLevelTextureLoader.cpp
    src/data/VGearsHRCFile.cpp
    src/data/VGearsHRCFileManager.cpp
    src/data/VGearsHRCFileSerializer.cpp
    src/data/VGearsHRCMeshLoader.cpp
    src/data/VGearsHRCSkeletonLoader.cpp
    src/data/VGearsLGPArchive.cpp
    src/data/VGearsLGPArchiveFactory.cpp
    src/data/VGearsLGPArchiveSerializer.cpp
    src/data/VGearsLZSDataStream.cpp
    src/data/VGearsLZSFLevelFile.cpp
    src/data/VGearsLZSFLevelFileManager.cpp
    src/data/VGearsPaletteFile.cpp
    src/data/VGearsPaletteFileManager.cpp
    src/data/VGearsPaletteFileSerializer.cpp
    src/data/VGearsPFile.cpp
    src/data/VGearsPFileManager.cpp
    src/data/VGearsPFileSerializer.cpp
    src/data/VGearsRSDFile.cpp
    src/data/VGearsRSDFileManager.cpp
    src/data/VGearsRSDFileSerializer.cpp
    src/data/VGearsSerializer.cpp
    src/data/VGearsTexCodec.cpp
    src/data/VGearsTexFile.cpp
    src/data/VGearsTriggersFile.cpp
    src/data/VGearsMapListFile.cpp
    src/data/FinalFantasy7/FF7ModelListFile.cpp
    src/data/FinalFantasy7/FF7ModelListFileManager.cpp
    src/data/FinalFantasy7/FF7ModelListFileSerializer.cpp
)

set(SOURCE_FILES
    src/modules/worldmap/WorldMapModule.cpp
    src/common/VGearsManualObject.cpp
    src/common/VGearsResource.cpp
    src/common/VGearsStringUtil.cpp
    src/common/TypeDefine.cpp
    src/common/LzsFile.cpp
    src/common/File.cpp
    src/common/FileSystem.cpp
    src/core/particles/emitters/ParticlePointEmitter.cpp
    src/core/particles/Particle.cpp
    src/core/particles/ParticleEmitter.cpp
    src/core/particles/ParticleEmitterDictionary.cpp
    src/core/particles/ParticleEmitterTranslator.cpp
    src/core/particles/ParticleRenderer.cpp
    src/core/particles/ParticleRendererTranslator.cpp
    src/core/particles/ParticleSystem.cpp
    src/core/particles/ParticleSystemFactory.cpp
    src/core/particles/ParticleSystemManager.cpp
    src/core/particles/ParticleSystemTranslator.cpp
    src/core/particles/ParticleSystemTranslatorManager.cpp
    src/core/particles/ParticleTechnique.cpp
    src/core/particles/ParticleTechniqueTranslator.cpp
    src/core/particles/ParticleVisual.cpp
    src/core/particles/renderer/ParticleEntityRenderer.cpp
    src/core/particles/renderer/ParticleEntityRendererDictionary.cpp
    src/core/Background2D.cpp
    src/core/Background2DAnimation.cpp
    src/core/CameraManager.cpp
    src/core/ConfigCmd.cpp
    src/core/ConfigCmdManager.cpp
    src/core/ConfigFile.cpp
    src/core/ConfigVar.cpp
    src/core/ConfigVarManager.cpp
    src/core/Console.cpp
    src/core/DebugDraw.cpp
    src/core/Entity.cpp
    src/core/EntityCollision.cpp
    src/core/EntityDirection.cpp
    src/core/EntityManager.cpp
    src/core/EntityModel.cpp
    src/core/EntityPoint.cpp
    src/core/EntityTrigger.cpp
    src/core/GameFrameListener.cpp
    src/core/InputManager.cpp
    src/core/ScriptManager.cpp
    src/core/Timer.cpp
    src/core/UiAnimation.cpp
    src/core/UiFont.cpp
    src/core/UiManager.cpp
    src/core/UiSprite.cpp
    src/core/UiTextArea.cpp
    src/core/UiWidget.cpp
    src/core/Utilites.cpp
    src/core/Walkmesh.cpp
    src/core/XmlBackground2DFile.cpp
    src/core/XmlFile.cpp
    src/core/XmlFontFile.cpp
    src/core/XmlFontsFile.cpp
    src/core/XmlMapFile.cpp
    src/core/XmlMapsFile.cpp
    src/core/XmlPrototypesFile.cpp
    src/core/XmlScreenFile.cpp
    src/core/XmlScreensFile.cpp
    src/core/XmlScriptsFile.cpp
    src/core/XmlTextFile.cpp
    src/core/XmlTextsFile.cpp
    src/core/DialogsManager.cpp
    src/core/TextManager.cpp
    src/map/VGearsBackground2DFile.cpp
    src/map/VGearsBackground2DFileManager.cpp
    src/map/VGearsBackground2DFileXMLSerializer.cpp
    src/map/VGearsMapFile.cpp
    src/map/VGearsMapFileManager.cpp
    src/map/VGearsMapFileXMLSerializer.cpp
    src/map/VGearsWalkmeshFile.cpp
    src/map/VGearsWalkmeshFileManager.cpp
    src/map/VGearsWalkmeshFileXMLSerializer.cpp
    #src/viewer/ViewerModule.cpp
    src/VGearsGameState.cpp
    src/data/VGearsXMLSerializer.cpp
    src/common/FinalFantasy7/FF7NameLookup.cpp
    src/map/FinalFantasy7/FF7WalkmeshFileSerializer.cpp
    src/FF7Character.cpp
    src/FF7Common.cpp
)

# If sound, add more files.
if(VGears_SOUND)
    set(HEADER_FILES
        ${HEADER_FILES}
        include/core/AudioManager.h
        include/core/XmlMusicsFile.h
    )
    set(SOURCE_FILES
        ${SOURCE_FILES}
        src/core/AudioManager.cpp
        src/core/XmlMusicsFile.cpp
    )
endif()

# Find LibOIS. TODO: Find it.
set(ois_lib "/usr/lib/x86_64-linux-gnu/libOIS.so")

# TODO: Find OIS
include_directories(
    ${VGears_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /usr/include/OIS/

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

add_executable(v-gears src/Main.cpp)
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
    LuaBind
    liblua
    tinyxml
    ${ois_lib}
)

target_link_libraries(v-gears 
    V-Gears
    ${Boost_LIBRARIES}
    v-gears-data
    ${ois_lib}
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