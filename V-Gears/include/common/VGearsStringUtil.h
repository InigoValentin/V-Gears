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

namespace VGears{

    /**
     * Static class for common string manipulations.
     */
    class StringUtil : public Ogre::StringUtil{

        public:

            /**
             * Extract a file path.
             *
             * Extracts a file path from a fully qualified file name (i.e: full
             * filepath + filename).
             *
             * @param[in] name Fully qualified file name.
             * @param[out] path Path to the file, without the file name.
             */
            static void splitPath(const String &name, String &path);

            /**
             * Extract a file name without extension.
             *
             * Extracts a file name, without the file extension, from a fully
             * qualified file name (i.e: full filepath + filename).
             *
             * @param[in] name Fully qualified file name.
             * @param[out] basename File name, without extension.
             */
            static void splitBase(const String &name, String &basename);

            /**
             * Extract a file name without extension.
             *
             * Extracts a file name, without the file extension, from a full
             * file name without path.
             *
             * @param[in] name File name.
             * @param[out] basename File name, without extension.
             */
            static void splitFull(const String &name, String &basename);
    };
}
