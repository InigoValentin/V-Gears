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

#include "common/TypeDefine.h"
#include "QGearsCameraMatrixFile.h"
#include "QGearsSerializer.h"

namespace QGears{

    /**
     * Handles the serialization of camera matrix files.
     */
    class CameraMatrixFileSerializer : public Serializer{

        public:

            /**
             * Constructor.
             */
            CameraMatrixFileSerializer();

            /**
             * Destructor.
             */
            virtual ~CameraMatrixFileSerializer();

            enum{

                /**
                 * Rows in a camera matrix.
                 *
                 * Same as {@see CAMERA_MATRIX_COL_COUNT}.
                 */
                CAMERA_MATRIX_ROW_COUNT = 3,

                /**
                 * Columns in a camera matrix.
                 *
                 * Same as {@see CAMERA_MATRIX_ROW_COUNT}.
                 */
                CAMERA_MATRIX_COL_COUNT = CAMERA_MATRIX_ROW_COUNT,

                /**
                 * Elements in a camera matrix.
                 *
                 * Same as {@see CAMERA_MATRIX_ROW_COUNT} *
                 * {@see CAMERA_MATRIX_COL_COUNT}.
                 */
                CAMERA_MATRIX_ENTRY_COUNT
                  = CAMERA_MATRIX_ROW_COUNT * CAMERA_MATRIX_COL_COUNT,

                /**
                 * Total size of a camera matrix.
                 */
                TOTAL_DATA_SIZE = 0x26,
            };

            /**
             * Imports an camera matrix file.
             *
             * @param stream[in] The contents of the file.
             * @param dest[out] The formad camera matrix file.
             */
            virtual void ImportCameraMatrixFile(
              Ogre::DataStreamPtr &stream, CameraMatrixFile* dest
            );
    };
}
