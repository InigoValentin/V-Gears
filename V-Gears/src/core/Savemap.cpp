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

#include <string>
#include <iostream>
#include <algorithm>
#include "core/Savemap.h"
#include "common/TypeDefine.h"

Savemap::Savemap():
  empty_(true), control_(""), money_(0), seconds_(0), countdown_(0), field_map_("")
{
    for (int i = 0; i < 4; i ++){
        window_colours_[i][0] = 0;
        window_colours_[i][1] = 0;
        window_colours_[i][2] = 0;
    }
    // TODO: Characters? Needed to initialize?
    party_[0] = -1;
    party_[1] = -1;
    party_[2] = -1;
    for (int i = 0; i < 500; i ++){
        items_[i].id = -1;
        items_[i].quantity = 0;
        materia_[i].id = -1;
        materia_[i].ap = 0;
        materia_stash_[i].id = -1;
        materia_stash_[i].ap = 0;
    }
    for (int i = 0; i < 50; i ++){
        key_items_[i] = false;
    }
    for (int b = 0; b < 15; b ++){
        for (int a = 0; a < 15; a ++){
            data_[b][a] = 0;
        }
    }
}

Savemap::~Savemap(){}

void Savemap::operator = (const Savemap &to_copy){
    empty_ = to_copy.empty_;
    control_ = to_copy.control_;
    for (int i = 0; i < 4; i ++){
        window_colours_[i][0] = to_copy.window_colours_[i][0];
        window_colours_[i][1] = to_copy.window_colours_[i][0];
        window_colours_[i][2] = to_copy.window_colours_[i][0];
    }
    for (int i = 0; i < 11; i ++) characters_[i] = to_copy.characters_[i];
    for (int i = 0; i < 3; i ++) party_[i] = to_copy.party_[i];
}

bool Savemap::IsEmpty(){return empty_;}

void Savemap::Read(std::string file_name){
    // TODO
}

void Savemap::SetData(const unsigned int bank, const unsigned int address, const int value){
    if (bank < BANK_COUNT && address < BANK_ADDRESS_COUNT) data_[bank][address] = value;
}

void Savemap::SetControlKey(const std::string control){control_ = control;}

/**
 * Sets the window colours in the current savemap.
 *
 * @param[in] t_l_r Top-left corner, red component.
 * @param[in] t_l_r Top-left corner, green component.
 * @param[in] t_l_r Top-left corner, blue component.
 * @param[in] t_r_r Top-right corner, red component.
 * @param[in] t_r_r Top-right corner, green component.
 * @param[in] t_r_r Top-right corner, blue component.
 * @param[in] b_r_r Bottom-right corner, red component.
 * @param[in] b_r_r Bottom-right corner, green component.
 * @param[in] b_r_r Bottom-right corner, blue component.
 * @param[in] b_l_r Bottom-left corner, red component.
 * @param[in] b_l_r Bottom-left corner, green component.
 * @param[in] b_l_r Bottom-left corner, blue component.
 */
void Savemap::SetWindowColours(
  const unsigned int t_l_r, const unsigned int t_l_g, const unsigned int t_l_b,
  const unsigned int t_r_r, const unsigned int t_r_g, const unsigned int t_r_b,
  const unsigned int b_r_r, const unsigned int b_r_g, const unsigned int b_r_b,
  const unsigned int b_l_r, const unsigned int b_l_g, const unsigned int b_l_b
){
    window_colours_[0][0] = std::min(MAX_COLOUR, t_l_r);
    window_colours_[0][1] = std::min(MAX_COLOUR, t_l_g);
    window_colours_[0][2] = std::min(MAX_COLOUR, t_l_b);
    window_colours_[1][0] = std::min(MAX_COLOUR, t_r_r);
    window_colours_[1][1] = std::min(MAX_COLOUR, t_r_g);
    window_colours_[1][2] = std::min(MAX_COLOUR, t_r_b);
    window_colours_[2][0] = std::min(MAX_COLOUR, b_r_r);
    window_colours_[2][1] = std::min(MAX_COLOUR, b_r_g);
    window_colours_[2][2] = std::min(MAX_COLOUR, b_r_b);
    window_colours_[3][0] = std::min(MAX_COLOUR, b_l_r);
    window_colours_[3][1] = std::min(MAX_COLOUR, b_l_g);
    window_colours_[3][2] = std::min(MAX_COLOUR, b_l_b);
}

std::string Savemap::GetControlKey() const {return control_;}

void Savemap::SetMoney(const unsigned int money){money_ = money;}

