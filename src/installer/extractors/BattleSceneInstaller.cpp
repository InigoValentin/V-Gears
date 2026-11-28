/*
 * V-Gears
 * Copyright (C) 2022 V-Gears Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <cstdio>
#include <algorithm>
#include <tinyxml.h>
#include "installer/extractors/BattleSceneInstaller.h"

namespace{

/**
 * Adds a new element at the end of another one.
 */
TiXmlElement& AddElement(TiXmlElement& parent, const char* name){
    TiXmlElement* element = new TiXmlElement(name);
    // The parent takes care of deleting it.
    parent.LinkEndChild(element);
    return *element;
}

/**
 * Adds an element with an identifier and a value.
 */
template<typename T> void AddParameter(
  TiXmlElement& parent, const char* name, const char* id, const T& value
){
    TiXmlElement& element = AddElement(parent, name);
    element.SetAttribute("id", id);
    element.SetAttribute("value", value);
}

}

BattleSceneInstaller::BattleSceneInstaller(DiskImage& disk_image, const std::string& data_dir):
  data_dir_(data_dir), next_scene_(0), total_scenes_(0),
  scene_bin_(*disk_image.filePath("data/battle/scene.bin"))
{}

unsigned int BattleSceneInstaller::InitializeScenes(){
    next_scene_ = 0;
    scenes_.clear();
    enemies_.clear();
    attacks_.clear();
    scenes_uncompressed_.clear();
    total_scenes_ = 0;
    // scene.bin is a sequence of 0x2000 bytes blocks (33 in the original game). Each block starts
    // with up to 16 pointers (in 4-byte units) to gzipped scenes, terminated by 0xFFFFFFFF. A scene
    // ends where the next one starts, or at the end of the block. There are 256 scenes in total.
    // Scene indexes are positional: every slot must be kept, even if it can't be extracted,
    // because battle IDs (scene * 4 + setup) are derived from them.
    const u32 block_size = 0x2000;
    const u32 blocks = scene_bin_.GetFileSize() / block_size;
    for (u32 i = 0; i < blocks; i ++){
        File scene_bin_block(&scene_bin_, i * block_size, block_size);
        u32 offsets[16];
        u32 count = 0;
        for (; count < 16; count ++){
            scene_bin_block.SetOffset(4 * count);
            u32 pointer = scene_bin_block.readU32LE();
            if (pointer == 0xFFFFFFFF || pointer * 4 >= block_size) break;
            offsets[count] = pointer * 4;
        }
        for (u32 o = 0; o < count; o ++){
            u32 start_offset = offsets[o];
            u32 end_offset = (o + 1 < count) ? offsets[o + 1] : block_size;
            if (end_offset <= start_offset) end_offset = block_size;
            File scene_bin_fragment(&scene_bin_block, start_offset, end_offset - start_offset);
            // NULL if it can't be extracted. Kept to preserve scene indexes.
            scenes_uncompressed_.push_back(ExtractGZipScene(scene_bin_fragment));
            total_scenes_ ++;
        }
    }
    return total_scenes_;
}

unsigned int BattleSceneInstaller::ProcessScene(){
    if (next_scene_ >= scenes_uncompressed_.size()) return total_scenes_;
    File* scene_file = scenes_uncompressed_[next_scene_].get();
    if (scene_file == NULL){
        // Keep battle IDs stable: an unreadable scene still takes its 4 battle slots.
        std::cout << "Warning: scene " << next_scene_ << " could not be extracted." << std::endl;
        for (int f = 0; f < 4; f ++){
            Formation empty;
            empty.id = next_scene_ * 4 + f;
            formations_.push_back(empty);
        }
        next_scene_ ++;
        return next_scene_;
    }
    BattleSceneFile scene(*scene_file);
    scenes_.push_back(scene);
    // Add new enemies to the list.
    std::vector<Enemy> enemies = scene.GetEnemies();
    for (int e = 0; e < enemies.size(); e ++){
        if (std::any_of(
          enemies_.begin(), enemies_.end(), [&](const Enemy& existing){ return existing.id == enemies[e].id; }
        )) continue;
        enemies_.push_back(enemies[e]);
    }
    // Add new attacks to the list.
    std::vector<Attack> attacks = scene.GetAttacks();
    for (int a = 0; a < attacks.size(); a ++){
        if (std::any_of(
          attacks_.begin(), attacks_.end(), [&](const Attack& existing){ return existing.id == attacks[a].id; }
        )) continue;
        attacks_.push_back(attacks[a]);
    }
    // Battle ID = scene index * 4 + setup index, as referenced by field scripts and encounters.
    std::vector<Formation> formations = scene.GetFormations();
    for (int f = 0; f < formations.size(); f ++){
        formations[f].id = next_scene_ * 4 + f;
        formations_.push_back(formations[f]);
    }
    next_scene_ ++;
    return next_scene_;
}

