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
     * Defines game dimensions.
     */
    enum{

        /**
         * Game horizontal resolution, in pixels.
         */
        SCREEN_WIDTH = 320,

        /**
         * Game vertical resolution, in pixels.
         */
        SCREEN_HEIGHT = 240,

        /**
         * @todo Understand and document.
         */
        FIELD_POSITION_SCALE = 128,

        /**
         * @todo Understand and document.
         */
        FIELD_DEPTH_SCALE = 32
    };

    /**
     * Extension for A files.
     */
    extern const String EXT_A;
}
