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

#include "core/Logger.h"
#include "core/UiFont.h"

UiFont::UiFont(const Ogre::String& name, const Ogre::String& language):
  name_(name),
  language_(language),
  image_name_(""),
  image_width_(0),
  image_height_(0),
  height_(0)
{
    // Insets special symbol of next row
    UiCharData data;
    data.char_code = 10;
    data.x = 0;
    data.y = 0;
    data.width = 0;
    data.height = 0;
    data.pre = 0;
    data.post = 0;
    char_data_.push_back(data);
}

UiFont::~UiFont(){}

const Ogre::String& UiFont::GetName() const{return name_;}

const Ogre::String& UiFont::GetLanguage() const{return language_;}

void UiFont::SetImage(
  const Ogre::String& image, const int width, const int height
){
    image_name_ = image;
    image_width_ = width;
    image_height_ = height;
}

const Ogre::String& UiFont::GetImageName() const{return image_name_;}

int UiFont::GetImageWidth() const{return image_width_;
}

int UiFont::GetImageHeight() const{return image_height_;}

void UiFont::SetHeight(const int height){height_ = height;}

int UiFont::GetHeight() const{return height_;}

void UiFont::AddCharData(const UiCharData& data){char_data_.push_back(data);}

UiCharData UiFont::GetCharData(const int char_code) const{
    for (size_t i = 0; i < char_data_.size(); ++ i)
        if(char_data_[i].char_code == char_code) return char_data_[i];
    LOG_ERROR(
      "There is no char with char code "
      + Ogre::StringConverter::toString(char_code) + " in font " + name_ + "."
    );
    UiCharData ret = {};
    return ret;
}
