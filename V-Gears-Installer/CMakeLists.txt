# Include directories.
include_directories(
    ${VGears_INCLUDE_DIRS}
    ${VGears_SOURCE_DIR}/V-Gears/include
    ${VGears_SOURCE_DIR}/SupportedGames/FinalFantasy7/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

# Define headers and source files for the installer.
set(HEADER_FILES
    include/common/FontFile.h
    include/VGearsUtility.h
    include/common/Logger.h
    include/common/Surface.h
    include/common/TimToVram.h
    include/common/BinGZipFile.h
    include/common/OgreBase.h
    include/common/OgreGenUtilites.h
    include/common/Vram.h
    include/common/DrawSkeleton.h
    include/common/Logger.h
    include/decompiler/sudm.h
    include/decompiler/LuaLanguage.h
    include/decompiler/CodeGenerator.h
    include/decompiler/ControlFlow.h
    include/decompiler/Disassembler.h
    include/decompiler/Engine.h
    include/decompiler/Function.h
    include/decompiler/graph.h
    include/decompiler/instruction.h
    include/decompiler/objectFactory.h
    include/decompiler/refcounted.h
    include/decompiler/simple_disassembler.h
    include/decompiler/stack.h
    include/decompiler/unknown_opcode_exception.h
    include/decompiler/value.h
    include/decompiler/wrongtype.h
    include/decompiler/field/instruction/FieldBackgroundInstruction.h
    include/decompiler/field/instruction/FieldCameraInstruction.h
    include/decompiler/field/instruction/FieldCondJumpInstruction.h
    include/decompiler/field/instruction/FieldControlFlowInstruction.h
    include/decompiler/field/instruction/FieldMathInstruction.h
    include/decompiler/field/instruction/FieldMediaInstruction.h
    include/decompiler/field/instruction/FieldModelInstruction.h
    include/decompiler/field/instruction/FieldModuleInstruction.h
    include/decompiler/field/instruction/FieldNoOperationInstruction.h
    include/decompiler/field/instruction/FieldPartyInstruction.h
    include/decompiler/field/instruction/FieldUncategorizedInstruction.h
    include/decompiler/field/instruction/FieldUncondJumpInstruction.h
    include/decompiler/field/instruction/FieldWalkmeshInstruction.h
    include/decompiler/field/instruction/FieldWindowInstruction.h
    include/decompiler/field/FieldDisassembler.h
    include/decompiler/field/FieldEngine.h
    include/decompiler/field/FieldCodeGenerator.h
    include/decompiler/world/instruction/WorldBinaryEqualStackInstruction.h
    include/decompiler/world/instruction/WorldCondJumpInstruction.h
    include/decompiler/world/instruction/WorldKernelCallInstruction.h
    include/decompiler/world/instruction/WorldLoadBankInstruction.h
    include/decompiler/world/instruction/WorldLoadInstruction.h
    include/decompiler/world/instruction/WorldNoOutputInstruction.h
    include/decompiler/world/instruction/WorldStackInstruction.h
    include/decompiler/world/instruction/WorldStoreInstruction.h
    include/decompiler/world/instruction/WorldSubStackInstruction.h
    include/decompiler/world/instruction/WorldUncondJumpInstruction.h
    include/decompiler/world/WorldCodeGenerator.h
    include/decompiler/world/WorldDisassembler.h
    include/decompiler/world/WorldEngine.h
    include/common/scummsys.h
    include/common/BinaryReader.h
    include/common/Lzs.h
)


set(SOURCE_FILES
    src/common/Logger.cpp
    src/common/BinGZipFile.cpp
    src/common/DrawSkeleton.cpp
    src/common/Surface.cpp
    src/common/OgreGenUtilites.cpp
    src/common/FontFile.cpp
    src/common/TimToVram.cpp
    src/common/Vram.cpp
    src/common/OgreBase.cpp
    src/VGearsUtility.cpp
    src/decompiler/sudm.cpp
    src/decompiler/LuaLanguage.cpp
    src/decompiler/CodeGenerator.cpp
    src/decompiler/ControlFlow.cpp
    src/decompiler/Disassembler.cpp
    src/decompiler/Engine.cpp
    src/decompiler/graph.cpp
    src/decompiler/instruction.cpp
    src/decompiler/simple_disassembler.cpp
    src/decompiler/unknown_opcode_exception.cpp
    src/decompiler/value.cpp
    src/decompiler/field/instruction/FieldBackgroundInstruction.cpp
    src/decompiler/field/instruction/FieldCameraInstruction.cpp
    src/decompiler/field/instruction/FieldCondJumpInstruction.cpp
    src/decompiler/field/instruction/FieldControlFlowInstruction.cpp
    src/decompiler/field/instruction/FieldMathInstruction.cpp
    src/decompiler/field/instruction/FieldMediaInstruction.cpp
    src/decompiler/field/instruction/FieldModelInstruction.cpp
    src/decompiler/field/instruction/FieldModuleInstruction.cpp
    src/decompiler/field/instruction/FieldNoOperationInstruction.cpp
    src/decompiler/field/instruction/FieldPartyInstruction.cpp
    src/decompiler/field/instruction/FieldUncategorizedInstruction.cpp
    src/decompiler/field/instruction/FieldUncondJumpInstruction.cpp
    src/decompiler/field/instruction/FieldWalkmeshInstruction.cpp
    src/decompiler/field/instruction/FieldWindowInstruction.cpp
    src/decompiler/field/FieldDisassembler.cpp
    src/decompiler/field/FieldEngine.cpp
    src/decompiler/field/FieldCodeGenerator.cpp
    src/decompiler/world/instruction/WorldBinaryEqualStackInstruction.cpp
    src/decompiler/world/instruction/WorldCondJumpInstruction.cpp
    src/decompiler/world/instruction/WorldKernelCallInstruction.cpp
    src/decompiler/world/instruction/WorldLoadBankInstruction.cpp
    src/decompiler/world/instruction/WorldLoadInstruction.cpp
    src/decompiler/world/instruction/WorldNoOutputInstruction.cpp
    src/decompiler/world/instruction/WorldStackInstruction.cpp
    src/decompiler/world/instruction/WorldStoreInstruction.cpp
    src/decompiler/world/instruction/WorldSubStackInstruction.cpp
    src/decompiler/world/instruction/WorldUncondJumpInstruction.cpp
    src/decompiler/world/WorldCodeGenerator.cpp
    src/decompiler/world/WorldDisassembler.cpp
    src/decompiler/world/WorldEngine.cpp
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
    include/mainwindow.h
    include/ff7DataInstaller.h
    include/ff7FieldTextWriter.h
)

set(v-gears-launcher_SRCS
    src/main.cpp
    src/mainwindow.cpp
    src/ff7DataInstaller.cpp
    src/ff7FieldTextWriter.cpp
)

set(v-gears-launcher_UIS
    src/mainwindow.ui
)
QT5_WRAP_UI(UIS ${v-gears-launcher_UIS})

message(UIS: ${UIS})

set(v-gears-launcher_MOCS
    include/mainwindow.h
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
    #Sudm_Lib
)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
target_link_libraries ( v-gears-launcher  Qt5::Widgets Qt5::Core  ${LIBRARIES} )

if(WIN32 OR APPLE)
    install(TARGETS v-gears-launcher DESTINATION .)
else()
    install(TARGETS v-gears-launcher RUNTIME DESTINATION bin)
endif()