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

#include <memory>
#include "common/TypeDefine.h"
#include "common/Surface.h"

/**
 * Emulates a VRAM block.
 *
 * The blocks have the same size they had in the Play Station 1 (2048 * 512).
 */
class Vram{

    public:

        /**
         * Creates a VRAM block.
         *
         * @return The VRAM block.
         */
        static std::unique_ptr<Vram> MakeInstance(){return std::unique_ptr<Vram>(new Vram());}

        /**
         * Destructor.
         */
        virtual ~Vram();

        /**
         * Retrieves the block width.
         *
         * @return The block width.
         */
        u16 GetWidth() const;

        /**
         * Retrieves the block width.
         *
         * @return The block width.
         */
        u16 GetHeight() const;

        /**
         * Saves the contents of the VRAM block to a file.
         *
         * @param file[in] Path to the file to save.
         */
        void Save(const Ogre::String& file);

        // TODO: Where is this implemented?:
        /**
         * Adds an image buffer to an existing buffer.
         *
         * @param x[in] X coordinate for the start of the image buffer.
         * @param y[in] Y coordinate for the start of the image buffer.
         * @param width[in] Width of the image buffer.
         * @param height[in] Height of the image buffer.
         * @param buffer[out] The buffer to add the image buffer to.
         */
        void AddImageBuffer(u16 x, u16 y, u16 width, u16 height, u8* buffer);

        /**
         * Writes 8 bytes to the VRAM block.
         *
         * @param x[in] X coordinate of the block where to write.
         * @param y[in] Y coordinate of the block where to write.
         * @param byte[in] The data to write.
         */
        void PutU8(u16 x, u16 y, u8 byte);

        /**
         * Reads 8 bytes from the VRAM block.
         *
         * @param x[in] X coordinate of the block where to read.
         * @param y[in] Y coordinate of the block where to read.
         * @return The read bytes.
         */
        u8 GetU8(u16 x, u16 y) const;

        /**
         * Writes 16 bytes to the VRAM block.
         *
         * @param x[in] X coordinate of the block where to write.
         * @param y[in] Y coordinate of the block where to write.
         * @param byte[in] The data to write.
         */
        void PutU16(u16 x, u16 y, u16 bytes);

        /**
         * Reads 16 bytes from the VRAM block.
         *
         * @param x[in] X coordinate of the block where to read.
         * @param y[in] Y coordinate of the block where to read.
         * @return The read bytes.
         */
        u16 GetU16(u16 x, u16 y) const;

    private:

        /**
         * Constructor.
         */
        Vram();

        /**
         * The data block
         */
        u8 vram_[2048 * 512];

        /**
         * The VRAM block width.
         */
        u16 width_;

        /**
         * The VRAM block height.
         */
        u16 height_;
};
