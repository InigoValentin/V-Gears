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
#include <OgreMath.h>
#include "engine/Timer.h"
#include "engine/entity/EntityManager.h"
#include "common/Logger.h"
#include "engine/script/ScriptManager.h"

void EntityManager::UpdateField(){
    // Update all entity scripts
    ScriptManager::getSingleton().Update(ScriptManager::ENTITY);
    // The player is walking if there is a move set by the input.
    const bool walked = player_entity_ != nullptr && player_move_ != Ogre::Vector3::ZERO;
    if (walked) ApplyPlayerInput();

    for (unsigned int i = 0; i < entity_.size(); ++ i){
        entity_[i]->Update();
        // It's not needed to update movements it delta time == 0 (for example
        // if time is stopped)
        if (Timer::getSingleton().GetGameTimeDelta() == 0) continue;
        UpdateFieldEntity(entity_[i]);
    }
    if (IsFieldModule()) triggers_.CheckContacts(player_entity_, player_lock_, entity_);
    // Reset player move. It already must be handled in update
    player_move_ = Ogre::Vector3::ZERO;
    player_run_ = false;
    if (background_2d_.GetScrollType() != Background2D::NONE) SetNextScrollStep();
    background_2d_.Update();
    // This can start a battle and change the module, so it's the last thing to do.
    encounters_.Update(player_entity_, player_lock_, walked);
    // The movie follows the speed of the game, so the scenes that are timed with it stay in sync.
    // Its sound doesn't, so at any other speed they drift apart.
    movie_player_.Update(Timer::getSingleton().GetGameTimeDelta());
    screen_fade_.Update(Timer::getSingleton().GetGameTimeDelta());
}

void EntityManager::ApplyPlayerInput(){
    const Entity::State state = player_entity_->GetState();
    if (state == Entity::WALKMESH || state == Entity::NONE){
        // A scripted walk of an entity that isn't the player keeps the last triangle when it goes
        // over an edge, so the stored triangle can be far from the position. Reattach to the
        // walkmesh before moving manually, otherwise every move would be blocked.
        if (!movement_.IsInCurrentTriangle(*player_entity_))
            movement_.PlaceOnWalkmesh(player_entity_);

        // The directional keys are relative to the screen. Rotate them to the field's move
        // rotation here, once per frame. This can't be done in Input, which runs once for each
        // event, so the rotation would be applied several times when there is more than one event
        // in the frame (diagonals, running, ...). player_move_ stays as the raw input, which is
        // what ladders need.
        Ogre::Quaternion rotation;
        rotation.FromAngleAxis(player_move_rotation_, Ogre::Vector3::UNIT_Z);
        // Normalized, so diagonals aren't faster than straight moves.
        Ogre::Vector3 input = player_move_;
        input.normalise();
        const Ogre::Vector3 walk_move = rotation * input
          * (player_entity_->GetMoveWalkSpeed() * Timer::getSingleton().GetGameTimeDelta());
        player_entity_->SetMovePosition(player_entity_->GetPosition() + walk_move);
        player_entity_->SetState(Entity::WALKMESH);
    }

    // HACK: If manually moving, make playable entity solid.
    // Some scripts 'forget' to make the playable entity solid at the end of them, and if it
    // remains non-solid, it can't interact with lines (i.e. it cant' use gateways).
    player_entity_->SetSolid(true);
}

void EntityManager::UpdateFieldEntity(Entity* entity){
    // Update screen scroll.
    if (background_2d_.GetAutoScrollEntity() == entity)
        background_2d_.SetScroll(background_2d_.GetEntityScroll(entity));
    // Update offset.
    if (entity->GetOffsetType() != AT_NONE) SetNextOffsetStep(entity);
    // Update turn.
    if (entity->GetTurnType() != AT_NONE) SetNextTurnStep(entity);
    // Update movement.
    switch (entity->GetState()){
        case Entity::WALKMESH:
            UpdateWalkingEntity(entity);
            break;
        case Entity::LINEAR:
            CheckTriggers(entity, entity->GetPosition());
            movement_.LinearStep(entity);
            break;
        case Entity::JUMP:
            movement_.JumpStep(entity);
            break;
        case Entity::NEEDS_TO_REATTACH:
            if (movement_.PlaceOnWalkmesh(entity)) entity->SetState(Entity::WALKMESH);
            else entity->SetState(Entity::NONE);
            // The entity stands in this frame, whatever its new state is.
            UpdateStandingEntity(entity);
            break;
        case Entity::NONE:
            UpdateStandingEntity(entity);
            break;
    }
}

