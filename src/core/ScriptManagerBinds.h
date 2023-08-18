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

#include <iostream>

#include "../modules/worldmap/WorldmapModule.h"
#include "Console.h"
#include "Logger.h"
#include "CameraManager.h"
#include "Enemy.h"
#include "Entity.h"
#include "EntityManager.h"
#include "BattleManager.h"
#include "AudioManager.h"
#include "SavemapHandler.h"
#include "Timer.h"
#include "UiManager.h"
#include "UiWidget.h"
#include "XmlMapFile.h"
#include "XmlMapsFile.h"
#include "DialogsManager.h"
#include "TextHandler.h"


/*
 * TODO:
 *
 * If I manage to upgrade luabind version (or even better, remove it and use an external one),
 * I could pass entire objects to Lua scripts as tables, and this will be waaaaay easier.
 */

/**
 * Prints to the game console.
 *
 * @param[in] text The text to be printed.
 */
void ScriptPrint(const char* text){
    std::cout << "[SCRIPT] " << text << std::endl;
    Console::getSingleton().AddTextToOutput(text);
}

/**
 * Changes the current map.
 *
 * Reads the data for the next map, initializes all it's entities, scripts, etc
 * and loads it.
 *
 * @param[in] text The new map name.
 */
void ScriptMap(const char* text){
    EntityManager::getSingleton().Clear();
    XmlMapsFile xml("./data/fields/_fields.xml");
    Ogre::String file_name = xml.GetMapFileNameByName(text);
    XmlMapFile xml_map("./data/fields/" + file_name);
    xml_map.LoadMap();
}

/**
 * Executes a script in the game console.
 *
 * It should not be related to the game itself. The script output will be
 * written to the game console.
 *
 * @param[in] text The script command to execute.
 */
void ScriptConsole(const char* text){
    Console::getSingleton().ExecuteCommand(text);
}

