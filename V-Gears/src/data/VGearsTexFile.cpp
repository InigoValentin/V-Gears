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
#include <OgreImageCodec.h>
#include <OgreGpuProgramManager.h>
#include "data/VGearsTexFile.h"

using Ogre::Exception;
using Ogre::ImageCodec;
using Ogre::MemoryDataStream;
using Ogre::uint32;

namespace VGears{

    const Ogre::uint8 TexFile::USE_REFERENCE_ALPHA(0xFE);

    TexFile::TexFile(){}


    TexFile::~TexFile(){}

    void TexFile::Read(Ogre::DataStreamPtr& input){
        ReadHeader(input);
        size_t image_pixel_count(
          header_.image_data.width * header_.image_data.height);
        image_data_.resize(image_pixel_count);
        memset(&image_data_[0], 0, image_pixel_count * sizeof(TexImageData::value_type));

        if (header_.palette_data.flag) ReadPalleted(input);
        else{
            OGRE_EXCEPT(
              Exception::ERR_NOT_IMPLEMENTED,
              "PixelFormat Data not yet implemented",
              "VGears::TexFile::read"
            );
        }
    }

    void TexFile::ReadHeader(Ogre::DataStreamPtr& input){
        // Read header in full.
        input->read(&header_, sizeof(header_));
        // Endian flip if required, all 32-bit values.
        FlipEndian(&header_, 4, sizeof(header_) / 4);
        if (header_.version != 0x1){
            OGRE_EXCEPT(
              Exception::ERR_NOT_IMPLEMENTED,
              "Only TEX Version 0x1 implemented",
              "VGears::TexFile::read"
            );
        }
    }

    void TexFile::ReadPalleted(Ogre::DataStreamPtr& input){
        size_t image_pixel_count(image_data_.size());
        TexEntry total_color_count(header_.palette_data.total_color_count);
        if (!total_color_count)
            total_color_count = header_.palette_total_color_count;

        TexEntry current_palette(0);
        TexEntry color_key_flag(header_.color_key_flag);
        // if (header_.color_key_array_flag)
        //    color_key_flag = color_key_array[current_palette];

        size_t palette_data_size(
            total_color_count * sizeof(Palette::value_type)
        );
        palette.resize(total_color_count);
        void *ptr_palette(&palette[0]);
        input->read(ptr_palette, palette_data_size);
        FlipEndian(
          ptr_palette, sizeof(Palette::value_type), total_color_count
        );
        typedef std::vector<Ogre::uint8> t_IndexData;
        t_IndexData index_data(image_pixel_count);
        input->read(&index_data[0], image_pixel_count);
        for (size_t i(0); i < image_pixel_count; ++ i){
            t_IndexData::value_type index(index_data[i]);
            if (index < palette.size()) image_data_[i] = palette[index];
            else{
                OGRE_EXCEPT(
                  Exception::ERR_INVALIDPARAMS,
                  "PaletteIndexOutOfBounds",
                  "VGears::TexFile::readPalleted"
               );
            }
        }
    }

    //OgreColourValue TexFile::GetPalettedPixel(unsigned int pixel_index, unsigned int palette){}

    MemoryDataStream* TexFile::GetPixelData() const{
        size_t data_size(
          image_data_.size() * sizeof(TexImageData::value_type)
        );
        MemoryDataStream* data(OGRE_NEW MemoryDataStream(data_size));
        memcpy(data->getPtr(), &image_data_[0], data_size);
        return data;
    }

    ImageCodec::ImageData* TexFile::GetImageData() const{
        ImageCodec::ImageData *img_data = OGRE_NEW ImageCodec::ImageData();
        const ImageData &data(header_.image_data);
        img_data->width  = data.width;
        img_data->height = data.height;
        img_data->size = image_data_.size() * sizeof(TexImageData::value_type);
        img_data->num_mipmaps = 0;
        img_data->flags = 0;
        img_data->depth = 1;
        img_data->format = Ogre::PF_A8R8G8B8;
        return img_data;
    }

    void TexFile::FlipEndian(void * pData, size_t size, size_t count) const{
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        std::cout << "FLIP ENDIAN 1\n";
        for (unsigned int index = 0; index < count; index ++)
            FlipEndian((void *)((long)pData + (index * size)), size);
#endif
    }

    void TexFile::FlipEndian(void * pData, size_t size) const{
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        std::cout << "FLIP ENDIAN 2\n";
        char swap_byte;
        for(unsigned int byteIndex = 0; byteIndex < size/2; byteIndex ++){
            swap_byte = *(char *)((long)pData + byteIndex);
            *(char *)((long)pData + byteIndex)
              = *(char *)((long)pData + size - byteIndex - 1);
            *(char *)((long)pData + size - byteIndex - 1) = swap_byte;
        }
#endif
    }
}
