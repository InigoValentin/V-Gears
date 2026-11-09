---
description: "Scaffold a new FF7 data format parser (File/Serializer/Manager triplet) following project conventions"
agent: "agent"
argument-hint: "Format name and target directory, e.g. 'XYZ in src/data/worldmap'"
---

Scaffold a new FF7 binary format parser for the format named by the user:

1. Research first:
   - Check `doc/code/FFVII/` for documentation of the format.
   - Read the most similar existing triplet in `src/data/` (e.g. `TxzFile.h/.cpp`, `TxzFileSerializer.h/.cpp`, `TxzFileManager.h/.cpp` in `src/data/worldmap/`) and mirror its structure, base classes, and Doxygen comment style exactly.
2. Create the triplet in the requested directory:
   - `XFile` — passive data container for the parsed format.
   - `XFileSerializer` — binary read/write, deriving from the same serializer base as its neighbors.
   - `XFileManager` — Ogre `ResourceManager` singleton integration, matching the existing pattern.
3. Follow `doc/STYLE.md` strictly: C++14, `PascalCase` types, `snake_case` variables with trailing underscore for members, `#pragma once`, correct include order.
4. Register the new sources in the appropriate `src/CMakeLists.txt` source list (no globbing — add them explicitly, in the same grouped style as neighboring entries).
5. Do not add or reference original game data; document the binary layout from `doc/code/FFVII/` knowledge in the header comments instead.
6. Build the affected target to verify it compiles.