void Savemap::SetGameTime(const unsigned int seconds){seconds_ = seconds;}

void Savemap::SetCountdownTime(const unsigned int seconds){countdown_ = seconds;}

void Savemap::SetField(const std::string field){field_map_ = field;}

void Savemap::SetKeyItem(const unsigned int item, const bool owned){
    if (item < MAX_KEY_ITEM_SLOTS) key_items_[item] = owned;
}

void Savemap::SetParty(const int member_1, const int member_2, const int member_3){
    if (member_1 == -1 || (member_1 >= 0 && member_1 < MAX_CHARACTERS)) party_[0] = member_1;
    if (member_2 == -1 || (member_2 >= 0 && member_2 < MAX_CHARACTERS)) party_[1] = member_2;
    if (member_3 == -1 || (member_3 >= 0 && member_3 < MAX_CHARACTERS)) party_[2] = member_3;
}

void Savemap::SetItem(const unsigned int slot, const unsigned int id, const unsigned int quantity){
    if (slot < MAX_ITEM_SLOTS){
        items_[slot].id = id;
        items_[slot].quantity = quantity;
    }
}

void Savemap::SetMateria(const unsigned int slot, const int id, const unsigned int ap){
    if (slot < MAX_MATERIA_SLOTS){
        materia_[slot].id = id;
        materia_[slot].ap = ap;
        for (int i = 0; i < MAX_ENEMY_SKILLS; i ++) materia_[slot].enemy_skill_learned[i] = false;
    }
}

void Savemap::SetESkillMateria(const unsigned slot, const unsigned int skill, const bool learned){
    if (slot < MAX_MATERIA_SLOTS && skill < MAX_ENEMY_SKILLS){
        // TODO: Maybe set enemy skill materia ID here?
        materia_[slot].ap = 0;
        materia_[slot].enemy_skill_learned[skill] = learned;
    }
}

void Savemap::SetMateriaStash(const unsigned int slot, const int id, const unsigned int ap){
    if (slot < MAX_STASH_SLOTS){
        materia_stash_[slot].id = id;
        materia_stash_[slot].ap = ap;
        for (int i = 0; i < MAX_ENEMY_SKILLS; i ++)
            materia_stash_[slot].enemy_skill_learned[i] = false;
    }
}

void Savemap::SetESkillMateriaStash(
  const unsigned slot, const unsigned int skill, const bool learned
){
    if (slot < MAX_STASH_SLOTS && skill < MAX_ENEMY_SKILLS){
        // TODO: Maybe set enemy skill materia ID here?
        materia_stash_[slot].ap = 0;
        materia_stash_[slot].enemy_skill_learned[skill] = learned;
    }
}

void Savemap::SetLocation(
  const unsigned int x, const unsigned int y, const int z,
  const unsigned int triangle, const int angle
){
    if (z >= -1 && angle >= -360 && angle <= 360){
        location_.x = x;
        location_.y = y;
        location_.z = z;
        location_.triangle = triangle;
        location_.angle = angle;
    }
}

void Savemap::SetSetting(const unsigned int key, const unsigned int value){
    // TODO : Implements when settings are working.
}

void Savemap::SetCharacterInfo(
  const unsigned int id, const int char_id, const std::string name,
  const bool enabled, const bool locked,
  const unsigned int level, const unsigned int kills,
  const bool back_row, const unsigned int exp, const unsigned int exp_to_next,
  const unsigned int limit_level, const unsigned int limit_bar,
  const unsigned int weapon, const unsigned int armor, const int accessory
){
    if (id < MAX_CHARACTERS){
        characters_[id].id = id;
        characters_[id].char_id = char_id;
        characters_[id].name = name;
        characters_[id].enabled = enabled;
        characters_[id].locked = locked;
        characters_[id].level = level;
        characters_[id].kills = kills;
        characters_[id].back_row = back_row;
        characters_[id].exp = exp;
        characters_[id].exp_to_next = exp_to_next;
        characters_[id].limit_level = std::min(limit_level, MAX_LIMIT_LEVELS);
        characters_[id].limit_bar = std::min(limit_bar, MAX_LIMIT_BAR);
        characters_[id].weapon.id = weapon;
        characters_[id].armor.id = armor;
        characters_[id].accessory = std::max(accessory, -1);
    }
}

