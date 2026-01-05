add_subdirectory(luajit) # LuaJIT has its own configuration.

add_subdirectory(sfxdump) # SFXDump has its own configuration.

# Collect luabind sources and headers explicitly and create a modern target.
file(GLOB_RECURSE LUABIND_SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "luabind/src/*.cpp"
    "luabind/src/*.c"
)
file(GLOB_RECURSE LUABIND_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "luabind/luabind/*.h"
    "luabind/luabind/*.hpp"
    "luabind/luabind/detail/*.h"
    "luabind/luabind/detail/*.hpp"
)

add_library(libluabind STATIC ${LUABIND_SRC_FILES} ${LUABIND_HEADER_FILES})

target_include_directories(libluabind
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/luabind
        ${CMAKE_CURRENT_SOURCE_DIR}/luajit/src
        ${CMAKE_CURRENT_SOURCE_DIR}/luabind/luabind
        ${CMAKE_CURRENT_SOURCE_DIR}/luabind/luabind/detail
)

set_target_properties(libluabind PROPERTIES FOLDER "lib")
