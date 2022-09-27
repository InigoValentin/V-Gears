include_directories(
    ${VGears_INCLUDE_DIRS}
    ${VGears_SOURCE_DIR}/V-Gears/include
    ${VGears_SOURCE_DIR}/SupportedGames/FinalFantasy7/include
    ${CMAKE_CURRENT_SOURCE_DIR}/common/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

# define header and source files for the library
set(HEADER_FILES
./common/FontFile.h
./common/include/VGearsUtility.h
./common/Logger.h
./common/Surface.h
./common/TimToVram.h
./common/BinGZipFile.h
./common/OgreBase.h
./common/OgreGenUtilites.h
./common/Vram.h
./common/DrawSkeleton.h
)

set(SOURCE_FILES
./common/Logger.cpp
./common/BinGZipFile.cpp
./common/DrawSkeleton.cpp
./common/Surface.cpp
./common/OgreGenUtilites.cpp
./common/FontFile.cpp
./common/TimToVram.cpp
./common/Vram.cpp
./common/OgreBase.cpp
./common/src/VGearsUtility.cpp
)

add_library(Utility STATIC
  ${HEADER_FILES}
  ${SOURCE_FILES}
)

#DEBUG, no optimizations
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic -O0")

target_link_libraries(Utility
    ${OGRE_LIBRARIES}
    ${OIS_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
SET_PROPERTY(TARGET Utility PROPERTY FOLDER "utilities")

add_subdirectory(flevel)
add_subdirectory(lzs)
#add_subdirectory(v-gears-launcher)


find_package ( Qt5Widgets REQUIRED )
#include ( ${QT_USE_FILE} )
include_directories (
    ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
    )
set ( v-gears-launcher_HDRS
    include/mainwindow.h
    include/ff7DataInstaller.h
    include/ff7FieldTextWriter.h
    )

set ( v-gears-launcher_SRCS
    src/main.cpp
    src/mainwindow.cpp
    src/ff7DataInstaller.cpp
    src/ff7FieldTextWriter.cpp
    )

set ( v-gears-launcher_UIS
    src/mainwindow.ui
    )
QT5_WRAP_UI(UIS ${v-gears-launcher_UIS})

message(UIS: ${UIS})

set ( v-gears-launcher_MOCS
    include/mainwindow.h
    )
QT5_WRAP_CPP(MOCS ${v-gears-launcher_MOCS})

add_definitions(
    -DVGEARS_ADD_MANAGERS
)
add_executable ( v-gears-launcher ${v-gears-launcher_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS} )

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

#target_link_libraries ( v-gears-launcher  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${LIBRARIES} )
target_link_libraries ( v-gears-launcher  Qt5::Widgets Qt5::Core  ${LIBRARIES} )

if(WIN32 OR APPLE)
  install(TARGETS v-gears-launcher DESTINATION . ) 
else()
  install(TARGETS v-gears-launcher RUNTIME DESTINATION bin ) 
endif()


