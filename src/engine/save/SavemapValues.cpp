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
#include "engine/save/Savemap.h"

void Savemap::ClearContents(){
    for (Item& item : items_) item = {-1, 0};
    for (bool& owned : key_items_) owned = false;
    for (Materia& materia : materia_) materia = {-1, 0, false, {}};
    for (Character& character : characters_){
        for (Materia& materia : character.weapon.materia) materia = {-1, 0, false, {}};
        for (Materia& materia : character.armor.materia) materia = {-1, 0, false, {}};
    }
    for (auto& bank : data_) for (int& value : bank) value = 0;
}

void Savemap::SetData(const unsigned int bank, const unsigned int address, const int value){
    if (bank < BANK_COUNT && address < BANK_ADDRESS_COUNT) data_[bank][address] = value;
}

void Savemap::SetControlKey(const std::string control){
    control_ = control;
    empty_ = false;
}

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
        materia_[slot].enemy_skill = false;
        for (int i = 0; i < MAX_ENEMY_SKILLS; i ++) materia_[slot].enemy_skill_learned[i] = false;
    }
}

void Savemap::SetESkillMateria(const unsigned slot, const unsigned int skill, const bool learned){
    if (slot < MAX_MATERIA_SLOTS && skill < MAX_ENEMY_SKILLS){
        // TODO: Maybe set enemy skill materia ID here?
        materia_[slot].ap = 0;
        materia_[slot].enemy_skill_learned[skill] = learned;
        if (learned) materia_[slot].enemy_skill = true;
    }
}

void Savemap::SetMateriaStash(const unsigned int slot, const int id, const unsigned int ap){
    if (slot < MAX_STASH_SLOTS){
        materia_stash_[slot].id = id;
        materia_stash_[slot].ap = ap;
        materia_stash_[slot].enemy_skill = false;
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
        if (learned) materia_stash_[slot].enemy_skill = true;
    }
}

void Savemap::SetLocation(
  const float x, const float y, const float z,
  const unsigned int triangle, const int angle, std::string field, std::string name
){

    location_.x = x;
    location_.y = y;
    location_.z = z;
    location_.triangle = triangle;
    if (angle >= -360 && angle <= 360) location_.angle = angle;
    location_.field = field;
    location_.name = name;
}

void Savemap::SetSetting(const std::string& key, const int value){settings_[key] = value;}

bool Savemap::HasSetting(const std::string& key) const{return settings_.count(key) > 0;}

int Savemap::GetSetting(const std::string& key) const{
    const auto it = settings_.find(key);
    return (it == settings_.end()) ? 0 : it->second;
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
            characters_[id].weapon.materia[slot].id = materia;
            characters_[id].weapon.materia[slot].ap = ap;
            characters_[id].weapon.materia[slot].enemy_skill = false;
            for (int i = 0; i < MAX_ENEMY_SKILLS; i ++)
                characters_[id].weapon.materia[slot].enemy_skill_learned[i] = false;
        }
        else{
            characters_[id].armor.materia[slot].id = materia;
            characters_[id].armor.materia[slot].ap = ap;
            characters_[id].armor.materia[slot].enemy_skill = false;
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
            if (learned) characters_[id].weapon.materia[slot].enemy_skill = true;
        }
        else{
            // TODO: Maybe set enemy skill materia ID here?
            characters_[id].armor.materia[slot].ap = 0;
            characters_[id].armor.materia[slot].enemy_skill_learned[skill] = learned;
            if (learned) characters_[id].armor.materia[slot].enemy_skill = true;
        }
    }
}

void Savemap::SetCharacterStatus(
  const unsigned int id, const unsigned int status, const bool inflicted
){
    //TODO
}

bool Savemap::IsEmpty(){return empty_;}

std::string Savemap::GetControlKey(){return control_;}

unsigned int Savemap::GetWindowCornerColourComponent(
  const unsigned int corner, const unsigned int comp
){
    if (corner < 4 && comp < 3) return window_colours_[corner][comp];
    else return 0;
}

unsigned int Savemap::GetMoney(){return money_;}

unsigned int Savemap::GetGameTime(){return seconds_;}

unsigned int Savemap::GetCountdownTime(){return countdown_;}

int Savemap::GetPartyMember(const unsigned int pos){
    if (pos < MAX_PARTY_MEMBERS) return party_[pos];
    else return -1;
}

unsigned int Savemap::GetItemAtPosId(const unsigned int pos){
    if (pos >= MAX_ITEM_SLOTS) return 0;
    else if (items_[pos].quantity == 0) return 0;
    else return items_[pos].id;
}

unsigned int Savemap::GetItemAtPosQty(const unsigned int pos){
    if (pos >= MAX_ITEM_SLOTS) return 0;
    else return items_[pos].quantity;
}

bool Savemap::GetKeyItem(const unsigned int id){
    if (id >= MAX_KEY_ITEM_SLOTS) return false;
    else return key_items_[id];
}

