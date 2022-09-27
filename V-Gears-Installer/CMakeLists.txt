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
    include/decompiler/decompiler_codegen.h
    include/decompiler/control_flow.h
    include/decompiler/decompiler_disassembler.h
    include/decompiler/decompiler_engine.h
    include/decompiler/graph.h
    include/decompiler/instruction.h
    include/decompiler/objectFactory.h
    include/decompiler/refcounted.h
    include/decompiler/simple_disassembler.h
    include/decompiler/stack.h
    include/decompiler/unknown_opcode_exception.h
    include/decompiler/value.h
    include/decompiler/wrongtype.h
    include/decompiler/scummv6/codegen.h
    include/decompiler/scummv6/disassembler.h
    include/decompiler/scummv6/engine.h
    include/decompiler/field/ff7_field_disassembler.h
    include/decompiler/field/ff7_field_engine.h
    include/decompiler/field/ff7_field_codegen.h
    include/common/algorithm.h
    include/common/noncopyable.h
    include/common/scummsys.h
    include/common/str.h
    include/common/util.h
    include/common/binaryreader.h
    include/common/lzs.h
    include/common/make_unique.h
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
    src/decompiler/decompiler_codegen.cpp
    src/decompiler/control_flow.cpp
    src/decompiler/decompiler_disassembler.cpp
    src/decompiler/graph.cpp
    src/decompiler/instruction.cpp
    src/decompiler/simple_disassembler.cpp
    src/decompiler/unknown_opcode_exception.cpp
    src/decompiler/value.cpp
    src/decompiler/scummv6/codegen.cpp
    src/decompiler/scummv6/disassembler.cpp
    src/decompiler/scummv6/engine.cpp
    src/decompiler/field/ff7_field_disassembler.cpp
    src/decompiler/field/ff7_field_engine.cpp
    src/decompiler/field/ff7_field_codegen.cpp
    src/common/str.cpp
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