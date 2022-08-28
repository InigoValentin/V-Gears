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
#include "data/QGearsPaletteFileSerializer.h"

namespace QGears{

    const Ogre::PixelFormat PaletteFileSerializer::PIXEL_FORMAT(
      Ogre::PF_A1R5G5B5
    );

    PaletteFileSerializer::PaletteFileSerializer() : Serializer(){}

    PaletteFileSerializer::~PaletteFileSerializer(){}


    void PaletteFileSerializer::ImportPaletteFile(
      Ogre::DataStreamPtr& stream, PaletteFile* dest
    ){
        ReadFileHeader(stream);
        ReadVector(stream, dest->GetPages(), header_.page_count);
    }

    void PaletteFileSerializer::ReadFileHeader(Ogre::DataStreamPtr& stream){
        ReadUInt32(stream, header_.file_size);
        ReadShort(stream, header_.pal_x);
        ReadShort(stream, header_.pal_y);
        ReadShort(stream, header_.colors_per_page);
        ReadShort(stream, header_.page_count);
    }

    void PaletteFileSerializer::readObject(
      Ogre::DataStreamPtr& stream, Color& dest
    ){
        uint16 colour;
        ReadShort(stream, colour);
        //ConvertColour(colour);
        //Ogre::PixelUtil::unpackColour(&dest, PIXEL_FORMAT, &colour);
        dest.r = static_cast<float>(colour & BIT_MASK_RED);
        dest.g = static_cast<float>((colour & BIT_MASK_GREEN) >> 5);
        dest.b = static_cast<float>((colour & BIT_MASK_BLUE ) >> 10);
        dest /= BIT_SIZE;
        dest.a = 0.0f;
        if (colour & BIT_MASK_ALPHA) dest.a = 1.0f;
    }

    void PaletteFileSerializer::ConvertColour(uint16 &colour) const{
        colour
          = ((colour & BIT_MASK_RED) << 10)
            | (colour & BIT_MASK_GREEN)
            | ((colour & BIT_MASK_BLUE) >> 10)
            | ((colour) & BIT_MASK_ALPHA);
    }

    void PaletteFileSerializer::readObject(
      Ogre::DataStreamPtr &stream, Page &dest
    ){
        ReadVector(stream, dest, header_.colors_per_page);
    }

}
