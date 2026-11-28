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

#include "modules/battle/BattleBinds.h"
#include "engine/script/LuaIncludes.h"
#include "modules/battle/BattleManager.h"
#include "modules/battle/Enemy.h"

void BindBattleModule(lua_State* state){
    sol::state_view lua_view(state);
    sol::table lua = lua_view.globals();

    // Commands for the battle manager.
    lua.new_usertype<BattleManager>(
      "BattleManager",
      "start_battle", &BattleManager::StartBattle,
      "end_battle", &BattleManager::EndBattle,
      "get_enemy_count", &BattleManager::ScriptGetEnemyCount,
      "get_enemy", &BattleManager::ScriptGetEnemy,
      "add_attack_model", &BattleManager::ScriptAddAttackModel,
      "add_effect_model", &BattleManager::ScriptAddEffectModel,
      "remove_effect_model", &BattleManager::ScriptRemoveEffectModel,
      "has_attack", &BattleManager::HasAttackData,
      "set_camera", &BattleManager::ScriptSetCamera,
      "set_camera_position", &BattleManager::ScriptSetCameraPosition,
      "get_camera", &BattleManager::ScriptGetCamera,
      "create_dynamic_mesh", &BattleManager::ScriptCreateDynamicMesh,
      "set_dynamic_mesh", &BattleManager::ScriptSetDynamicMesh,
      "remove_dynamic_mesh", &BattleManager::ScriptRemoveDynamicMesh,
      "get_layout", &BattleManager::ScriptGetLayout,
      "get_intro_camera", &BattleManager::ScriptGetIntroCamera,
      "get_attack", [](BattleManager& manager, const int id, sol::this_state state) -> sol::object{
          sol::state_view view(state);
          const AttackData* attack = manager.GetAttackData(id);
          if (attack == nullptr) return sol::make_object(view, sol::nil);
          sol::table result = view.create_table();
          result["id"] = attack->id;
          result["name"] = attack->name;
          result["mp"] = attack->mp;
          result["reflectable"] = attack->reflectable;
          sol::table damage = view.create_table();
          for (const auto& p : attack->damage) damage[p.first] = p.second;
          result["damage"] = damage;
          sol::table target = view.create_table();
          for (const auto& p : attack->target) target[p.first] = p.second;
          result["target"] = target;
          sol::table animation = view.create_table();
          for (const auto& p : attack->animation) animation[p.first] = p.second;
          result["animation"] = animation;
          sol::table effects = view.create_table();
          int e = 1;
          for (const auto& effect : attack->effects){
              sol::table entry = view.create_table();
              entry["id"] = effect.id;
              entry["chance"] = effect.chance;
              entry["mode"] = effect.mode;
              effects[e ++] = entry;
          }
          result["effects"] = effects;
          sol::table elements = view.create_table();
          int n = 1;
          for (int element : attack->elements) elements[n ++] = element;
          result["elements"] = elements;
          return result;
      },
      "get_next_formation_id", &BattleManager::GetNextFormationId,
      "set_mode", &BattleManager::ScriptSetMode,
      "get_mode", &BattleManager::GetMode,
      "get_formation_id", &BattleManager::GetFormationId,
      "get_escape_difficulty", &BattleManager::GetEscapeDifficulty,
      "is_preemptive_allowed", &BattleManager::IsPreemptiveAllowed,
      "show_victory_pose", &BattleManager::ShowVictoryPose,
      "show_spoils", &BattleManager::ShowSpoils,
      "set_enemy_visible", &BattleManager::ScriptSetEnemyVisible,
      "get_enemy_entity_name", &BattleManager::ScriptGetEnemyEntityName,
      "get_party_entity_name", &BattleManager::ScriptGetPartyEntityName,
      "get_party_animation_prefix", &BattleManager::ScriptGetPartyAnimationPrefix,
      "get_party_model_name", &BattleManager::ScriptGetPartyModelName
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
      "get_camera_for_attack", &Enemy::ScriptGetCameraForAttack,
      "get_drop_count", &Enemy::ScriptGetDropCount,
      "get_animation_count", &Enemy::ScriptGetAnimationCount,
      "get_attack_animation", &Enemy::ScriptGetAttackAnimation,
      "get_element_count", &Enemy::ScriptGetElementCount,
      "get_ai_script", &Enemy::ScriptGetAiScript,
      "get_immunity_count", &Enemy::ScriptGetImmunityCount,
      "get_immunity", &Enemy::ScriptGetImmunity,
      "get_immunity_rate", &Enemy::ScriptGetImmunityRate,
      "get_element", &Enemy::ScriptGetElement,
      "get_element_factor", &Enemy::ScriptGetElementFactor,
      "get_animation", &Enemy::ScriptGetAnimation,
      "get_model_prefix", &Enemy::ScriptGetModelPrefix,
      "get_model", &Enemy::GetModel,
      "get_drop_id", &Enemy::ScriptGetDropId,
      "get_drop_rate", &Enemy::ScriptGetDropRate,
      "get_steal_count", &Enemy::ScriptGetStealCount,
      "get_steal_id", &Enemy::ScriptGetStealId,
      "get_steal_rate", &Enemy::ScriptGetStealRate,
      "get_morph", [](const Enemy& enemy){return static_cast<int>(enemy.GetMorph());},
      "get_manip_count", [](const Enemy& enemy){return static_cast<int>(enemy.GetManipulateAttacks().size());},
      "get_manip_attack", [](const Enemy& enemy, const int index){
          const std::vector<unsigned int> attacks = enemy.GetManipulateAttacks();
          if (index < 0 || index >= static_cast<int>(attacks.size())) return -1;
          return static_cast<int>(attacks[index]);
      }
      // TODO: Implement cover flags.
      // TODO: Implement immunities
      // TODO: Implement elements
      // TODO: Implement stealable items
      // TODO: Implement manipulability status and attacks.
    );

    lua["battle_manager"] = BattleManager::getSingletonPtr();
}