void Savemap::SetCharacterStat(
  const unsigned int id, const unsigned int stat,
  const unsigned int base, const unsigned int extra
){
    if (id < MAX_CHARACTERS){
        if (stat == 0){
            characters_[id].str.base = base;
            characters_[id].str.extra = extra;
        }
        else if (stat == 1){
            characters_[id].vit.base = base;
            characters_[id].vit.extra = extra;
        }
        else if (stat == 2){
            characters_[id].mag.base = base;
            characters_[id].mag.extra = extra;
        }
        else if (stat == 3){
            characters_[id].spr.base = base;
            characters_[id].spr.extra = extra;
        }
        else if (stat == 4){
            characters_[id].dex.base = base;
            characters_[id].dex.extra = extra;
        }
        else if (stat == 5){
            characters_[id].lck.base = base;
            characters_[id].lck.extra = extra;
        }
        else if (stat == 6){
            characters_[id].hp.base = base;
            characters_[id].hp.extra = extra;
        }
        else if (stat == 7){
            characters_[id].mp.base = base;
            characters_[id].mp.extra = extra;
        }
    }
}
void Savemap::SetCharacterLimitLearned(
  const unsigned int id, const unsigned int level,
  const unsigned int technique, const bool learned, const unsigned int uses
){
    if (id < MAX_CHARACTERS && level < MAX_LIMIT_LEVELS && technique < MAX_LIMIT_TECHNIQUES){
        characters_[id].limits_learned[level][technique] = learned;
        characters_[id].limit_uses[level] = uses;
    }
}

void Savemap::SetCharacterMateria(
  const unsigned int id, const bool weapon, const unsigned int slot,
  const int materia, const unsigned int ap
){
    if (id < MAX_CHARACTERS && slot < MAX_EQUIP_SLOTS){
        if (weapon){
            characters_[id].weapon.materia[slot].id = id;
            characters_[id].weapon.materia[slot].ap = ap;
            for (int i = 0; i < MAX_ENEMY_SKILLS; i ++)
                characters_[id].weapon.materia[slot].enemy_skill_learned[i] = false;
        }
        else{
            characters_[id].armor.materia[slot].id = id;
            characters_[id].armor.materia[slot].ap = ap;
            for (int i = 0; i < MAX_ENEMY_SKILLS; i ++)
                characters_[id].armor.materia[slot].enemy_skill_learned[i] = false;
        }
    }
}

void Savemap::SetCharacterESkillMateria(
  const unsigned int id, const bool weapon, const unsigned int slot,
  const unsigned int skill, const bool learned
){
    if (id < MAX_CHARACTERS && slot < MAX_EQUIP_SLOTS && skill < MAX_ENEMY_SKILLS){
        if (weapon){
            // TODO: Maybe set enemy skill materia ID here?
            characters_[id].weapon.materia[slot].ap = 0;
            characters_[id].weapon.materia[slot].enemy_skill_learned[skill] = learned;
        }
        else{
            // TODO: Maybe set enemy skill materia ID here?
            characters_[id].armor.materia[slot].ap = 0;
            characters_[id].armor.materia[slot].enemy_skill_learned[skill] = learned;
        }
    }
}

void Savemap::SetCharacterStatus(
  const unsigned int id, const unsigned int status, const bool inflicted
){
    //TODO
}

