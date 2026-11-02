/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/**
 * This file wraps the LUA includes for platform specific build workarounds.
 */

#pragma once

#include <limits>

#ifdef __APPLE__
// Something in the Apple headers seems to #define nil as __NULL,
// which breaks the sol2 binding headers.
#undef nil

// Causes a build error, usually an ICE on g++
#undef check
#endif

// sol2 v2.20.6 single header. It includes <lua.hpp> itself, so the Lua
// headers (lua.h, lualib.h, lauxlib.h) are always available after this.
// Note: on LuaJIT, sol.hpp bundles a compat53 shim that exposes the 5.2-style
// lua_resume(L, from, nargs) signature.
#include <third_party/sol/sol.hpp>

