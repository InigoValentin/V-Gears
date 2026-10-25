---
description: "Use when writing or editing C++ code (.cpp/.h files). V-Gears C++14 style rules: naming, includes, const usage, casts, class layout, and documentation."
applyTo: ["**/*.cpp", "**/*.h"]
---

# V-Gears C++ Style Essentials

Full guide: [doc/STYLE.md](../../doc/STYLE.md). Key rules enforced on every change:

## Language

- C++14 only — nothing newer, no GNU extensions. Must compile with the project's existing toolchain.
- Exceptions are allowed; specify `noexcept` when useful and correct.
- Avoid RTTI (`dynamic_cast`, `typeid`) except in tests. Avoid macros, complicated templates, and forward declarations (acceptable only to break include loops).

## Naming

- Types, functions, methods: `PascalCase` (`MyExcitingClass`, `AddTableEntry()`).
- Variables and parameters: `snake_case`; private/protected class members get a trailing underscore (`data_member_`).
- Constants and enumerators: `UPPER_SNAKE_CASE`.
- Files: `PascalCase.cpp` / `PascalCase.h`; every `.cpp` has a matching `.h` except `main()` files and tests.

## Files and Includes

- `#pragma once` for include guards. Headers must be self-contained.
- Include order, no blank lines between groups: C++ stdlib → C system → other libraries → related header → other V-Gears headers. No relative paths in includes.
- Don't inline functions except trivial ones (~10 lines, no loops/switches).
- Give `.cpp`-only code internal linkage with `static`; never in headers.

## Code Constructs

- `const` before the type: `const int* foo`, not `int const* foo`.
- `nullptr` for pointers, `'\0'` for chars, `sizeof(varname)` over `sizeof(type)`.
- C++ casts (`static_cast<>`), never C-style casts except to `void`.
- Prefix `++i` unless postfix semantics are needed.
- `explicit` single-argument constructors; `override` (or rarely `final`) on all overridden virtuals.
- Classes: data members private; section order `public:` → `protected:` → `private:`; structs only for passive data.
- Prefer return values over output parameters; inputs before outputs in parameter lists; keep functions short (~80 lines max).

## Documentation

- Document classes and functions with Doxygen JavaDoc-style comments, as in the existing code.
- Comment tricky/non-obvious code with proper sentences, capitalization, and punctuation.