void ScriptManager::InitBinds(){

    // Global functions.
    luabind::module(lua_state_)[
        luabind::def("print", (void(*)(const char*)) &ScriptPrint),
        luabind::def("map", (void(*)(const char*)) &ScriptMap),
        luabind::def("console", (void(*)(const char*)) &ScriptConsole)
    ];

    // Individual entity commands.
    luabind::module(lua_state_)[
        luabind::class_<Entity>("Entity")
          .def(
            "set_position",
            (void(Entity::*)(const float, const float, const float)) &Entity::ScriptSetPosition
          )
          // Internally returns 3 values:
          .def("get_position", (void(Entity::*)()) &Entity::ScriptGetPosition)
          .def("set_rotation", (void(Entity::*)(const float)) &Entity::ScriptSetRotation)
          .def("get_rotation", (float(Entity::*)()) &Entity::ScriptGetRotation)
          .def("set_solid_radius", (void(Entity::*)(const float)) &Entity::SetSolidRadius)
          .def("get_solid_radius", (float(Entity::*)()) &Entity::GetSolidRadius)
          .def("set_solid", (void(Entity::*)(const bool)) &Entity::SetSolid)
          .def("is_solid", (bool(Entity::*)()) &Entity::IsSolid)
          .def("set_talk_radius", (void(Entity::*)(const float)) &Entity::SetTalkRadius)
          .def("get_talk_radius", (float(Entity::*)()) &Entity::GetTalkRadius)
          // Some old test script use this, its just an alias for SetTalkable:
          .def("set_interactable", (void(Entity::*)(const bool)) &Entity::SetTalkable)
          .def("set_talkable", (void(Entity::*)(const bool)) &Entity::SetTalkable)
          .def("is_talkable", (bool(Entity::*)()) &Entity::IsTalkable)
          .def("set_visible", (void(Entity::*)(const bool)) &Entity::SetVisible)
          .def("is_visible", (bool(Entity::*)()) &Entity::IsVisible)
          .def("set_move_auto_speed", (void(Entity::*)(const float)) &Entity::SetMoveAutoSpeed)
          .def("get_move_auto_speed", (float(Entity::*)()) &Entity::GetMoveAutoSpeed)
          .def("get_move_triangle_id", (int(Entity::*)()) &Entity::GetMoveTriangleId)
          .def("set_move_triangle_id", (void(Entity::*)(const int)) &Entity::SetMoveTriangleId)
          .def("move_auto_rotation", (void(Entity::*)(const bool)) &Entity::SetMoveAutoRotation)
          .def("move_auto_animation", (void(Entity::*)(const bool)) &Entity::SetMoveAutoAnimation)
          .def(
            "move_to_position",
            (void(Entity::*)(const float, const float)) &Entity::ScriptMoveToPosition
          )
          .def("move_to_entity", (void(Entity::*)(Entity*)) &Entity::ScriptMoveToEntity)
          .def("move_sync", (int(Entity::*)()) &Entity::ScriptMoveSync, luabind::yield)
          .def(
            "linear_to_position",
            (void(Entity::*)(
              const float, const float, const float, const LinearMovement,
              const char*, const float, const int
            )) &Entity::ScriptLinearToPosition
          )
          .def(
            "linear_sync", (int(Entity::*)()) &Entity::ScriptLinearSync, luabind::yield
          )
          .def(
            "jump_to_position",
            (void(Entity::*)(const float, const float, const float, const float, const int))
              &Entity::ScriptJumpToPosition
          )
          .def("jump_sync", (int(Entity::*)()) &Entity::ScriptJumpSync, luabind::yield)
          .def(
            "offset_to_position",
            (void(Entity::*)(const float, const float, const float, const ActionType, const float))
              &Entity::ScriptOffsetToPosition
          )
          .def("offset_sync", (int(Entity::*)()) &Entity::ScriptOffsetSync, luabind::yield)
          .def(
            "turn_to_entity",
            (void(Entity::*)(Entity*, const TurnDirection, const float)) &Entity::ScriptTurnToEntity
          )
          .def(
            "turn_to_direction", (
              void(Entity::*)(const float, const TurnDirection, const ActionType, const float)
            ) &Entity::ScriptTurnToDirection)
          .def("turn_sync", (int(Entity::*)()) &Entity::ScriptTurnSync, luabind::yield)
          .def(
            "set_animation_speed", (void(Entity::*)(const float)) &Entity::ScriptSetAnimationSpeed
          )
          .def("play_animation", (void(Entity::*)(const char*)) &Entity::ScriptPlayAnimation)
          .def(
            "play_animation_stop", (void(Entity::*)(const char*)) &Entity::ScriptPlayAnimationStop
          )
          .def(
            "play_animation",
            (void(Entity::*)(const char*, const float, const float)) &Entity::ScriptPlayAnimation
          )
          .def(
            "play_animation_stop",
            (void(Entity::*)(const char*, const float, const float))
            &Entity::ScriptPlayAnimationStop
          )
          .def(
            "set_default_animation",
            (void(Entity::*)(const char*)) &Entity::ScriptSetDefaultAnimation
          )
          .def("animation_sync", (int(Entity::*)()) &Entity::ScriptAnimationSync, luabind::yield)
          .enum_("constants")[
             luabind::value("NONE", AT_NONE),
             luabind::value("LINEAR", AT_LINEAR),
             luabind::value("SMOOTH", AT_SMOOTH),
             luabind::value("CLOCKWISE", TD_CLOCKWISE),
             luabind::value("ANTICLOCKWISE", TD_ANTICLOCKWISE),
             luabind::value("CLOSEST", TD_CLOSEST),
             luabind::value("UP_TO_DOWN", LM_UP_TO_DOWN),
             luabind::value("DOWN_TO_UP", LM_DOWN_TO_UP),
             luabind::value("LEFT_TO_RIGHT", LM_LEFT_TO_RIGHT),
             luabind::value("RIGHT_TO_LEFT", LM_RIGHT_TO_LEFT)
           ]
    ];

    // Field commands.
    // TODO: Duplicated in group EntityManager, this can probably be deleted
    luabind::module(lua_state_)[
        luabind::class_<EntityManager>("Field")
          .def(
            "random_encounters_on",
            (float(EntityManager::*)(bool)) &EntityManager::SetRandomEncounters
          )
    ];

    // Entity individual point commands
    luabind::module(lua_state_)[
        luabind::class_<EntityPoint>("EntityPoint")
          // Internally returns 3 values:
          .def("get_position", (void(EntityPoint::*)()) &EntityPoint::ScriptGetPosition)
          .def("get_rotation", (float(EntityPoint::*)()) &EntityPoint::ScriptGetRotation)
    ];

    // Commands for the entity manager, not related to any particular entity.
    luabind::module(lua_state_)[
        luabind::class_<EntityManager>("EntityManager")
          .def("set_paused", (void(EntityManager::*)(const bool)) &EntityManager::ScriptSetPaused)
          .def(
             "add_entity",
             (void(EntityManager::*)(
               const char*, const char*, const float,const float,
               const float, const float, const int
             )) &EntityManager::ScriptAddEntity
          )
          .def(
             "add_entity_script",
             (void(EntityManager::*)(const char*)) &EntityManager::ScriptAddEntityScript
          )
          .def(
             "get_entity", (Entity*(EntityManager::*)(const char*)) &EntityManager::ScriptGetEntity
          )
          .def(
             "get_entity_from_index",
             (Entity*(EntityManager::*)(const int)) &EntityManager::GetEntityFromIndex
           )
          .def(
             "get_entity_from_character_id",
             (Entity*(EntityManager::*)(const int)) &EntityManager::GetEntityFromCharacterId
          )
          .def(
             "get_entity_point",
             (EntityPoint*(EntityManager::*)(const char*)) &EntityManager::ScriptGetEntityPoint
          )
          .def(
             "set_player_entity",
             (void(EntityManager::*)(const char*)) &EntityManager::ScriptSetPlayerEntity
          )
          .def(
             "get_player_entity",
             (Entity*(EntityManager::*)()) &EntityManager::ScriptGetPlayerEntity
          )
          .def(
            "unset_player_entity",
            (void(EntityManager::*)()) &EntityManager::ScriptUnsetPlayerEntity
          )
          .def(
            "player_lock", (void(EntityManager::*)(const bool)) &EntityManager::ScriptPlayerLock
          )
          .def(
            "random_encounters_on",
            (float(EntityManager::*)(bool)) &EntityManager::SetRandomEncounters
          )
          .def("is_key_on", (bool(EntityManager::*)(unsigned int)) &EntityManager::IsKeyOn)
          .def("is_key_off", (bool(EntityManager::*)(unsigned int)) &EntityManager::IsKeyOff)
          .def(
            "set_entity_to_character",
            (void(EntityManager::*)(const char*, unsigned int)) &EntityManager::SetEntityToCharacter
          )
    ];

    // Commands for the battle manager.
    luabind::module(lua_state_)[
        luabind::class_<BattleManager>("BattleManager")
          .def(
            "start_battle", (void(BattleManager::*)(const unsigned int)) &BattleManager::StartBattle
          )
          .def("end_battle", (void(BattleManager::*)()) &BattleManager::EndBattle)
          .def("get_enemy_count", (int(BattleManager::*)()) &BattleManager::ScriptGetEnemyCount)
          .def(
            "get_enemy",
            (Enemy*(BattleManager::*)(const unsigned int)) &BattleManager::ScriptGetEnemy
          )
          .def(
            "add_attack_model",
            (void(BattleManager::*)(
              const unsigned int, const char*, const float,
              const float, const float, const int, const char*
            )) &BattleManager::ScriptAddAttackModel
          )
    ];

    // Commands for enemy handling.
    luabind::module(lua_state_)[
    luabind::class_<Enemy>("Enemy")
      .def("get_id", (int(Enemy::*)()) &Enemy::GetId)
      .def("get_name", (std::string(Enemy::*)()) &Enemy::GetName)
      .def("get_level", (int(Enemy::*)()) &Enemy::GetLevel)
      .def("get_ap", (int(Enemy::*)()) &Enemy::GetAp)
      .def("get_exp", (int(Enemy::*)()) &Enemy::GetExp)
      .def("get_money", (int(Enemy::*)()) &Enemy::GetMoney)
      .def("get_str", (int(Enemy::*)()) &Enemy::GetStr)
      .def("get_def", (int(Enemy::*)()) &Enemy::GetDef)
      .def("get_mag", (int(Enemy::*)()) &Enemy::GetMag)
      .def("get_spr", (int(Enemy::*)()) &Enemy::GetSpr)
      .def("get_dex", (int(Enemy::*)()) &Enemy::GetDex)
      .def("get_lck", (int(Enemy::*)()) &Enemy::GetLck)
      .def("get_eva", (int(Enemy::*)()) &Enemy::GetEva)
      .def("get_meva", (int(Enemy::*)()) &Enemy::GetMeva)
      .def("get_hp", (int(Enemy::*)()) &Enemy::GetHp)
      .def("get_hp_max", (int(Enemy::*)()) &Enemy::GetHpMax)
      .def("get_mp", (int(Enemy::*)()) &Enemy::GetMp)
      .def("get_mp_max", (int(Enemy::*)()) &Enemy::GetMpMax)
      .def("get_position", (void(Enemy::*)()) &Enemy::ScriptGetPos)
      .def("is_front", (bool(Enemy::*)()) &Enemy::IsFront)
      .def("is_visible", (bool(Enemy::*)()) &Enemy::IsVisible)
      .def("is_targeteable", (bool(Enemy::*)()) &Enemy::IsTargeteable)
      .def("is_active", (bool(Enemy::*)()) &Enemy::IsActive)
      .def("get_attack_count", (int(Enemy::*)()) &Enemy::ScriptGetAttackCount)
      .def("get_attack", (int(Enemy::*)(const unsigned int)) &Enemy::ScriptGetAttack)
      .def(
        "get_camera_for_attack",
        (int(Enemy::*)(const unsigned int)) &Enemy::ScriptGetCameraForAttack
      )
      // TODO: Implement cover flags.
      // TODO: Implement immunities
      // TODO: Implement elements
      // TODO: Implement item drops
      // TODO: Implement stealable items
      // TODO: Implement manipulability status and attacks.
      // TODO: Implement morph item
];

    // Commands for the audio manager.
    luabind::module(lua_state_)[
        luabind::class_<AudioManager>("AudioManager")
          .def("play_music", (void(AudioManager::*)(const char*)) &AudioManager::ScriptPlayMusic)
          .def("play_sound", (void(AudioManager::*)(const char*)) &AudioManager::ScriptPlaySound)
          .def(
            "play_sound",
            (void(AudioManager::*)(const char*, const int)) &AudioManager::ScriptPlaySound
          )
          .def(
            "play_sounds",
            (void(AudioManager::*)(const char*, const char*, const char*, const char*))
            &AudioManager::ScriptPlaySounds
          )
          .def("get_track_id", (int(AudioManager::*)(int)) &AudioManager::ScriptGetTrack)
          .def("get_battle_track_id", (int(AudioManager::*)()) &AudioManager::ScriptGetBattleTrack)
          .def(
            "set_battle_track_id", (void(AudioManager::*)(int)) &AudioManager::ScriptSetBattleTrack
          )
    ];

    // Commands for the audio manager.
    luabind::module(lua_state_)[
        luabind::class_<CameraManager>("CameraManager")
          .def("set_camera",
             (void(CameraManager::*)(
               const int, const int, const int, const int, const int, const int
             )) &CameraManager::ScriptSetCamera
           )
    ];

    // Commands for the savemap handelr.
    luabind::module(lua_state_)[
        luabind::class_<SavemapHandler>("SavemapHandler")
          .def(
            "release", (void(SavemapHandler::*)()) &SavemapHandler::Release
          )
          .def(
            "get_current_savemap",
            (Savemap*(SavemapHandler::*)()) &SavemapHandler::GetCurrentSavemap
          )
          .def(
            "get_savemap",
            (Savemap*(SavemapHandler::*)(const unsigned int)) &SavemapHandler::GetSavemap
          )
          .def(
            "set_data",
            (void(SavemapHandler::*)(const unsigned int, const unsigned int, const int))
              &SavemapHandler::SetData
          )
          .def(
            "set_control_key", (void(SavemapHandler::*)(const char*)) &SavemapHandler::SetControlKey
          )
          .def(
            "set_window_colours",
            (void(SavemapHandler::*)(
              const unsigned int, const unsigned int, const unsigned int,
              const unsigned int, const unsigned int, const unsigned int,
              const unsigned int, const unsigned int, const unsigned int,
              const unsigned int, const unsigned int, const unsigned int
            )) &SavemapHandler::SetWindowColours
          )
          .def("set_money", (void(SavemapHandler::*)(const unsigned int)) &SavemapHandler::SetMoney)
          .def(
            "set_game_time",
            (void(SavemapHandler::*)(const unsigned int)) &SavemapHandler::SetGameTime
          )
          .def(
            "set_countdown_time",
            (void(SavemapHandler::*)(const unsigned int)) &SavemapHandler::SetCountdownTime
          )
          .def(
            "set_key_item",
            (void(SavemapHandler::*)(const unsigned int, const bool)) &SavemapHandler::SetKeyItem
          )
          .def(
            "set_party",
            (void(SavemapHandler::*)(const int, const int, const int)) &SavemapHandler::SetParty
          )
          .def(
            "set_item",
            (void(SavemapHandler::*)(const unsigned int, const unsigned int, const unsigned int))
              &SavemapHandler::SetItem
          )
          .def(
            "set_materia",
            (void(SavemapHandler::*)(const unsigned int, const unsigned int, const unsigned int))
              &SavemapHandler::SetMateria
          )
          .def(
            "set_e_skill_materia",
            (void(SavemapHandler::*)(const unsigned int, const unsigned int, const bool))
              &SavemapHandler::SetESkillMateria
          )
          .def(
            "set_materia_stash",
            (void(SavemapHandler::*)(const unsigned int, const unsigned int, const unsigned int))
              &SavemapHandler::SetMateriaStash
          )
          .def(
            "set_e_skill_materia_stash",
            (void(SavemapHandler::*)(const unsigned int, const unsigned int, const bool))
              &SavemapHandler::SetESkillMateriaStash
          )
          .def(
            "set_location",
            (void(SavemapHandler::*)(
              const float, const float, const float, const unsigned int, const int,
              const char*, const char*
            )) &SavemapHandler::SetLocation
          )
          .def(
            "set_setting",
            (void(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::SetSetting
          )
          .def(
            "set_character_info",
            (void(SavemapHandler::*)(
              const unsigned int, const int, const char*, const bool, const bool, const unsigned int,
              const unsigned int, const bool, const unsigned int, const unsigned int,
              const unsigned int, const unsigned int, const unsigned int, const unsigned int,
              const int
            )) &SavemapHandler::SetCharacterInfo
          )
          .def(
            "set_character_stat",
            (void(SavemapHandler::*)(
              const unsigned int, const unsigned int, const unsigned int, const unsigned int
            )) &SavemapHandler::SetCharacterStat
          )
          .def(
            "set_character_limit_learned",
            (void(SavemapHandler::*)(
              const unsigned int, const unsigned int, const unsigned int,
              const bool, const unsigned int
            )) &SavemapHandler::SetCharacterLimitLearned
          )
          .def(
            "set_character_materia",
            (void(SavemapHandler::*)(
              const unsigned int, const bool, const unsigned int,
              const unsigned int, const unsigned int
            )) &SavemapHandler::SetCharacterMateria
          )
          .def(
            "set_character_e_skill_materia",
            (void(SavemapHandler::*)(
              const unsigned int, const bool, const unsigned int, const unsigned int, const bool
            )) &SavemapHandler::SetCharacterESkillMateria
          )
          .def(
            "set_character_status",
            (void(SavemapHandler::*)(const unsigned int, const unsigned int, const bool))
              &SavemapHandler::SetCharacterStatus
          )
          .def(
            "save", (bool(SavemapHandler::*)(const unsigned int, const bool)) &SavemapHandler::Save
          )
          .def(
            "is_slot_empty",
            (bool(SavemapHandler::*)(const unsigned int)) &SavemapHandler::IsSlotEmpty
          )
          .def(
            "get_slot_control_key",
            (std::string(SavemapHandler::*)(const unsigned int)) &SavemapHandler::GetSlotControlKey
          )
          .def(
            "get_slot_control_key",
            (unsigned int(SavemapHandler::*)(
              const unsigned int, const unsigned int, const unsigned int
            )) &SavemapHandler::GetSlotWindowCornerColourComponent
          )
          .def(
            "get_slot_money",
            (unsigned int(SavemapHandler::*)(const unsigned int)) &SavemapHandler::GetSlotMoney
          )
          .def(
            "get_slot_game_time",
            (unsigned int(SavemapHandler::*)(const unsigned int)) &SavemapHandler::GetSlotGameTime
          )
          .def(
            "get_slot_countdown_time",
            (unsigned int(SavemapHandler::*)(const unsigned int))
              &SavemapHandler::GetSlotCountdownTime
          )
          .def(
            "get_slot_party_member",
            (int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotPartyMember
          )
          .def(
            "get_slot_item_at_pos_id",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotItemAtPosId
          )
          .def(
            "get_slot_item_at_pos_qty",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotItemAtPosQty
          )
          .def(
            "get_slot_key_item",
            (bool(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotKeyItem
          )
          .def(
            "get_slot_materia_at_pos_id",
            (int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotMateriaAtPosId
          )
          .def(
            "get_slot_materia_at_pos_ap",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotMateriaAtPosAp
          )
          .def(
            "is_slot_materia_at_pos_e_skill",
            (bool(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::IsSlotMateriaAtPosESkill
          )
          .def(
            "is_slot_materia_at_pos_e_skill_learned",
            (bool(SavemapHandler::*)(
              const unsigned int, const unsigned int, const unsigned int
            )) &SavemapHandler::IsSlotMateriaAtPosESkillLearned
          )
          .def(
            "get_slot_stash_at_pos_id",
            (int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotStashAtPosId
          )
          .def(
            "get_slot_stash_at_pos_ap",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotStashAtPosAp
          )
          .def(
            "is_slot_stash_at_pos_e_skill",
            (bool(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::IsSlotStashAtPosESkill
          )
          .def(
            "is_slot_stash_at_pos_e_skill_learned",
            (bool(SavemapHandler::*)(
              const unsigned int, const unsigned int, const unsigned int
            )) &SavemapHandler::IsSlotStashAtPosESkillLearned
          )
          .def(
            "get_slot_location_x",
            (float(SavemapHandler::*)(const unsigned int)) &SavemapHandler::GetSlotLocationX
          )
          .def(
            "get_slot_location_y",
            (float(SavemapHandler::*)(const unsigned int)) &SavemapHandler::GetSlotLocationY
          )
          .def(
            "get_slot_location_z",
            (float(SavemapHandler::*)(const unsigned int)) &SavemapHandler::GetSlotLocationZ
          )
          .def(
            "get_slot_location_triangle",
            (unsigned int(SavemapHandler::*)(const unsigned int))
              &SavemapHandler::GetSlotLocationTriangle
          )
          .def(
            "get_slot_location_angle",
            (unsigned int(SavemapHandler::*)(const unsigned int))
              &SavemapHandler::GetSlotLocationAngle
          )
          .def(
            "get_slot_location_field",
            (std::string(SavemapHandler::*)(const unsigned int))
              &SavemapHandler::GetSlotLocationField
          )
          .def(
            "get_slot_location_name",
            (std::string(SavemapHandler::*)(const unsigned int))
              &SavemapHandler::GetSlotLocationName
          )
          .def(
            "get_slot_setting",
            (int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotSetting
          )
          .def(
            "get_slot_character_char_id",
            (int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterCharId
          )
          .def(
            "get_slot_character_name",
            (std::string(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterName
          )
          .def(
            "get_slot_character_level",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterLevel
          )
          .def(
            "get_slot_character_kills",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterKills
          )
          .def(
            "is_slot_character_enabled",
            (bool(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::IsSlotCharacterEnabled
          )
          .def(
            "is_slot_character_locked",
            (bool(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::IsSlotCharacterLocked
          )
          .def(
            "is_slot_character_back_row",
            (bool(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::IsSlotCharacterBackRow
          )
          .def(
            "get_slot_character_exp",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterExp
          )
          .def(
            "get_slot_character_exp_to_next",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterExpToNext
          )
          .def(
            "get_slot_character_limit_level",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterLimitLevel
          )
          .def(
            "get_slot_character_limit_bar",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterLimitBar
          )
          .def(
            "get_slot_character_weapon_id",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterWeaponId
          )
          .def(
            "get_slot_character_armor_id",
            (unsigned int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterArmorId
          )
          .def(
            "get_slot_character_accessory_id",
            (int(SavemapHandler::*)(const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotCharacterAccessoryId
          )
          .def(
            "get_slot_character_stat_base",
            (unsigned int(SavemapHandler::*)(
              const unsigned int, const unsigned int, const unsigned int
            )) &SavemapHandler::GetSlotCharacterStatBase
          )
          .def(
            "get_slot_character_stat_extra",
            (unsigned int(SavemapHandler::*)(
              const unsigned int, const unsigned int, const unsigned int
            )) &SavemapHandler::GetSlotCharacterStatExtra
          )
          .def(
            "get_slot_character_limit_uses",
            (unsigned int(SavemapHandler::*)(
              const unsigned int, const unsigned int, const unsigned int
            )) &SavemapHandler::GetSlotCharacterLimitUses
          )
          .def(
            "is_slot_character_limit_learned",
            (unsigned int(SavemapHandler::*)(
              const unsigned int, const unsigned int, const unsigned int, const unsigned int
            )) &SavemapHandler::IsSlotCharacterLimitLearned
          )
          .def(
            "get_slot_character_materia_id",
            (int(SavemapHandler::*)(
              const unsigned int, const unsigned int, const bool, const unsigned int
            )) &SavemapHandler::GetSlotCharacterMateriaId
          )
          .def(
            "get_slot_character_materia_ap",
            (unsigned int(SavemapHandler::*)(
              const unsigned int, const unsigned int, const bool, const unsigned int
            )) &SavemapHandler::GetSlotCharacterMateriaAp
          )
          .def(
            "get_slot_character_materia_e_skill",
            (bool(SavemapHandler::*)(
              const unsigned int, const unsigned int, const bool,
              const unsigned int, const unsigned int
            )) &SavemapHandler::IsSlotCharacterMateriaESkill
          )
          .def(
            "get_slot_character_materia_e_skill_learned",
            (bool(SavemapHandler::*)(
              const unsigned int, const unsigned int, const bool,
              const unsigned int, const unsigned int
            )) &SavemapHandler::IsSlotCharacterMateriaESkillLearned
          )
          .def(
            "get_slot_data",
            (int(SavemapHandler::*)(const unsigned int, const unsigned int, const unsigned int))
              &SavemapHandler::GetSlotData
          )
    ];

    // 2D background and camera commands
    luabind::module(lua_state_)[
        luabind::class_<Background2D>("Background2D")
          .def(
            "autoscroll_to_entity",
            (void(Background2D::*)(Entity*)) &Background2D::ScriptAutoScrollToEntity
          )
          .def(
            "scroll_to_player",
            (void(Background2D::*)(const Background2D::SCROLL_TYPE, const unsigned int))
              &Background2D::ScriptScrollToPlayer
          )
          .def(
            "scroll_to_position",
            (void(Background2D::*)(
              const float, const float, const Background2D::SCROLL_TYPE, const float
            )) &Background2D::ScriptScrollToPosition
          )
          .def(
            "scroll_sync", (int(Background2D::*)()) &Background2D::ScriptScrollSync, luabind::yield
          )
          .def(
            "offset", (void(Background2D::*)(const float, const float)) &Background2D::ScriptOffset
          )
          .def(
            "play_animation_looped",
            (void(Background2D::*)(const char*)) &Background2D::ScriptPlayAnimationLooped
          )
          .def(
            "play_animation_once",
            (void(Background2D::*)(const char*)) &Background2D::ScriptPlayAnimationOnce
          )
          .def(
            "animation_sync",
            (int(Background2D::*)(const char*)) &Background2D::ScriptAnimationSync,
            luabind::yield
          )
          .enum_("constants")[
            luabind::value("NONE", AT_NONE),
            luabind::value("LINEAR", AT_LINEAR),
            luabind::value("SMOOTH", AT_SMOOTH)
          ]
    ];

    // Walkmesh commands
    luabind::module(lua_state_)[
        luabind::class_<Walkmesh>("Walkmesh")
          .def("lock_walkmesh", (void(Walkmesh ::*)(unsigned int, bool)) &Walkmesh ::LockWalkmesh)
          .def("is_locked", (bool(Walkmesh ::*)(unsigned int)) &Walkmesh ::IsLocked)
    ];

    // Text handler commands
    luabind::module(lua_state_)[
        luabind::class_<TextHandler>("TextHandler")
          .def(
            "set_character_name",
            (void(TextHandler ::*)(unsigned int, const char*)) &TextHandler::SetCharacterName
          )
          .def("set_party", (void(TextHandler ::*)(int, int, int)) &TextHandler::SetParty)
    ];

    // Dialog commands
    luabind::module(lua_state_)[
        luabind::class_<DialogsManager>("Dialog")
          .def(
            "dialog_open",
            (void(DialogsManager::*)(const char*, int, int, int, int)) &DialogsManager::OpenDialog
          )
          .def(
            "set_numeric",
            (void(DialogsManager::*)(const char*, const bool, const bool))
              &DialogsManager::SetNumeric
          )
          .def(
            "update_timer",
            (void(DialogsManager::*)(const unsigned int)) &DialogsManager::UpdateTimer
          )
          .def(
            "set_mode",
            (void(DialogsManager::*)(const char*, const unsigned int, const bool))
              &DialogsManager::SetMode
          )
          .def(
            "dialog_set_text",
            (void(DialogsManager::*)(const char*, const char*)) &DialogsManager::SetText
          )
          .def(
            "dialog_wait_for_close",
            (int(DialogsManager::*)(const char*)) &DialogsManager::Sync,
            luabind::yield
          )
          .def("dialog_close", (void(DialogsManager::*)(const char*)) &DialogsManager::Hide)
          .def(
            "set_variable",
            (void(DialogsManager::*)(const char*, const char*, const char*))
              &DialogsManager::SetVariable
          )
          .def(
            "set_clickable",
            (void(DialogsManager::*)(const char*, const bool)) &DialogsManager::SetClickable
          )
          .def(
            "set_cursor",
            (void(DialogsManager::*)(const char*, const int, const int)) &DialogsManager::SetCursor
          )
          .def("get_cursor", (int(DialogsManager::*)(const char*)) &DialogsManager::GetCursor)
          .enum_("constants")[
            luabind::value("SOLID", MSL_SOLID),
            luabind::value("TRANSPARENT", MSL_TRANSPARENT),
            luabind::value("NONE", MSL_NONE)
          ]
          .def(
            "set_map_name", (void(DialogsManager::*)(const char*)) &DialogsManager::ScriptSetMapName
          )
          .def(
            "get_map_name", (std::string(DialogsManager::*)()) &DialogsManager::GetMapName
          )
    ];

    // UI widget commands
    luabind::module(lua_state_)[
        luabind::class_<UiWidget>("UiWidget")
          .def("set_visible", (void(UiWidget::*)(const bool)) &UiWidget::SetVisible)
          .def("is_visible", (bool(UiWidget::*)()) &UiWidget::IsVisible)
          .def("play_animation", (void(UiWidget::*)(const char*)) &UiWidget::ScriptPlayAnimation)
          .def(
            "play_animation_stop",
            (void(UiWidget::*)(const char*)) &UiWidget::ScriptPlayAnimationStop
          )
          .def(
            "play_animation",
            (void(UiWidget::*)(const char*, const float, const float))
              &UiWidget::ScriptPlayAnimation
          )
          .def(
            "play_animation_stop",
            (void(UiWidget::*)(const char*, const float, const float))
            &UiWidget::ScriptPlayAnimationStop
          )
          .def(
            "set_default_animation",
            (void(UiWidget::*)(const char*)) &UiWidget::ScriptSetDefaultAnimation
          )
          .def(
            "animation_sync", (int(UiWidget::*)()) &UiWidget::ScriptAnimationSync, luabind::yield
          )
          .def(
            "set_colour",
            (void(UiWidget::*)(const float, const float, const float)) &UiWidget::SetColour
          )
          .def("set_alpha", (void(UiWidget::*)(const float)) &UiWidget::SetAlpha)
          .def("set_x", (void(UiWidget::*)(const float, const float)) &UiWidget::SetX)
          .def("set_y", (void(UiWidget::*)(const float, const float)) &UiWidget::SetY)
          .def("set_z", (void(UiWidget::*)(const float)) &UiWidget::SetZ)
          .def("get_width", (float(UiWidget::*)()) &UiWidget::ScriptGetWidth)
          .def("set_width", (void(UiWidget::*)(const float)) &UiWidget::ScriptSetWidth)
          .def("set_width", (void(UiWidget::*)(const float, const float)) &UiWidget::SetWidth)
          .def("get_height", (float(UiWidget::*)()) &UiWidget::ScriptGetHeight)
          .def("set_height", (void(UiWidget::*)(const float)) &UiWidget::ScriptSetHeight)
          .def("set_height", (void(UiWidget::*)(const float, const float)) &UiWidget::SetHeight)
          .def("set_text", (void(UiWidget::*)(const char*)) &UiWidget::SetText)
          .def("set_image", (void(UiWidget::*)(const char*)) &UiWidget::SetImage)
    ];

    // UI manager commands. Use to get a specific widget.
    luabind::module(lua_state_)[
        luabind::class_<UiManager>("UiManager")
          .def("get_widget", (UiWidget*(UiManager::*)(const char*)) &UiManager::ScriptGetWidget)
    ];

    // Timer command. To show a in-game timer.
    luabind::module(lua_state_)[
        luabind::class_<Timer>("Timer")
          .def("get_game_time_total", (float(Timer::*)()) &Timer::GetGameTimeTotal)
          .def("set_timer", (float(Timer::*)(const float)) &Timer::SetGameTimer)
          .def("get_timer", (int(Timer::*)()) &Timer::GetGameTimer)
    ];

    // Commands that control script execution
    luabind::module(lua_state_)[
        luabind::class_<ScriptManager>("Script")
          .def(
            "wait", (int(ScriptManager::*)(const float)) &ScriptManager::ScriptWait, luabind::yield
          )
          .def(
            "request",
            (void(ScriptManager::*)(const ScriptManager::Type, const char*, const char*, const int))
              &ScriptManager::ScriptRequest
          )
          .def(
            "request_start_sync",
            (int(ScriptManager::*)(const ScriptManager::Type, const char*, const char*, const int))
              &ScriptManager::ScriptRequestStartSync,
            luabind::yield
          )
          .def(
            "request_end_sync",
            (int(ScriptManager::*) (const ScriptManager::Type, const char*, const char*, const int))
              &ScriptManager::ScriptRequestEndSync,
            luabind::yield
          )
          .enum_("constants")[
            luabind::value("SYSTEM", ScriptManager::SYSTEM),
            luabind::value("ENTITY", ScriptManager::ENTITY),
            luabind::value("UI", ScriptManager::UI)
          ]
    ];

    // Commnds to initiate modules
    luabind::module(lua_state_)[
        luabind::class_<VGears::WorldmapModule>("world_map_module")
          .def("init", (void(VGears::WorldmapModule::*)()) &VGears::WorldmapModule::Init)
    ];

    // Register all command handlers
    auto a = boost::ref(*(EntityManager::getSingletonPtr()));
    luabind::globals(lua_state_);
    //auto b = luabind::globals(lua_state_)["entity_manager"];
    luabind::globals(lua_state_)["camera_manager"]
      = boost::ref(*(CameraManager::getSingletonPtr()));
    luabind::globals(lua_state_)["entity_manager"]
      = boost::ref(*(EntityManager::getSingletonPtr()));
    luabind::globals(lua_state_)["battle_manager"]
      = boost::ref(*(BattleManager::getSingletonPtr()));
    luabind::globals(lua_state_)["audio_manager"] = boost::ref(*(AudioManager::getSingletonPtr()));
    luabind::globals(lua_state_)["savemap_manager"]
      = boost::ref(*(SavemapHandler::getSingletonPtr()));
    luabind::globals(lua_state_)["background2d"]
      = boost::ref(*(EntityManager::getSingletonPtr()->GetBackground2D()));
    luabind::globals(lua_state_)["walkmesh"]
      = boost::ref(*(EntityManager::getSingletonPtr()->GetWalkmesh()));
    luabind::globals(lua_state_)["text_manager"] = boost::ref(*(TextHandler::getSingletonPtr()));
    luabind::globals(lua_state_)["dialog"] = boost::ref(*(DialogsManager::getSingletonPtr()));
    luabind::globals(lua_state_)["ui_manager"] = boost::ref(*(UiManager::getSingletonPtr()));
    luabind::globals(lua_state_)["world_map_module"]
      = boost::ref(*(VGears::WorldmapModule::getSingletonPtr()));
    luabind::globals(lua_state_)["timer"] = boost::ref(*(Timer::getSingletonPtr()));
    luabind::globals(lua_state_)["script"] = boost::ref(*this);
}
