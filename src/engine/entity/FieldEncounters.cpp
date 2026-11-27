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

#include <cstdlib>
#include "common/Logger.h"
#include "engine/Timer.h"
#include "engine/config/ConfigVar.h"
#include "engine/entity/Entity.h"
#include "engine/entity/FieldEncounters.h"
#include "engine/script/ScriptManager.h"

ConfigVar cv_encounter_multiplier(
  "encounter_multiplier", "Multiplier of the random encounter rate (0 disables them)", "1"
);
ConfigVar cv_debug_encounters("debug_encounters", "Log random encounter checks", "false");

/**
 * Frames of walking (at 30 frames per second) between two encounter checks. The original game
 * checks every 8 frames.
 */
static const float ENCOUNTER_CHECK_FRAMES = 8.0f;

/**
 * Value added to the encounter accumulator in each check, before dividing it by the rate of the
 * table. The original game uses the speed of the player, which is not known here, so this is tuned
 * to a battle every 15-30 seconds of walking in most fields. The encounter_multiplier cvar scales
 * it.
 */
static const float ENCOUNTER_STEP_VALUE = 5.0f;

float FieldEncounters::GetMultiplier(){return cv_encounter_multiplier.GetF();}

bool FieldEncounters::IsDebug(){return cv_debug_encounters.GetB();}

void FieldEncounters::SetActive(bool active){active_ = active;}

void FieldEncounters::Clear(){
    tables_.clear();
    table_ = 1;
    distance_ = 0.0f;
    last_position_valid_ = false;
    accumulator_ = 0;
    last_battle_ = -1;
}

void FieldEncounters::AddTable(
  const int id, const int rate, const std::vector<std::pair<int, int>>& battles,
  const std::vector<std::pair<int, int>>& specials
){
    Table table;
    table.rate = rate;
    table.battles = battles;
    table.specials = specials;
    tables_[id] = table;
}

void FieldEncounters::SetTable(const int id){table_ = id;}

int FieldEncounters::RollBattle() const{
    auto found = tables_.find(table_);
    if (found == tables_.end()) return -1;
    const Table& table = found->second;
    // Like the original game: a roll out of 64 is checked against the special battles one after
    // the other (each one adds its probability). If none is hit, a new roll chooses a standard
    // battle, the last one being the default.
    int roll = std::rand() % 64;
    int sum = 0;
    for (const auto& special : table.specials){
        sum += special.second;
        if (roll < sum) return special.first;
    }
    if (table.battles.empty()) return -1;
    roll = std::rand() % 64;
    sum = 0;
    for (const auto& battle : table.battles){
        sum += battle.second;
        if (roll < sum) return battle.first;
    }
    return table.battles.back().first;
}

void FieldEncounters::Update(const Entity* player, const bool player_locked, const bool walked){
    if (player == nullptr){
        last_position_valid_ = false;
        return;
    }
    const Ogre::Vector3 position = player->GetPosition();
    const float distance
      = last_position_valid_ ? (position - last_position_).length() : 0.0f;
    last_position_ = position;
    last_position_valid_ = true;
    // Only walking counts: not scripted movements, and not jumps to another place.
    if (!walked || player_locked || distance > 2.0f) return;
    if (!active_ || tables_.empty()) return;
    auto found = tables_.find(table_);
    if (found == tables_.end()) return;
    const float multiplier = cv_encounter_multiplier.GetF();
    if (multiplier <= 0.0f) return;
    // Same as the original game. Every few frames of walking the accumulator grows by a value
    // divided by the rate (the lower the rate, the more often the encounters), and an encounter
    // starts if a random number is lower than the accumulator.
    const int rate = found->second.rate;
    if (rate <= 0) return;
    distance_ += Timer::getSingleton().GetGameTimeDelta() * 30.0f;
    while (distance_ >= ENCOUNTER_CHECK_FRAMES){
        distance_ -= ENCOUNTER_CHECK_FRAMES;
        // Enemy Away and Enemy Lure materia change the rate of the party (see
        // Party.get_encounter_modifier).
        const float materia_multiplier
          = ScriptManager::getSingleton().EvaluateNumber("Party.get_encounter_modifier()", 1.0f);
        accumulator_
          += static_cast<int>(ENCOUNTER_STEP_VALUE * multiplier * materia_multiplier * 256.0f) / rate;
        const int roll = std::rand() % 256 * 256;
        if (cv_debug_encounters.GetB()){
            LOG_TRIVIAL(
              "Encounter check: rate " + std::to_string(rate) + ", accumulator "
              + std::to_string(accumulator_ / 256.0f) + "/256, roll "
              + std::to_string(roll / 256)
            );
        }
        if (roll >= accumulator_) continue;
        int battle = RollBattle();
        // The same battle is not repeated: it's rolled again once.
        if (battle == last_battle_) battle = RollBattle();
        if (battle < 0) return;
        distance_ = 0.0f;
        accumulator_ = 0;
        last_battle_ = battle;
        LOG_TRIVIAL("Random encounter: battle " + std::to_string(battle));
        // The battle is started by a script, as it has to change the module of every manager.
        ScriptManager::getSingleton().RunString("Encounter.start(" + std::to_string(battle) + ")");
        return;
    }
}
