# Sources of this folder (see vgears_sources in src/CMakeLists.txt).
vgears_sources(
    FieldCodeGenerator.cpp
    FieldDecompiler.cpp
    FieldDisassembler.cpp
    FieldEngine.cpp
    FieldScriptFormatter.cpp
)

add_subdirectory(instruction)
