include_directories(
    luabind
    luabind/luabind
    luabind/luabind/detail
    ${LUA_INCLUDE_DIR}
)

add_subdirectory(sfxdump) # SFXDump has it's own configuration.

aux_source_directory(luabind/luabind LUABIND_HEADER_FILES)
aux_source_directory(luabind/luabind/details LUABIND_HEADER_DETAILS_FILES)
aux_source_directory(luabind/src LUABIND_SRC_FILES)
add_library(libluabind STATIC ${LUABIND_SRC_FILES} ${LUABIND_HEADER_FILES} ${LUABIND_HEADER_DETAILS_FILES})
