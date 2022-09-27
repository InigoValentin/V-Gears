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

#include "common/Surface.h"
#include "common/Vram.h"
#include "common/File.h"

/**
 * Handles font files.
 */
class FontFile : public File{

    public:
        // explicit FontFile(const Ogre::String& file);

        /**
         * Constructor.
         *
         * @param file[in] The font file.
         */
        explicit  FontFile(File* file);

        /**
         * Constructor.
         *
         * @param file[in] The font file.
         * @param offset[in] The offset at which to start reading the file.
         * @param length[in] Length of the data to read.
         */
        FontFile(File* file, const u32& offset, const u32& length);

        /**
         * Constructor.
         *
         * @param file[in] Buffer with the contents of the font file.
         * @param offset[in] The offset at which to start reading the file.
         * @param length[in] Length of the data to read.
         */
        FontFile(u8* buffer, const u32& offset, const u32& length);

        /**
         * Desrtuctor/
         */
        virtual ~FontFile(void);

        /**
         * Retrieves the font surface.
         */
        Surface* GetSurface(void);
};
