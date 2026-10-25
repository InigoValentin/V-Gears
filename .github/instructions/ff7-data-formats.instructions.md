---
description: "Use when working on FF7 binary format parsers, serializers, resource managers, LGP/LZS archives, the data installer, or the script decompiler. Covers the File/Serializer/Manager triplet pattern and known installer gotchas."
applyTo: ["src/data/**", "src/installer/**", "test/data/**", "test/installer/**"]
---

# FF7 Data Formats and Installer

## Parser Triplet Pattern

Most formats in `src/data/` follow this structure — mirror it when adding formats:

- `XFile` — plain data container for the parsed format.
- `XFileSerializer` — reads/writes the binary layout (derives from `VGearsSerializer`).
- `XFileManager` — Ogre `ResourceManager` integration (singleton + `Resource`).

Format documentation lives in [doc/code/FFVII/](../../../doc/code/FFVII/). Check there and at existing similar parsers (e.g. `src/data/worldmap/TxzFile*`) before writing a new one.

## Installer Gotchas (verified, don't regress)

- World map HRC loads must create meshes via `HRCFile::GetMeshFileName()` (i.e. `NameLookup::model(splitBase(hrc_name)) + ".mesh"`). Do NOT use `FF7Data::GetWorldMapModelName()` — that friendly-name table can diverge from the actual Ogre mesh resource name and triggers `FileNotFoundException` (e.g. `barrier.mesh`).
- `WorldInstaller::ExportMesh()` expects a directory prefix, not a full filename prefix.
- The decompiler (`src/installer/decompiler/`) emits Lua — keep output close to [doc/STYLE-LUA.md](../../../doc/STYLE-LUA.md), but it is exempt from strict conformance.
- The installer is Qt-based and handles copyrighted game data at runtime on the user's machine: never write code that copies extracted assets into the repo or build tree.
