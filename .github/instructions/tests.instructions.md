---
description: "Use when writing, adding, or fixing tests in the test/ directory. Covers Boost.Test conventions, registering test sources in the CMakeLists.txt of their folder, and running the v-gears-tests suite."
applyTo: "test/**"
---

# V-Gears Testing Conventions

- Tests use Boost.Test with dynamic linking (`BOOST_TEST_DYN_LINK`); the suite builds as the `v-gears-tests` target and is registered with CTest.
- Test files mirror the path of the code under test (`test/formats/model/AFile.cpp` tests `src/formats/model/AFile.cpp`). Follow this existing convention for placement.
- Every new test source file must be added manually to the `CMakeLists.txt` of its folder, with `vgears_tests()` (or `vgears_installer_tests()` for the tests of the installer logic) — there is no globbing. A new folder also needs an `add_subdirectory()` in its parent.
- RTTI (`dynamic_cast`, `typeid`) is allowed in tests even though production code avoids it.
- Never use original game data in tests. Construct binary fixtures in-code (byte buffers, temp files) instead.

## Running

```bash
cmake --build build --target v-gears-tests -j$(nproc)
ctest --test-dir build --output-on-failure
```

The test binary can also be run directly from the build output directory to filter individual test cases.
