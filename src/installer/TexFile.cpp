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

#include <string>
#include <iostream>
#include <OgreImage.h>
#include <OgreColourValue.h>
#include "TexFile.h"

TexFile::TexFile(std::string path){
    File file(path);
    Read(file);
}

TexFile::TexFile(File file){
    Read(file);
}

void TexFile::Read(File file){
    file.SetOffset(0);
    version_ = file.readU32LE();
    if (version_ != 0x00000001){
        std::cout << "[WARNING] Tried to read an invalid TEX file " << file.GetFileName()
          << " to png. Version ID: " << version_ << std::endl;
        return;
    }
    unknown_0_ = file.readU32LE();
    colour_key_ = file.readU32LE();
    unknown_1_ = file.readU32LE();
    unknown_2_ = file.readU32LE();
    min_bpc_ = file.readU32LE();
    max_bpc_ = file.readU32LE();
    min_alpha_bits_ = file.readU32LE();
    max_alpha_bits_ = file.readU32LE();
    min_bpp_ = file.readU32LE();
    max_bpp_ = file.readU32LE();
    unknown_3_ = file.readU32LE();
    palette_count_ = file.readU32LE();
    palette_colour_count_ = file.readU32LE();
    bit_depth_ = file.readU32LE();
    width_ = file.readU32LE();
    height_ = file.readU32LE();
    pitch_ = file.readU32LE();
    unknown_4_ = file.readU32LE();
    has_palette_ = file.readU32LE();
    bits_per_index_ = file.readU32LE();
    indexed_to_8_bit_ = file.readU32LE();
    palette_size_ = file.readU32LE();
    colours_per_palette_ = file.readU32LE();
    run_0_ = file.readU32LE();
    bits_per_pixel_ = file.readU32LE();
    bytes_per_pixel_ = file.readU32LE();
    bits_red_ = file.readU32LE();
    bits_green_ = file.readU32LE();
    bits_blue_ = file.readU32LE();
    bits_alpha_ = file.readU32LE();
    bitmask_red_ = file.readU32LE();
    bitmask_green_ = file.readU32LE();
    bitmask_blue_ = file.readU32LE();
    bitmask_alpha_ = file.readU32LE();
    shift_red_ = file.readU32LE();
    shift_green_ = file.readU32LE();
    shift_blue_ = file.readU32LE();
    shift_alpha_ = file.readU32LE();
    bits_red_2_ = file.readU32LE();
    bits_green_2_ = file.readU32LE();
    bits_blue_2_ = file.readU32LE();
    bits_alpha_2_ = file.readU32LE();
    max_red_ = file.readU32LE();
    max_green_ = file.readU32LE();
    max_blue_ = file.readU32LE();
    max_alpha_ = file.readU32LE();
    colour_k_array_ = file.readU32LE();
    run_1_ = file.readU32LE();
    ref_alpha_ = file.readU32LE();
    run_2_ = file.readU32LE();
    unknown_5_ = file.readU32LE();
    palette_index_ = file.readU32LE();
    run_3_ = file.readU32LE();
    run_4_ = file.readU32LE();
    unknown_6_ = file.readU32LE();
    unknown_7_ = file.readU32LE();
    unknown_8_ = file.readU32LE();
    unknown_9_ = file.readU32LE();
    if (has_palette_ == 0){
        // Non palleted images.
        // Skip unused pallete data.
        for (int c = 0; c < palette_size_; c ++) file.readU32LE();
        // Read 32 byte colours in format BGRA.
        for (int p = 0; p < width_ * height_; p ++){
            float b = file.readU8() / 255.0f;
            float g = file.readU8() / 255.0f;
            float r = file.readU8() / 255.0f;
            float a = file.readU8() / 255.0f;
            Ogre::ColourValue colour(r, g, b, a);
            pixel_colour_.push_back(colour);
        }
    }
    else {
        // Palleted_images.
        palette_count_ = palette_size_ / palette_colour_count_;
        // Read palette data.
        for (int c = 0; c < palette_count_; c ++){
            std::vector<Ogre::ColourValue> palette_colours;
            for (int p = 0; p < palette_colour_count_; p ++){
                float b = file.readU8() / 255.0f;
                float g = file.readU8() / 255.0f;
                float r = file.readU8() / 255.0f;
                float a = file.readU8() / 255.0f;
                Ogre::ColourValue colour(r, g, b, a);
                palette_colours.push_back(colour);
            }
            palettes_.push_back(palette_colours);
        }
        // Read bytes as references to a palette colour.
        for (int p = 0; p < width_ * height_; p ++) pixel_ref_.push_back(file.readU8());
    }
}

TexFile::~TexFile(){}

