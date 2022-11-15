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
#include "Console.h"
#include "Logger.h"
#include "Entity.h"
#include "EntityManager.h"
#include "AudioManager.h"
#include "SavemapManager.h"
#include "Timer.h"
#include "UiManager.h"
#include "UiWidget.h"
#include "XmlMapFile.h"
#include "XmlMapsFile.h"
#include "DialogsManager.h"
#include "TextManager.h"
#include "modules/worldmap/WorldMapModule.h"

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
    XmlMapsFile xml("./data/maps.xml");
    Ogre::String file_name = xml.GetMapFileNameByName(text);
    XmlMapFile xml_map("./data/" + file_name);
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
          .def("start_battle", (void(EntityManager::*)(unsigned int)) &EntityManager::StartBattle)
          // TODO: Run? Set battle flags
          .def("battle_run", (void(EntityManager::*)(unsigned int)) &EntityManager::StartBattle)
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
          .def("start_battle", (void(EntityManager::*)(unsigned int)) &EntityManager::StartBattle)
          // TODO: Run? Set battle flags
          .def("battle_run", (void(EntityManager::*)(unsigned int)) &EntityManager::StartBattle)
          .def("is_key_on", (bool(EntityManager::*)(unsigned int)) &EntityManager::IsKeyOn)
          .def("is_key_off", (bool(EntityManager::*)(unsigned int)) &EntityManager::IsKeyOff)
          .def(
            "set_entity_to_character",
            (void(EntityManager::*)(const char*, unsigned int)) &EntityManager::SetEntityToCharacter
          )
          .def("get_track_id", (int(EntityManager::*)(int)) &EntityManager::GetTrack)
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
    ];



    // Commands for the savemap manager.
    luabind::module(lua_state_)[
        luabind::class_<SavemapManager>("SavemapManager")
          .def(
            "get_current_savemap",
            (Savemap*(SavemapManager::*)()) &SavemapManager::GetCurrentSavemap
          )
          .def(
            "get_savemap",
            (Savemap*(SavemapManager::*)(const unsigned int)) &SavemapManager::GetSavemap
          )
          .def(
            "set_data",
            (void(SavemapManager::*)(const unsigned int, const unsigned int, const int))
              &SavemapManager::SetData
          )
          .def(
            "set_control_key", (void(SavemapManager::*)(const char*)) &SavemapManager::SetControlKey
          )
          .def(
            "set_window_colours",
            (void(SavemapManager::*)(
              const unsigned int, const unsigned int, const unsigned int,
              const unsigned int, const unsigned int, const unsigned int,
              const unsigned int, const unsigned int, const unsigned int,
              const unsigned int, const unsigned int, const unsigned int
            )) &SavemapManager::SetWindowColours
          )
          .def("set_money", (void(SavemapManager::*)(const unsigned int)) &SavemapManager::SetMoney)
          .def(
            "set_game_time",
            (void(SavemapManager::*)(const unsigned int)) &SavemapManager::SetGameTime
          )
          .def(
            "set_countdown_time",
            (void(SavemapManager::*)(const unsigned int)) &SavemapManager::SetCountdownTime
          )
          .def(
            "set_key_item",
            (void(SavemapManager::*)(const unsigned int, const bool)) &SavemapManager::SetKeyItem
          )
          .def(
            "set_party",
            (void(SavemapManager::*)(const int, const int, const int)) &SavemapManager::SetParty
          )
          .def(
            "set_item",
            (void(SavemapManager::*)(const unsigned int, const unsigned int, const unsigned int))
              &SavemapManager::SetItem
          )
          .def(
            "set_materia",
            (void(SavemapManager::*)(const unsigned int, const unsigned int, const unsigned int))
              &SavemapManager::SetMateria
          )
          .def(
            "set_e_skill_materia",
            (void(SavemapManager::*)(const unsigned int, const unsigned int, const bool))
              &SavemapManager::SetESkillMateria
          )
          .def(
            "set_materia_stash",
            (void(SavemapManager::*)(const unsigned int, const unsigned int, const unsigned int))
              &SavemapManager::SetMateriaStash
          )
          .def(
            "set_e_skill_materia_stash",
            (void(SavemapManager::*)(const unsigned int, const unsigned int, const bool))
              &SavemapManager::SetESkillMateriaStash
          )
          .def(
            "set_location",
            (void(SavemapManager::*)(
              const unsigned int, const unsigned int, const int, const unsigned int, const int,
              const char*, const char*
            )) &SavemapManager::SetLocation
          )
          .def(
            "set_setting",
            (void(SavemapManager::*)(const unsigned int, const unsigned int))
              &SavemapManager::SetSetting
          )
          .def(
            "set_character_info",
            (void(SavemapManager::*)(
              const unsigned int, const int, const char*, const bool, const bool, const unsigned int,
              const unsigned int, const bool, const unsigned int, const unsigned int,
              const unsigned int, const unsigned int, const unsigned int, const unsigned int,
              const int
            )) &SavemapManager::SetCharacterInfo
          )
          .def(
            "set_character_stat",
            (void(SavemapManager::*)(
              const unsigned int, const unsigned int, const unsigned int, const unsigned int
            )) &SavemapManager::SetCharacterStat
          )
          .def(
            "set_character_limit_learned",
            (void(SavemapManager::*)(
              const unsigned int, const unsigned int, const unsigned int,
              const bool, const unsigned int
            )) &SavemapManager::SetCharacterLimitLearned
          )
          .def(
            "set_character_materia",
            (void(SavemapManager::*)(
              const unsigned int, const bool, const unsigned int,
              const unsigned int, const unsigned int
            )) &SavemapManager::SetCharacterMateria
          )
          .def(
            "set_character_e_skill_materia",
            (void(SavemapManager::*)(
              const unsigned int, const bool, const unsigned int, const unsigned int, const bool
            )) &SavemapManager::SetCharacterESkillMateria
          )
          .def(
            "set_character_status",
            (void(SavemapManager::*)(const unsigned int, const unsigned int, const bool))
              &SavemapManager::SetCharacterStatus
          )
          .def(
            "save", (bool(SavemapManager::*)(const unsigned int, const bool)) &SavemapManager::Save
          )
          .def(
            "slot_is_empty",
            (bool(SavemapManager::*)(const unsigned int)) &SavemapManager::SlotIsEmpty
          )
          .def(
            "get_preview_data",
             (std::string(SavemapManager::*)(const unsigned int)) &SavemapManager::GetPreviewData
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

    // Text manager commands
    luabind::module(lua_state_)[
        luabind::class_<TextManager>("TextManager")
          .def(
            "set_character_name",
            (void(TextManager ::*)(unsigned int, const char*)) &TextManager::SetCharacterName
          )
          .def("set_party", (void(TextManager ::*)(int, int, int)) &TextManager::SetParty)
    ];

    // Dialog commands
    luabind::module(lua_state_)[
        luabind::class_<DialogsManager>("Dialog")
          .def(
            "dialog_open",
            (void(DialogsManager::*)(const char*, int, int, int, int)) &DialogsManager::OpenDialog
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
              "set_map_name",
              (void(DialogsManager::*)(const char*)) &DialogsManager::ScriptSetMapName
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
          .def("get_height", (float(UiWidget::*)()) &UiWidget::ScriptGetHeight)
          .def("set_height", (void(UiWidget::*)(const float)) &UiWidget::ScriptSetHeight)
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
        luabind::class_<VGears::WorldMapModule>("world_map_module")
          .def("init", (void(VGears::WorldMapModule::*)()) &VGears::WorldMapModule::Init)
    ];

    // Register all command handlers
    auto a = boost::ref(*(EntityManager::getSingletonPtr()));
    luabind::globals(lua_state_);
    //auto b = luabind::globals(lua_state_)["entity_manager"];
    luabind::globals(lua_state_)["entity_manager"]
      = boost::ref(*(EntityManager::getSingletonPtr()));
    luabind::globals(lua_state_)["audio_manager"] = boost::ref(*(AudioManager::getSingletonPtr()));
    luabind::globals(lua_state_)["savemap_manager"]
      = boost::ref(*(SavemapManager::getSingletonPtr()));
    luabind::globals(lua_state_)["background2d"]
      = boost::ref(*(EntityManager::getSingletonPtr()->GetBackground2D()));
    luabind::globals(lua_state_)["walkmesh"]
      = boost::ref(*(EntityManager::getSingletonPtr()->GetWalkmesh()));
    luabind::globals(lua_state_)["text_manager"] = boost::ref(*(TextManager::getSingletonPtr()));
    luabind::globals(lua_state_)["dialog"] = boost::ref(*(DialogsManager::getSingletonPtr()));
    luabind::globals(lua_state_)["ui_manager"] = boost::ref(*(UiManager::getSingletonPtr()));
    luabind::globals(lua_state_)["world_map_module"]
      = boost::ref(*(VGears::WorldMapModule::getSingletonPtr()));
    luabind::globals(lua_state_)["timer"] = boost::ref(*(Timer::getSingletonPtr()));
    luabind::globals(lua_state_)["script"] = boost::ref(*this);
}
