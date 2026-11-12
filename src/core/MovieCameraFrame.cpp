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

#include <algorithm>
#include <cstring>
#include <OgreMath.h>
#include <OgreMatrix3.h>
#include "core/MovieCameraFrame.h"

namespace{

    // Height of the screen the focal length refers to.
    const float SCREEN_HEIGHT = 240.0f;

    int16_t ReadS16(const uint8_t* data, const size_t offset){
        int16_t value;
        std::memcpy(&value, data + offset, sizeof(value));
        return value;
    }

    int32_t ReadS32(const uint8_t* data, const size_t offset){
        int32_t value;
        std::memcpy(&value, data + offset, sizeof(value));
        return value;
    }
}

MovieCameraFrame MovieCameraFrame::Parse(const uint8_t* data, const float field_scale){
    MovieCameraFrame frame;
    // The projection (the last 8 bytes) can be set even if there isn't a camera.
    if (std::all_of(data, data + 32, [](uint8_t byte){return byte == 0;})) return frame;
    // Same steps as the installer takes with the camera of a field.
    Ogre::Matrix3 matrix;
    for (int column = 0; column < 3; ++ column)
        for (int row = 0; row < 3; ++ row)
            matrix[row][column] = static_cast<float>(ReadS16(data, (column * 3 + row) * 2)) / 4096.0f;
    Ogre::Vector3 position;
    for (int i = 0; i < 3; ++ i) position[i] = -static_cast<float>(ReadS32(data, 20 + i * 4));
    Ogre::Quaternion orientation;
    orientation.FromRotationMatrix(matrix);
    position = orientation * (position / 128.0f);
    for (int row = 0; row < 3; ++ row){
        matrix[row][1] = -matrix[row][1];
        matrix[row][2] = -matrix[row][2];
    }
    const float focal_length = static_cast<float>(static_cast<uint16_t>(ReadS16(data, 36)));
    if (focal_length <= 0) return frame;
    frame.position = position / (field_scale > 0 ? field_scale : 1.0f);
    frame.orientation = Ogre::Quaternion(matrix);
    frame.fov = Ogre::Radian(2 * Ogre::Math::ATan(SCREEN_HEIGHT / (2 * focal_length)));
    frame.valid = true;
    return frame;
}
