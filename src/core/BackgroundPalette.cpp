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


#include <algorithm>
#include <OgreResourceGroupManager.h>
#include "core/BackgroundPalette.h"
#include "core/Logger.h"

namespace{
    /**
     * Reads little endian values from a buffer.
     */
    class Reader{
        public:
            Reader(const char* data, size_t size): data_(data), size_(size), pos_(0), ok_(true){}
            uint16_t U16(){
                if (pos_ + 2 > size_){ok_ = false; return 0;}
                const uint16_t v = static_cast<uint8_t>(data_[pos_])
                  | (static_cast<uint8_t>(data_[pos_ + 1]) << 8);
                pos_ += 2;
                return v;
            }
            uint8_t U8(){
                if (pos_ + 1 > size_){ok_ = false; return 0;}
                return static_cast<uint8_t>(data_[pos_ ++]);
            }
            bool Read(void* dest, size_t size){
                if (pos_ + size > size_){ok_ = false; return false;}
                std::copy(data_ + pos_, data_ + pos_ + size, static_cast<char*>(dest));
                pos_ += size;
                return true;
            }
            bool Ok() const{return ok_;}
        private:
            const char* data_;
            size_t size_;
            size_t pos_;
            bool ok_;
    };

    /**
     * Value of a byte parameter as signed.
     */
    int SignedByte(int value){
        value &= 0xFF;
        return (value & 0x80) ? value - 256 : value;
    }
}

bool BackgroundPalette::Load(const Ogre::String& name){
    Clear();
    Ogre::DataStreamPtr stream;
    try{
        stream = Ogre::ResourceGroupManager::getSingleton().openResource(name);
    }
    catch (const Ogre::Exception&){
        LOG_ERROR("Background palette: can't open '" + name + "'.");
        return false;
    }
    std::vector<char> data(stream->size());
    if (!data.empty()) stream->read(data.data(), data.size());
    Reader reader(data.data(), data.size());
    char magic[4] = {0, 0, 0, 0};
    reader.Read(magic, 4);
    if (!reader.Ok() || std::string(magic, 4) != "VGPL"){
        LOG_ERROR("Background palette: '" + name + "' is not a palette file.");
        return false;
    }
    const int page_count = reader.U16();
    const int colors = reader.U16();
    pages_.assign(page_count, std::vector<uint16_t>(colors, 0));
    for (auto& page : pages_){
        for (auto& color : page) color = reader.U16();
    }
    const uint32_t tile_count = reader.U16() | (static_cast<uint32_t>(reader.U16()) << 16);
    for (uint32_t i = 0; i < tile_count && reader.Ok(); ++ i){
        Tile tile;
        tile.x = reader.U16();
        tile.y = reader.U16();
        tile.width = reader.U16();
        tile.height = reader.U16();
        tile.page = reader.U16();
        tile.first_hidden = reader.U8() != 0;
        tile.indexes.resize(static_cast<size_t>(tile.width) * tile.height);
        reader.Read(tile.indexes.data(), tile.indexes.size());
        tiles_.push_back(std::move(tile));
    }
    if (!reader.Ok()){
        LOG_ERROR("Background palette: '" + name + "' is truncated.");
        Clear();
        return false;
    }
    dirty_.assign(pages_.size(), false);
    return true;
}

void BackgroundPalette::Clear(){
    pages_.clear();
    tiles_.clear();
    dirty_.clear();
    std::fill(array_.begin(), array_.end(), 0);
}

bool BackgroundPalette::IsEmpty() const{return tiles_.empty();}

void BackgroundPalette::Store(int page, int slot, int count){
    if (page < 0 || page >= static_cast<int>(pages_.size())) return;
    const int first = slot * SLOT_COLORS;
    for (int i = 0; i < count; ++ i){
        if (first + i < 0 || first + i >= ARRAY_SIZE) break;
        if (i >= static_cast<int>(pages_[page].size())) break;
        array_[first + i] = pages_[page][i];
    }
}

void BackgroundPalette::Load(int slot, int page, int count){
    if (page < 0 || page >= static_cast<int>(pages_.size())) return;
    const int first = slot * SLOT_COLORS;
    for (int i = 0; i < count; ++ i){
        if (first + i < 0 || first + i >= ARRAY_SIZE) break;
        if (i >= static_cast<int>(pages_[page].size())) break;
        pages_[page][i] = array_[first + i];
    }
    dirty_[page] = true;
}

void BackgroundPalette::Copy(int source, int destination, int count){
    const int from = source * SLOT_COLORS;
    const int to = destination * SLOT_COLORS;
    for (int i = 0; i < count; ++ i){
        if (from + i < 0 || from + i >= ARRAY_SIZE || to + i < 0 || to + i >= ARRAY_SIZE) break;
        array_[to + i] = array_[from + i];
    }
}

