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
#include <set>
#include <string>
#include <tinyxml.h>
#include "decompiler/field/FieldDecompiler.h"
#include "formats/field/FLevelFile.h"
#include "FieldDataTypes.h"

/**
 * Writes the background of a field: the image with its tiles (tiles.png), their description
 * (bg.xml) and, if a script animates the palettes, the data to repaint them (palette.bin).
 */
class FieldBackgroundInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] data_dir The game data folder.
         */
        explicit FieldBackgroundInstaller(const std::string& data_dir);

        /**
         * Writes the background of a field.
         *
         * @param[in] field The field.
         * @param[in] decompiled The decompiled script of the field. It tells which palettes the
         * scripts animate.
         * @param[in] scale The scale factor of the field.
         */
        void Install(
          const VGears::FLevelFilePtr& field, const FieldDecompiler::DecompiledScript& decompiled,
          const float scale
        );

        /**
         * Names the blending mode of a tile.
         *
         * Type 3 is source + 0.25 * destination, written as "add" with an intensity.
         *
         * @param[in] blending The blending type of the original game.
         * @return The name of the blending mode, or "unknown".
         */
        static std::string BlendingName(const int blending);

    private:

        /**
         * Scale of the background, from the screen of the original game.
         */
        static const int SCALE_UP_FACTOR;

        /**
         * Width of the screen of the original game.
         */
        static const int SCREEN_WIDTH;

        /**
         * Height of the screen of the original game.
         */
        static const int SCREEN_HEIGHT;

        /**
         * Writes the camera, the range and the clip area of a background.
         *
         * @param[in] field The field.
         * @param[in] scale The scale factor of the field.
         * @param[out] background The background element.
         */
        static void WriteCamera(const VGears::FLevelFilePtr& field, const float scale, TiXmlElement& background);

        /**
         * Creates the element of a tile.
         *
         * @param[in] sprite The tile.
         * @param[in] layer Index of the layer of the tile.
         * @param[in] width Width of the image with the tiles.
         * @param[in] height Height of the image with the tiles.
         * @param[in] scale The scale factor of the field.
         * @return The element of the tile.
         */
        static std::unique_ptr<TiXmlElement> CreateTile(
          const VGears::BackgroundFile::SpriteData& sprite, const int layer, const int width,
          const int height, const float scale
        );

        /**
         * Writes the data the engine needs to animate the palettes of a background.
         *
         * Format (little endian): "VGPL", u16 number of pages, u16 colors per page, the colors of
         * the pages as the u16 values of the original game (5 bits per channel and the 0x8000
         * flag), u32 number of tiles, and for each tile: u16 x, y, width, height in the image,
         * u16 page, u8 first color hidden, and one byte per pixel with the palette index.
         *
         * @param[in] path Path of the file.
         * @param[in] palette The palettes of the field.
         * @param[in] tiles The tiles of the background that use a palette.
         * @param[in] pages The palette pages that the scripts animate.
         * @param[in] all_pages True if the scripts may animate any page.
         * @return True if the file was written. It is not when no tile uses an animated palette.
         */
        static bool WritePaletteFile(
          const std::string& path, const VGears::PaletteFilePtr& palette,
          const std::vector<VGears::BackgroundFile::PaletteTile>& tiles,
          const std::set<int>& pages, bool all_pages
        );

        /**
         * The game data folder.
         */
        std::string data_dir_;
};
