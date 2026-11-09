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
#include "core/ScreenFadeState.h"

namespace{

    // Frames per second the NFADE durations are given in.
    const float FRAMES_PER_SECOND = 30.0f;

    // The colour of the opcode as it is given.
    FadeColour Standard(const int r, const int g, const int b){
        FadeColour colour;
        colour.r = r / 255.0f;
        colour.g = g / 255.0f;
        colour.b = b / 255.0f;
        return colour;
    }

    // 255 minus each component of the colour.
    FadeColour Inverse1(const int r, const int g, const int b){
        FadeColour colour;
        colour.r = (255 - r) / 255.0f;
        colour.g = (255 - g) / 255.0f;
        colour.b = (255 - b) / 255.0f;
        return colour;
    }

    // Four times 255 minus each component of the colour. It can go over full intensity.
    FadeColour Inverse4(const int r, const int g, const int b){
        FadeColour colour = Inverse1(r, g, b);
        colour.r *= 4;
        colour.g *= 4;
        colour.b *= 4;
        return colour;
    }

    // Black adds or subtracts nothing.
    FadeColour Black(){return FadeColour();}

    float Clamp(const float value){return std::min(1.0f, std::max(0.0f, value));}
}

float ScreenFadeState::SpeedToSeconds(const int speed){
    if (speed <= 0) return 0;
    return 8.0f / speed;
}

void ScreenFadeState::Begin(
  const Blend blend, const FadeColour& from, const FadeColour& to, const float alpha_from,
  const float alpha_to, const float seconds, const bool hold
){
    blend_ = blend;
    from_ = from;
    to_ = to;
    alpha_from_ = alpha_from;
    alpha_to_ = alpha_to;
    duration_ = seconds;
    elapsed_ = 0;
    hold_ = hold;
    fading_ = seconds > 0;
    holding_ = !fading_ && hold;
    if (!fading_ && !hold) Reset();
}

bool ScreenFadeState::StartFade(
  const int type, const int r, const int g, const int b, const int speed
){
    const float seconds = SpeedToSeconds(speed);
    switch (type){
        case 1:
            Begin(BLEND_SUBTRACT, Inverse4(r, g, b), Black(), 1, 1, seconds, false);
            return true;
        case 2:
            Begin(BLEND_SUBTRACT, Black(), Inverse4(r, g, b), 1, 1, seconds, true);
            return true;
        case 4:
            // The screen goes black at once and stays that way.
            Begin(BLEND_NORMAL, Black(), Black(), 1, 1, 0, true);
            return true;
        case 5:
            Begin(BLEND_ADD, Standard(r, g, b), Black(), 1, 1, seconds, false);
            return true;
        case 6:
            Begin(BLEND_NORMAL, Standard(r, g, b), Standard(r, g, b), 0, 1, seconds, true);
            return true;
        case 7:
            Begin(BLEND_SUBTRACT, Inverse1(r, g, b), Inverse1(r, g, b), 1, 1, seconds, false);
            return true;
        case 8:
            Begin(BLEND_SUBTRACT, Inverse1(r, g, b), Inverse1(r, g, b), 1, 1, seconds, true);
            return true;
        case 9:
            Begin(BLEND_ADD, Standard(r, g, b), Standard(r, g, b), 1, 1, seconds, false);
            return true;
        case 10:
            Begin(BLEND_ADD, Standard(r, g, b), Standard(r, g, b), 1, 1, seconds, true);
            return true;
        default: return false;
    }
}

bool ScreenFadeState::StartNFade(
  const int type, const int r, const int g, const int b, const int frames
){
    const float seconds = frames > 0 ? frames / FRAMES_PER_SECOND : 0;
    switch (type){
        case 0:
            Reset();
            return true;
        case 11:
            Begin(BLEND_ADD, Black(), Standard(r, g, b), 1, 1, seconds, true);
            return true;
        case 12:
            Begin(BLEND_SUBTRACT, Black(), Standard(r, g, b), 1, 1, seconds, true);
            return true;
        default: return false;
    }
}

bool ScreenFadeState::Advance(const float seconds){
    if (!fading_) return false;
    elapsed_ += seconds;
    if (elapsed_ < duration_) return false;
    fading_ = false;
    holding_ = hold_;
    if (!holding_) Reset();
    return true;
}

void ScreenFadeState::Reset(){*this = ScreenFadeState();}

bool ScreenFadeState::IsFading() const{return fading_;}

bool ScreenFadeState::IsVisible() const{return fading_ || holding_;}

ScreenFadeState::Blend ScreenFadeState::GetBlend() const{return IsVisible() ? blend_ : BLEND_NONE;}

FadeColour ScreenFadeState::GetColour() const{
    FadeColour colour = to_;
    if (fading_){
        const float progress = duration_ > 0 ? Clamp(elapsed_ / duration_) : 1.0f;
        colour.r = from_.r + (to_.r - from_.r) * progress;
        colour.g = from_.g + (to_.g - from_.g) * progress;
        colour.b = from_.b + (to_.b - from_.b) * progress;
    }
    colour.r = Clamp(colour.r);
    colour.g = Clamp(colour.g);
    colour.b = Clamp(colour.b);
    return colour;
}

float ScreenFadeState::GetAlpha() const{
    if (!fading_) return Clamp(alpha_to_);
    const float progress = duration_ > 0 ? Clamp(elapsed_ / duration_) : 1.0f;
    return Clamp(alpha_from_ + (alpha_to_ - alpha_from_) * progress);
}
