# Find packages required for the istaller.
find_package(Qt5Widgets REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)

# Source files for the installer.
set(INSTALLER_SOURCE_FILES
    main.cpp
    extractors/BattleDataInstaller.cpp
    cli/CliInstaller.cpp
    steps/DataInstaller.cpp
    formats/DiskImage.cpp
    extractors/FieldDataInstaller.cpp
    extractors/FieldTextWriter.cpp
    cli/InstallCommandLine.cpp
    steps/InstallLayout.cpp
    steps/InstallLog.cpp
    steps/InstallOptions.cpp
    steps/InstallSteps.cpp
    steps/ExternalTools.cpp
    steps/InstallVerifier.cpp
    steps/VideoFiles.cpp
    extractors/MovieCamera.cpp
    extractors/KernelDataInstaller.cpp
    gui/MainWindow.cpp
    extractors/MediaDataInstaller.cpp
    extractors/ModelsAndAnimationsDb.cpp
    extractors/ScopedLgp.cpp
    formats/TexFile.cpp
    support/VGearsUtility.cpp
    extractors/WindowFonts.cpp
    extractors/WorldInstaller.cpp
    formats/BinGZipFile.cpp
    support/DrawSkeleton.cpp
    support/Logger.cpp
    support/OgreBase.cpp
    support/OgreGenUtilites.cpp
    support/Vram.cpp
    formats/AbFile.cpp
    formats/BattleSceneFile.cpp
    formats/DaFile.cpp
    formats/WorldMapWalkmesh.cpp
    decompiler/CodeGenerator.cpp
    decompiler/ControlFlow.cpp
    decompiler/DecompilerException.cpp
    decompiler/Disassembler.cpp
    decompiler/Engine.cpp
    decompiler/field/FieldCodeGenerator.cpp
    decompiler/field/FieldDecompiler.cpp
    decompiler/field/FieldDisassembler.cpp
    decompiler/field/FieldEngine.cpp
    decompiler/field/FieldScriptFormatter.cpp
    decompiler/field/instruction/FieldBackgroundInstruction.cpp
    decompiler/field/instruction/FieldCameraInstruction.cpp
    decompiler/field/instruction/FieldCondJumpInstruction.cpp
    decompiler/field/instruction/FieldControlFlowInstruction.cpp
    decompiler/field/instruction/FieldMathInstruction.cpp
    decompiler/field/instruction/FieldMediaInstruction.cpp
    decompiler/field/instruction/FieldModelInstruction.cpp
    decompiler/field/instruction/FieldModuleInstruction.cpp
    decompiler/field/instruction/FieldNoOperationInstruction.cpp
    decompiler/field/instruction/FieldPartyInstruction.cpp
    decompiler/field/instruction/FieldUncategorizedInstruction.cpp
    decompiler/field/instruction/FieldUncondJumpInstruction.cpp
    decompiler/field/instruction/FieldWalkmeshInstruction.cpp
    decompiler/field/instruction/FieldWindowInstruction.cpp
    decompiler/Graph.cpp
    decompiler/instruction/BinaryOpStackInstruction.cpp
    decompiler/instruction/CondJumpInstruction.cpp
    decompiler/instruction/Instruction.cpp
    decompiler/instruction/KernelCallInstruction.cpp
    decompiler/instruction/LoadInstruction.cpp
    decompiler/instruction/StackInstruction.cpp
    decompiler/instruction/StoreInstruction.cpp
    decompiler/instruction/UncondJumpInstruction.cpp
    decompiler/LuaLanguage.cpp
    decompiler/ScriptFormatter.cpp
    decompiler/Value.cpp
    decompiler/world/instruction/WorldBinaryEqualStackInstruction.cpp
    decompiler/world/instruction/WorldCondJumpInstruction.cpp
    decompiler/world/instruction/WorldKernelCallInstruction.cpp
    decompiler/world/instruction/WorldLoadBankInstruction.cpp
    decompiler/world/instruction/WorldLoadInstruction.cpp
    decompiler/world/instruction/WorldNoOutputInstruction.cpp
    decompiler/world/instruction/WorldStackInstruction.cpp
    decompiler/world/instruction/WorldStoreInstruction.cpp
    decompiler/world/instruction/WorldSubStackInstruction.cpp
    decompiler/world/instruction/WorldUncondJumpInstruction.cpp
    decompiler/world/WorldCodeGenerator.cpp
    decompiler/world/WorldDisassembler.cpp
    decompiler/world/WorldEngine.cpp
)

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
