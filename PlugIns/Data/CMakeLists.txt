set(V-Gears_SOURCE_DIR ../../V-Gears)

# Define header and source files for the library.
set(HEADER_FILES
    ${V-Gears_SOURCE_DIR}/include/VGearsPrerequisites.h
    ${V-Gears_SOURCE_DIR}/include/common/VGearsManualObject.h
    ${V-Gears_SOURCE_DIR}/include/common/VGearsStringUtil.h
    ${V-Gears_SOURCE_DIR}/include/common/TypeDefine.h
    include/VGearsDataPlugin.h
)
set(SOURCE_FILES
    ${V-Gears_SOURCE_DIR}/src/common/VGearsManualObject.cpp
    ${V-Gears_SOURCE_DIR}/src/common/VGearsStringUtil.cpp
    ${V-Gears_SOURCE_DIR}/src/common/TypeDefine.cpp
    src/VGearsDataPlugin.cpp
    src/VGearsDataPluginDll.cpp
)

include_directories(
    ${OIS_INCLUDE_DIRS}
    ${OGRE_INCLUDE_DIRS}
    ${OGRE_SAMPLES_INCLUDEPATH}
    ${V-Gears_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib
)

# Defines.
#add_definitions(-DBOOST_ALL_DYN_LINK)

add_library(DataPlugin MODULE
    ${HEADER_FILES}
    ${SOURCE_FILES}
)
SET_PROPERTY(TARGET DataPlugin PROPERTY FOLDER "v-gears")

set_target_properties(DataPlugin PROPERTIES
    PREFIX ${PLUGIN_PREFIX}
    OUTPUT_NAME Data
)

set(LIBRARIES
    ${OGRE_LIBRARIES}
    v-gears-data
)

target_link_libraries(DataPlugin ${LIBRARIES})

if(NOT VGEARS_STATIC)
    set_target_properties(DataPlugin PROPERTIES COMPILE_DEFINITIONS VGEARS_DATAPLUGIN_EXPORTS)
    if(WIN32 OR APPLE)
        install(TARGETS DataPlugin LIBRARY DESTINATION .)
    else()
        install(TARGETS DataPlugin LIBRARY DESTINATION ./lib/v-gears/plugins/ )
    endif()
endif()
