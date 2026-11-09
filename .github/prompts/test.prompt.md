---
description: "Build and run the v-gears-tests Boost.Test suite, then diagnose any failures"
agent: "agent"
argument-hint: "Optional: specific test case or area to focus on"
---

Build and run the V-Gears test suite:

1. Build the test target: `cmake --build build --target v-gears-tests -j$(nproc)`
   (configure `build/` first if it doesn't exist: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`).
2. Run the suite: `ctest --test-dir build --output-on-failure`.
3. For each failure, read the failing test and the code under test, fix the root cause, and rerun.
   - Tests use Boost.Test with dynamic linking.
   - New test sources must be registered manually in `test/CMakeLists.txt`.
   - Never use original game data in tests — build fixtures in code.
4. Report pass/fail counts and summarize what was fixed.
