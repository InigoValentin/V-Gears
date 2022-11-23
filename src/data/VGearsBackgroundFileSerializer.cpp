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

#include <OgreException.h>
#include <OgreLogManager.h>
#include <OgreStringConverter.h>
#include "data/VGearsBackgroundFileSerializer.h"
#include "data/VGearsPaletteFileSerializer.h"

namespace VGears{

    const String BackgroundFileSerializer::SECTION_NAME_PALETTE("PALETTE");

    const String BackgroundFileSerializer::SECTION_NAME_BACK("BACK");

    const String BackgroundFileSerializer::SECTION_NAME_TEXTURE("TEXTURE");

    const String BackgroundFileSerializer::TAG_FILE_END("END");

    const Ogre::Real BackgroundFileSerializer::SRC_BIG_SCALE(10000000.0);

    BackgroundFileSerializer::BackgroundFileSerializer() :
        Serializer(), layer_index_(BackgroundFile::LAYER_COUNT)
    {}

    BackgroundFileSerializer::~BackgroundFileSerializer(){}

    void BackgroundFileSerializer::ImportBackgroundFile(
      Ogre::DataStreamPtr &stream, BackgroundFile *dest
   ){
        ReadFileHeader(stream);
        Read1ByteBool(stream, dest->GetLayers()[0].enabled);
        ReadPallete(stream, dest);
        stream->skip(2 * 2); // 2 *  uint16 unused;
        ReadBackground(stream, dest);
        ReadTexture(stream, dest);
        ReadEnd(stream);
    }

    void BackgroundFileSerializer::ReadFileHeader(Ogre::DataStreamPtr &stream){
        uint16 data[2];
        readShorts(stream, data, 2);
        header_.unused = data[0];
        header_.sort_sprites_by_palette = data[1];
    }

    void
    BackgroundFileSerializer::ReadSectionHeader(Ogre::DataStreamPtr &stream
                                                ,const String &section_name)
    {
        String actual(readString(stream, section_name.size()));
        if (actual != section_name)
        {
            Ogre::LogManager::getSingleton().stream()
                << "Warming: Section didn't start with"
                << " expected String '" << section_name << "'"
                << " actual was '" << actual << "'";
        }

    }

    void BackgroundFileSerializer::ReadEnd(Ogre::DataStreamPtr &stream){
        ReadEndString(stream, TAG_FILE_END);
    }

    void BackgroundFileSerializer::ReadPallete(
      Ogre::DataStreamPtr &stream, BackgroundFile *dest){
        ReadSectionHeader(stream, SECTION_NAME_PALETTE);
        stream->read(
          dest->GetPalette().data(), BackgroundFile::PALETTE_ENTRY_COUNT
        );
    }

    void BackgroundFileSerializer::ReadBackground(
      Ogre::DataStreamPtr &stream, BackgroundFile *dest
    ){
        ReadSectionHeader(stream, SECTION_NAME_BACK);
        auto& layers(dest->GetLayers());
        for (size_t i(0); i < BackgroundFile::LAYER_COUNT; ++ i){
            if (i != 0) Read1ByteBool(stream, layers[i].enabled);
            ReadLayer(stream, &layers[i], i);
        }
    }

    void BackgroundFileSerializer::ReadLayer(
      Ogre::DataStreamPtr &stream, Layer *dest, size_t layer_index
    ){
        if (!dest->enabled) return;
        uint16 tmp[4], sprite_count;
        readShorts(stream, tmp, 4);
        dest->width        = tmp[0];
        dest->height       = tmp[1];
        sprite_count        = tmp[2];
        dest->unknown_06   = tmp[3];
        switch(layer_index)
        {
          case 1:
            readShorts(stream, dest->unknown_08, 3);
          case 2:
          case 3:
            readShorts(stream, dest->unknown_0E, 4);
        }
        stream->skip(2 * 2); // 2 * uint16 unused.
        layer_index_ = layer_index;
        ReadVector(stream, dest->sprites, sprite_count);
        RemoveBuggySprites(dest->sprites);
    }

