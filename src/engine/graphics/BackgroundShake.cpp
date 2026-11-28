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
#include "BackgroundShake.h"

namespace{
    // Random factors the original game applies to each bounce of a shake.
    const float SHAKE_FACTORS[] = {
      -1.0f, 1.14f, -1.34f, 0.61f, -0.49f, 0.64f, -0.25f, 0.47f, -1.20f, -0.01f, 1.14f, -0.86f,
      0.81f, -0.58f, 0.68f, -1.31f, 0.44f, -0.91f, 0.03f, -0.97f, 1.25f, -0.76f, 0.48f, -0.99f,
      1.34f, -0.85f, 0.55f, -0.65f, 0.07f, -0.83f
    };
    const size_t SHAKE_FACTOR_COUNT = sizeof(SHAKE_FACTORS) / sizeof(SHAKE_FACTORS[0]);

    // Frames per second the shake times are given in.
    const float SHAKE_FRAMES_PER_SECOND = 30.0f;

    // Quadratic in and out easing.
    float ShakeEase(const float t){return t < 0.5f ? 2 * t * t : 1 - 2 * (1 - t) * (1 - t);}
}

void BackgroundShake::Axis::Configure(const float new_amplitude, const float new_seconds){
    amplitude = new_amplitude;
    seconds = std::max(new_seconds, 1.0f / SHAKE_FRAMES_PER_SECOND);
    stopping = false;
    if (!running){
        running = true;
        StartBounce();
    }
}

void BackgroundShake::Axis::Stop(const float seconds_to_center){
    if (!running) return;
    stopping = true;
    stop_seconds = std::max(seconds_to_center, 1.0f / SHAKE_FRAMES_PER_SECOND);
}

void BackgroundShake::Axis::StartBounce(){
    from = value;
    elapsed = 0;
    if (stopping){
        to = 0;
        duration = stop_seconds;
    }
    else{
        // Half the amplitude, times the random factor, is the distance in pixels.
        to = amplitude / 2 * SHAKE_FACTORS[step % SHAKE_FACTOR_COUNT];
        ++ step;
        duration = seconds;
    }
}

bool BackgroundShake::Axis::Advance(const float delta){
    if (!running) return false;
    const float old_value = value;
    elapsed += delta;
    while (elapsed >= duration){
        elapsed -= duration;
        value = to;
        if (stopping && to == 0){
            // Back in the center: the shake is over.
            *this = Axis();
            return old_value != 0;
        }
        StartBounce();
    }
    value = from + (to - from) * ShakeEase(elapsed / duration);
    return value != old_value;
}

bool BackgroundShake::Set(
  const int type, const int x_amplitude, const int x_frames, const int y_amplitude,
  const int y_frames
){
    const float x_seconds = x_frames / SHAKE_FRAMES_PER_SECOND;
    const float y_seconds = y_frames / SHAKE_FRAMES_PER_SECOND;
    if (type == 0){
        const float seconds = std::max(x_seconds, y_seconds);
        x_.Stop(seconds);
        y_.Stop(seconds);
        return true;
    }
    if (type < 0 || type > 3) return false;
    // The bits of the type are the axes.
    if (type & 1) x_.Configure(static_cast<float>(x_amplitude), x_seconds);
    if (type & 2) y_.Configure(static_cast<float>(y_amplitude), y_seconds);
    return true;
}

bool BackgroundShake::Update(const float delta){
    if (!IsRunning()) return false;
    const bool x_changed = x_.Advance(delta);
    const bool y_changed = y_.Advance(delta);
    return x_changed || y_changed;
}
