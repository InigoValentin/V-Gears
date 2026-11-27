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

#pragma once



namespace VGears {

    /// Rotation speed of the camera with L1 and R1, in radians per second.
    static const float CAMERA_ROTATION_SPEED = 1.5f;

    /// Snow map (WM3): the camera turns by itself after this many seconds (random in the range) ...
    static const float SNOW_CAMERA_MIN_WAIT = 6.0f;
    static const float SNOW_CAMERA_MAX_WAIT = 12.0f;
    /// ... by this many degrees (random in the range, to any side), at the speed of the manual rotation.
    static const float SNOW_CAMERA_MIN_DEGREES = 30.0f;
    static const float SNOW_CAMERA_MAX_DEGREES = 160.0f;

    /// Closest distance of the flying vehicles to the ground.
    static const float FLIGHT_MIN_CLEARANCE = 0.1f;

    /**
     * Highest terrain height the submarine can reach in WM2. The terrain above it is the top
     * layer. The submarine can move over all terrain types below this height.
     *
     * @todo The value was chosen from the height distribution of the WM2 triangles, and not
     * verified against the original game.
     */
    static const float UNDERWATER_MAX_HEIGHT = -0.3f;

    /**
     * Enables the submarine height limit of WM2.
     *
     * The submarine can't go over terrain that is closer than FLIGHT_MIN_CLEARANCE to the
     * ceiling.
     */
    static const bool UNDERWATER_HEIGHT_LIMIT_ENABLED = true;
}
