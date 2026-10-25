---
description: "Use when writing, adding, or fixing tests in the test/ directory. Covers Boost.Test conventions, registering test sources in test/CMakeLists.txt, and running the v-gears-tests suite."
applyTo: "test/**"
---

# V-Gears Testing Conventions

- Tests use Boost.Test with dynamic linking (`BOOST_TEST_DYN_LINK`); the suite builds as the `v-gears-tests` target and is registered with CTest.
- Test files mirror the path of the code under test (`test/data/VGearsLGPArchive.cpp` tests `src/data/VGearsLGPArchive.cpp`). Follow this existing convention for placement.
- Every new test source file must be added manually to `SOURCE_FILES` in [test/CMakeLists.txt](../../test/CMakeLists.txt) — there is no globbing.
- RTTI (`dynamic_cast`, `typeid`) is allowed in tests even though production code avoids it.
- Never use original game data in tests. Construct binary fixtures in-code (byte buffers, temp files) instead.

## Running

```bash
cmake --build build --target v-gears-tests -j$(nproc)
ctest --test-dir build --output-on-failure
```

The test binary can also be run directly from the build output directory to filter individual test cases.
