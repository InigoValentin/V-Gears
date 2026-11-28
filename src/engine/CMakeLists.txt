# Sources of this folder (see vgears_sources in src/CMakeLists.txt).
vgears_sources(
    GameState.cpp
    Manager.cpp
    Timer.cpp
)

add_subdirectory(audio)
add_subdirectory(config)
add_subdirectory(entity)
add_subdirectory(graphics)
add_subdirectory(input)
add_subdirectory(particles)
add_subdirectory(save)
add_subdirectory(script)
add_subdirectory(ui)
add_subdirectory(xml)
