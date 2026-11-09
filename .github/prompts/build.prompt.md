---
description: "Configure and build V-Gears with CMake (engine, installer, or test suite)"
agent: "agent"
argument-hint: "Optional target or build type, e.g. 'v-gears-installer Release'"
---

Build the V-Gears project:

1. If no `build/` directory or cache exists, configure first:
   `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
   (Use the build type the user asked for, Debug by default. Never build in-tree.)
2. Build with `cmake --build build -j$(nproc)`, optionally restricted to the requested target (`v-gears`, `v-gears-installer`, `v-gears-tests`).
3. If compilation fails, read the full error output, fix the root cause following `doc/STYLE.md` and the scoped instructions (C++14 only), and rebuild.
4. For dependency lookup problems, try `-DUSE_DETERMINISTIC_FALLBACKS=OFF` on a fresh configure to get clean diagnostics before changing any CMake code.
5. Report the final build status and the location of the produced binaries.
