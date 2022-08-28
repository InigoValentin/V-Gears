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

#include <OgreStringConverter.h>
#include "Vram.h"
#include "Logger.h"
#include <string.h>

Vram::Vram(): width_(2048), height_(512){memset(vram_, 0, 2048 * 512);}

Vram::~Vram(){}

u16 Vram::GetWidth() const{return width_;}

u16 Vram::GetHeight() const{return height_;}

void Vram::Save(const Ogre::String& file){
    Ogre::TexturePtr ptex;
    Ogre::HardwarePixelBufferSharedPtr buffer;
    ptex = Ogre::TextureManager::getSingleton().createManual(
      "DynaTex", "General", Ogre::TEX_TYPE_2D, width_ / 2, height_,
      0, Ogre::PF_R8G8B8A8, Ogre::TU_STATIC
    );
    buffer = ptex->getBuffer(0, 0);
    buffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
    const Ogre::PixelBox& pb = buffer->getCurrentLock();
    // IVV type cast
    u32* data = static_cast<u32*>((unsigned int *) pb.data);
    for (int i = 0, size = width_ * height_ / 2; i < size; ++ i){
        u16 col = (vram_[i * 2 + 1] << 8) | vram_[i * 2 + 0];
        data[i]
          = (((col & 0x1f) * 255 / 31) << 0x18)
          | ((((col >>  5) & 0x1f) * 255 / 31) << 0x10)
          | ((((col >> 10) & 0x1f) * 255 / 31) << 0x8)
          | 0xff;
    }
    Ogre::Image image;
    image.loadDynamicImage(
      (Ogre::uchar*) pb.data, width_ / 2, height_, Ogre::PF_R8G8B8A8
    );
    image.save(file + ".png");
    buffer->unlock();
}

void Vram::PutU8(u16 x, u16 y, u8 byte){
    if (x > width_ || y > height_){
        LOGGER->Log(
          "Vram::PutU8: Error: Tried to put byte in Vram x("
          + Ogre::StringConverter::toString(x) + ") y("
          + Ogre::StringConverter::toString(y) + ")\n"
        );
        return;
    }
    vram_[y * width_ + x] = byte;
}

u8 Vram::GetU8(u16 x, u16 y) const{
    if (x > width_ || y > height_){
        LOGGER->Log(
          "Vram::GetU8: Error: Tried to get byte from Vram x("
          + Ogre::StringConverter::toString(x) + ") y("
          + Ogre::StringConverter::toString(y) + ")\n"

        );
        return 0;
    }
    return vram_[y * width_ + x];
}

void Vram::PutU16(u16 x, u16 y, u16 bytes){
    if (x + 1 > width_ || y > height_){
        LOGGER->Log(
          "Vram::PutU16: Error: Tried to put pixel in Vram x("
          + Ogre::StringConverter::toString(x) + ") y("
          + Ogre::StringConverter::toString(y) + ")\n"
        );
        return;
    }
    // This byte order is used to make possible store everything in VRAM
    // as U8 but get as U16LE.
    vram_[y * width_ + x + 1] = bytes >> 8;
    vram_[y * width_ + x + 0] = bytes & 0x00ff;
}

u16 Vram::GetU16(u16 x, u16 y) const{
    if (x + 1 > width_ || y > height_){
        LOGGER->Log(
          "Vram::GetU16: Error: Tried to get pixel from Vram x("
          + Ogre::StringConverter::toString(x) + ") y("
          + Ogre::StringConverter::toString(y) + ")\n"
        );
        return 0;
    }
    // This byte order is used to make possible store everything in VRAM
    // as U8 but get as U16LE.
    return (vram_[y * width_ + x + 1] << 8) | vram_[y * width_ + x + 0];
}
