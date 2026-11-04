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
  VGears::WorldmapModule::getSingleton().Suspend();
  EntityManager::getSingleton().SetFieldModule();
    EntityManager::getSingleton().Clear();
    XmlMapsFile xml("./data/fields/_fields.xml");
    Ogre::String file_name = xml.GetMapFileNameByName(text);
    XmlMapFile xml_map("./data/fields/" + file_name);
    xml_map.LoadMap();
}

void ScriptWorldMap(const int map, const unsigned int x, const unsigned int y){
    EntityManager::getSingleton().Clear();
    XmlMapFile xml_map("./data/fields/wm" + std::to_string(map) + ".xml");
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

/**
 * Binds a yielding member function to a class table.
 *
 * sol2 v2.20.6 function wrappers don't understand sol::yielding_t, so
 * sol::yielding can't be used with new_usertype or set_function. The raw
 * stack pusher does support it, so the yielding closure is pushed manually
 * and set as a field of the class table.
 *
 * @param[in,out] table The class table to bind the function to.
 * @param[in] name Function name for scripts.
 * @param[in] function The member function to bind.
 */
template <typename F>
void BindYielding(sol::table& table, const char* name, F function){
    table.push();
    sol::stack::push(table.lua_state(), sol::yielding(function));
    lua_setfield(table.lua_state(), -2, name);
    lua_pop(table.lua_state(), 1);
}

void ScriptManager::InitBinds(){
    sol::state_view lua_view(lua_state_);
    sol::table lua = lua_view.globals();

    // Global functions.
    lua.set_function("print", &ScriptPrint);
    lua.set_function("map", &ScriptMap);
    lua.set_function("console", &ScriptConsole);

    // Individual entity commands.
    lua.new_usertype<Entity>(
      "Entity",
      "set_position", &Entity::ScriptSetPosition,
      "move_by", &Entity::ScriptMoveBy,
      "get_position", &Entity::ScriptGetPosition,
      "get_name", &Entity::GetName,
      "set_rotation", &Entity::ScriptSetRotation,
      "get_rotation", &Entity::ScriptGetRotation,
      "set_solid_radius", &Entity::SetSolidRadius,
      "get_solid_radius", &Entity::GetSolidRadius,
      "set_solid", &Entity::SetSolid,
      "is_solid", &Entity::IsSolid,
      "set_talk_radius", &Entity::SetTalkRadius,
      "get_talk_radius", &Entity::GetTalkRadius,
      // Some old test script use this, its just an alias for SetTalkable:
      "set_interactable", &Entity::SetTalkable,
      "set_talkable", &Entity::SetTalkable,
      "is_talkable", &Entity::IsTalkable,
      "set_visible", &Entity::SetVisible,
      "is_visible", &Entity::IsVisible,
      "set_move_auto_speed", &Entity::SetMoveAutoSpeed,
      "get_move_auto_speed", &Entity::GetMoveAutoSpeed,
      "get_move_triangle_id", &Entity::GetMoveTriangleId,
      "set_move_triangle_id", &Entity::SetMoveTriangleId,
      "move_auto_rotation", &Entity::SetMoveAutoRotation,
      "move_auto_animation", &Entity::SetMoveAutoAnimation,
      "move_to_position", &Entity::ScriptMoveToPosition,
      "move_to_entity", &Entity::ScriptMoveToEntity,
      "linear_to_position", &Entity::ScriptLinearToPosition,
      "jump_to_position", &Entity::ScriptJumpToPosition,
      "offset_to_position", &Entity::ScriptOffsetToPosition,
      "turn_to_entity", &Entity::ScriptTurnToEntity,
      "turn_to_direction", &Entity::ScriptTurnToDirection,
      "set_animation_speed", &Entity::ScriptSetAnimationSpeed,
      "play_animation", sol::overload(
        sol::resolve<void(const char*)>(&Entity::ScriptPlayAnimation),
        sol::resolve<void(const char*, const float, const float)>(&Entity::ScriptPlayAnimation)
      ),
      "play_animation_stop", sol::overload(
        sol::resolve<void(const char*)>(&Entity::ScriptPlayAnimationStop),
        sol::resolve<void(const char*, const float, const float)>(
          &Entity::ScriptPlayAnimationStop
        )
      ),
      "set_default_animation", &Entity::ScriptSetDefaultAnimation
    );
    sol::table entity_type = lua["Entity"];
    BindYielding(entity_type, "move_sync", &Entity::ScriptMoveSync);
    BindYielding(entity_type, "linear_sync", &Entity::ScriptLinearSync);
    BindYielding(entity_type, "jump_sync", &Entity::ScriptJumpSync);
    BindYielding(entity_type, "offset_sync", &Entity::ScriptOffsetSync);
    BindYielding(entity_type, "turn_sync", &Entity::ScriptTurnSync);
    BindYielding(entity_type, "animation_sync", &Entity::ScriptAnimationSync);
    entity_type["NONE"] = AT_NONE;
    entity_type["LINEAR"] = AT_LINEAR;
    entity_type["SMOOTH"] = AT_SMOOTH;
    entity_type["CLOCKWISE"] = TD_CLOCKWISE;
    entity_type["ANTICLOCKWISE"] = TD_ANTICLOCKWISE;
    entity_type["CLOSEST"] = TD_CLOSEST;
    entity_type["UP_TO_DOWN"] = LM_UP_TO_DOWN;
    entity_type["DOWN_TO_UP"] = LM_DOWN_TO_UP;
    entity_type["LEFT_TO_RIGHT"] = LM_LEFT_TO_RIGHT;
    entity_type["RIGHT_TO_LEFT"] = LM_RIGHT_TO_LEFT;

    // Entity individual point commands
    lua.new_usertype<EntityPoint>(
      "EntityPoint",
      // Internally returns 3 values:
      "get_position", &EntityPoint::ScriptGetPosition,
      "get_rotation", &EntityPoint::ScriptGetRotation
    );

    // Commands for the entity manager, not related to any particular entity.
    lua.new_usertype<EntityManager>(
      "EntityManager",
      "set_paused", &EntityManager::ScriptSetPaused,
      "add_entity", &EntityManager::ScriptAddEntity,
      "add_entity_script", &EntityManager::ScriptAddEntityScript,
      "get_entity", &EntityManager::ScriptGetEntity,
      "get_entity_from_index", &EntityManager::GetEntityFromIndex,
      "get_entity_from_character_id", &EntityManager::GetEntityFromCharacterId,
      "get_entity_point", &EntityManager::ScriptGetEntityPoint,
      "set_player_entity", &EntityManager::ScriptSetPlayerEntity,
      "place_player_at_field_entry", &EntityManager::ScriptPlacePlayerAtFieldEntry,
      "get_player_entity", &EntityManager::ScriptGetPlayerEntity,
      "unset_player_entity", &EntityManager::ScriptUnsetPlayerEntity,
      "player_lock", &EntityManager::ScriptPlayerLock,
      "random_encounters_on", &EntityManager::SetRandomEncounters,
      "is_key_on", &EntityManager::IsKeyOn,
      "is_key_off", &EntityManager::IsKeyOff,
      "set_entity_to_character", &EntityManager::SetEntityToCharacter
    );

    // Commands for the battle manager.
    lua.new_usertype<BattleManager>(
      "BattleManager",
      "start_battle", &BattleManager::StartBattle,
      "end_battle", &BattleManager::EndBattle,
      "get_enemy_count", &BattleManager::ScriptGetEnemyCount,
      "get_enemy", &BattleManager::ScriptGetEnemy,
      "add_attack_model", &BattleManager::ScriptAddAttackModel
    );

    // Commands for enemy handling.
    lua.new_usertype<Enemy>(
      "Enemy",
      "get_id", &Enemy::GetId,
      "get_name", &Enemy::GetName,
      "get_level", &Enemy::GetLevel,
      "get_ap", &Enemy::GetAp,
      "get_exp", &Enemy::GetExp,
      "get_money", &Enemy::GetMoney,
      "get_str", &Enemy::GetStr,
      "get_def", &Enemy::GetDef,
      "get_mag", &Enemy::GetMag,
      "get_spr", &Enemy::GetSpr,
      "get_dex", &Enemy::GetDex,
      "get_lck", &Enemy::GetLck,
      "get_eva", &Enemy::GetEva,
      "get_meva", &Enemy::GetMeva,
      "get_hp", &Enemy::GetHp,
      "get_hp_max", &Enemy::GetHpMax,
      "get_mp", &Enemy::GetMp,
      "get_mp_max", &Enemy::GetMpMax,
      "get_position", &Enemy::ScriptGetPos,
      "is_front", &Enemy::IsFront,
      "is_visible", &Enemy::IsVisible,
      "is_targeteable", &Enemy::IsTargeteable,
      "is_active", &Enemy::IsActive,
      "get_attack_count", &Enemy::ScriptGetAttackCount,
      "get_attack", &Enemy::ScriptGetAttack,
      "get_camera_for_attack", &Enemy::ScriptGetCameraForAttack
      // TODO: Implement cover flags.
      // TODO: Implement immunities
      // TODO: Implement elements
      // TODO: Implement item drops
      // TODO: Implement stealable items
      // TODO: Implement manipulability status and attacks.
      // TODO: Implement morph item
    );

    // Commands for the audio manager.
    lua.new_usertype<AudioManager>(
      "AudioManager",
      "play_music", &AudioManager::ScriptPlayMusic,
      "play_sound", sol::overload(
        sol::resolve<void(const char*)>(&AudioManager::ScriptPlaySound),
        sol::resolve<void(const char*, const int)>(&AudioManager::ScriptPlaySound)
      ),
      "play_sounds", &AudioManager::ScriptPlaySounds,
      "get_track_id", &AudioManager::ScriptGetTrack,
      "get_battle_track_id", &AudioManager::ScriptGetBattleTrack,
      "set_battle_track_id", &AudioManager::ScriptSetBattleTrack
    );

    // Commands for the camera manager.
    lua.new_usertype<CameraManager>(
      "CameraManager",
      "set_camera", &CameraManager::ScriptSetCamera
    );

    // Commands for the savemap handler.
    lua.new_usertype<SavemapHandler>(
      "SavemapHandler",
      "release", &SavemapHandler::Release,
      "get_current_savemap", &SavemapHandler::GetCurrentSavemap,
      "get_savemap", &SavemapHandler::GetSavemap,
      "set_data", &SavemapHandler::SetData,
      "set_control_key", &SavemapHandler::SetControlKey,
      "set_window_colours", &SavemapHandler::SetWindowColours,
      "set_money", &SavemapHandler::SetMoney,
      "set_game_time", &SavemapHandler::SetGameTime,
      "set_countdown_time", &SavemapHandler::SetCountdownTime,
      "set_key_item", &SavemapHandler::SetKeyItem,
      "set_party", &SavemapHandler::SetParty,
      "set_item", &SavemapHandler::SetItem,
      "set_materia", &SavemapHandler::SetMateria,
      "set_e_skill_materia", &SavemapHandler::SetESkillMateria,
      "set_materia_stash", &SavemapHandler::SetMateriaStash,
      "set_e_skill_materia_stash", &SavemapHandler::SetESkillMateriaStash,
      "set_location", &SavemapHandler::SetLocation,
      "set_setting", &SavemapHandler::SetSetting,
      "set_character_info", &SavemapHandler::SetCharacterInfo,
      "set_character_stat", &SavemapHandler::SetCharacterStat,
      "set_character_limit_learned", &SavemapHandler::SetCharacterLimitLearned,
      "set_character_materia", &SavemapHandler::SetCharacterMateria,
      "set_character_e_skill_materia", &SavemapHandler::SetCharacterESkillMateria,
      "set_character_status", &SavemapHandler::SetCharacterStatus,
      "save", sol::resolve<bool(unsigned int, bool)>(&SavemapHandler::Save),
      "is_slot_empty", &SavemapHandler::IsSlotEmpty,
      "get_slot_control_key", sol::overload(
        &SavemapHandler::GetSlotControlKey,
        // Registered under the same name in the original bindings.
        &SavemapHandler::GetSlotWindowCornerColourComponent
      ),
      "get_slot_money", &SavemapHandler::GetSlotMoney,
      "get_slot_game_time", &SavemapHandler::GetSlotGameTime,
      "get_slot_countdown_time", &SavemapHandler::GetSlotCountdownTime,
      "get_slot_party_member", &SavemapHandler::GetSlotPartyMember,
      "get_slot_item_at_pos_id", &SavemapHandler::GetSlotItemAtPosId,
      "get_slot_item_at_pos_qty", &SavemapHandler::GetSlotItemAtPosQty,
      "get_slot_key_item", &SavemapHandler::GetSlotKeyItem,
      "get_slot_materia_at_pos_id", &SavemapHandler::GetSlotMateriaAtPosId,
      "get_slot_materia_at_pos_ap", &SavemapHandler::GetSlotMateriaAtPosAp,
      "is_slot_materia_at_pos_e_skill", &SavemapHandler::IsSlotMateriaAtPosESkill,
      "is_slot_materia_at_pos_e_skill_learned",
        &SavemapHandler::IsSlotMateriaAtPosESkillLearned,
      "get_slot_stash_at_pos_id", &SavemapHandler::GetSlotStashAtPosId,
      "get_slot_stash_at_pos_ap", &SavemapHandler::GetSlotStashAtPosAp,
      "is_slot_stash_at_pos_e_skill", &SavemapHandler::IsSlotStashAtPosESkill,
      "is_slot_stash_at_pos_e_skill_learned",
        &SavemapHandler::IsSlotStashAtPosESkillLearned,
      "get_slot_location_x", &SavemapHandler::GetSlotLocationX,
      "get_slot_location_y", &SavemapHandler::GetSlotLocationY,
      "get_slot_location_z", &SavemapHandler::GetSlotLocationZ,
      "get_slot_location_triangle", &SavemapHandler::GetSlotLocationTriangle,
      "get_slot_location_angle", &SavemapHandler::GetSlotLocationAngle,
      "get_slot_location_field", &SavemapHandler::GetSlotLocationField,
      "get_slot_location_name", &SavemapHandler::GetSlotLocationName,
      "get_slot_setting", &SavemapHandler::GetSlotSetting,
      "get_slot_character_char_id", &SavemapHandler::GetSlotCharacterCharId,
      "get_slot_character_name", &SavemapHandler::GetSlotCharacterName,
      "get_slot_character_level", &SavemapHandler::GetSlotCharacterLevel,
      "get_slot_character_kills", &SavemapHandler::GetSlotCharacterKills,
      "is_slot_character_enabled", &SavemapHandler::IsSlotCharacterEnabled,
      "is_slot_character_locked", &SavemapHandler::IsSlotCharacterLocked,
      "is_slot_character_back_row", &SavemapHandler::IsSlotCharacterBackRow,
      "get_slot_character_exp", &SavemapHandler::GetSlotCharacterExp,
      "get_slot_character_exp_to_next", &SavemapHandler::GetSlotCharacterExpToNext,
      "get_slot_character_limit_level", &SavemapHandler::GetSlotCharacterLimitLevel,
      "get_slot_character_limit_bar", &SavemapHandler::GetSlotCharacterLimitBar,
      "get_slot_character_weapon_id", &SavemapHandler::GetSlotCharacterWeaponId,
      "get_slot_character_armor_id", &SavemapHandler::GetSlotCharacterArmorId,
      "get_slot_character_accessory_id", &SavemapHandler::GetSlotCharacterAccessoryId,
      "get_slot_character_stat_base", &SavemapHandler::GetSlotCharacterStatBase,
      "get_slot_character_stat_extra", &SavemapHandler::GetSlotCharacterStatExtra,
      "get_slot_character_limit_uses", &SavemapHandler::GetSlotCharacterLimitUses,
      "is_slot_character_limit_learned", &SavemapHandler::IsSlotCharacterLimitLearned,
      "get_slot_character_materia_id", &SavemapHandler::GetSlotCharacterMateriaId,
      "get_slot_character_materia_ap", &SavemapHandler::GetSlotCharacterMateriaAp,
      "get_slot_character_materia_e_skill", &SavemapHandler::IsSlotCharacterMateriaESkill,
      "get_slot_character_materia_e_skill_learned",
        &SavemapHandler::IsSlotCharacterMateriaESkillLearned,
      "get_slot_data", &SavemapHandler::GetSlotData
    );

    // 2D background and camera commands
    lua.new_usertype<Background2D>(
      "Background2D",
      "autoscroll_to_entity", &Background2D::ScriptAutoScrollToEntity,
      "scroll_to_player", &Background2D::ScriptScrollToPlayer,
      "scroll_to_position", &Background2D::ScriptScrollToPosition,
      "offset", &Background2D::ScriptOffset,
      "play_animation_looped", &Background2D::ScriptPlayAnimationLooped,
      "play_animation_once", &Background2D::ScriptPlayAnimationOnce
    );
    sol::table background_type = lua["Background2D"];
    BindYielding(background_type, "scroll_sync", &Background2D::ScriptScrollSync);
    BindYielding(background_type, "animation_sync", &Background2D::ScriptAnimationSync);
    background_type["NONE"] = AT_NONE;
    background_type["LINEAR"] = AT_LINEAR;
    background_type["SMOOTH"] = AT_SMOOTH;

    // Walkmesh commands
    lua.new_usertype<Walkmesh>(
      "Walkmesh",
      "lock_walkmesh", &Walkmesh::LockWalkmesh,
      "is_locked", &Walkmesh::IsLocked
    );

    // Text handler commands
    lua.new_usertype<TextHandler>(
      "TextHandler",
      "set_character_name", &TextHandler::SetCharacterName,
      "set_party", &TextHandler::SetParty
    );

    // Dialog commands
    lua.new_usertype<DialogsManager>(
      "Dialog",
      "dialog_open", &DialogsManager::OpenDialog,
      "set_numeric", &DialogsManager::SetNumeric,
      "update_timer", &DialogsManager::UpdateTimer,
      "set_mode", &DialogsManager::SetMode,
      "dialog_set_text", &DialogsManager::SetText,
      "show_message", &DialogsManager::ShowScriptMessage,
      "dialog_close", &DialogsManager::Hide,
      "set_variable", &DialogsManager::SetVariable,
      "set_clickable", &DialogsManager::SetClickable,
      "set_cursor", &DialogsManager::SetCursor,
      "get_cursor", &DialogsManager::GetCursor,
      "set_map_name", &DialogsManager::ScriptSetMapName,
      "get_map_name", &DialogsManager::GetMapName
    );
    sol::table dialog_type = lua["Dialog"];
    BindYielding(dialog_type, "dialog_wait_for_close", &DialogsManager::Sync);
    dialog_type["SOLID"] = MSL_SOLID;
    dialog_type["TRANSPARENT"] = MSL_TRANSPARENT;
    dialog_type["NONE"] = MSL_NONE;

    // UI widget commands
    lua.new_usertype<UiWidget>(
      "UiWidget",
      "set_visible", &UiWidget::SetVisible,
      "is_visible", &UiWidget::IsVisible,
      "play_animation", sol::overload(
        sol::resolve<void(const char*)>(&UiWidget::ScriptPlayAnimation),
        sol::resolve<void(const char*, const float, const float)>(
          &UiWidget::ScriptPlayAnimation
        )
      ),
      "play_animation_stop", sol::overload(
        sol::resolve<void(const char*)>(&UiWidget::ScriptPlayAnimationStop),
        sol::resolve<void(const char*, const float, const float)>(
          &UiWidget::ScriptPlayAnimationStop
        )
      ),
      "set_default_animation", &UiWidget::ScriptSetDefaultAnimation,
      "set_colour", &UiWidget::SetColour,
      "set_alpha", &UiWidget::SetAlpha,
      "set_x", &UiWidget::SetX,
      "set_y", &UiWidget::SetY,
      "set_z", &UiWidget::SetZ,
      "get_width", &UiWidget::ScriptGetWidth,
      "set_width", sol::overload(&UiWidget::ScriptSetWidth, &UiWidget::SetWidth),
      "get_height", &UiWidget::ScriptGetHeight,
      "set_height", sol::overload(&UiWidget::ScriptSetHeight, &UiWidget::SetHeight),
      "set_text", &UiWidget::SetText,
      "set_image", &UiWidget::SetImage
    );
    sol::table ui_widget_type = lua["UiWidget"];
    BindYielding(ui_widget_type, "animation_sync", &UiWidget::ScriptAnimationSync);

    // UI manager commands. Use to get a specific widget.
    lua.new_usertype<UiManager>(
      "UiManager",
      "get_widget", &UiManager::ScriptGetWidget
    );

    // Timer command. To show a in-game timer.
    lua.new_usertype<Timer>(
      "Timer",
      "get_game_time_total", &Timer::GetGameTimeTotal,
      "set_timer", &Timer::SetGameTimer,
      "get_timer", &Timer::GetGameTimer
    );

    // Commands that control script execution
    lua.new_usertype<ScriptManager>(
      "Script",
      "request", sol::resolve<void(
        const ScriptManager::Type, const char*, const char*, const int
      )>(&ScriptManager::ScriptRequest)
    );
    sol::table script_type = lua["Script"];
    BindYielding(script_type, "wait", &ScriptManager::ScriptWait);
    BindYielding(script_type, "request_start_sync", &ScriptManager::ScriptRequestStartSync);
    BindYielding(script_type, "request_end_sync", &ScriptManager::ScriptRequestEndSync);
    script_type["SYSTEM"] = ScriptManager::SYSTEM;
    script_type["ENTITY"] = ScriptManager::ENTITY;
    script_type["UI"] = ScriptManager::UI;

    // Commands to initiate modules
    lua.new_usertype<VGears::WorldmapModule>(
      "world_map_module",
      "init", &VGears::WorldmapModule::Init,
      "suspend", &VGears::WorldmapModule::Suspend,
      "clear_vehicle_terrains", &VGears::WorldmapModule::ClearVehicleTerrains,
      "add_vehicle_terrain", &VGears::WorldmapModule::AddVehicleTerrain,
      "get_terrain_type_under_entity", &VGears::WorldmapModule::GetTerrainTypeUnderEntity,
      "get_terrain_triangle_under_entity", &VGears::WorldmapModule::GetTerrainTriangleUnderEntity,
      "unboard_vehicle", &VGears::WorldmapModule::UnboardVehicle
    );

    // Register all command handlers
    lua["camera_manager"] = CameraManager::getSingletonPtr();
    lua["entity_manager"] = EntityManager::getSingletonPtr();
    lua["battle_manager"] = BattleManager::getSingletonPtr();
    lua["audio_manager"] = AudioManager::getSingletonPtr();
    lua["savemap_manager"] = SavemapHandler::getSingletonPtr();
    lua["background2d"] = EntityManager::getSingletonPtr()->GetBackground2D();
    lua["walkmesh"] = EntityManager::getSingletonPtr()->GetWalkmesh();
    lua["text_manager"] = TextHandler::getSingletonPtr();
    lua["dialog"] = DialogsManager::getSingletonPtr();
    lua["ui_manager"] = UiManager::getSingletonPtr();
    lua["world_map_module"] = VGears::WorldmapModule::getSingletonPtr();
    lua["timer"] = Timer::getSingletonPtr();
    lua["script"] = this;
}
