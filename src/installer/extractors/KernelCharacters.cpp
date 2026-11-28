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
#include <fstream>
#include <iostream>
#include <iterator>
#include "formats/FF7NameLookup.h"
#include "installer/extractors/KernelDataInstaller.h"

using namespace KernelData;

int KernelDataInstaller::ReadCharacters(){

    // TODO: Do something about young Cloud and Sephiroth.

    // Empty the item list
    characters_.clear();

    File characters_file = kernel_.ExtractGZip(KERNEL_BATTLE_AND_GROWTH_DATA);
    int character_count = 0;

    for (int i = 0; i < 11; i ++){ // 11 characters

        CharacterData data;

        // Read data from the item data section.
        data.curve_str = characters_file.readU8();
        data.curve_vit = characters_file.readU8();
        data.curve_mag = characters_file.readU8();
        data.curve_spr = characters_file.readU8();
        data.curve_dex = characters_file.readU8();
        data.curve_lck = characters_file.readU8();
        data.curve_hp = characters_file.readU8();
        data.curve_mp = characters_file.readU8();
        data.curve_exp = characters_file.readU8();
        data.unused_0 = characters_file.readU8();
        data.initial_level = characters_file.readU8();
        data.unused_1 = characters_file.readU8();
        data.limit_1_1 = characters_file.readU8();
        data.limit_1_2 = characters_file.readU8();
        data.limit_1_3 = characters_file.readU8();
        data.limit_2_1 = characters_file.readU8();
        data.limit_2_2 = characters_file.readU8();
        data.limit_2_3 = characters_file.readU8();
        data.limit_3_1 = characters_file.readU8();
        data.limit_3_2 = characters_file.readU8();
        data.limit_3_3 = characters_file.readU8();
        data.limit_4_1 = characters_file.readU8();
        data.limit_4_2 = characters_file.readU8();
        data.limit_4_3 = characters_file.readU8();
        data.limit_2_kills = characters_file.readU16LE();
        data.limit_3_kills = characters_file.readU16LE();
        data.limit_1_2_uses = characters_file.readU16LE();
        data.limit_1_3_uses = characters_file.readU16LE();
        data.limit_2_2_uses = characters_file.readU16LE();
        data.limit_2_3_uses = characters_file.readU16LE();
        data.limit_3_2_uses = characters_file.readU16LE();
        data.limit_3_3_uses = characters_file.readU16LE();
        data.limit_1_div = characters_file.readU32LE();
        data.limit_2_div = characters_file.readU32LE();
        data.limit_3_div = characters_file.readU32LE();
        data.limit_4_div = characters_file.readU32LE();

        // Derive data from the read data.
        data.id = i;
        data.name = VGears::NameLookup::CharName(i);

        // Add to the list of characters.
        characters_.push_back(data);
        character_count ++;
    }

    return character_count;
}

