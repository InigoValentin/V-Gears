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

#include <OgreLogManager.h>
#include <OgreException.h>
#include "data/VGearsCameraMatrixFileSerializer.h"

namespace VGears{

    CameraMatrixFileSerializer::CameraMatrixFileSerializer() : Serializer(){}

    CameraMatrixFileSerializer::~CameraMatrixFileSerializer(){}

    void CameraMatrixFileSerializer::ImportCameraMatrixFile(
      Ogre::DataStreamPtr &stream, CameraMatrixFile* dest
    ){
        // Read matrix.
        sint16 matrix_sint16[CAMERA_MATRIX_ROW_COUNT][CAMERA_MATRIX_COL_COUNT];
        ReadInt16(stream, matrix_sint16[0][0]);
        ReadInt16(stream, matrix_sint16[1][0]);
        ReadInt16(stream, matrix_sint16[2][0]);
        ReadInt16(stream, matrix_sint16[0][1]);
        ReadInt16(stream, matrix_sint16[1][1]);
        ReadInt16(stream, matrix_sint16[2][1]);
        ReadInt16(stream, matrix_sint16[0][2]);
        ReadInt16(stream, matrix_sint16[1][2]);
        ReadInt16(stream, matrix_sint16[2][2]);
        stream->skip(2); // Unused.
        // Array to matrix.
        Ogre::Matrix3 camera_matrix;
        camera_matrix[0][0] = static_cast<float>(matrix_sint16[0][0]) / 4096.0f;
        camera_matrix[1][0] = static_cast<float>(matrix_sint16[1][0]) / 4096.0f;
        camera_matrix[2][0] = static_cast<float>(matrix_sint16[2][0]) / 4096.0f;
        camera_matrix[0][1] = static_cast<float>(matrix_sint16[0][1]) / 4096.0f;
        camera_matrix[1][1] = static_cast<float>(matrix_sint16[1][1]) / 4096.0f;
        camera_matrix[2][1] = static_cast<float>(matrix_sint16[2][1]) / 4096.0f;
        camera_matrix[0][2] = static_cast<float>(matrix_sint16[0][2]) / 4096.0f;
        camera_matrix[1][2] = static_cast<float>(matrix_sint16[1][2]) / 4096.0f;
        camera_matrix[2][2] = static_cast<float>(matrix_sint16[2][2]) / 4096.0f;
        // Read POS
        sint32 pos[CAMERA_MATRIX_ROW_COUNT];
        readInts(
          stream, reinterpret_cast<uint32*>(pos), CAMERA_MATRIX_ROW_COUNT
       );
        // Array to vector.
        Ogre::Vector3 position;
        for (int i = 0; i < 3; i ++) position[i] = -Ogre::Real(pos[i]);
        Ogre::Quaternion orientation;
        orientation.FromRotationMatrix(camera_matrix);
        position = orientation * (position / 128.0f);
        // Matrix fix up.
        //camera_matrix[0][0] = camera_matrix[0][0];
        //camera_matrix[1][0] = camera_matrix[1][0];
        //camera_matrix[2][0] = camera_matrix[2][0];
        camera_matrix[0][1] = -camera_matrix[0][1];
        camera_matrix[1][1] = -camera_matrix[1][1];
        camera_matrix[2][1] = -camera_matrix[2][1];
        camera_matrix[0][2] = -camera_matrix[0][2];
        camera_matrix[1][2] = -camera_matrix[1][2];
        camera_matrix[2][2] = -camera_matrix[2][2];
        dest->SetMatrix(camera_matrix);
        dest->SetPosition(position);
        Pixel offset;
        readObject(stream, offset);
        dest->SetOffset(offset);
        uint16 focal_length;
        ReadShort(stream, focal_length);
        dest->SetFocalLength(focal_length);
        size_t count(1);
        while (!stream->eof()){
            stream->skip(TOTAL_DATA_SIZE);
            ++ count;
        }
        dest->SetCount(count);
    }

}
