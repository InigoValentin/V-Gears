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
#include "core/Background2DAnimation.h"
#include "core/Background2D.h"
#include "core/Logger.h"

Background2DAnimation::Background2DAnimation(
  const Ogre::String& name, Background2D* background, const int tile_index):
    name_(name),
    background_(background),
    tile_index_(tile_index),
    time_(0),
    length_(0)
{}

Background2DAnimation::~Background2DAnimation(){}

void Background2DAnimation::AddTime(const float time){
    time_ += time;
    if(time_ > length_) time_ = length_;
    if(uv_.size() > 0){
        int min_id = -1;
        float min_time = 0;
        for (unsigned int i = 0; i < uv_.size(); ++ i){
            if (uv_[i].time <= time_ && uv_[i].time >= min_time){
                min_id = i;
                min_time = uv_[i].time;
            }
        }

        if(min_id != -1){
            background_->UpdateTileUV(
              tile_index_, uv_[min_id].u1, uv_[min_id].v1,
              uv_[min_id].u2, uv_[min_id].v2);
        }
    }
}

const Ogre::String& Background2DAnimation::GetName() const{return name_;}

void Background2DAnimation::SetTime(const float time){time_ = time;}

float Background2DAnimation::GetTime() const{return time_;}

void Background2DAnimation::SetLength(const float time){length_ = time;}

float Background2DAnimation::GetLength() const{return length_;}

void Background2DAnimation::AddUVKeyFrame(const VGears::KeyFrame key_frame){
    AddUVKeyFrame(key_frame.time, key_frame.uv);
}

void Background2DAnimation::AddUVKeyFrame(
  const float time, const Ogre::Vector4& uv
){
    AddUVKeyFrame(time, uv.x, uv.y, uv.z, uv.w);
}

void Background2DAnimation::AddUVKeyFrame(
  const float time,
  const float u1, const float v1, const float u2, const float v2
){
    Background2DKeyFrameUV uv_key_frame;
    uv_key_frame.time = time;
    uv_key_frame.u1 = u1;
    uv_key_frame.v1 = v1;
    uv_key_frame.u2 = u2;
    uv_key_frame.v2 = v2;
    uv_.push_back(uv_key_frame);
}