void BattleSceneInstaller::WriteAttacks(){
    for (const Attack& attack : attacks_){
        TiXmlDocument xml;
        std::unique_ptr<TiXmlElement> container(new TiXmlElement("Attack"));
        container->SetAttribute("id", attack.id);
        container->SetAttribute("name", attack.name);
        container->SetAttribute("mp", attack.mp);
        container->SetAttribute("reflectable", attack.reflectable);
        AddAttackDamage(*container, attack);
        AddAttackTarget(*container, attack);
        TiXmlElement& effects = AddElement(*container, "Effects");
        for (const Attack::StatusEffect& effect : attack.status_effects){
            TiXmlElement& xml_effect = AddElement(effects, "Effect");
            xml_effect.SetAttribute("id", effect.id);
            xml_effect.SetDoubleAttribute("chance", effect.chance);
            xml_effect.SetAttribute("mode", GameData::STATUS_EFFECT_MODE::to_string(effect.type));
        }
        TiXmlElement& elements = AddElement(*container, "Elements");
        for (unsigned int element : attack.elements)
            AddElement(elements, "Element").SetAttribute("id", element);
        AddAttackAnimation(*container, attack);
        xml.LinkEndChild(container.release());
        std::string file_name = BuildAttackFileName(attack);
        xml.SaveFile(data_dir_ + file_name);
        attack_map_[attack.id] = file_name;
    }
    // Index of the attack files by ID, as file names include the attack name.
    TiXmlDocument index;
    std::unique_ptr<TiXmlElement> attacks(new TiXmlElement("attacks"));
    for (const auto& entry : attack_map_){
        std::unique_ptr<TiXmlElement> xml_attack(new TiXmlElement("attack"));
        xml_attack->SetAttribute("id", entry.first);
        // The file name in the map has the battle/attacks/ path.
        xml_attack->SetAttribute("file", entry.second.substr(entry.second.find_last_of('/') + 1));
        attacks->LinkEndChild(xml_attack.release());
    }
    index.LinkEndChild(attacks.release());
    index.SaveFile(data_dir_ + "battle/attacks/_index.xml");
}

void BattleSceneInstaller::AddAttackDamage(TiXmlElement& parent, const Attack& attack){
    TiXmlElement& damage = AddElement(parent, "Damage");
    AddParameter(damage, "Parameter", "power", attack.damage.power);
    AddParameter(damage, "Parameter", "formula", attack.damage.formula);
    AddParameter(damage, "Parameter", "modifier", attack.damage.modifier);
    // Accuracy is a fraction between 0 and 1, it can't be written as an integer.
    TiXmlElement& accuracy = AddElement(damage, "Parameter");
    accuracy.SetAttribute("id", "accuracy");
    accuracy.SetDoubleAttribute("value", attack.damage.accuracy);
    AddParameter(damage, "Parameter", "affected_by_darkness", attack.damage.affected_by_darkness);
    AddParameter(damage, "Parameter", "restore", static_cast<int>(attack.damage.restore));
    AddParameter(damage, "Parameter", "damage_mp", attack.damage.damage_mp);
    AddParameter(damage, "Parameter", "drain_hp", attack.damage.drain_hp);
    AddParameter(damage, "Parameter", "drain_mp", attack.damage.drain_mp);
    AddParameter(damage, "Parameter", "ignore_status_defense", attack.damage.ignore_status_defense);
    AddParameter(damage, "Parameter", "always_critical", attack.damage.critical);
}

