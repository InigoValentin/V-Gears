find_package ( Qt5Widgets REQUIRED )
#include ( ${QT_USE_FILE} )
include_directories (
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR}
	)
set ( v-gears-launcher_HDRS
	src/mainwindow.h
	src/ff7DataInstaller.h
    src/ff7FieldTextWriter.h
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

set ( v-gears-launcher_MOCS
	src/mainwindow.h
	)
QT5_WRAP_CPP(MOCS ${v-gears-launcher_MOCS})

add_definitions(
    -DQGEARS_ADD_MANAGERS
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
    QGearsMain
    v-gears-data
    Sudm_Lib
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