uint16_t BackgroundPalette::MultiplyColor(uint16_t color, int red, int green, int blue){
    // The original game doubles the 5 bit channels and multiplies, so the multiplier 64 keeps the
    // channel. The green and blue channels take one bit of the channel under them, as it does.
    int r = (((color << 1) & 0x3F) * (red & 0xFF)) >> 7;
    int g = (((color >> 4) & 0x3F) * (green & 0xFF)) >> 7;
    int b = (((color >> 9) & 0x3F) * (blue & 0xFF)) >> 7;
    r = std::min(r, 31);
    g = std::min(g, 31);
    b = std::min(b, 31);
    uint16_t result = static_cast<uint16_t>((b << 10) | (g << 5) | r | (color & 0x8000));
    // Black stays opaque black, not an empty color.
    if (result == 0) result = 0x8000;
    return result;
}

uint16_t BackgroundPalette::AddToColor(uint16_t color, int red, int green, int blue){
    const int r = std::max(0, std::min(31, (color & 0x1F) + red));
    const int g = std::max(0, std::min(31, ((color >> 5) & 0x1F) + green));
    const int b = std::max(0, std::min(31, ((color >> 10) & 0x1F) + blue));
    uint16_t result = static_cast<uint16_t>((b << 10) | (g << 5) | r | (color & 0x8000));
    if (result == 0 && color != 0) result = 0x8000;
    return result;
}

void BackgroundPalette::Multiply(
  int source, int destination, int start, int count, int red, int green, int blue
){
    const int from = source * SLOT_COLORS;
    const int to = destination * SLOT_COLORS;
    for (int i = start; i < start + count; ++ i){
        if (from + i < 0 || from + i >= ARRAY_SIZE || to + i < 0 || to + i >= ARRAY_SIZE) break;
        const uint16_t color = array_[from + i];
        // Empty colors are not changed.
        if (color == 0) continue;
        array_[to + i] = MultiplyColor(color, red, green, blue);
    }
}

void BackgroundPalette::Add(int source, int destination, int red, int green, int blue, int count){
    const int from = source * SLOT_COLORS;
    const int to = destination * SLOT_COLORS;
    red = SignedByte(red);
    green = SignedByte(green);
    blue = SignedByte(blue);
    for (int i = 0; i < count; ++ i){
        if (from + i < 0 || from + i >= ARRAY_SIZE || to + i < 0 || to + i >= ARRAY_SIZE) break;
        array_[to + i] = AddToColor(array_[from + i], red, green, blue);
    }
}

bool BackgroundPalette::IsBrightFirstColor(uint16_t first){
    // A channel of 24 or more out of 31 (see the installer).
    return (first & 0x1F) >= 24 || ((first >> 5) & 0x1F) >= 24 || ((first >> 10) & 0x1F) >= 24;
}

void BackgroundPalette::ToRgba(
  uint16_t color, uint16_t first, int index, bool first_hidden, uint8_t* rgba
){
    // Strange PC version behavior, as in the installer: empty colors are replaced by the first
    // color, and the first color is hidden by a flag.
    const bool empty = (color == 0);
    if (color == 0) color = first;
    const int r = color & 0x1F;
    const int g = (color >> 5) & 0x1F;
    const int b = (color >> 10) & 0x1F;
    rgba[0] = static_cast<uint8_t>(r / 31.0f * 255.0f);
    rgba[1] = static_cast<uint8_t>(g / 31.0f * 255.0f);
    rgba[2] = static_cast<uint8_t>(b / 31.0f * 255.0f);
    // The empty entries that took a visible first color are hidden too (see the installer).
    if (index == 0 && first_hidden) rgba[3] = 0;
    else if (empty && first_hidden && IsBrightFirstColor(first)) rgba[3] = 0;
    else if ((color & 0x8000) && (r != 0 || g != 0 || b != 0)) rgba[3] = 127;
    else rgba[3] = 255;
}

bool BackgroundPalette::Paint(Ogre::Image& image){
    if (std::none_of(dirty_.begin(), dirty_.end(), [](bool d){return d;})) return false;
    uint8_t* data = image.getData();
    const size_t width = image.getWidth();
    const size_t height = image.getHeight();
    if (data == nullptr || image.getFormat() != Ogre::PF_BYTE_RGBA) return false;
    for (const Tile& tile : tiles_){
        if (tile.page >= pages_.size() || !dirty_[tile.page]) continue;
        if (static_cast<size_t>(tile.x) + tile.width > width) continue;
        if (static_cast<size_t>(tile.y) + tile.height > height) continue;
        const std::vector<uint16_t>& colors = pages_[tile.page];
        const uint16_t first = colors.empty() ? 0 : colors[0];
        for (int y = 0; y < tile.height; ++ y){
            for (int x = 0; x < tile.width; ++ x){
                const int index = tile.indexes[static_cast<size_t>(y) * tile.width + x];
                const uint16_t color = index < static_cast<int>(colors.size()) ? colors[index] : 0;
                uint8_t* pixel = data + ((tile.y + y) * width + tile.x + x) * 4;
                ToRgba(color, first, index, tile.first_hidden, pixel);
            }
        }
    }
    std::fill(dirty_.begin(), dirty_.end(), false);
    return true;
}