void BattleSceneInstaller::AddAttackTarget(TiXmlElement& parent, const Attack& attack){
    TiXmlElement& target = AddElement(parent, "Target");
    AddParameter(target, "Parameter", "selection_enabled", attack.target.selection_enabled);
    AddParameter(target, "Parameter", "default_enemy", attack.target.default_enemy);
    AddParameter(target, "Parameter", "default_multiple", attack.target.default_multiple);
    AddParameter(target, "Parameter", "toggle_multiple", attack.target.toggle_multiple);
    AddParameter(target, "Parameter", "fixed_row", attack.target.fixed_row);
    AddParameter(target, "Parameter", "short_range", attack.target.short_range);
    AddParameter(target, "Parameter", "all_rows", attack.target.all_rows);
    AddParameter(target, "Parameter", "random", attack.target.random);
    AddParameter(target, "Parameter", "only_for_dead", attack.target.only_for_dead);
    AddParameter(target, "Parameter", "no_retarget", attack.target.no_re_target);
}

void BattleSceneInstaller::AddAttackAnimation(TiXmlElement& parent, const Attack& attack){
    TiXmlElement& animation = AddElement(parent, "Animation");
    AddParameter(animation, "Parameter", "effect", attack.animation.effect);
    AddParameter(animation, "Parameter", "impact_effect", attack.animation.impact_effect);
    AddParameter(animation, "Parameter", "sound", attack.animation.sound);
    AddParameter(animation, "Parameter", "hurt_anim", attack.animation.hurt_anim);
    AddParameter(animation, "Parameter", "camera_single", attack.animation.camera_single);
    AddParameter(animation, "Parameter", "camera_multiple", attack.animation.camera_multiple);
}

void BattleSceneInstaller::WriteEnemies(){
    for (const Enemy& enemy : enemies_){
        TiXmlDocument xml;
        std::unique_ptr<TiXmlElement> container(new TiXmlElement("enemy"));
        container->SetAttribute("id", enemy.id);
        container->SetAttribute("model", enemy.model);
        container->SetAttribute("name", enemy.name);
        container->SetAttribute("level", enemy.level);
        container->SetAttribute("exp", enemy.exp);
        container->SetAttribute("ap", enemy.ap);
        container->SetAttribute("money", enemy.money);
        container->SetAttribute("morph", enemy.morph);
        container->SetDoubleAttribute("back_damage", enemy.back_damage);
        AddEnemyStats(*container, enemy);
        AddEnemyResistances(*container, enemy);
        AddEnemyAttacks(*container, enemy);
        AddEnemyItems(*container, "steal", enemy.steal);
        AddEnemyItems(*container, "drop", enemy.drop);
        TiXmlElement& animations = AddElement(*container, "animations");
        for (unsigned int animation : enemy.animations)
            AddElement(animations, "animation").SetAttribute("id", animation);
        AddEnemyScripts(*container, enemy);
        xml.LinkEndChild(container.release());
        xml.SaveFile(data_dir_ + BuildEnemyFileName(enemy));
    }
}

void BattleSceneInstaller::AddEnemyStats(TiXmlElement& parent, const Enemy& enemy){
    TiXmlElement& stats = AddElement(parent, "stats");
    AddParameter(stats, "stat", "str", enemy.str);
    AddParameter(stats, "stat", "mag", enemy.mag);
    AddParameter(stats, "stat", "def", enemy.def);
    AddParameter(stats, "stat", "mdef", enemy.mdef);
    AddParameter(stats, "stat", "spd", enemy.spd);
    AddParameter(stats, "stat", "lck", enemy.lck);
    AddParameter(stats, "stat", "eva", enemy.eva);
    AddParameter(stats, "stat", "hp", enemy.hp);
    AddParameter(stats, "stat", "mp", enemy.mp);
}

