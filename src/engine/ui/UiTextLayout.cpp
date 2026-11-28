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
#include <OgreMath.h>
#include "UiTextLayout.h"

namespace{

    /**
     * Height of the screen the sizes are for.
     */
    const float REFERENCE_HEIGHT = 720.0f;
}

UiTextLayout::UiTextLayout(
  const Area& area, const float align_width, const float scroll, const int line_height
):
  area_(area), line_height_(line_height)
{
    line_start_ = -area_.origin.x - ScaleX(align_width - area_.padding_left);
    x_ = line_start_;
    y_ = -area_.origin.y + ScaleY(scroll + area_.padding_top);
}

void UiTextLayout::NewLine(){
    x_ = line_start_;
    y_ += ScaleY(line_height_);
}

bool UiTextLayout::Place(const UiCharData& character, Box& box){
    x_ += ScaleX(character.pre);
    if (
      x_ + ScaleX(character.width) + ScaleX(character.post)
      > area_.size.x - ScaleX(area_.padding_right)
    ) NewLine();
    box.left = x_;
    box.top = y_;
    box.right = x_ + ScaleX(character.width);
    box.bottom = y_ + ScaleY(character.height);
    if (box.bottom > area_.size.y - ScaleY(area_.padding_bottom)) return false;
    x_ += ScaleX(character.width + character.post);
    return true;
}

void UiTextLayout::PlaceImage(const float width, float& x, float& y){
    x = x_ / (area_.scale.x * area_.screen_height / REFERENCE_HEIGHT);
    y = y_ / (area_.scale.y * area_.screen_height / REFERENCE_HEIGHT);
    x_ += ScaleX(width);
}

float UiTextLayout::TextWidth(
  const std::vector<TextChar>& text, const std::function<UiCharData(int)>& character_data
){
    float width = 0;
    float longest = 0;
    for (const TextChar& character : text){
        const UiCharData data = character_data(character.char_code);
        // Line feed.
        if (data.char_code == 10){
            longest = std::max(longest, width);
            width = 0;
        }
        else width += data.pre + data.width + data.post;
    }
    return std::max(longest, width);
}

QuadWriter::Corners UiTextLayout::ScreenCorners(
  const Box& box, const Ogre::Vector2& translate, const float rotation,
  const Ogre::Vector2& screen
){
    QuadWriter::Corners corners{{
      {box.left, box.top}, {box.right, box.top}, {box.right, box.bottom}, {box.left, box.bottom}
    }};
    float cos = 1;
    float sin = 0;
    if (rotation != 0){
        cos = Ogre::Math::Cos(Ogre::Radian(Ogre::Degree(rotation)));
        sin = Ogre::Math::Sin(Ogre::Radian(Ogre::Degree(rotation)));
    }
    for (Ogre::Vector2& corner : corners){
        // The corners are on whole pixels.
        const int x = corner.x * cos - corner.y * sin + translate.x;
        const int y = corner.x * sin + corner.y * cos + translate.y;
        corner.x = (x / screen.x) * 2 - 1;
        corner.y = -((y / screen.y) * 2 - 1);
    }
    return corners;
}

float UiTextLayout::ScaleX(const float size) const{
    return size * area_.scale.x * area_.screen_height / REFERENCE_HEIGHT;
}

float UiTextLayout::ScaleY(const float size) const{
    return size * area_.scale.y * area_.screen_height / REFERENCE_HEIGHT;
}