void TexFile::SavePng(
  std::string file_name, unsigned int x, unsigned int y, unsigned int w, unsigned int h,
  unsigned int palette
){
    if (version_ != 0x00000001){
        std::cout
          << "[WARNING] Tried to convert an invalid TEX file to png. Version ID: "
          << version_ << std::endl;
        return;
    }
    // TODO: Warn if wrong palette index.
    Ogre::Image* image = new Ogre::Image(Ogre::PF_R8G8B8A8_UINT, w, h);
    int target_x = 0;
    int target_y = 0;
    for (int c_y = y; c_y < height_ && c_y < y + h; c_y ++){
        for (int c_x = x; c_x < width_ && c_x < x + w; c_x ++){
            int i = (width_ * c_y) + c_x;
            float r;
            float g;
            float b;
            float a;
            if (has_palette_ == 0){
                r = pixel_colour_.at(i).r;
                g = pixel_colour_.at(i).g;
                b = pixel_colour_.at(i).b;
                a = pixel_colour_.at(i).a;
            }
            else{
                if (palettes_.at(palette).size() <= pixel_ref_.at(i)){
                    // If invalid index, use alpha
                    r = 0;
                    g = 0;
                    b = 0;
                    a = 0;
                }
                else{
                    r = palettes_.at(palette).at(pixel_ref_.at(i)).r;
                    g = palettes_.at(palette).at(pixel_ref_.at(i)).g;
                    b = palettes_.at(palette).at(pixel_ref_.at(i)).b;
                    a = palettes_.at(palette).at(pixel_ref_.at(i)).a;
                }
            }
            image->setColourAt(Ogre::ColourValue(r, g, b, a), target_x, target_y, 0);
            target_x ++;
            if (target_x >= w){
                target_y ++;
                target_x = 0;
            }
        }
    }
    image->save(file_name);
}

void TexFile::SavePng(std::string file_name, unsigned int palette){
    //if (invalid_) return;
    SavePng(file_name, 0, 0, width_, height_, palette);
}

void TexFile::SavePng(
  std::string file_name, unsigned int x1, unsigned int x2, unsigned int y1, unsigned int y2,
  unsigned int w1, unsigned int w2, unsigned int h, unsigned int palette
){
    if (version_ != 0x00000001){
        std::cout
          << "[WARNING] Tried to convert an invalid TEX file to png. Version ID: "
          << version_ << std::endl;
        return;
    }
    // TODO: Warn if wrong palette index.
    Ogre::Image* image = new Ogre::Image(Ogre::PF_R8G8B8A8_UINT, w1 + w2, h);
    int target_x = 0;
    int target_y = 0;
    // First image
    for (int c_y = y1; c_y < height_ && c_y < y1 + h; c_y ++){
        for (int c_x = x1; c_x < width_ && c_x < x1 + w1; c_x ++){
            int i = (width_ * c_y) + c_x;
            float r;
            float g;
            float b;
            float a;
            if (has_palette_ == 0){
                r = pixel_colour_.at(i).r;
                g = pixel_colour_.at(i).g;
                b = pixel_colour_.at(i).b;
                a = pixel_colour_.at(i).a;
            }
            else{
                r = palettes_.at(palette).at(pixel_ref_.at(i)).r;
                g = palettes_.at(palette).at(pixel_ref_.at(i)).g;
                b = palettes_.at(palette).at(pixel_ref_.at(i)).b;
                a = palettes_.at(palette).at(pixel_ref_.at(i)).a;
            }
            image->setColourAt(Ogre::ColourValue(r, g, b, a), target_x, target_y, 0);
            target_x ++;
            if (target_x >= w1){
                target_y ++;
                target_x = 0;
            }
        }
    }
    target_x = w1;
    target_y = 0;
    for (int c_y = y2; c_y < height_ && c_y < y2 + h; c_y ++){
        for (int c_x = x2; c_x < width_ && c_x < x2 + w2; c_x ++){
            int i = (width_ * c_y) + c_x;
            float r;
            float g;
            float b;
            float a;
            if (has_palette_ == 0){
                r = pixel_colour_.at(i).r;
                g = pixel_colour_.at(i).g;
                b = pixel_colour_.at(i).b;
                a = pixel_colour_.at(i).a;
            }
            else{
                r = palettes_.at(palette).at(pixel_ref_.at(i)).r;
                g = palettes_.at(palette).at(pixel_ref_.at(i)).g;
                b = palettes_.at(palette).at(pixel_ref_.at(i)).b;
                a = palettes_.at(palette).at(pixel_ref_.at(i)).a;
            }
            image->setColourAt(Ogre::ColourValue(r, g, b, a), target_x, target_y, 0);
            target_x ++;
            if (target_x >= w1 + w2){
                target_y ++;
                target_x = w1;
            }
        }
    }
    image->save(file_name);
}