void KernelDataInstaller::WriteCharacters(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (CharacterData character : characters_){
        file
          << "Game.Characters[" << character.id << "] = {\n"
          << "    id = " << character.id << ",\n"
          << "    name = \"" << character.name << "\",\n"
          << "    initial_level = \"" << static_cast<int>(character.initial_level) << "\",\n"
          << "    curves = {\n"
          << "        str = " << static_cast<int>(character.curve_str) << ",\n"
          << "        vit = " << static_cast<int>(character.curve_vit) << ",\n"
          << "        mag = " << static_cast<int>(character.curve_mag) << ",\n"
          << "        spr = " << static_cast<int>(character.curve_spr) << ",\n"
          << "        dex = " << static_cast<int>(character.curve_dex) << ",\n"
          << "        lck = " << static_cast<int>(character.curve_lck) << ",\n"
          << "        hp = " << static_cast<int>(character.curve_hp) << ",\n"
          << "        mp = " << static_cast<int>(character.curve_mp) << ",\n"
          << "        exp = " << static_cast<int>(character.curve_exp) << ",\n"
          << "    },\n"
          << "    limit_divisors = {" << character.limit_1_div << ", " << character.limit_2_div << ", "
          << character.limit_3_div << ", " << character.limit_4_div << "},\n"
          << "    limits = {\n"
          << "        [1] = {\n"
          << "            kills = 0,\n"
          << "            techniques = {\n"
          << "                [1] = {\n"
          << "                    command = " << static_cast<int>(character.limit_1_1) << ",\n"
          << "                    uses = 0\n"
          << "                },\n";
        if (character.limit_1_2 < 255){ // Some characters only have 1 limit per level.
            file
              << "                [2] = {\n"
              << "                    command = " << static_cast<int>(character.limit_1_2) << ",\n"
              << "                    uses = " << character.limit_1_2_uses << "\n"
              << "                },\n";
        }
        file
          << "            }\n"
          << "        },\n";
        // Limit level 2.
        if (character.limit_2_kills < 65535){
            file
              << "        [2] = {\n"
              << "            kills = " << character.limit_2_kills << ",\n"
              << "            techniques = {\n"
              << "                [1] = {\n"
              << "                    command = " << static_cast<int>(character.limit_2_1) << ",\n"
              << "                    uses = 0\n"
              << "                },\n";
            if (character.limit_2_2 < 255){ // Some characters only have 1 limit per level.
                file
                  << "                [2] = {\n"
                  << "                    command = "
                  << static_cast<int>(character.limit_2_2) << ",\n"
                  << "                    uses = " << character.limit_2_2_uses << "\n"
                  << "                },\n";
            }
            file
              << "            }\n"
              << "        },\n";
        }
        // Limit level 3.
        if (character.limit_3_kills < 65535){
            file
              << "        [3] = {\n"
              << "            kills = " << character.limit_3_kills << ",\n"
              << "            techniques = {\n"
              << "                [1] = {\n"
              << "                    command = " << static_cast<int>(character.limit_3_1) << ",\n"
              << "                    uses = 0\n"
              << "                },\n";
            if (character.limit_3_2 < 255){ // Some characters only have 1 limit per level.
                file
                  << "                [2] = {\n"
                  << "                    command = "
                  << static_cast<int>(character.limit_3_2) << ",\n"
                  << "                    uses = " << character.limit_3_2_uses << "\n"
                  << "                },\n";
            }
            file
              << "            }\n"
              << "        },\n";
        }
        // Limit level 4.
        if (character.limit_4_1 < 255){
            file
              << "        [4] = {\n"
              << "            kills = 0,\n"
              << "            techniques = {\n"
              << "                [1] = {\n"
              << "                    command = " << static_cast<int>(character.limit_4_1) << ",\n"
              << "                    uses = 0\n"
              << "                },\n"
              << "            }\n"
              << "        },\n";
        }
        file
          << "    }\n}\n";
    }
    file.close();
}

void KernelDataInstaller::ReadGrowth(){

    File growth_file = kernel_.ExtractGZip(KERNEL_BATTLE_AND_GROWTH_DATA);
    growth_file.SetOffset(0x01F8); //Skip character data.

    for (int i = 0; i < 12; i ++) growth_.bonus_stat[i] = growth_file.readU8();
    for (int i = 0; i < 12; i ++) growth_.bonus_hp[i] = growth_file.readU8();
    for (int i = 0; i < 12; i ++) growth_.bonus_mp[i] = growth_file.readU8();
    for (int i = 0; i < 64; i ++){
        growth_.curves[i].id = i;
        if (i < 37) growth_.curves[i].type = CURVE_PRIMARY;
        else if (i >=37 && i < 46) growth_.curves[i].type = CURVE_HP;
        else if (i >=46 && i < 55) growth_.curves[i].type = CURVE_MP;
        else growth_.curves[i].type = CURVE_EXP;
        for (int j = 0; j < 8; j ++){
            growth_.curves[i].gradient[j] = growth_file.readU8();
            growth_.curves[i].base[j] = static_cast<s8>(growth_file.readU8());
        }
    }
}

void KernelDataInstaller::WriteGrowth(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    file << "Game.Growth.bonus.primary = {";
    for (int i = 0; i < 12; i ++) file << "[" << i << "] = " << static_cast<int>(growth_.bonus_stat[i]) << ", ";
    file << "}\nGame.Growth.bonus.hp = {";
    for (int i = 0; i < 12; i ++) file << "[" << i << "] = " << static_cast<int>(growth_.bonus_hp[i]) << ", ";
    file << "}\nGame.Growth.bonus.mp = {";
    for (int i = 0; i < 12; i ++) file << "[" << i << "] = " << static_cast<int>(growth_.bonus_mp[i]) << ", ";
    file << "}\n";
    for (int i = 0; i < 64; i ++){
        file << "Game.Growth.curves[" << i << "] = {\n";
        for (int j = 0; j < 8; j ++){
            file << "    [" << j << "] = {gradient = " << static_cast<int>(growth_.curves[i].gradient[j])
              << ", base = " << static_cast<int>(growth_.curves[i].base[j]) << "},\n";
        }
        file << "}\n";
    }

    file.close();
}

