# Find packages required for the istaller.
find_package(Qt5Widgets REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)

# Source files for the installer.
set(INSTALLER_SOURCE_FILES
    main.cpp
    BattleDataInstaller.cpp
    DataInstaller.cpp
    FieldDataInstaller.cpp
    FieldTextWriter.cpp
    KernelDataInstaller.cpp
    MainWindow.cpp
    MediaDataInstaller.cpp
    ModelsAndAnimationsDb.cpp
    ScopedLgp.cpp
    TexFile.cpp
    VGearsUtility.cpp
    WorldInstaller.cpp
    common/BinGZipFile.cpp
    common/DrawSkeleton.cpp
    common/FontFile.cpp
    common/Logger.cpp
    common/OgreBase.cpp
    common/OgreGenUtilites.cpp
    common/Surface.cpp
    common/TimToVram.cpp
    common/Vram.cpp
    data/AbFile.cpp
    data/BattleSceneFile.cpp
    data/DaFile.cpp
    data/WorldMapWalkmesh.cpp
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
    decompiler/instruction/BoolNegateStackInstruction.cpp
    decompiler/instruction/CallInstruction.cpp
    decompiler/instruction/CondJumpInstruction.cpp
    decompiler/instruction/DupStackInstruction.cpp
    decompiler/instruction/Instruction.cpp
    decompiler/instruction/KernelCallInstruction.cpp
    decompiler/instruction/KernelCallStackInstruction.cpp
    decompiler/instruction/LoadInstruction.cpp
    decompiler/instruction/ReturnInstruction.cpp
    decompiler/instruction/StackInstruction.cpp
    decompiler/instruction/StoreInstruction.cpp
    decompiler/instruction/UnaryOpPostfixStackInstruction.cpp
    decompiler/instruction/UnaryOpPrefixStackInstruction.cpp
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
set(v-gears-installer_UIS MainWindow.ui)
QT5_WRAP_UI(UIS ${v-gears-installer_UIS})
set(v-gears-installer_MOCS MainWindow.h)
QT5_WRAP_CPP(MOCS ${v-gears-installer_MOCS})


# Compiler options.
if (UNIX) # For SUDM. TODO: Add elses?
    add_definitions(-DPOSIX)
endif()


# Generate v-gears-installer executable.
add_executable (v-gears-installer ${INSTALLER_SOURCE_FILES} ${UIS} ${MOCS})
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
    ${OIS_LIBRARIES}
    ${TinyXML_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OGRE_LIBRARIES}
    ${ZLIB_LIBRARIES}
)


# Install v-gears-installer.
if(WIN32 OR APPLE)
    install(TARGETS v-gears-installer DESTINATION .)
else()
    install(TARGETS v-gears-installer RUNTIME DESTINATION bin)
endif()
