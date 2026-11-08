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

#include <boost/test/unit_test.hpp>
#include "LuaIncludes.h"
#include "core/EntityPoint.h"

BOOST_AUTO_TEST_CASE(TestEntityPoint){
        EntityPoint point("entry");
        point.SetPosition(Ogre::Vector3(4.0f, -2.0f, 5.5f));

        sol::state lua;
        lua.open_libraries(sol::lib::base, sol::lib::coroutine);
        lua.new_usertype<EntityPoint>("EntityPoint", "get_position", &EntityPoint::ScriptGetPosition);
        lua["point"] = &point;
        const sol::protected_function_result result = lua.safe_script(
            "local thread = coroutine.create(function() return point:get_position() end)\n"
            "local ok, x, y, z = coroutine.resume(thread)\n"
            "assert(ok and x == 4 and y == -2 and z == 5.5)"
        );
        BOOST_REQUIRE(result.valid());
}