void KernelDataInstaller::ReadInitialSaveMap(){

    File savemap_file = kernel_.ExtractGZip(KERNEL_INITIALIZATION_DATA);
    // The initial savemap in KERNEL.BIN startas at 0x0054, directly into Cloud's level.
    // It skips the header (PC and PSX, checksum, preview data and window colour).
    for (int i = 0; i < 9; i ++) ReadSaveMapCharacter(savemap_file, savemap_.characters[i]);
    // The initial savemap ends up after character data, end now.
}

void KernelDataInstaller::ReadSaveMapCharacter(File& savemap_file, SaveMap::Character& character){
    character.identifier = savemap_file.readU8();
    character.level = savemap_file.readU8();
    character.str = savemap_file.readU8();
    character.vit = savemap_file.readU8();
    character.mag = savemap_file.readU8();
    character.spr = savemap_file.readU8();
    character.dex = savemap_file.readU8();
    character.lck = savemap_file.readU8();
    character.str_bonus = savemap_file.readU8();
    character.vit_bonus = savemap_file.readU8();
    character.mag_bonus = savemap_file.readU8();
    character.spr_bonus = savemap_file.readU8();
    character.dex_bonus = savemap_file.readU8();
    character.lck_bonus = savemap_file.readU8();
    character.limit_level = savemap_file.readU8();
    character.limit_bar = savemap_file.readU8();
    bool name_end = false;
    character.name = "";
    for (int j = 0; j < 12; j ++){
        character.name_raw[j] = savemap_file.readU8();
        if (name_end == false){
            if (character.name_raw[j] != 0XFF){
                character.name += ENGLISH_CHARS[character.name_raw[j]];
            }
            else name_end = true;
        }
    }
    character.weapon = savemap_file.readU8();
    character.armor = savemap_file.readU8();
    character.accessory = savemap_file.readU8();
    character.status_raw = savemap_file.readU8();
    if (character.status_raw == (character.status_raw | 0x10)){
        character.sadness = true;
        character.fury = false;
    }
    else if (character.status_raw == (character.status_raw | 0x20)){
        character.sadness = false;
        character.fury = true;
    }
    else{
        character.sadness = false;
        character.fury = false;
    }
    character.row_raw = savemap_file.readU8();
    if (character.row_raw == 0xFE) character.back_row = true;
    else character.back_row = false;
    character.level_progress = savemap_file.readU8();
    character.learned_limits_raw = savemap_file.readU16LE();
    for (int j = 0; j < 4; j ++){
        character.limits_learned[j][0] = false;
        character.limits_learned[j][1] = false;
    }
    if (character.learned_limits_raw == (character.learned_limits_raw | 0x0001))
        character.limits_learned[0][0] = true;
    if (character.learned_limits_raw == (character.learned_limits_raw | 0x0002))
        character.limits_learned[0][1] = true;
    if (character.learned_limits_raw == (character.learned_limits_raw | 0x0008))
        character.limits_learned[1][0] = true;
    if (character.learned_limits_raw == (character.learned_limits_raw | 0x0010))
        character.limits_learned[1][1] = true;
    if (character.learned_limits_raw == (character.learned_limits_raw | 0x0040))
        character.limits_learned[2][0] = true;
    if (character.learned_limits_raw == (character.learned_limits_raw | 0x0080))
        character.limits_learned[2][1] = true;
    if (character.learned_limits_raw == (character.learned_limits_raw | 0x0200))
        character.limits_learned[3][0] = true;
    character.kills = savemap_file.readU16LE();
    character.limit_uses[0] = savemap_file.readU16LE();
    character.limit_uses[1] = savemap_file.readU16LE();
    character.limit_uses[2] = savemap_file.readU16LE();
    character.hp = savemap_file.readU16LE();
    character.base_hp = savemap_file.readU16LE();
    character.mp = savemap_file.readU16LE();
    character.base_mp = savemap_file.readU16LE();
    character.unknown = savemap_file.readU32LE();
    character.max_hp = savemap_file.readU16LE();
    character.max_mp = savemap_file.readU16LE();
    character.exp = savemap_file.readU32LE();
    for (int m = 0; m < 8; m ++){
        character.weapon_materia[m].id = savemap_file.readU8();
        // TODO: Identify Enemy Skill, do special treatment
        character.weapon_materia[m].ap // WARNING: AP has only three bytes.
          = savemap_file.readU8() + (savemap_file.readU8() << 8) + (savemap_file.readU8() << 16);
    }
    for (int m = 0; m < 8; m ++){
        character.armor_materia[m].id = savemap_file.readU8();
        // TODO: Identify Enemy Skill, do special treatment
        character.armor_materia[m].ap // WARNING: AP has only three bytes.
          = savemap_file.readU8() + (savemap_file.readU8() << 8) + (savemap_file.readU8() << 16);
    }
    character.exp_to_next = savemap_file.readU32LE();
}

