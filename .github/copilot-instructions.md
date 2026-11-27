# V-Gears Project Guidelines

V-Gears is a free, cross-platform reimplementation of the Final Fantasy VII engine (C++14), continued from the stalled q-gears project. It runs the user's own original game data on modern systems — it is not an emulator and ships no game assets.

## Legal Boundary (never cross)

- NEVER add copyrighted game data to the repo: no ripped textures, models, audio, field scripts, kernel text, or any other extracted asset.
- Game data is extracted locally by the `v-gears-installer` from the user's own copy of the game, at install time, on the user's machine only.
- Do not commit screenshots or conversion outputs containing game assets.

## Architecture

- `src/app/` — the game entry point, the application class and the game state.
- `src/common/` — files, logger, strings, types and other utilities shared by all targets.
- `src/formats/` — FF7 binary format parsers, by kind: `lgp/`, `field/`, `model/`, `texture/` and `worldmap/`. Most formats follow a triplet: `*File` (data), `*FileSerializer` (read/write), `*FileManager` (Ogre resource loading).
- `src/engine/` — engine runtime, by subsystem: `audio/`, `config/`, `entity/`, `graphics/`, `input/`, `particles/`, `save/`, `script/` (`ScriptManager`, Lua bindings via sol2 and system LuaJIT), `ui/` and `xml/` (XML loaders).
- `src/modules/` — game modules: `battle/` and `worldmap/`.
- `src/installer/` — Qt data installer (`v-gears-installer`): `gui/`, `cli/`, `steps/` (the installation process), `extractors/` (game data conversion), `formats/`, `support/`, `tables/` (data tables) and `decompiler/` (converts field and world scripts into Lua).
- `src/third_party/sol/` — vendored sol2 single header. Do not restyle, modernize, or refactor it.
- LuaJIT is a system dependency.
- `test/` — Boost.Test suite mirroring `src/` paths; sources are listed manually in `test/CMakeLists.txt`.
- `data/` — runtime configuration only (Ogre plugins/resources cfgs). Not game data.
- `doc/` — all guides. The original binary formats are documented in the header comments of the parsers and in [the FF7 wiki at wiki.ffrtt.ru](https://wiki.ffrtt.ru/index.php/FF7).
- `_OLD/` — legacy q-gears code kept for reference. Do not modify or reuse without asking.

## Build and Test

Dependencies and details: see [doc/BUILD.md](../doc/BUILD.md).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug       # configure once
cmake --build build -j$(nproc)                      # build everything
cmake --build build --target v-gears-tests -j$(nproc)
ctest --test-dir build --output-on-failure          # run the test suite
```

- Always build out-of-source (`build/`, `build-normal/`, `build-asan/`); never in-tree.
- Main targets: `v-gears` (engine), `v-gears-installer` (data installer), `v-gears-tests`.
- CMake option `USE_DETERMINISTIC_FALLBACKS` (default `ON`) pragmatically finds libraries; set it `OFF` for clean dependency diagnosis or packaging.
- Binaries are picky about working directory and cfg files — read [doc/RUN.md](../doc/RUN.md) before launching anything.

## Code Style

Follow [doc/STYLE.md](../doc/STYLE.md) (C++) and [doc/STYLE-LUA.md](../doc/STYLE-LUA.md). Non-negotiable core rules:

- C++14 only. No newer standard features, no GNU/compiler-specific extensions.
- Files `PascalCase.cpp` / `.h`; types and functions `PascalCase`; variables `snake_case`; class data members `trailing_underscore_`; constants and enumerators `UPPER_SNAKE_CASE`.
- `#pragma once` include guards; include order: C++ stdlib → C system → other libraries → related header → V-Gears headers.
- `const` before the type, `nullptr`, prefix `++i`, `explicit` single-argument constructors, `override` on overridden virtuals.
- Readability over performance; consistency with surrounding code beats personal preference.

## Sanitizer Builds (ASAN/UBSAN)

Verified working recipe — keep these quirks intact:

1. Configure `build-asan` with `-DCMAKE_C_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer -g'` (same for CXX flags, plus `-DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'`). System LuaJIT is used as an uninstrumented dependency.
2. Build the project normally with the sanitizer flags enabled.
3. Run with a UBSAN suppression for `vptr:*/OgreSingleton.h` — a false positive inherent to OGRE's singleton pattern, not a real bug.
