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

#include "engine/entity/EntityManager.h"
#include "common/Logger.h"
#include "engine/input/InputManager.h"
#include "engine/script/ScriptManager.h"

void EntityManager::Input(const VGears::Event& event){
    background_2d_.InputDebug(event);
    if (paused_ == true) return;
    if (ignore_held_walk_){
        InputManager& input = InputManager::getSingleton();
        if (
          !input.IsGameEventPressed("walk_left") && !input.IsGameEventPressed("walk_right")
          && !input.IsGameEventPressed("walk_up") && !input.IsGameEventPressed("walk_down")
        ) ignore_held_walk_ = false;
    }
    if (player_entity_ != NULL && player_lock_ == false){
        // Keys held since the world map are ignored until they are released.
        const bool key_ignored = ignore_held_walk_ && (
          event.event == "walk_left" || event.event == "walk_right" || event.event == "walk_up"
          || event.event == "walk_down" || event.event == "run"
        );
        if (!key_ignored){
            if (event.type == VGears::ET_KEY_REPEAT && event.event == "walk_left") player_move_.x = -1;
            else if (event.type == VGears::ET_KEY_REPEAT && event.event == "walk_right")
                player_move_.x = 1;
            if (event.type == VGears::ET_KEY_REPEAT && event.event == "walk_down") player_move_.y = -1;
            else if (event.type == VGears::ET_KEY_REPEAT && event.event == "walk_up")
                player_move_.y = 1;
            if (event.type == VGears::ET_KEY_REPEAT && event.event == "run") player_run_ = true;
        }
        if (event.type == VGears::ET_KEY_PRESS && event.event == "CONFIRM"){
            CheckEntityInteract();
            for (unsigned int i = 0; i < entity_triggers_.size(); ++ i){
                if (
                  entity_triggers_[i]->IsEnabled() == true
                  && entity_triggers_[i]->CheckApproached() == true
                ){
                    ScriptEntity* scr_entity = ScriptManager::getSingleton().GetScriptEntityByName(
                      ScriptManager::ENTITY, entity_triggers_[i]->GetName()
                    );
                    bool added = ScriptManager::getSingleton().ScriptRequest(
                      scr_entity, "on_interact", INTERACT_SCRIPT_PRIORITY, "", "", false, false
                    );
                    if (added == false){
                        LOG_WARNING(
                          "Script 'on_interact' for entity '" + entity_triggers_[i]->GetName()
                          + "' doesn't exist."
                        );
                    }
                }
            }
        }
    }
}

void EntityManager::ScriptSetPlayerEntity(const char* name){
    if (IsBattleModule()){
        LOG_WARNING("Tried to set a player entity but the EntityManager is in battle mode.");
        return;
    }
    for (unsigned int i = 0; i < entity_.size(); ++ i){
        if (entity_[i]->GetName() == name){
            // A script can read the walkmesh triangle of the player right after this, so if it was
            // placed by position the triangle has to be known. Done before it's the player entity
            // so that the entity not being on the walkmesh yet isn't logged as an error.
            if (entity_[i]->GetMoveTriangleId() == -1 && entity_[i]->GetPosition().isZeroLength()){
                // A model that has no position of its own (the fields put the dressed Cloud away
                // until a script makes it the player) takes the place of the previous player,
                // which is where the field entry put the party, as in the original game.
                if (player_entity_ != nullptr && player_entity_ != entity_[i]){
                    entity_[i]->SetPosition(player_entity_->GetPosition());
                    entity_[i]->SetRotation(player_entity_->GetRotation());
                }
            }
            if (entity_[i]->GetMoveTriangleId() == -1) SetEntityOnWalkmesh(entity_[i]);
            LOG_TRIVIAL("Player entity set to '" + std::string(name) + "'.");
            // The camera that follows the player keeps following it when another entity takes over
            // (Cloud and the dressed Cloud, for example); a camera that follows some other entity
            // is left as it is.
            if (player_entity_ != nullptr && background_2d_.GetAutoScrollEntity() == player_entity_)
                background_2d_.ScriptAutoScrollToEntity(entity_[i]);
            // The camera follows the player when no script has said anything about it (the fields
            // without a Director, such as ujunon1, have no opcode that starts it).
            else if (!background_2d_.HasScrollControl())
                background_2d_.ScriptAutoScrollToEntity(entity_[i]);
            player_entity_ = entity_[i];
            // Also, make it non-talkable. Visibility and solidity are left to the scripts, as in
            // the original game: some fields use hidden helper entities (like the 'camera') as
            // the player entity to scroll the screen, and they must stay invisible and
            // non-solid. When the player moves manually, UpdateField makes it solid anyway.
            entity_[i]->SetTalkable(false);
        }
    }
}