void KernelDataInstaller::ReadRecruitRecords(const std::string& exe_path){
    recruit_records_found_ = false;
    std::ifstream exe(exe_path, std::ios::binary);
    const std::vector<u8> data(
      (std::istreambuf_iterator<char>(exe)), std::istreambuf_iterator<char>()
    );
    // The data of Cait Sith and Vincent are two character records like the ones of the savemap, one
    // after the other (at 0x9138d0 in the 1998 build). They are found by their contents: the
    // character IDs 6 and 7, valid levels, weapons of each one and names that end.
    const size_t size = 0x84;
    for (size_t offset = 0; offset + 2 * size <= data.size(); offset ++){
        bool found = true;
        for (int r = 0; r < 2 && found; r ++){
            const u8* record = &data[offset + r * size];
            const int first_weapon = r == 0 ? 101 : 114;
            const int last_weapon = r == 0 ? 113 : 127;
            found = record[0] == 6 + r && record[1] >= 1 && record[1] <= 99
              && record[0x1C] >= first_weapon && record[0x1C] <= last_weapon
              && std::find(record + 0x10, record + 0x1C, 0xFF) != record + 0x1C;
        }
        if (!found) continue;
        File records(&data[offset], 0, 2 * size);
        for (int r = 0; r < 2; r ++) ReadSaveMapCharacter(records, recruit_records_[r]);
        recruit_records_found_ = true;
        return;
    }
    std::cerr << "[WARNING] Can't find the data of Cait Sith and Vincent in ff7.exe." << std::endl;
}

void KernelDataInstaller::WriteInitialSaveMap(std::string file_name){
    // The initial save map only contains character data.
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    for (int c = 0; c < 9; c ++){
        file << "Characters[" << c << "] = ";
        WriteSaveMapCharacter(file, savemap_.characters[c]);
        file << "Characters.recalculate_stats(" << c << ")\n\n";
    }
    if (recruit_records_found_){
        file << "-- Data that Cait Sith and Vincent get when they join, from ff7.exe.\n"
          << "Game.RecruitRecords = {}\n";
        for (int r = 0; r < 2; r ++){
            file << "Game.RecruitRecords[" << 6 + r << "] = ";
            WriteSaveMapCharacter(file, recruit_records_[r]);
        }
    }
    file.close();
}