int Savemap::GetMateriaAtPosId(const unsigned int pos){
    if (pos >= MAX_MATERIA_SLOTS) return -1;
    else return materia_[pos].id;
}

unsigned int Savemap::GetMateriaAtPosAp(const unsigned int pos){
    if (pos >= MAX_MATERIA_SLOTS) return 0;
    else if (materia_[pos].id < 0) return 0;
    else if (materia_[pos].enemy_skill) return 0;
    else return materia_[pos].ap;
}

bool Savemap::IsMateriaAtPosESkill(const unsigned int pos){
    if (pos >= MAX_MATERIA_SLOTS) return false;
    else if (materia_[pos].id < 0) return false;
    else return materia_[pos].enemy_skill;
}

bool Savemap::IsMateriaAtPosESkillLearned(const unsigned int pos, const unsigned int skill){
    if (pos >= MAX_MATERIA_SLOTS) return false;
    else if (materia_[pos].id < 0) return false;
    else if (materia_[pos].enemy_skill == false) return false;
    else if (skill >= MAX_ENEMY_SKILLS) return false;
    else return materia_[pos].enemy_skill_learned[skill];
}

int Savemap::GetStashAtPosId(const unsigned int pos){
    if (pos >= MAX_STASH_SLOTS) return -1;
    else return materia_stash_[pos].id;
}

unsigned int Savemap::GetStashAtPosAp(const unsigned int pos){
    if (pos >= MAX_STASH_SLOTS) return 0;
    else if (materia_stash_[pos].id < 0) return 0;
    else if (materia_stash_[pos].enemy_skill) return 0;
    else return materia_stash_[pos].ap;
}

bool Savemap::IsStashAtPosESkill(const unsigned int pos){
    if (pos >= MAX_STASH_SLOTS) return false;
    else if (materia_stash_[pos].id < 0) return false;
    else return materia_stash_[pos].enemy_skill;
}

bool Savemap::IsStashAtPosESkillLearned(const unsigned int pos, const unsigned int skill){
    if (pos >= MAX_STASH_SLOTS) return false;
    else if (materia_stash_[pos].id < 0) return false;
    else if (materia_stash_[pos].enemy_skill == false) return false;
    else if (skill >= MAX_ENEMY_SKILLS) return false;
    else return materia_stash_[pos].enemy_skill_learned[skill];
}

float Savemap::GetLocationX(){return location_.x;}

float Savemap::GetLocationY(){return location_.y;}

float Savemap::GetLocationZ(){return location_.z;}

unsigned int Savemap::GetLocationTriangle(){return location_.triangle;}

int Savemap::GetLocationAngle(){return location_.angle;}

std::string Savemap::GetLocationField(){return location_.field;}

std::string Savemap::GetLocationName(){return location_.name;}

int Savemap::GetCharacterCharId(const unsigned int id){
    if (id >= MAX_CHARACTERS) return -1;
    else return characters_[id].char_id;
}

std::string Savemap::GetCharacterName(const unsigned int id){
    if (id >= MAX_CHARACTERS) return std::string("");
    else return characters_[id].name;
}

unsigned int Savemap::GetCharacterLevel(const unsigned int id){
    if (id >= MAX_CHARACTERS) return 1;
    else return characters_[id].level;
}

unsigned int Savemap::GetCharacterKills(const unsigned int id){
    if (id >= MAX_CHARACTERS) return 0;
    else return characters_[id].kills;
}

bool Savemap::IsCharacterEnabled(const unsigned int id){
    if (id >= MAX_CHARACTERS) return false;
    else return characters_[id].enabled;
}

bool Savemap::IsCharacterLocked(const unsigned int id){
    if (id >= MAX_CHARACTERS) return false;
    else return characters_[id].locked;
}

bool Savemap::IsCharacterBackRow(const unsigned int id){
    if (id >= MAX_CHARACTERS) return false;
    else return characters_[id].back_row;
}

unsigned int Savemap::GetCharacterExp(const unsigned int id){
    if (id >= MAX_CHARACTERS) return 0;
    else return characters_[id].exp;
}

unsigned int Savemap::GetCharacterExpToNext(const unsigned int id){
    if (id >= MAX_CHARACTERS) return 0;
    else return characters_[id].exp_to_next;
}

unsigned int Savemap::GetCharacterLimitLevel(const unsigned int id){
    if (id >= MAX_CHARACTERS) return 0;
    else return characters_[id].limit_level;
}

unsigned int Savemap::GetCharacterLimitBar(const unsigned int id){
    if (id >= MAX_CHARACTERS) return 0;
    else return characters_[id].limit_bar;
}

unsigned int Savemap::GetCharacterWeaponId(const unsigned int id){
    if (id >= MAX_CHARACTERS) return 0;
    else return characters_[id].weapon.id;
}

unsigned int Savemap::GetCharacterArmorId(const unsigned int id){
    if (id >= MAX_CHARACTERS) return 0;
    else return characters_[id].armor.id;
}

