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

#include "BackgroundScroll.h"

BackgroundScroll::BackgroundScroll(const Resume& resume):
  resume_(resume), position_(Ogre::Vector2::ZERO), range_(Ogre::AxisAlignedBox::BOX_INFINITE),
  followed_entity_(nullptr), target_entity_(nullptr), start_(Ogre::Vector2::ZERO),
  end_(Ogre::Vector2::ZERO), type_(NONE), seconds_(0), current_seconds_(0), free_(false),
  pending_position_(Ogre::Vector2::ZERO), has_pending_(false)
{}

void BackgroundScroll::SetPosition(const Ogre::Vector2& position){position_ = Clamp(position);}

Ogre::Vector2 BackgroundScroll::Clamp(const Ogre::Vector2& position) const{
    return free_ ? position : ClampToRange(position);
}

Ogre::Vector2 BackgroundScroll::ClampToRange(const Ogre::Vector2& position) const{
    Ogre::Vector2 clamped = position;
    if (!range_.contains(Ogre::Vector3(position.x, position.y, 0))){
        const Ogre::Vector3 maximum = range_.getMaximum();
        const Ogre::Vector3 minimum = range_.getMinimum();
        clamped.makeCeil(Ogre::Vector2(minimum.x, minimum.y));
        clamped.makeFloor(Ogre::Vector2(maximum.x, maximum.y));
    }
    return clamped;
}

void BackgroundScroll::Follow(Entity* entity){
    followed_entity_ = entity;
    free_ = false;
}

bool BackgroundScroll::HasControl() const{
    return followed_entity_ != nullptr || target_entity_ != nullptr || free_ || has_pending_;
}

bool BackgroundScroll::ToEntity(
  Entity* entity, const Ogre::Vector2& position, const SCROLL_TYPE type, const float seconds
){
    followed_entity_ = nullptr;
    return Begin(position, type, seconds, entity);
}

void BackgroundScroll::ToPosition(
  const Ogre::Vector2& position, const SCROLL_TYPE type, const float seconds
){
    followed_entity_ = nullptr;
    free_ = true;
    if (type == NONE || seconds <= 0){
        // The scripts set a position and then scroll from it to another one, in the same frame.
        // The camera is only moved at the end of the frame, so if another scroll comes first the
        // movement starts from where the camera was, and there is no jump.
        // TODO: Some scenes still show a small camera jump (mds7pb_1, when the NPCs go down the
        // pinball machine and when Marlene says "Papa"), and the camera stays fixed after them
        // until the script scrolls to the player. How the original game handles it is not known.
        Abandon();
        pending_position_ = position;
        has_pending_ = true;
        return;
    }
    Begin(position, type, seconds, nullptr);
}

bool BackgroundScroll::ApplyPending(){
    if (!has_pending_) return false;
    has_pending_ = false;
    SetPosition(pending_position_);
    return true;
}

bool BackgroundScroll::Wait(const ScriptId& script){
    if (type_ == NONE) return false;
    waiting_.push_back(script);
    return true;
}

void BackgroundScroll::Stop(){
    type_ = NONE;
    if (target_entity_ != nullptr) Follow(target_entity_);
    target_entity_ = nullptr;
    for (const ScriptId& script : waiting_) resume_(script);
    waiting_.clear();
}

void BackgroundScroll::Clear(){
    target_entity_ = nullptr;
    Stop();
    position_ = Ogre::Vector2::ZERO;
    range_ = Ogre::AxisAlignedBox::BOX_INFINITE;
    followed_entity_ = nullptr;
    start_ = Ogre::Vector2::ZERO;
    end_ = Ogre::Vector2::ZERO;
    seconds_ = 0;
    current_seconds_ = 0;
    free_ = false;
    has_pending_ = false;
}

bool BackgroundScroll::Begin(
  const Ogre::Vector2& position, const SCROLL_TYPE type, const float seconds, Entity* target
){
    // A newer scroll replaces an instant one that has not been applied yet, and finishes the
    // previous one.
    has_pending_ = false;
    Abandon();
    target_entity_ = target;
    if (type == NONE || seconds <= 0){
        // The camera that goes to an entity is limited to the background.
        if (target != nullptr) free_ = false;
        SetPosition(position);
        Stop();
        return true;
    }
    start_ = position_;
    end_ = (target != nullptr) ? ClampToRange(position) : Clamp(position);
    type_ = type;
    seconds_ = seconds;
    current_seconds_ = 0;
    return false;
}

void BackgroundScroll::Abandon(){
    target_entity_ = nullptr;
    Stop();
}
