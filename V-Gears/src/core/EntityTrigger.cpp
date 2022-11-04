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

#include "core/DebugDraw.h"
#include "core/Entity.h"
#include "core/EntityTrigger.h"
#include "core/ConfigVar.h"

ConfigVar cv_debug_trigger(
  "debug_trigger", "Draw entity trigger debug info", "0"
);

int EntityTrigger::NEAR_EVENT_COOLDOWN_FRAMES = 5;

EntityTrigger::EntityTrigger(const Ogre::String& name):
  name_(name),
  enabled_(false),
  point_1_(Ogre::Vector3::ZERO),
  point_2_(Ogre::Vector3::ZERO),
  approached_(false),
  crossed_(false),
  near_single_event_triggered_(false),
  near_event_cooldown_(0)
{}

EntityTrigger::~EntityTrigger(){}

void EntityTrigger::UpdateDebug(){
    if (cv_debug_trigger.GetB() == true){
        if (enabled_ == false) DEBUG_DRAW.SetColour(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
        else if (approached_ > 0) DEBUG_DRAW.SetColour(Ogre::ColourValue(0.04f, 0.9f, 0.5f));
        else DEBUG_DRAW.SetColour(Ogre::ColourValue(0.04f, 0.5f, 0.9f));
        DEBUG_DRAW.Line3d(point_1_, point_2_);
        DEBUG_DRAW.SetColour(Ogre::ColourValue::White);
        DEBUG_DRAW.SetScreenSpace(true);
        DEBUG_DRAW.SetTextAlignment(DEBUG_DRAW.CENTER);
        DEBUG_DRAW.SetFadeDistance(30, 40);
        Ogre::Vector3 center = point_2_ - ((point_2_ - point_1_) / 2);
        DEBUG_DRAW.Text(center, 0, 0, name_);
        DEBUG_DRAW.SetColour(Ogre::ColourValue(0.04f, 0.9f, 0.5f));
    }
}

const Ogre::String& EntityTrigger::GetName() const{return name_;}

void EntityTrigger::SetEnabled(const bool enabled){
    enabled_ = enabled;
    if (enabled == false) Clear();
}

bool EntityTrigger::IsEnabled() const{return enabled_;}

void EntityTrigger::SetPoints(
  const Ogre::Vector3& point1, const Ogre::Vector3& point2
){
    point_1_ = point1;
    point_2_ = point2;
}

const Ogre::Vector3& EntityTrigger::GetPoint1() const{return point_1_;}

const Ogre::Vector3& EntityTrigger::GetPoint2() const{return point_2_;}

const bool EntityTrigger::CheckApproached(){return approached_;}

void EntityTrigger::SetApproached(const bool approached){approached_ = approached;}

const bool EntityTrigger::CheckCrossed(){return crossed_;}

void EntityTrigger::SetCrossed(const bool crossed){crossed_ = crossed;}

const bool EntityTrigger::CheckNearSingleEventTriggered(){return near_single_event_triggered_;}

void EntityTrigger::SetNearSingleEventTriggered(const bool triggered){
    near_single_event_triggered_ = triggered;
}

const int EntityTrigger::GetNearEventCooldown(){return near_event_cooldown_;}

void EntityTrigger::DecreaseNearEventCooldown(){
    if (near_event_cooldown_ > 0) near_event_cooldown_ --;
}

void EntityTrigger::ResetNearEventCooldown(){near_event_cooldown_ = NEAR_EVENT_COOLDOWN_FRAMES;}

void EntityTrigger::ClearNearEventCooldown(){near_event_cooldown_ = 0;}

void EntityTrigger::Clear(){
    approached_ = false;
    crossed_ = false;
    near_single_event_triggered_ = false;
    near_event_cooldown_ = 0;
}

