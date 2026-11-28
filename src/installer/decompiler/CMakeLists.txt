# Sources of this folder (see vgears_sources in src/CMakeLists.txt).
vgears_sources(
    CodeGenerator.cpp
    ControlFlow.cpp
    DecompilerException.cpp
    Disassembler.cpp
    Engine.cpp
    Graph.cpp
    LuaLanguage.cpp
    ScriptFormatter.cpp
    Value.cpp
)

add_subdirectory(field)
add_subdirectory(instruction)
add_subdirectory(world)