void BattleSceneInstaller::AddEnemyResistances(TiXmlElement& parent, const Enemy& enemy){
    TiXmlElement& elements = AddElement(parent, "elements");
    for (const Enemy::Element& element : enemy.elements){
        TiXmlElement& xml_element = AddElement(elements, "element");
        xml_element.SetAttribute("id", element.id);
        xml_element.SetDoubleAttribute("factor", element.factor);
    }
    TiXmlElement& immunities = AddElement(parent, "immunities");
    for (const Enemy::Immunity& immunity : enemy.immunities){
        TiXmlElement& xml_immunity = AddElement(immunities, "immunity");
        xml_immunity.SetAttribute("status", immunity.status);
        xml_immunity.SetDoubleAttribute("rate", immunity.rate);
    }
}

void BattleSceneInstaller::AddEnemyAttacks(TiXmlElement& parent, const Enemy& enemy){
    TiXmlElement& attacks = AddElement(parent, "attacks");
    for (const Enemy::Attack& attack : enemy.attacks){
        TiXmlElement& xml_attack = AddElement(attacks, "attack");
        xml_attack.SetAttribute("id", attack.id);
        xml_attack.SetAttribute("animation", attack.animation);
        xml_attack.SetAttribute("camera", attack.camera);
    }
    TiXmlElement& manipulate = AddElement(parent, "manipulate");
    manipulate.SetAttribute("manipulable", enemy.manipulate_attacks.size() > 0 ? 1 : 0);
    for (u16 attack : enemy.manipulate_attacks)
        AddElement(manipulate, "attack").SetAttribute("id", attack);
}

void BattleSceneInstaller::AddEnemyItems(
  TiXmlElement& parent, const char* name, const std::vector<Enemy::Item>& items
){
    TiXmlElement& list = AddElement(parent, name);
    for (const Enemy::Item& item : items){
        TiXmlElement& xml_item = AddElement(list, "item");
        xml_item.SetAttribute("id", item.id);
        xml_item.SetDoubleAttribute("rate", item.rate);
    }
}

void BattleSceneInstaller::AddEnemyScripts(TiXmlElement& parent, const Enemy& enemy){
    // AI scripts, as hexadecimal bytecode.
    static const char* AI_SLOT_NAMES[16] = {
      "init", "main", "counter_general", "counter_death", "counter_physical", "counter_magic",
      "battle_end", "pre_turn", "event_1", "event_2", "event_3", "event_4", "event_5",
      "event_6", "event_7", "event_8"
    };
    TiXmlElement& ai = AddElement(parent, "ai");
    for (size_t slot = 0; slot < enemy.ai_scripts.size() && slot < 16; slot ++){
        if (enemy.ai_scripts[slot].empty()) continue;
        std::string hex;
        char digits[3];
        for (u8 byte : enemy.ai_scripts[slot]){
            snprintf(digits, sizeof(digits), "%02x", byte);
            hex += digits;
        }
        TiXmlElement& xml_script = AddElement(ai, "script");
        xml_script.SetAttribute("slot", static_cast<int>(slot));
        xml_script.SetAttribute("name", AI_SLOT_NAMES[slot]);
        xml_script.SetAttribute("data", hex);
    }
}

