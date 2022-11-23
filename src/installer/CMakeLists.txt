# Include directories.
include_directories(
    ${VGears_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)
message("XXXXXXXXXX ${CMAKE_CURRENT_SOURCE_DIR}")

# Define headers and source files for the installer.
set(HEADER_FILES
    common/FontFile.h
    VGearsUtility.h
    common/Logger.h
    common/Surface.h
    common/TimToVram.h
    common/BinGZipFile.h
    common/OgreBase.h
    common/OgreGenUtilites.h
    common/Vram.h
    common/DrawSkeleton.h
    common/Logger.h
    decompiler/ScriptFormatter.h
    decompiler/LuaLanguage.h
    decompiler/CodeGenerator.h
    decompiler/ControlFlow.h
    decompiler/Disassembler.h
    decompiler/Decompiler.h
    decompiler/Engine.h
    decompiler/Graph.h
    decompiler/Function.h
    decompiler/instruction/Instruction.h
    decompiler/instruction/BinaryOpInstruction.h
    decompiler/instruction/BinaryOpStackInstruction.h
    decompiler/instruction/BoolNegateInstruction.h
    decompiler/instruction/BoolNegateStackInstruction.h
    decompiler/instruction/CallInstruction.h
    decompiler/instruction/CondJumpInstruction.h
    decompiler/instruction/DupInstruction.h
    decompiler/instruction/DupStackInstruction.h
    decompiler/instruction/JumpInstruction.h
    decompiler/instruction/KernelCallInstruction.h
    decompiler/instruction/KernelCallStackInstruction.h
    decompiler/instruction/LoadInstruction.h
    decompiler/instruction/ReturnInstruction.h
    decompiler/instruction/StackInstruction.h
    decompiler/instruction/StoreInstruction.h
    decompiler/instruction/UnaryOpInstruction.h
    decompiler/instruction/UnaryOpPostfixStackInstruction.h
    decompiler/instruction/UnaryOpPrefixStackInstruction.h
    decompiler/instruction/UncondJumpInstruction.h
    decompiler/ObjectFactory.h
    decompiler/RefCounted.h
    decompiler/Stack.h
    decompiler/DecompilerException.h
    decompiler/Value.h
    decompiler/field/instruction/FieldBackgroundInstruction.h
    decompiler/field/instruction/FieldCameraInstruction.h
    decompiler/field/instruction/FieldCondJumpInstruction.h
    decompiler/field/instruction/FieldControlFlowInstruction.h
    decompiler/field/instruction/FieldMathInstruction.h
    decompiler/field/instruction/FieldMediaInstruction.h
    decompiler/field/instruction/FieldModelInstruction.h
    decompiler/field/instruction/FieldModuleInstruction.h
    decompiler/field/instruction/FieldNoOperationInstruction.h
    decompiler/field/instruction/FieldPartyInstruction.h
    decompiler/field/instruction/FieldUncategorizedInstruction.h
    decompiler/field/instruction/FieldUncondJumpInstruction.h
    decompiler/field/instruction/FieldWalkmeshInstruction.h
    decompiler/field/instruction/FieldWindowInstruction.h
    decompiler/field/FieldDisassembler.h
    decompiler/field/FieldEngine.h
    decompiler/field/FieldCodeGenerator.h
    decompiler/field/FieldDecompiler.h
    decompiler/field/FieldScriptFormatter.h
    decompiler/world/instruction/WorldBinaryEqualStackInstruction.h
    decompiler/world/instruction/WorldCondJumpInstruction.h
    decompiler/world/instruction/WorldKernelCallInstruction.h
    decompiler/world/instruction/WorldLoadBankInstruction.h
    decompiler/world/instruction/WorldLoadInstruction.h
    decompiler/world/instruction/WorldNoOutputInstruction.h
    decompiler/world/instruction/WorldStackInstruction.h
    decompiler/world/instruction/WorldStoreInstruction.h
    decompiler/world/instruction/WorldSubStackInstruction.h
    decompiler/world/instruction/WorldUncondJumpInstruction.h
    decompiler/world/WorldCodeGenerator.h
    decompiler/world/WorldDisassembler.h
    decompiler/world/WorldEngine.h
    common/scummsys.h
    common/BinaryReader.h
    common/Lzs.h
    ModelsAndAnimationsDb.h
    ScopedLgp.h
    TexFile.h
    SpawnPointDb.h
    KernelDataInstaller.h
    MediaDataInstaller.h
    FieldDataInstaller.h
)


set(SOURCE_FILES
    common/Logger.cpp
    common/BinGZipFile.cpp
    common/DrawSkeleton.cpp
    common/Surface.cpp
    common/OgreGenUtilites.cpp
    common/FontFile.cpp
    common/TimToVram.cpp
    common/Vram.cpp
    common/OgreBase.cpp
    VGearsUtility.cpp
    decompiler/ScriptFormatter.cpp
    decompiler/LuaLanguage.cpp
    decompiler/CodeGenerator.cpp
    decompiler/ControlFlow.cpp
    decompiler/Disassembler.cpp
    decompiler/Engine.cpp
    decompiler/Graph.cpp
    decompiler/instruction/Instruction.cpp
    decompiler/instruction/BinaryOpStackInstruction.cpp
    decompiler/instruction/BoolNegateStackInstruction.cpp
    decompiler/instruction/CallInstruction.cpp
    decompiler/instruction/CondJumpInstruction.cpp
    decompiler/instruction/DupStackInstruction.cpp
    decompiler/instruction/KernelCallInstruction.cpp
    decompiler/instruction/KernelCallStackInstruction.cpp
    decompiler/instruction/LoadInstruction.cpp
    decompiler/instruction/ReturnInstruction.cpp
    decompiler/instruction/StackInstruction.cpp
    decompiler/instruction/StoreInstruction.cpp
    decompiler/instruction/UnaryOpPostfixStackInstruction.cpp
    decompiler/instruction/UnaryOpPrefixStackInstruction.cpp
    decompiler/instruction/UncondJumpInstruction.cpp
    decompiler/DecompilerException.cpp
    decompiler/Value.cpp
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
    decompiler/field/FieldDisassembler.cpp
    decompiler/field/FieldEngine.cpp
    decompiler/field/FieldCodeGenerator.cpp
    decompiler/field/FieldDecompiler.cpp
    decompiler/field/FieldScriptFormatter.cpp
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
    ModelsAndAnimationsDb.cpp
    ScopedLgp.cpp
    TexFile.cpp
    KernelDataInstaller.cpp
    MediaDataInstaller.cpp
    FieldDataInstaller.cpp
)

# For SUDM. TODO: Add elses.
if (UNIX)
    add_definitions(-DPOSIX)
endif()

add_library(Utility STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

target_link_libraries(Utility
    ${OGRE_LIBRARIES}
    ${OIS_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
SET_PROPERTY(TARGET Utility PROPERTY FOLDER "utilities")

# DEBUG, no optimizations
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic -O0")

find_package ( Qt5Widgets REQUIRED )
include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
)
set ( v-gears-launcher_HDRS
    MainWindow.h
    DataInstaller.h
    FieldTextWriter.h
)

set(v-gears-launcher_SRCS
    main.cpp
    MainWindow.cpp
    DataInstaller.cpp
    FieldTextWriter.cpp
)

set(v-gears-launcher_UIS
    MainWindow.ui
)
QT5_WRAP_UI(UIS ${v-gears-launcher_UIS})

message(UIS: ${UIS})

set(v-gears-launcher_MOCS
    MainWindow.h
)
QT5_WRAP_CPP(MOCS ${v-gears-launcher_MOCS})

add_definitions(-DVGEARS_ADD_MANAGERS)
add_executable (v-gears-launcher ${v-gears-launcher_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})

SET_PROPERTY(TARGET v-gears-launcher PROPERTY FOLDER "utilities")

if (APPLE)
    target_link_libraries(v-gears-launcher "-framework CoreFoundation -framework Cocoa -framework IOKit")
endif()

if(WIN32)
    set_target_properties(v-gears-launcher PROPERTIES WIN32_EXECUTABLE ON)
    if (MSVC)
        set_target_properties(v-gears-launcher PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    endif()
endif()

set(LIBRARIES
    Utility
    V-Gears
    v-gears-data
)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
target_link_libraries ( v-gears-launcher  Qt5::Widgets Qt5::Core  ${LIBRARIES} )

if(WIN32 OR APPLE)
    install(TARGETS v-gears-launcher DESTINATION .)
else()
    install(TARGETS v-gears-launcher RUNTIME DESTINATION bin)
endif()
