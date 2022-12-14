# Find packages required to compile tests.
find_package(Boost COMPONENTS unit_test_framework REQUIRED)


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
    VGearsGameState.cpp
    FF7Character.cpp
    FF7Common.cpp
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
    data/FinalFantasy7/FF7ModelListFileManager.cpp
    data/FinalFantasy7/FF7ModelListFileSerializer.cpp
    data/FinalFantasy7/FF7ModelListFile.cpp
    data/worldmap/MapFile.cpp
    data/worldmap/MapFileManager.cpp
    data/worldmap/MapFileSerializer.cpp
    data/worldmap/TxzFile.cpp
    data/worldmap/TxzFileManager.cpp
    data/worldmap/TxzFileSerializer.cpp
    common/Application.cpp
    common/File.cpp
    common/FileSystem.cpp
    common/LzsFile.cpp
    common/VGearsManualObject.cpp
    common/VGearsResource.cpp
    common/VGearsStringUtil.cpp
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
    core/DialogsManager.cpp
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
    core/TextManager.cpp
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
    core/XmlScreensFile.cpp
    core/XmlSoundsFile.cpp
    core/XmlTextFile.cpp
    core/XmlTextsFile.cpp
    core/XmlScreenFile.cpp
    core/XmlScriptsFile.cpp
    core/XmlWalkmeshFile.cpp
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
    core/particles/ParticleTechniqueTranslator.cpp
    core/particles/ParticleTechnique.cpp
    core/particles/ParticleVisual.cpp
    core/particles/emitters/PointEmitter.cpp
    core/particles/emitters/PointEmitterFactory.cpp
    core/particles/renderer/ParticleEntityRenderer.cpp
    core/particles/renderer/ParticleEntityRendererDictionary.cpp
    installer/DataInstaller.cpp
    installer/FieldDataInstaller.cpp
    installer/FieldTextWriter.cpp
    installer/KernelDataInstaller.cpp
    installer/MainWindow.cpp
    installer/MediaDataInstaller.cpp
    installer/ModelsAndAnimationsDb.cpp
    installer/ScopedLgp.cpp
    installer/TexFile.cpp
    installer/VGearsUtility.cpp
    installer/WorldInstaller.cpp
    installer/common/BinGZipFile.cpp
    installer/common/DrawSkeleton.cpp
    installer/common/FontFile.cpp
    installer/common/Logger.cpp
    installer/common/OgreBase.cpp
    installer/common/OgreGenUtilites.cpp
    installer/common/Surface.cpp
    installer/common/TimToVram.cpp
    installer/common/Vram.cpp
    installer/decompiler/CodeGenerator.cpp
    installer/decompiler/ControlFlow.cpp
    installer/decompiler/DecompilerException.cpp
    installer/decompiler/Disassembler.cpp
    installer/decompiler/Engine.cpp
    installer/decompiler/Graph.cpp
    installer/decompiler/LuaLanguage.cpp
    installer/decompiler/ScriptFormatter.cpp
    installer/decompiler/Value.cpp
    installer/decompiler/field/FieldCodeGenerator.cpp
    installer/decompiler/field/FieldEngine.cpp
    installer/decompiler/field/FieldDecompiler.cpp
    installer/decompiler/field/FieldDisassembler.cpp
    installer/decompiler/field/FieldScriptFormatter.cpp
    installer/decompiler/field/instruction/FieldBackgroundInstruction.cpp
    installer/decompiler/field/instruction/FieldCameraInstruction.cpp
    installer/decompiler/field/instruction/FieldCondJumpInstruction.cpp
    installer/decompiler/field/instruction/FieldControlFlowInstruction.cpp
    installer/decompiler/field/instruction/FieldMathInstruction.cpp
    installer/decompiler/field/instruction/FieldMediaInstruction.cpp
    installer/decompiler/field/instruction/FieldModelInstruction.cpp
    installer/decompiler/field/instruction/FieldModuleInstruction.cpp
    installer/decompiler/field/instruction/FieldNoOperationInstruction.cpp
    installer/decompiler/field/instruction/FieldPartyInstruction.cpp
    installer/decompiler/field/instruction/FieldUncategorizedInstruction.cpp
    installer/decompiler/field/instruction/FieldUncondJumpInstruction.cpp
    installer/decompiler/field/instruction/FieldWalkmeshInstruction.cpp
    installer/decompiler/field/instruction/FieldWindowInstruction.cpp
    installer/decompiler/instruction/BinaryOpStackInstruction.cpp
    installer/decompiler/instruction/BoolNegateStackInstruction.cpp
    installer/decompiler/instruction/CallInstruction.cpp
    installer/decompiler/instruction/CondJumpInstruction.cpp
    installer/decompiler/instruction/DupStackInstruction.cpp
    installer/decompiler/instruction/Instruction.cpp
    installer/decompiler/instruction/KernelCallInstruction.cpp
    installer/decompiler/instruction/KernelCallStackInstruction.cpp
    installer/decompiler/instruction/LoadInstruction.cpp
    installer/decompiler/instruction/ReturnInstruction.cpp
    installer/decompiler/instruction/StackInstruction.cpp
    installer/decompiler/instruction/StoreInstruction.cpp
    installer/decompiler/instruction/UnaryOpPostfixStackInstruction.cpp
    installer/decompiler/instruction/UnaryOpPrefixStackInstruction.cpp
    installer/decompiler/instruction/UncondJumpInstruction.cpp
    installer/decompiler/world/WorldDisassembler.cpp
    installer/decompiler/world/WorldCodeGenerator.cpp
    installer/decompiler/world/WorldEngine.cpp
    installer/decompiler/world/instruction/WorldBinaryEqualStackInstruction.cpp
    installer/decompiler/world/instruction/WorldCondJumpInstruction.cpp
    installer/decompiler/world/instruction/WorldKernelCallInstruction.cpp
    installer/decompiler/world/instruction/WorldLoadBankInstruction.cpp
    installer/decompiler/world/instruction/WorldLoadInstruction.cpp
    installer/decompiler/world/instruction/WorldNoOutputInstruction.cpp
    installer/decompiler/world/instruction/WorldStackInstruction.cpp
    installer/decompiler/world/instruction/WorldStoreInstruction.cpp
    installer/decompiler/world/instruction/WorldSubStackInstruction.cpp
    installer/decompiler/world/instruction/WorldUncondJumpInstruction.cpp
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
    modules/worldmap/WorldMapModule.cpp
    viewer/ViewerModule.cpp
)


# Generate tests
add_executable(v-gears-tests ${SOURCE_FILES})
SET_PROPERTY(TARGET v-gears-tests PROPERTY FOLDER "build/v-gears-test")
target_link_libraries(v-gears-tests
    libvgears
    Qt5::Widgets
    Qt5::Core
    ${OIS_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${OGGVORBIS_LIBRARIES}
    ${TinyXML_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OGRE_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
add_test(NAME v-gears-tests COMMAND v-gears-tests)