void BattleSceneInstaller::WriteFormations(){
    for (Formation formation : formations_){
        TiXmlDocument xml;
        std::unique_ptr<TiXmlElement> container(new TiXmlElement("formation"));
        container->SetAttribute("id", formation.id);
        container->SetAttribute("next", formation.next_formation);
        container->SetDoubleAttribute("escape_difficulty", formation.escape_counter);
        container->SetAttribute("escapable", formation.escapable);
        container->SetAttribute("skip_victory_pose", formation.skip_victory_pose);
        container->SetAttribute("skip_spoils", formation.skip_spoils);
        container->SetAttribute("skip_victory_pose", formation.skip_victory_pose);
        container->SetAttribute("preemptive_disabled", formation.preemptive_disabled);
        container->SetAttribute("layout", static_cast<int>(formation.layout));
        std::unique_ptr<TiXmlElement> location(new TiXmlElement("location"));
        location->SetAttribute("id", formation.location);
        location->SetAttribute("name", formation.location_name);
        container->LinkEndChild(location.release());
        std::unique_ptr<TiXmlElement> enemies(new TiXmlElement("enemies"));
        for (Formation::Enemy enemy : formation.enemies){
            std::unique_ptr<TiXmlElement> enemy_xml(new TiXmlElement("enemy"));
            enemy_xml->SetAttribute("id", enemy.id);
            enemy_xml->SetAttribute("x", enemy.x);
            enemy_xml->SetAttribute("y", enemy.y);
            enemy_xml->SetAttribute("z", enemy.z);
            enemy_xml->SetAttribute("row", enemy.row);
            std::string cover = "";
            for (int c = 0; c < 5; c ++) cover += enemy.cover[c];
            enemy_xml->SetAttribute("cover", cover);
            enemy_xml->SetAttribute("visible", enemy.visible);
            enemy_xml->SetAttribute("direction", enemy.direction);
            enemy_xml->SetAttribute("targeteable", enemy.targeteable);
            enemy_xml->SetAttribute("main_script_active", enemy.main_script_active);
            enemies->LinkEndChild(enemy_xml.release());
        }
        container->LinkEndChild(enemies.release());
        std::unique_ptr<TiXmlElement> cameras(new TiXmlElement("camera"));
        cameras->SetAttribute("initial", formation.initial_camera_position);
        cameras->SetAttribute("intro", formation.intro_camera);
        int c = 0;
        for (Formation::Camera camera : formation.camera_positions){
            std::unique_ptr<TiXmlElement> camera_xml(new TiXmlElement("position"));
            camera_xml->SetAttribute("id", c);
            camera_xml->SetDoubleAttribute("x", camera.x);
            camera_xml->SetDoubleAttribute("y", camera.y);
            camera_xml->SetDoubleAttribute("z", camera.z);
            camera_xml->SetDoubleAttribute("direction_x", camera.d_x);
            camera_xml->SetDoubleAttribute("direction_y", camera.d_y);
            camera_xml->SetDoubleAttribute("direction_z", camera.d_z);
            cameras->LinkEndChild(camera_xml.release());
            c ++;
        }
        container->LinkEndChild(cameras.release());
        std::unique_ptr<TiXmlElement> arena(new TiXmlElement("arena"));
        arena->SetAttribute("is_arena", formation.is_arena_battle);
        for (int next : formation.next_arena_formation_candidates){
            std::unique_ptr<TiXmlElement> next_xml(new TiXmlElement("next_candidate"));
            next_xml->SetAttribute("id", next);
            arena->LinkEndChild(next_xml.release());
        }
        container->LinkEndChild(arena.release());
        xml.LinkEndChild(container.release());
        std::string file_name = std::to_string(formation.id);
        while (file_name.length() < 4) file_name = "0" + file_name;
        file_name = "battle/formations/" + file_name + ".xml";
        xml.SaveFile(data_dir_ + file_name);
    }
}

std::string BattleSceneInstaller::BuildEnemyFileName(Enemy enemy){
    std::string file_name = "battle/enemies/";
    std::string id = std::to_string(enemy.id);
    while (id.size() < 4) id = "0" + id;
    file_name += id + ".xml";
    return file_name;
}

std::string BattleSceneInstaller::BuildAttackFileName(Attack attack){
    std::string file_name = "battle/attacks/";
    std::string id = std::to_string(attack.id);
    while (id.size() < 4) id = "0" + id;
    file_name += (id + "_");
    for (int n = 0; n < attack.name.size(); n ++){
        if (
          (attack.name[n] >= '0' && attack.name[n] <= '9')
          || (attack.name[n] >= 'a' && attack.name[n] <= 'z')
          || (attack.name[n] >= 'A' && attack.name[n] <= 'Z')
        ) file_name += attack.name[n];
    }
    file_name += ".xml";
    return file_name;
}

std::unique_ptr<File> BattleSceneInstaller::ExtractGZipScene(const File& file){
    std::vector<u8> compressed(file.GetFileSize());
    if (!compressed.empty()) file.GetFileBuffer(compressed.data(), 0, compressed.size());
    std::vector<u8> out;
    std::string error;
    if (compressed.empty() || !BinGZipFile::Inflate(compressed.data(), compressed.size(), out, error) || out.empty()){
        std::cout << "Warning: " << (error.empty() ? "empty data" : error) << " in file " << file.GetFileName() << std::endl;
        return nullptr;
    }
    return std::make_unique<File>(out.data(), 0, static_cast<u32>(out.size()));
}
