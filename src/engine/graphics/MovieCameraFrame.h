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

#include <cstddef>
#include <cstdint>
#include <OgreQuaternion.h>
#include <OgreMath.h>
#include <OgreVector3.h>

/**
 * The camera of a frame of a movie, from the camera file the installer extracts from
 * moviecam.lgp. It has the same layout as the camera of a field, and the same conversion applies.
 */
struct MovieCameraFrame{

    /**
     * Size in bytes of the data of a frame.
     */
    static const size_t SIZE = 40;

    /**
     * Whether the frame has a camera. The frames without one are all zero, and no model can be
     * drawn over them.
     */
    bool valid = false;

    /**
     * Position of the camera, in the units of the field.
     */
    Ogre::Vector3 position = Ogre::Vector3::ZERO;

    /**
     * Orientation of the camera.
     */
    Ogre::Quaternion orientation = Ogre::Quaternion::IDENTITY;

    /**
     * Vertical field of view.
     */
    Ogre::Radian fov;

    /**
     * Reads the camera of a frame.
     *
     * @param[in] data The SIZE bytes of the frame.
     * @param[in] field_scale Scale factor of the field. The camera is in the units of the
     * original game and the field divides them by it.
     * @return The camera. If there isn't one, it's not valid.
     */
    static MovieCameraFrame Parse(const uint8_t* data, const float field_scale);
};
