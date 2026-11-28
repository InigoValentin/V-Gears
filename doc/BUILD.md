# <img src="../v-gears.png" alt="V-Gears" width="50"/> V-Gears Building Guide

## Install dependencies

If you are running Debian (or derivates), you can install the required dependencies with:

```bash
sudo apt install git build-essential cmake qtbase5-dev zlib1g-dev libarchive-dev libogre-1.12-dev libois-dev libvorbis-dev libboost-dev libboost-program-options-dev libboost-test-dev libboost-filesystem-dev libboost-thread-dev libopenal-dev libtinyxml-dev libluajit-5.1-dev
```

This guide was checked on Debian forky/sid. Other releases and distributions may name some packages differently, especially `libogre-1.12-dev`.

To convert the sound effects, the music and the videos, the data installer also uses FFmpeg and TiMidity. They are not needed to build V-Gears, only to install the game data (see the [data installation guide](INSTALL_DATA.md "Install Data")).

```bash
sudo apt install ffmpeg timidity
```

## Get the code

Clone the repository and enter its directory. All the commands in this guide are run from there, which is called the project directory.

```bash
git clone https://github.com/InigoValentin/V-Gears.git
cd V-Gears
```

## Build using CMake

From the project directory, run these commands:

```bash
cmake -S . -B build
cmake --build build
```

If you want to speed it up (it can take several minutes, depending on your system), add `-j` to the second command to use multiple threads. For instance, `cmake --build build -j3` will use three cores for the compilation. The build uses a lot of memory, so on machines with little RAM, use a low number (`-j2`).

Once the build process finishes, you should have the following files (from the project directory):

- `build/bin/v-gears`, the engine executable.
- `build/bin/v-gears-installer`, the data installer.

If you add `-DCMAKE_BUILD_TYPE=Debug` to the first command, they are in `build/bin_debug/` instead.

The engine can be launched from anywhere: it finds its data directory (`~/.v-gears/` by default) and fills it from the `data/` folder of the repository the first time. See the [run guide](RUN.md "Run V-Gears") for details.

## Run the tests (optional)

The tests are not built by default: enable them with `-DBUILD_TESTS=ON`. It's best to use a separate build directory. From the project directory:

```bash
cmake -S . -B build-tests -DBUILD_TESTS=ON
cmake --build build-tests --target v-gears-tests v-gears-installer-tests
cd build-tests
ctest --output-on-failure
```

There are two test programs:

- `v-gears-tests`, the tests of the engine, the file formats, the data extractors of the installer and the script decompiler. It links the engine library, so it takes a while to build the first time, and it needs the same dependencies as V-Gears.
- `v-gears-installer-tests`, the tests of the installer logic (the option rules, the command line, the installation layout and record, the clean installation, the verifier, the log, the external tools and the movie camera data) and of the data directory and Ogre plugin lookup. It doesn't need the rest of the engine, so it builds quickly.

To build and run only one of them, pass just its name to `--target` and run it from the build directory, for example `./bin/v-gears-installer-tests`.

## Source layout

You don't need this to build V-Gears, only to change it.

| Directory | What's in it |
|--|--|
| `src/app/` | The entry point of the game, the application and the game loop. |
| `src/common/` | Files, logger, strings, types and other utilities used by all the programs. |
| `src/formats/` | Readers and writers of the file formats, by kind: `lgp/`, `field/`, `model/`, `texture/` and `worldmap/`. |
| `src/engine/` | The engine, by subsystem: `audio/`, `config/`, `entity/`, `graphics/`, `input/`, `particles/`, `save/`, `script/`, `ui/` and `xml/`. |
| `src/modules/` | The game modules: `battle/` and `worldmap/`. |
| `src/installer/` | The data installer: `gui/` (window), `cli/` (command line), `steps/` (installation process), `extractors/` (data conversion), `decompiler/` (field and world scripts to Lua), `formats/`, `support/` and `tables/`. |
| `src/third_party/` | Code from other projects. |
| `test/` | The tests, in the same layout as `src/`. |
| `data/` | The files that come with V-Gears: Ogre configuration, `engine/` (scripts, screens, fonts, texts, shaders) and `defaults/` (initial game configuration). No data of the original game. |

The directories of the first five rows go from the lowest layer to the highest: `common`, `formats`, `engine`, `modules` and `app`. A layer never includes headers of a higher one. The engine reaches the application and the modules through its interfaces (`src/engine/*Interface.h`).

Sources are not found automatically. Each folder of `src/` and `test/` lists its files in its own `CMakeLists.txt`, so add new files there, and add a new folder with `add_subdirectory()`.

## Make packages (optional)