void Savemap::Write(std::string file_name){
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    file
      << "Savemap = {\n"
      << "    control = \"" << control_ << "\",\n"
      << "    window_colours = {},\n"
      << "    party = {" << party_[0] << ", " << party_[1] << ", " << party_[2] << "},\n"
      << "    characters = {\n";
    for (int c = 0; c < 9; c ++){
        int total_count; // Utility to count in loops.
        Character character = characters_[c];
        file
          << "        [" << c << "] = {\n"
          << "            char_id = " << static_cast<int>(character.char_id) << ",\n"
          << "            name = \"" << character.name << "\",\n"
          << "            enabled = " << (character.enabled ? "true" : "false") << ",\n"
          << "            locked = " << (character.locked ? "true" : "false") << ",\n"
          << "            level = " << static_cast<int>(character.level) << ",\n"
          << "            kills = " << character.kills << ",\n"
          << "            back_row = " << (character.back_row ? "true" : "false") << ",\n"
          << "            exp = " << character.exp << ",\n"
          << "            exp_to_next = " << character.exp_to_next << " ,\n"
          << "            stats = {\n"
          << "                str = {base = " << character.str.base
          << ", bonus = " << character.str.extra
          << ", total = " << (character.str.base + character.str.extra) << "},\n"
          << "                vit = {base = " << character.vit.base
          << ", bonus = " << character.vit.extra
          << ", total = " << (character.vit.base + character.vit.extra) << "},\n"
          << "                mag = {base = " << character.mag.base
          << ", bonus = " << character.mag.extra
          << ", total = " << (character.mag.base + character.mag.extra) << "},\n"
          << "                spr = {base = " << character.spr.base
          << ", bonus = " << character.spr.extra
          << ", total = " << (character.spr.base + character.spr.extra) << "},\n"
          << "                dex = {base = " << character.dex.base
          << ", bonus = " << character.dex.extra
          << ", total = " << (character.dex.base + character.dex.extra) << "},\n"
          << "                lck = {base = " << character.lck.base
          << ", bonus = " << character.lck.extra
          << ", total = " << (character.lck.base + character.lck.extra) << "},\n"
          << "                hp = {current = " << character.hp.extra
          << ", base = " << character.hp.base << ", max = 0},\n"
          << "                mp = {current = " << character.mp.extra
          << ", base = " << character.mp.base << ", max = 0},\n"
          << "                atk = 0,\n"
          << "                acc = 0,\n"
          << "                def = 0,\n"
          << "                eva = 0,\n"
          << "                matk = 0,\n"
          << "                mdef = 0,\n"
          << "                meva = 0\n"
          << "            },\n"
          << "            limit = {\n"
          << "                current = " << static_cast<int>(character.limit_level) << ",\n"
          << "                bar = " << static_cast<int>(character.limit_bar) << ",\n"
          << "                learned = {\n"
          << "                    [1] = {[1] = "
          << (character.limits_learned[0][0] == 1 ? "true" : "false")
          << ", [2] = " << (character.limits_learned[0][1] == 1 ? "true" : "false") << "},\n"
          << "                [2] = {[1] = "
          << (character.limits_learned[1][0] == 1 ? "true" : "false")
          << ", [2] = " << (character.limits_learned[1][1] == 1 ? "true" : "false") << "},\n"
          << "                [3] = {[1] = "
          << (character.limits_learned[2][0] == 1 ? "true" : "false")
          << ", [2] = " << (character.limits_learned[2][1] == 1 ? "true" : "false") << "},\n"
          << "                [4] = {[1] = "
          << (character.limits_learned[3][0] == 1 ? "true" : "false")
          << ", [2] = " << (character.limits_learned[3][1] == 1 ? "true" : "false") << "}\n"
          << "                },\n"
          << "                uses = {[1] = " << character.limit_uses[0] << ", [2] = "
          << character.limit_uses[1] << ", [3] = " << character.limit_uses[2] << ", [4] = 0}\n"
          << "            },\n"
          << "            weapon = {\n"
          << "                id = " << static_cast<int>(character.weapon.id) << ",\n"
          << "                materia = {";
        total_count = 0;
        for (int m = 0; m < 8; m ++){
            if (character.weapon.materia[m].id < 255){
                total_count ++;
                file << "\n                    [" << m << "] = {id = "
                  << static_cast<int>(character.weapon.materia[m].id) << ", ap = "
                  << character.weapon.materia[m].ap << "},";
                // TODO: Enemy skills
            }
        }
        if (total_count > 0) file << "\n                }\n        },\n";
        else file << "        }\n        },\n";
        file
          << "            armor = {\n"
          << "                id = " << static_cast<int>(character.armor.id) << ",\n"
          << "                materia = {";
        total_count = 0;
        for (int m = 0; m < 8; m ++){
            if (character.armor.materia[m].id < 255){
                total_count ++;
                file << "\n                    [" << m << "] = {id = "
                  << static_cast<int>(character.armor.materia[m].id) << ", ap = "
                  << character.armor.materia[m].ap << "},";
                // TODO: Enemy skills
            }
        }
        if (total_count > 0) file << "\n                }\n        },\n";
        else file << "        }\n        },\n";
        file << "            accessory = ";
        if (character.accessory < 0xFF) file << static_cast<int>(character.accessory);
        else file << "nil";
        file << ",\n            status = {";
        // TODO: Loop statuses
        //if (character.fury) file << FURY;
        //if (character.sadness) file << SADNESS;
        file << "}\n        },\n";

    }
    file << "    },\n    data = {\n";
    for (int b = 0; b < BANK_COUNT; b ++){
        file << "        [" << b << "] = {\n";
        for (int a = 0; a < BANK_ADDRESS_COUNT; a ++){
            if (data_[b][a] != 0) file << "            [" << a << "] = " << data_[b][a] << ",\n";
        }
        file << "        },\n";
    }
    file << "    }\n}\n";
    file.close();
}