    void BackgroundFileSerializer::RemoveBuggySprites(SpriteList &sprites){
        auto it = sprites.begin();
        while (it != sprites.end()){
            const SpriteData &sprite = (*it);

            if (
              std::abs(sprite.dst.x) > SPRITE_DST_MAX
              || std::abs(sprite.dst.y) > SPRITE_DST_MAX
            ){
                it = sprites.erase(it);
            }
            else ++ it;
        }
    }

    void BackgroundFileSerializer::readObject(
      Ogre::DataStreamPtr &stream, SpriteData &dest
    ){
        readObject(stream, dest.dst);
        readShorts(stream, dest.unknown_04, 2);
        readObject(stream, dest.src);
        readObject(stream, dest.src2);
        ReadShort(stream, dest.width);
        ReadShort(stream, dest.height);

        uint16 size;
        // width and height are sometimes incorrect in the file
        if (layer_index_ < 2) size = 16;
        else if (layer_index_ < BackgroundFile::LAYER_COUNT) size = 32;
        else{
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "layer_index_ not set correctly",
              "BackgroundFileSerializer::readObject"
            );
        }
        dest.width = dest.height = size;
        ReadShort(stream, dest.palette_page);
        ReadShort(stream, dest.depth);
        // Force depth values
        switch (layer_index_){
            case 0:
                dest.depth = 4095;
                break;
            case 2:
                dest.depth = 4096;
                break;
            case 3:
                dest.depth = 0;
                break;
        }
        uint8 animation[2];
        stream->read(animation, sizeof(animation));
        dest.animation_id = animation[0];
        dest.animation_frame = animation[1];
        uint8 has_blending[2];
        stream->read(has_blending, sizeof(has_blending));
        dest.has_blending[0] = has_blending[0] > 0;
        dest.has_blending[1] = has_blending[1] > 0;
        ReadShort(stream, dest.blending);
        ReadShort(stream, dest.data_page);
        ReadShort(stream, dest.data_page2);
        // When data_page2 != 0, it must be used instead of data_page (not for
        // the first layer).
        if (layer_index_ > 0 && dest.data_page2){
            dest.src = dest.src2;
            dest.data_page = dest.data_page2;
        }
        ReadShort(stream, dest.colour_depth);
        readObject(stream, dest.src_big);
        dest.src_big /= SRC_BIG_SCALE;
        stream->skip(2 * 2); // 2 * uint16 unused
    }

    void BackgroundFileSerializer::ReadTexture(
      Ogre::DataStreamPtr &stream, BackgroundFile *dest
    ){
        ReadSectionHeader(stream, SECTION_NAME_TEXTURE);
        for (auto& page : dest->GetPages()) readObject(stream, page);
    }

    void BackgroundFileSerializer::readObject(
      Ogre::DataStreamPtr& stream, Color& dest
    ){
        uint16 colour;
        ReadShort(stream, colour);
        dest.r = static_cast<float>((colour & BIT_MASK_RED) >> 11);
        dest.g = static_cast<float>((colour & BIT_MASK_GREEN) >> 6);
        dest.b = static_cast<float>(colour & BIT_MASK_BLUE);
        dest /= BIT_SIZE;
        dest.a = 1.0f;
        if (colour == 0) dest.a = 0.0f;
    }

    void BackgroundFileSerializer::readObject(
      Ogre::DataStreamPtr &stream, Page &dest
    ){
        Read2ByteBool(stream, dest.enabled);
        if (!dest.enabled) return;
        ReadShort(stream, dest.unknown_02);
        ReadShort(stream, dest.value_size);
        if (dest.value_size != 1 && dest.value_size != 2){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "Page value_size other then 1 and 2 is not supported",
              "BackgroundFileSerializer::readObject"
            );
        }
        size_t color_count(BackgroundFile::PAGE_DATA_SIZE);
        dest.colors.clear();
        dest.data.clear();
        if (dest.value_size == 2){
            dest.colors.reserve(color_count);
            for (size_t i(color_count); i --;){
                Color colourDest;
                readObject(stream, colourDest);
                dest.colors.push_back(colourDest);
            }
        }
        else{
            dest.data.resize(color_count);
            stream->read(&dest.data[0], color_count);
        }
    }

}