V-Gears can be packaged for Linux as a `.deb` package and as a `.tar.gz` archive. None of them includes data from the original game: the player still needs the data installer and their own copy of the game.

### Build the packages

Use a separate Release build, so it doesn't interfere with your development build. From the project directory:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j4 --target v-gears v-gears-installer
cd build-release
cpack
```

The first build takes a while. Adjust `-j` as explained in [Build using CMake](#build-using-cmake). Making the `.deb` needs `dpkg-dev`, which comes with `build-essential`.

When `cpack` finishes, `build-release/` contains:

- `v-gears_<version>_amd64.deb`, the Debian package.
- `v-gears-<version>-Linux-x86_64.tar.gz`, the archive.
- A `.sha256` checksum file for each of them. Publish them along with the packages.

The version is the one in the `project()` line of the main `CMakeLists.txt`. To make only one of the packages, use `cpack -G DEB` or `cpack -G TGZ`.

### What's in the packages

Both have the same files:

- `bin/v-gears` and `bin/v-gears-installer`, the engine and the data installer.
- `share/v-gears/`, the V-Gears data files (configuration, scripts, screens, fonts...). The engine copies them to the user's data directory (`~/.v-gears/`) the first time it runs, and the data installer adds the game data there.
- `share/doc/v-gears/`, the documentation for players (not the build and style guides) and the license.
- `share/applications/` and `share/icons/`, the desktop files and the icons.

The `.deb` installs them under `/usr`, so both programs show in the applications menu. Install it with `apt`, so it installs the dependencies too:

```bash
sudo apt install ./v-gears_<version>_amd64.deb
```

The `.tar.gz` has the same layout in a single folder. It can be extracted anywhere and run from there (`bin/v-gears`, `bin/v-gears-installer`), without root. It doesn't add anything to the applications menu.

The Ogre plugins (`RenderSystem_GL`, `Codec_STBI`) are not in the packages: the engine loads the ones of the system's Ogre.

### Which systems can use them

The packages don't include any library: they use the ones installed in the system. The dependencies of the `.deb` are taken from the libraries the programs were built with, so it can only be installed in systems that have the same versions (the same Ogre and Boost versions, especially). In practice, the packages work in the same distribution and version where they were built.

To make packages for a given distribution, build them in that distribution, for example in a container or a virtual machine. The `.tar.gz` has the same limits, but it gives no warning: if a library is missing, the programs don't start.

### Packaging options

These can be set when running `cmake` (`-D<option>=<value>`):

|Option|Default|Description|
|--|--|--|
|`CMAKE_BUILD_TYPE`|`Release`|Use `Release` for packages. With `Debug`, the programs are slower and the archive name ends in `-debug`.|
|`VGEARS_OGRE_PLUGIN_DIR`|Found at build time|Directory of the system's Ogre plugins. The engine looks for the plugins there after its data directory and its own directory. Set it if the plugins of the target system are in another place.|
|`VGEARS_PACKAGE_MAINTAINER`|The project maintainer|Name and email of the maintainer in the `.deb`.|

### Install without a package

To install V-Gears directly in the system, with the same files and layout as the packages, build it and then install it. From the project directory:

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j4
sudo cmake --install build-release --prefix /usr/local
```

CMake writes the list of installed files to `build-release/install_manifest.txt`. To uninstall, remove those files. This leaves their directories, and your data in `~/.v-gears/`.

```bash
sudo xargs -d '\n' rm -f -- < build-release/install_manifest.txt
```

## Advanced build options

### Deterministic fallbacks

You don't need this for a normal build. It only matters if CMake can't find a library that is installed.

The CMake scripts include an option `USE_DETERMINISTIC_FALLBACKS` (default `ON`).
- When `ON`, CMake will attempt pragmatic fallbacks to find libraries when
	`find_package`/pkg-config do not succeed (SONAME `find_library` passes,
	appending common `/usr/lib` paths, or injecting generic `-lboost_*` flags).
- When `OFF`, CMake will not perform these fallbacks; instead require proper
	dev packages or explicit hints.

**Leave it `ON` for now, also for packages.** On Linux, the regular library detection in `src/CMakeLists.txt` is skipped because of a known bug in the script, so with the fallbacks `OFF` the build fails to link Boost and OgreBites.

If you see warnings about missing Boost or OgreBites, prefer installing the
appropriate `-dev` packages listed above, or point CMake at your OGRE/Boost
installation with `-DOGRE_ROOT=...` and `-DBoost_DIR=...`/`-DBoost_LIBRARY_DIRS`.

## Next steps

- [Install data](INSTALL_DATA.md "Install Data")
- [Run V-Gears](RUN.md "Run V-Gears")