void EntityManager::UpdateWalkingEntity(Entity* entity){
    // Try set entity on walkmesh if it's still don't has triangle.
    if (entity->GetMoveTriangleId() == -1 && !movement_.PlaceOnWalkmesh(entity)){
        LOG_TRIVIAL("Can't set entity '" + entity->GetName() + "' on walkmesh. Finish move.");
        entity->UnsetMove();
    }
    if (entity->GetState() != Entity::WALKMESH) return;

    const float speed = GetWalkingSpeed(entity);
    const bool moved = WalkEntity(entity, speed);
    // The scripts set the animation of a scene as the default one (DFANM) and don't set it back,
    // but the player stands normally again as soon as they walk in the original game (blin67_3,
    // where Cloud ends the scene crouched).
    if (
      moved && !player_lock_ && player_entity_ == entity
      && entity->GetDefaultAnimationName() != "Idle"
    ){
        entity->ScriptSetDefaultAnimation("Idle");
    }
    if (entity->GetMoveAutoAnimation() == false) return;
    if (moved){
        entity->PlayAnimationContinue(
          speed >= entity->GetMoveRunSpeed()
            ? entity->GetMoveAnimationRunName() : entity->GetMoveAnimationWalkName()
        );
    }
    else if (entity->GetAnimationState() != Entity::REQUESTED_ANIMATION)
        entity->PlayAnimationContinue(entity->GetDefaultAnimationName());
}

void EntityManager::UpdateStandingEntity(Entity* entity){
    CheckTriggers(entity, entity->GetPosition());
    if (entity->GetAnimationState() != Entity::REQUESTED_ANIMATION)
        entity->PlayAnimationContinue(entity->GetDefaultAnimationName());
}

float EntityManager::GetWalkingSpeed(const Entity* entity) const{
    if (player_lock_ || player_entity_ != entity) return entity->GetMoveAutoSpeed();
    float speed = entity->GetMoveWalkSpeed();
    // TODO: Set as a constant. Or even better, as a configurable variable.
    // The run key inverts the default movement.
    if (player_run_ != player_default_run_) speed *= 4;
    return speed;
}

bool EntityManager::WalkEntity(Entity* entity, const float speed){
    float remaining_time = Timer::getSingleton().GetGameTimeDelta();
    if (!entity->IsScriptedMove() || !(speed > 0.0f))
        return movement_.Walk(entity, speed, remaining_time);
    const float max_step_time = 0.2f / speed;
    bool any_move = false;
    while (remaining_time > 0.0f && entity->GetState() == Entity::WALKMESH){
        const float step_time = std::min(remaining_time, max_step_time);
        if (!movement_.Walk(entity, speed, step_time)) break;
        any_move = true;
        remaining_time -= step_time;
    }
    return any_move;
}

void EntityManager::SetNextOffsetStep(Entity* entity){
    // TODO: Allow this while on battle mode?
    ActionType type = entity->GetOffsetType();
    float total = entity->GetOffsetSeconds();
    float current = entity->GetOffsetCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Vector3 start = entity->GetOffsetPositionStart();
    Ogre::Vector3 end = entity->GetOffsetPositionEnd();
    // An offset of zero seconds is done at once. Dividing would give NaN.
    float x = (total > 0.0f) ? current / total : 1.0f;
    if (std::isnan(x)) x = 0.0f;
    float smooth_modifier = (type == AT_SMOOTH) ? -2 * x * x * x + 3 * x * x : x;
    Ogre::Vector3 offset = start + (end - start) * smooth_modifier;
    entity->SetOffset(offset);
    if (current == total) entity->UnsetOffset();
    else entity->SetOffsetCurrentSeconds(current);
}

void EntityManager::SetNextTurnStep(Entity* entity){
    // TODO: Allow this while on battle mode?
    ActionType type = entity->GetTurnType();
    float total = entity->GetTurnSeconds();
    float current = entity->GetTurnCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Degree start = entity->GetTurnDirectionStart();
    Ogre::Degree end = entity->GetTurnDirectionEnd();
    // A turn of zero seconds is done at once. Dividing would give NaN, and a NaN rotation breaks
    // the bounding box of the entity.
    float x = (total > 0.0f) ? current / total : 1.0f;
    float smooth_modifier = (type == AT_SMOOTH) ? -2 * x * x * x + 3 * x * x : x;
    Ogre::Degree rotation = start + (end - start) * smooth_modifier;
    entity->SetRotation(rotation);
    if (current == total) entity->UnsetTurn();
    else entity->SetTurnCurrentSeconds(current);
}

void EntityManager::SetNextScrollStep(){
    // TODO: Allow this on battle mode?
    background_2d_.UpdateScrollTarget();
    Background2D::SCROLL_TYPE type = background_2d_.GetScrollType();
    float total = background_2d_.GetScrollSeconds();
    float current = background_2d_.GetScrollCurrentSeconds();
    current += Timer::getSingleton().GetGameTimeDelta();
    current = (current > total) ? total : current;
    Ogre::Vector2 start = background_2d_.GetScrollPositionStart();
    Ogre::Vector2 end = background_2d_.GetScrollPositionEnd();
    float x = (total > 0.0f) ? current / total : 1.0f;
    float smooth_modifier = (type == Background2D::SMOOTH) ? -2 * x * x * x + 3 * x * x : x;
    Ogre::Vector2 scroll = start + (end - start) * smooth_modifier;
    background_2d_.SetScroll(scroll);
    if (current == total) background_2d_.UnsetScroll();
    else background_2d_.SetScrollCurrentSeconds(current);
}