void EntityManager::ScriptPlacePlayerAtFieldEntry(
    const float x, const float y, const int triangle_id, const float direction
){
        if (player_entity_ == nullptr || walkmesh_.GetNumberOfTriangles() <= 0) return;

        int safe_triangle_id = triangle_id;
        if (safe_triangle_id < 0 || safe_triangle_id >= walkmesh_.GetNumberOfTriangles()){
                LOG_WARNING("World-map field entry has an invalid walkmesh triangle; using triangle 0.");
                safe_triangle_id = 0;
        }

        player_entity_->SetPosition(Ogre::Vector3(
            x, y, walkmesh_.GetA(static_cast<unsigned int>(safe_triangle_id)).z
        ));
        player_entity_->SetRotation(Ogre::Degree(direction));
        player_entity_->SetMoveTriangleId(safe_triangle_id);
        // In the WALKMESH state the entity walks to its move position: without this one it walks to
        // whatever it had (the origin of the field), and the player enters the town walking.
        player_entity_->SetMovePosition(player_entity_->GetPosition());
        player_entity_->SetState(Entity::WALKMESH);
        // The player starts standing still: no move or run from the world map, and the held keys
        // don't count until they are released.
        player_move_ = Ogre::Vector3::ZERO;
        player_run_ = false;
        ignore_held_walk_ = true;
        player_entity_->PlayAnimationContinue(player_entity_->GetDefaultAnimationName());
}

Entity* EntityManager::ScriptGetPlayerEntity() const{
    if (IsBattleModule()){
        LOG_WARNING("Tried to set a player entity but the EntityManager is in battle mode.");
        return nullptr;
    }
    return player_entity_;
}

void EntityManager::ScriptUnsetPlayerEntity(){
    if (IsBattleModule()){
        LOG_WARNING("Tried to unset a player entity but the EntityManager is in battle mode.");
        return;
    }
    player_entity_ = nullptr;
}

void EntityManager::ScriptSetPlayerSlip(const bool slip){player_slip_ = slip;}

void EntityManager::ScriptPlayerLock(const bool lock){
    if (IsBattleModule()){
        LOG_WARNING("Tried to set player lock but the EntityManager is in battle mode.");
        return;
    }
    player_lock_ = lock;
    if (lock == true) player_move_ = Ogre::Vector3::ZERO;
    // A field with no entity for the leader of the party (the parade of junonr4, where the player
    // is a soldier that no PC opcode names) gives the control to the entity whose script lets go
    // of it.
    else if (player_entity_ == nullptr && IsFieldModule()){
        const ScriptId script = ScriptManager::getSingleton().GetCurrentScriptId();
        Entity* entity = GetEntity(script.entity);
        if (entity != nullptr && !entity->IsLine()){
            LOG_TRIVIAL("The control goes to '" + script.entity + "', there is no player entity.");
            ScriptSetPlayerEntity(script.entity.c_str());
            if (!background_2d_.HasScrollControl()) background_2d_.ScriptAutoScrollToEntity(entity);
        }
    }
}

bool EntityManager::IsPlayerLocked() const{return player_lock_;}

void EntityManager::SetPlayerDefaultRun(const bool run){player_default_run_ = run;}

void EntityManager::SetPlayerMoveRotation(const Ogre::Radian rotation){
    if (IsBattleModule()){
        LOG_WARNING("Tried to set player move rotation the EntityManager is in battle mode.");
        return;
    }
    player_move_rotation_ = rotation;
}

bool EntityManager::IsStartKeyRead() const{
    return std::chrono::steady_clock::now() - start_key_read_time_ < std::chrono::milliseconds(300);
}

bool EntityManager::IsKeyOn(unsigned int key_code){
    if (key_code & 2048) start_key_read_time_ = std::chrono::steady_clock::now();
    // The keys are read even if the player is locked: scenes lock the player and still ask for a
    // button, as the minigames that need to be pressed at the right time do. The scripts that
    // must not read the keys at some point (while a message is open, for example) have their own
    // flag for it. The triggers that check keys aren't run while the player is locked.
    // The key code of the scripts is a mask of the pad buttons, and the check is true if any of
    // them is pressed. Translate each to its game event (see bind_game_event in config.cfg), so
    // the keys chosen in the controller settings are the ones that count. For example 32 is
    // "Circle/Action", the OK button.
    static const struct{unsigned int mask; const char* event;} BUTTONS[] = {
      {1, "L2"}, {2, "R2"}, {4, "L1"}, {8, "R1"}, {16, "MENU"}, {32, "CONFIRM"}, {64, "CANCEL"},
      {128, "EXTRA"}, {256, "SELECT"}, {2048, "START"},
      {4096, "Up"}, {8192, "Right"}, {16384, "Down"}, {32768, "Left"}
    };
    for (const auto& entry : BUTTONS)
        if ((key_code & entry.mask) && InputManager::getSingleton().IsGameEventPressed(entry.event))
            return true;
    // TODO: In the original game the button that makes the player run is the Cancel button (64),
    // so scripts that check Cancel are checking if the player runs (the sneak out of ealin_2, for
    // example). Here running has its own game event, "run", so it counts as Cancel too. Scripts
    // that really want the Cancel button would see it pressed while running. Check which scripts
    // need which, and maybe split them.
    if ((key_code & 64) && InputManager::getSingleton().IsGameEventPressed("run")) return true;
    return false;
}

bool EntityManager::IsKeyOff(unsigned int key_code){return !IsKeyOn(key_code);}
