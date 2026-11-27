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

#include "modules/worldmap/WorldmapEncounters.h"
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <tinyxml.h>
#include "common/Logger.h"
#include "engine/entity/EntityManager.h"
#include "engine/script/ScriptManager.h"
#include "modules/worldmap/WorldmapTerrain.h"

namespace VGears {

    void WorldmapEncounters::Load(){
        encounter_records_.clear();
        TiXmlDocument document("./game/world/world_encounters.xml");
        if (!document.LoadFile()){
            LOG_WARNING("World map encounters were not found. There will be no random battles.");
            return;
        }
        TiXmlHandle handle(&document);
        for (
          TiXmlElement* record = handle.FirstChildElement("world_encounters")
            .FirstChildElement("record").ToElement();
          record != nullptr; record = record->NextSiblingElement("record")
        ){
            Record data;
            int enabled = 0;
            data.rate = 0;
            data.terrain = 0;
            record->QueryIntAttribute("enabled", &enabled);
            record->QueryIntAttribute("rate", &data.rate);
            record->QueryIntAttribute("terrain", &data.terrain);
            data.enabled = enabled != 0;
            for (unsigned int i = 0; i < 15; ++ i) data.words[i] = 0;
            std::istringstream words(record->Attribute("words") ? record->Attribute("words") : "");
            for (unsigned int i = 0; i < 15; ++ i){
                unsigned int word = 0;
                if (!(words >> word)) break;
                data.words[i] = static_cast<uint16>(word);
            }
            encounter_records_.push_back(data);
        }
        LOG_TRIVIAL("World map encounters loaded: " + std::to_string(encounter_records_.size()));
    }

    int WorldmapEncounters::RollBattle(const Record& record) const{
        // Same order as the original game: the special battles first, then the standard ones. The
        // chance of a word is its probability out of 64, as the roll is a 16 bit value and the
        // probability sits in the 6 highest bits.
        const uint16* w = record.words;
        int battle = -1;
        int roll = (std::rand() % 256) << 8;
        // Words 7 and 8 (indices 6 and 7): back attacks.
        if (w[6] > roll) battle = w[6] & 0x3FF;
        else if (w[6] + w[7] > roll) battle = w[7] & 0x3FF;
        // Word 9: side attack. Needs a party of more than one member.
        if (battle < 0){
            roll = (std::rand() % 256) << 8;
            if (w[8] > roll) battle = w[8] & 0x3FF;
        }
        // Word 10: pincer attack.
        if (battle < 0){
            roll = (std::rand() % 256) << 8;
            if (w[9] > roll) battle = w[9] & 0x3FF;
        }
        // Words 1-6: standard battles. The last battle is not repeated (one reroll).
        for (unsigned int attempt = 0; battle < 0 && attempt < 2; ++ attempt){
            roll = (std::rand() % 256) << 8;
            int sum = 0;
            for (unsigned int i = 0; i < 6; ++ i){
                sum += w[i];
                if (sum > roll){
                    battle = w[i] & 0x3FF;
                    break;
                }
            }
            if (battle == encounter_last_battle_ && attempt == 0) battle = -1;
        }
        return battle;
    }

    void WorldmapEncounters::Update(const float delta_time, const TerrainTriangle& triangle){
        if (encounters_locked_ || encounter_records_.size() < 64) return;
        // The original game checks every 16 frames of walking, at 30 frames per second.
        encounter_frames_ += delta_time * 30.0f;
        if (encounter_frames_ < 16.0f) return;
        encounter_frames_ -= 16.0f;

        // Region and terrain group. The terrain types 0x10 and 0x18 count as 0 and 8.
        const int region = std::min<int>(triangle.region, 15);
        int terrain = triangle.walkability_type;
        if (terrain == 0x10) terrain = 0;
        else if (terrain == 0x18) terrain = 8;
        int group = 0;
        for (int i = 0; i < 4; ++ i){
            if (encounter_records_[region * 4 + i].terrain == terrain){
                group = i;
                break;
            }
        }
        const Record& record = encounter_records_[region * 4 + group];

        // The accumulator grows on every check, so the chance gets higher until a battle starts.
        const float multiplier = EntityManager::getSingleton().GetEncounterMultiplier();
        if (multiplier <= 0.0f) return;
        const int speed = static_cast<int>(16 * multiplier);
        encounter_accumulator_ += (record.rate != 0) ? (speed << 10) / record.rate : 0x7FFF;
        const int check = std::rand() % 256;
        if (EntityManager::getSingleton().IsDebugEncounters()){
            LOG_TRIVIAL(
              "World encounter check: region " + std::to_string(region) + ", terrain "
              + std::to_string(terrain) + ", group " + std::to_string(group) + ", rate "
              + std::to_string(record.rate) + ", chance " + std::to_string(encounter_accumulator_ >> 8)
              + "/256, roll " + std::to_string(check)
            );
        }
        if (check >= (encounter_accumulator_ >> 8) || !record.enabled) return;
        encounter_accumulator_ = 0;
        const int battle = RollBattle(record);
        if (battle < 0) return;
        encounter_last_battle_ = battle;
        LOG_TRIVIAL("World random encounter: battle " + std::to_string(battle));
        ScriptManager::getSingleton().RunString("Encounter.start(" + std::to_string(battle) + ")");
    }

    void WorldmapEncounters::Clear(){encounter_records_.clear();}

    void WorldmapEncounters::SetLocked(const bool locked){encounters_locked_ = locked;}

    void WorldmapEncounters::ResetTimer(){encounter_frames_ = 0;}

} // namespace VGears
