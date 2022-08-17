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

#include <Ogre.h>
#include "core/UiAnimation.h"
#include "core/UiWidget.h"

UiAnimation::UiAnimation(const Ogre::String& name, UiWidget* widget):
  name_(name),
  widget_(widget),
  time_(0),
  length_(0)
{}

UiAnimation::~UiAnimation(){}

void UiAnimation::AddTime(const float time){
    time_ += time;
    if (time_ > length_) time_ = length_;

    if (scale_.size() > 0) widget_->SetScale(KeyFrameGetValue(scale_));

    if (x_.size() > 0){
        Ogre::Vector2 value = KeyFrameGetValue(x_);
        widget_->SetX(value.x, value.y);
    }

    if (y_.size() > 0){
        Ogre::Vector2 value = KeyFrameGetValue(y_);
        widget_->SetY(value.x, value.y);
    }

    if (width_.size() > 0){
        Ogre::Vector2 value = KeyFrameGetValue(width_);
        widget_->SetWidth(value.x, value.y);
    }

    if (height_.size() > 0){
        Ogre::Vector2 value = KeyFrameGetValue(height_);
        widget_->SetHeight(value.x, value.y);
    }

    if (rotation_.size() > 0) widget_->SetRotation(KeyFrameGetValue(rotation_));

    if (alpha_.size() > 0) widget_->SetAlpha(KeyFrameGetValue(alpha_));

    if (scissor_x_top_.size() > 0){
        Ogre::Vector2 value1 =  KeyFrameGetValue(scissor_x_top_);
        Ogre::Vector2 value2 =  KeyFrameGetValue(scissor_y_left_);
        Ogre::Vector2 value3 =  KeyFrameGetValue(scissor_x_bottom_);
        Ogre::Vector2 value4 =  KeyFrameGetValue(scissor_y_right_);
        widget_->SetScissorArea(
          value1.x, value1.y, value2.x, value2.y,
          value3.x, value3.y, value4.x, value4.y
        );
    }
}

const Ogre::String& UiAnimation::GetName() const{return name_;}

void UiAnimation::SetTime(const float time){time_ = time;}

float UiAnimation::GetTime() const{return time_;}

void UiAnimation::SetLength(const float time){length_ = time;}

float UiAnimation::GetLength() const{return length_;}

void UiAnimation::AddScaleKeyFrame(const UiKeyFrameVector2& key_frame){
    scale_.push_back(key_frame);
}

void UiAnimation::AddXKeyFrame(const UiKeyFrameVector2& key_frame){
    x_.push_back(key_frame);
}

void UiAnimation::AddYKeyFrame(const UiKeyFrameVector2& key_frame){
    y_.push_back(key_frame);
}

void UiAnimation::AddWidthKeyFrame(const UiKeyFrameVector2& key_frame){
    width_.push_back(key_frame);
}

void UiAnimation::AddHeightKeyFrame(const UiKeyFrameVector2& key_frame){
    height_.push_back(key_frame);
}

void UiAnimation::AddRotationKeyFrame(const UiKeyFrameFloat& key_frame){
    rotation_.push_back(key_frame);
}

void UiAnimation::AddAlphaKeyFrame(const UiKeyFrameFloat& key_frame){
    alpha_.push_back(key_frame);
}

void UiAnimation::AddScissorKeyFrame(
  const UiKeyFrameVector2& x1, const UiKeyFrameVector2& y1,
  const UiKeyFrameVector2& x2, const UiKeyFrameVector2& y2
){
    scissor_x_top_.push_back(x1);
    scissor_y_left_.push_back(y1);
    scissor_x_bottom_.push_back(x2);
    scissor_y_right_.push_back(y2);
}

float UiAnimation::KeyFrameGetValue(std::vector<UiKeyFrameFloat>& data){
    float min_value = data[0].value;
    float max_value = min_value;
    float min = 0;
    float max = length_;

    for (unsigned int i = 0; i < data.size(); ++ i){
        if (data[i].time < time_ && data[i].time > min){
            min_value = data[i].value;
            min = data[i].time;
        }
        if (data[i].time >= time_ && data[i].time <= max){
            max_value = data[i].value;
            max = data[i].time;
        }
    }
    float value;
    if (time_ == 0) value = min_value;
    else{
        value
          = min_value + (max_value - min_value) * ((time_ - min) / (max - min));
    }
    return value;
}



Ogre::Vector2 UiAnimation::KeyFrameGetValue(
  std::vector<UiKeyFrameVector2>& data
){
    Ogre::Vector2 min_value = data[0].value;
    Ogre::Vector2 max_value = min_value;
    float min = 0;
    float max = length_;
    for (unsigned int i = 0; i < data.size(); ++ i){
        if (data[i].time < time_ && data[i].time > min){
            min_value = data[i].value;
            min = data[i].time;
        }
        if (data[i].time >= time_ && data[i].time <= max){
            max_value = data[i].value;
            max = data[i].time;
        }
    }
    Ogre::Vector2 value;
    if (time_ == 0) value = min_value;
    else{
        value
          = min_value + (max_value - min_value) * ((time_ - min) / (max - min));
    }
    return value;
}
