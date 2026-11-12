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
#include <cmath>
#include "core/MovieState.h"

// Defined here too, as the tests and the scripts take it by reference.
const int MovieState::NO_MOVIE_FRAME;

void MovieState::Start(const int frame_count, const float frames_per_second){
    frame_count_ = std::max(frame_count, 0);
    // A movie without frames is one that isn't there.
    started_ = frame_count_ > 0;
    frames_per_second_ = frames_per_second > 0 ? frames_per_second : 15.0f;
    elapsed_ = 0;
    playing_ = frame_count_ > 0;
}

void MovieState::Prepare(){
    started_ = true;
    playing_ = false;
    frame_count_ = 0;
    elapsed_ = 0;
}

void MovieState::Stop(){playing_ = false;}

bool MovieState::Advance(const float seconds){
    if (!playing_) return false;
    elapsed_ += seconds;
    if (GetFrame() < frame_count_) return false;
    playing_ = false;
    return true;
}

bool MovieState::IsPlaying() const{return playing_;}

int MovieState::GetFrame() const{
    if (!started_) return NO_MOVIE_FRAME;
    const int frame = static_cast<int>(std::floor(elapsed_ * frames_per_second_));
    return std::min(frame, frame_count_);
}
