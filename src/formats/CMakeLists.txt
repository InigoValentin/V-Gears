# Sources of this folder (see vgears_sources in src/CMakeLists.txt).
vgears_sources(
    FF7NameLookup.cpp
    LZSDataStream.cpp
    Serializer.cpp
    XMLSerializer.cpp
)

add_subdirectory(field)
add_subdirectory(lgp)
add_subdirectory(model)
add_subdirectory(texture)
add_subdirectory(worldmap)