void KernelDataInstaller::WriteSaveMapCharacter(
  std::ostream& file, const SaveMap::Character& character
){
    int total_count; // Utility to count in loops.
    file
      << "{\n"
      << "    char_id = " << static_cast<int>(character.identifier) << ",\n"
      << "    name = \"" << character.name << "\",\n"
      << "    enabled = false,\n"
      << "    locked = false,\n"
      << "    level = " << static_cast<int>(character.level) << ",\n"
      << "    kills = " << character.kills << ",\n"
      << "    back_row = " << (character.back_row ? "true" : "false") << ",\n"
      << "    exp = " << character.exp << ",\n"
      << "    exp_to_next = " << character.exp_to_next << ",\n"
      << "    stats = {\n"
      << "        str = {base = " << static_cast<int>(character.str)
      << ", bonus = " << static_cast<int>(character.str_bonus)
      << ", total = " << static_cast<int>(character.str + character.str_bonus) << "},\n"
      << "        vit = {base = " << static_cast<int>(character.vit)
      << ", bonus = " << static_cast<int>(character.vit_bonus)
      << ", total = " << static_cast<int>(character.vit + character.vit_bonus) << "},\n"
      << "        mag = {base = " << static_cast<int>(character.mag)
      << ", bonus = " << static_cast<int>(character.mag_bonus)
      << ", total = " << static_cast<int>(character.mag + character.mag_bonus) << "},\n"
      << "        spr = {base = " << static_cast<int>(character.spr)
      << ", bonus = " << static_cast<int>(character.spr_bonus)
      << ", total = " << static_cast<int>(character.spr + character.spr_bonus) << "},\n"
      << "        dex = {base = " << static_cast<int>(character.dex)
      << ", bonus = " << static_cast<int>(character.dex_bonus)
      << ", total = " << static_cast<int>(character.dex + character.dex_bonus) << "},\n"
      << "        lck = {base = " << static_cast<int>(character.lck)
      << ", bonus = " << static_cast<int>(character.lck_bonus)
      << ", total = " << static_cast<int>(character.lck + character.lck_bonus) << "},\n"
      << "        hp = {current = " << character.hp  << ", base = " << character.base_hp
      << ", max = " << character.max_hp << "},\n"
      << "        mp = {current = " << character.mp  << ", base = " << character.base_mp
      << ", max = " << character.max_mp << "},\n"
      << "        atk = 0,\n"
      << "        acc = 0,\n"
      << "        def = 0,\n"
      << "        eva = 0,\n"
      << "        matk = 0,\n"
      << "        mdef = 0,\n"
      << "        meva = 0\n"
      << "    },\n"
      << "    limit = {\n"
      << "        current = " << static_cast<int>(character.limit_level) << ",\n"
      << "        bar = " << static_cast<int>(character.limit_bar) << ",\n"
      << "        learned = {\n"
      << "            [1] = {[1] = " << (character.limits_learned[0][0] == 1 ? "true" : "false")
      << ", [2] = " << (character.limits_learned[0][1] == 1 ? "true" : "false") << "},\n"
      << "            [2] = {[1] = " << (character.limits_learned[1][0] == 1 ? "true" : "false")
      << ", [2] = " << (character.limits_learned[1][1] == 1 ? "true" : "false") << "},\n"
      << "            [3] = {[1] = " << (character.limits_learned[2][0] == 1 ? "true" : "false")
      << ", [2] = " << (character.limits_learned[2][1] == 1 ? "true" : "false") << "},\n"
      << "            [4] = {[1] = " << (character.limits_learned[3][0] == 1 ? "true" : "false")
      << ", [2] = " << (character.limits_learned[3][1] == 1 ? "true" : "false") << "}\n"
      << "        },\n"
      << "        uses = {[1] = " << character.limit_uses[0] << ", [2] = "
      << character.limit_uses[1] << ", [3] = " << character.limit_uses[2] << ", [4] = 0}\n"
      << "    },\n"
      << "    weapon = {\n"
      << "        id = " << static_cast<int>(character.weapon) + 128 << ",\n"
      << "        materia = {";
    total_count = 0;
    for (int m = 0; m < 8; m ++){
        if (character.weapon_materia[m].id < 255){
            total_count ++;
            file << "\n            [" << m << "] = {id = "
              << static_cast<int>(character.weapon_materia[m].id) << ", ap = "
              << character.weapon_materia[m].ap << "},";
        }
    }
    if (total_count > 0) file << "\n        }\n    },\n";
    else file << "}\n    },\n";
    file
      << "    armor = {\n"
      << "        id = " << static_cast<int>(character.armor) + 256 << ",\n"
      << "        materia = {";
    total_count = 0;
    for (int m = 0; m < 8; m ++){
        if (character.armor_materia[m].id < 255){
            total_count ++;
            file << "\n            [" << m << "] = {id = "
              << static_cast<int>(character.armor_materia[m].id) << ", ap = "
              << character.armor_materia[m].ap << "},";
        }
    }
    if (total_count > 0) file << "\n        }\n    },\n";
    else file << "}\n    },\n";
    file << "    accessory = ";
    if (character.accessory < 0xFF) file << static_cast<int>(character.accessory) + 288;
    else file << "nil";
    file << ",\n    status = {";
    if (character.fury) file << FURY;
    if (character.sadness) file << SADNESS;
    file << "}\n}\n";
}
