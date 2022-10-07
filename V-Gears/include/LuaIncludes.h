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

#ifdef __APPLE__
// Something in the Apple headers seems to #define nil as __NULL 
// this then causes a build error in luabind:
// luabind/nil.hpp:36:40: error: expected unqualified-id
// extern LUABIND_API detail::nil_type nil;
#undef nil

// Causes a build error, usually an ICE on g++
#undef check
#endif

#include <luabind/luabind.hpp>
#include <luabind/yield_policy.hpp>

extern "C"{
    #include "lua.h"
    #include <lualib.h>
    #include <lauxlib.h>
}