int Savemap::GetCharacterAccessoryId(const unsigned int id){
    if (id >= MAX_CHARACTERS) return -1;
    else return characters_[id].accessory;
}

unsigned int Savemap::GetCharacterStatBase(const unsigned int id, const unsigned int stat){
    if (id >= MAX_CHARACTERS) return 0;
    switch (stat){
        case STAT::STR: return characters_[id].str.base; break;
        case STAT::VIT: return characters_[id].vit.base; break;
        case STAT::MAG: return characters_[id].mag.base; break;
        case STAT::SPR: return characters_[id].spr.base; break;
        case STAT::DEX: return characters_[id].dex.base; break;
        case STAT::LCK: return characters_[id].lck.base; break;
        case STAT::HP: return characters_[id].hp.base; break;
        case STAT::MP: return characters_[id].mp.base; break;
        default: return 0;
    }
}

unsigned int Savemap::GetCharacterStatExtra(const unsigned int id, const unsigned int stat){
    if (id >= MAX_CHARACTERS) return 0;
    switch (stat){
        case STAT::STR: return characters_[id].str.extra; break;
        case STAT::VIT: return characters_[id].vit.extra; break;
        case STAT::MAG: return characters_[id].mag.extra; break;
        case STAT::SPR: return characters_[id].spr.extra; break;
        case STAT::DEX: return characters_[id].dex.extra; break;
        case STAT::LCK: return characters_[id].lck.extra; break;
        case STAT::HP: return characters_[id].hp.extra; break;
        case STAT::MP: return characters_[id].mp.extra; break;
        default: return 0;
    }
}

unsigned int Savemap::GetCharacterLimitUses(const unsigned int id, const unsigned int level){
    if (id >= MAX_CHARACTERS) return 0;
    else if (level >= MAX_LIMIT_LEVELS) return 0;
    else return characters_[id].limit_uses[level];
}

bool Savemap::IsCharacterLimitLearned(
  const unsigned int id, const unsigned int level, const unsigned int tech
){
    if (id >= MAX_CHARACTERS) return false;
    else if (level >= MAX_LIMIT_LEVELS) return false;
    else if (tech >= MAX_LIMIT_TECHNIQUES) return false;
    else return characters_[id].limits_learned[level][tech];
}


int Savemap::GetCharacterMateriaId(
  const unsigned int id, const bool weapon, const unsigned int pos
){
    if (id >= MAX_CHARACTERS) return -1;
    else if (pos >= MAX_EQUIP_SLOTS) return -1;
    if (weapon) return characters_[id].weapon.materia[pos].id;
    else return characters_[id].armor.materia[pos].id;
}

unsigned int Savemap::GetCharacterMateriaAp(
  const unsigned int id, const bool weapon, const unsigned int pos
){
    if (id >= MAX_CHARACTERS) return 0;
    else if (pos >= MAX_EQUIP_SLOTS) return 0;
    if (weapon){
        if (characters_[id].weapon.materia[pos].id == -1) return 0;
        else if (characters_[id].weapon.materia[pos].enemy_skill) return 0;
        else return characters_[id].weapon.materia[pos].ap;
    }
    else{
        if (characters_[id].armor.materia[pos].id == -1) return 0;
        else if (characters_[id].armor.materia[pos].enemy_skill) return 0;
        else return characters_[id].armor.materia[pos].ap;
    }
}

bool Savemap::IsCharacterMateriaESkill(
  const unsigned int id, const bool weapon, const unsigned int pos
){
    if (id >= MAX_CHARACTERS) return false;
    else if (pos >= MAX_EQUIP_SLOTS) return false;
    if (weapon){
        if (characters_[id].weapon.materia[pos].id == -1) return false;
        else return characters_[id].weapon.materia[pos].enemy_skill;
    }
    else{
        if (characters_[id].armor.materia[pos].id == -1) return false;
        else return characters_[id].armor.materia[pos].enemy_skill;
    }
}

bool Savemap::IsCharacterMateriaESkillLearned(
  const unsigned int id, const bool weapon, const unsigned int pos, const unsigned int skill
){
    if (id >= MAX_CHARACTERS) return false;
    else if (pos >= MAX_EQUIP_SLOTS) return false;
    else if (skill >= MAX_ENEMY_SKILLS) return false;
    if (weapon){
        if (characters_[id].weapon.materia[pos].id == -1) return false;
        else if (characters_[id].weapon.materia[pos].enemy_skill == false) return false;
        else return characters_[id].weapon.materia[pos].enemy_skill_learned[skill];
    }
    else{
        if (characters_[id].armor.materia[pos].id == -1) return false;
        else if (characters_[id].armor.materia[pos].enemy_skill == false) return false;
        else return characters_[id].armor.materia[pos].enemy_skill_learned[skill];
    }
}

int Savemap::GetData(const unsigned int bank, const unsigned int address){
    if (bank >= BANK_COUNT) return 0;
    else if (address >= BANK_ADDRESS_COUNT) return 0;
    else return data_[bank][address];
}
