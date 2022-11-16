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
#include <tinyxml.h>
#include "core/Savemap.h"
#include "core/Logger.h"
#include "core/XmlFile.h"
#include "common/TypeDefine.h"

Savemap::Savemap():
  empty_(true), control_(""), money_(0), seconds_(0), countdown_(0), slot_(-1)
{
    for (int i = 0; i < 4; i ++){
        window_colours_[i][0] = 0;
        window_colours_[i][1] = 0;
        window_colours_[i][2] = 0;
    }
    party_[0] = -1;
    party_[1] = -1;
    party_[2] = -1;
    location_.x = 0;
    location_.y = 0;
    location_.z = -1;
    location_.triangle = 0;
    location_.angle = 0;
    location_.field = "";
    location_.name = "";
    for (int i = 0; i < MAX_ITEM_SLOTS; i ++){
        items_[i].id = -1;
        items_[i].quantity = 0;
    }
    for (int i = 0; i < MAX_MATERIA_SLOTS; i ++){
        materia_[i].id = -1;
        materia_[i].ap = 0;
        materia_[i].enemy_skill = false;
        for (int s = 0; s < MAX_ENEMY_SKILLS; s ++) materia_[i].enemy_skill_learned[s] = false;
    }
    for (int i = 0; i < MAX_STASH_SLOTS; i ++){
        materia_stash_[i].id = -1;
        materia_stash_[i].ap = 0;
        materia_stash_[i].enemy_skill = false;
        for (int s = 0; s < MAX_ENEMY_SKILLS; s ++)
            materia_stash_[i].enemy_skill_learned[s] = false;
    }
    for (int i = 0; i < MAX_KEY_ITEM_SLOTS; i ++){
        key_items_[i] = false;
    }
    for (int i = 0; i < MAX_CHARACTERS; i ++){
        characters_[i].id = i;
        characters_[i].char_id = -1;
        characters_[i].enabled = false;
        characters_[i].locked = false;
        characters_[i].name = "";
        characters_[i].level = 1;
        characters_[i].kills = 0;
        characters_[i].back_row = false;
        characters_[i].exp = 0;
        characters_[i].exp_to_next = 1;
        characters_[i].str.base = 0;
        characters_[i].str.extra = 0;
        characters_[i].vit.base = 0;
        characters_[i].vit.extra = 0;
        characters_[i].mag.base = 0;
        characters_[i].mag.extra = 0;
        characters_[i].spr.base = 0;
        characters_[i].spr.extra = 0;
        characters_[i].dex.base = 0;
        characters_[i].dex.extra = 0;
        characters_[i].lck.base = 0;
        characters_[i].lck.extra = 0;
        characters_[i].hp.base = 0;
        characters_[i].hp.extra = 0;
        characters_[i].mp.base = 0;
        characters_[i].mp.extra = 0;
        characters_[i].limit_level = 1;
        characters_[i].limit_bar = 0;
        for (int l = 0; l < MAX_LIMIT_LEVELS; l ++){
            for (int t = 0; t < MAX_LIMIT_TECHNIQUES; t ++)
                characters_[i].limits_learned[l][t] = false;
            characters_[i].limit_uses[l] = 0;
        }
        characters_[i].weapon.id = 0;
        characters_[i].armor.id = 0;
        characters_[i].accessory = -1;
        for (int s = 0; s < MAX_EQUIP_SLOTS; s ++){
            characters_[i].weapon.materia[s].id = -1;
            characters_[i].armor.materia[s].id = -1;
            characters_[i].weapon.materia[s].ap = 0;
            characters_[i].armor.materia[s].ap = 0;
            characters_[i].weapon.materia[s].enemy_skill = false;
            characters_[i].armor.materia[s].enemy_skill = false;
            for (int e = 0; e < MAX_ENEMY_SKILLS; e ++){
                characters_[i].weapon.materia[s].enemy_skill_learned[e] = false;
                characters_[i].armor.materia[s].enemy_skill_learned[e] = false;
            }
        }
    }
    for (int b = 0; b < BANK_COUNT; b ++){
        for (int a = 0; a < BANK_ADDRESS_COUNT; a ++){
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

void Savemap::SetData(const unsigned int bank, const unsigned int address, const int value){
    if (bank < BANK_COUNT && address < BANK_ADDRESS_COUNT) data_[bank][address] = value;
}

void Savemap::SetControlKey(const std::string control){
    control_ = control;
    empty_ = false;
}

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
  const unsigned int x, const unsigned int y, const int z,
  const unsigned int triangle, const int angle, std::string field, std::string name
){
    if (z >= -1 && angle >= -360 && angle <= 360){
        location_.x = x;
        location_.y = y;
        location_.z = z;
        location_.triangle = triangle;
        location_.angle = angle;
        location_.field = field;
        location_.name = name;
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
            characters_[id].weapon.materia[slot].enemy_skill = false;
            for (int i = 0; i < MAX_ENEMY_SKILLS; i ++)
                characters_[id].weapon.materia[slot].enemy_skill_learned[i] = false;
        }
        else{
            characters_[id].armor.materia[slot].id = id;
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

unsigned int Savemap::GetLocationX(){return location_.x;}

unsigned int Savemap::GetLocationY(){return location_.y;}

int Savemap::GetLocationZ(){return location_.z;}

unsigned int Savemap::GetLocationTriangle(){return location_.triangle;}

int Savemap::GetLocationAngle(){return location_.angle;}

std::string Savemap::GetLocationField(){return location_.field;}

std::string Savemap::GetLocationName(){return location_.name;}

int Savemap::GetSetting(const unsigned int key){
    // TODO: Implement.
    return 0;
}

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

void Savemap::Write(int slot, std::string file_name){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("Savemap"));
    // Basic data
    container->SetAttribute("slot", slot);
    container->SetAttribute("key", control_);

    // Window colours
    std::unique_ptr<TiXmlElement> colours(new TiXmlElement("WindowColours"));
    std::unique_ptr<TiXmlElement> colour_t_l(new TiXmlElement("Corner"));
    colour_t_l->SetAttribute("position", "top_left");
    colour_t_l->SetAttribute("red", window_colours_[0][0]);
    colour_t_l->SetAttribute("green", window_colours_[0][1]);
    colour_t_l->SetAttribute("blue", window_colours_[0][2]);
    colours->LinkEndChild(colour_t_l.release());
    std::unique_ptr<TiXmlElement> colour_t_r(new TiXmlElement("Corner"));
    colour_t_r->SetAttribute("position", "top_right");
    colour_t_r->SetAttribute("red", window_colours_[1][0]);
    colour_t_r->SetAttribute("green", window_colours_[1][1]);
    colour_t_r->SetAttribute("blue", window_colours_[1][2]);
    colours->LinkEndChild(colour_t_r.release());
    std::unique_ptr<TiXmlElement> colour_b_r(new TiXmlElement("Corner"));
    colour_b_r->SetAttribute("position", "bottom_right");
    colour_b_r->SetAttribute("red", window_colours_[2][0]);
    colour_b_r->SetAttribute("green", window_colours_[2][1]);
    colour_b_r->SetAttribute("blue", window_colours_[2][2]);
    colours->LinkEndChild(colour_b_r.release());
    std::unique_ptr<TiXmlElement> colour_b_l(new TiXmlElement("Corner"));
    colour_b_l->SetAttribute("position", "bottom_left");
    colour_b_l->SetAttribute("red", window_colours_[3][0]);
    colour_b_l->SetAttribute("green", window_colours_[3][1]);
    colour_b_l->SetAttribute("blue", window_colours_[3][2]);
    colours->LinkEndChild(colour_b_l.release());
    container->LinkEndChild(colours.release());

    // Single data (when applicable)
    if (money_ > 0){
        std::unique_ptr<TiXmlElement> money(new TiXmlElement("Money"));
        money->SetAttribute("quantity", money_);
        container->LinkEndChild(money.release());
    }
    if (seconds_ > 0){
        std::unique_ptr<TiXmlElement> time(new TiXmlElement("GameTime"));
        time->SetAttribute("seconds", seconds_);
        container->LinkEndChild(time.release());
    }
    if (seconds_ > 0){
        std::unique_ptr<TiXmlElement> countdown(new TiXmlElement("Countdown"));
        countdown->SetAttribute("seconds", countdown_);
        container->LinkEndChild(countdown.release());
    }

    // Location.
    std::unique_ptr<TiXmlElement> location(new TiXmlElement("Location"));
    location->SetAttribute("x", location_.x);
    location->SetAttribute("y", location_.y);
    if (location_.z >= 0) location->SetAttribute("z", location_.z);
    location->SetAttribute("triangle", location_.triangle);
    location->SetAttribute("orientation", location_.angle);
    location->SetAttribute("field", location_.field);
    location->SetAttribute("name", location_.name);
    container->LinkEndChild(location.release());

    // Party.
    std::unique_ptr<TiXmlElement> party(new TiXmlElement("Party"));
    for (int p = 0; p < 3; p ++){
        if (party_[p] >= 0 && party_[p] < MAX_CHARACTERS){
            std::unique_ptr<TiXmlElement> member(new TiXmlElement("Member"));
            member->SetAttribute("position", p);
            member->SetAttribute("character_id", party_[p]);
            party->LinkEndChild(member.release());
        }
    }
    container->LinkEndChild(party.release());

    // Items.
    std::unique_ptr<TiXmlElement> items(new TiXmlElement("Items"));
    for (int i = 0; i < MAX_ITEM_SLOTS; i ++){
        if (items_[i].quantity > 0){
            std::unique_ptr<TiXmlElement> item(new TiXmlElement("Item"));
            item->SetAttribute("slot", i);
            item->SetAttribute("id", items_[i].id);
            item->SetAttribute("quantity", items_[i].quantity);
            items->LinkEndChild(item.release());
        }
    }
    container->LinkEndChild(items.release());

    // Key items.
    std::unique_ptr<TiXmlElement> key_items(new TiXmlElement("KeyItems"));
    for (int i = 0; i < MAX_KEY_ITEM_SLOTS; i ++){
        if (key_items_[i]){
            std::unique_ptr<TiXmlElement> key_item(new TiXmlElement("Item"));
            key_item->SetAttribute("id", key_items_[i]);
            key_item->SetAttribute("owned", true);
            key_items->LinkEndChild(key_item.release());
        }
    }
    container->LinkEndChild(key_items.release());

    // Materia.
    std::unique_ptr<TiXmlElement> materias(new TiXmlElement("Materia"));
    for (int i = 0; i < MAX_MATERIA_SLOTS; i ++){
        if (materia_[i].id >= 0){
            std::unique_ptr<TiXmlElement> materia(new TiXmlElement("Materia"));
            materia->SetAttribute("slot", i);
            materia->SetAttribute("id", materia_[i].id);
            materia->SetAttribute("ap", materia_[i].ap);
            if (materia_[i].enemy_skill){
                materia->SetAttribute("enemy_skill", true);
                std::unique_ptr<TiXmlElement> enemy_skills(new TiXmlElement("EnemySkill"));
                for (int s = 0; s < MAX_ENEMY_SKILLS; s ++){
                    if (materia_[i].enemy_skill_learned[s]){
                        std::unique_ptr<TiXmlElement> enemy_skill(new TiXmlElement("Skill"));
                        enemy_skill->SetAttribute("id", s);
                        enemy_skill->SetAttribute("learned", true);
                        enemy_skills->LinkEndChild(enemy_skill.release());
                    }
                    materia->LinkEndChild(enemy_skills.release());
                }
            }
            materias->LinkEndChild(materia.release());
        }
    }
    container->LinkEndChild(materias.release());

    // Materia stash.
    std::unique_ptr<TiXmlElement> stash(new TiXmlElement("MateriaStash"));
    for (int i = 0; i < MAX_STASH_SLOTS; i ++){
        if (materia_stash_[i].id >= 0){
            std::unique_ptr<TiXmlElement> materia(new TiXmlElement("Materia"));
            materia->SetAttribute("slot", i);
            materia->SetAttribute("id", materia_[i].id);
            materia->SetAttribute("ap", materia_[i].ap);
            if (materia_stash_[i].enemy_skill){
                materia->SetAttribute("enemy_skill", true);
                std::unique_ptr<TiXmlElement> enemy_skills(new TiXmlElement("EnemySkill"));
                for (int s = 0; s < MAX_ENEMY_SKILLS; s ++){
                    if (materia_stash_[i].enemy_skill_learned[s]){
                        std::unique_ptr<TiXmlElement> enemy_skill(new TiXmlElement("Skill"));
                        enemy_skill->SetAttribute("id", s);
                        enemy_skill->SetAttribute("learned", true);
                        enemy_skills->LinkEndChild(enemy_skill.release());
                    }
                    materia->LinkEndChild(enemy_skills.release());
                }
            }
            stash->LinkEndChild(materia.release());
        }
    }
    container->LinkEndChild(stash.release());

    // TODO: Settings

    // Characters
    std::unique_ptr<TiXmlElement> characters(new TiXmlElement("Characters"));
    for (int c = 0; c < MAX_CHARACTERS; c ++){
        if (characters_[c].char_id == -1) continue;

        std::unique_ptr<TiXmlElement> character(new TiXmlElement("Character"));

        // Basic information.
        character->SetAttribute("id", characters_[c].id);
        character->SetAttribute("char_id", characters_[c].char_id);
        character->SetAttribute("name", characters_[c].name);
        character->SetAttribute("level", characters_[c].level);
        character->SetAttribute("enabled", characters_[c].enabled);
        character->SetAttribute("locked", characters_[c].locked);
        character->SetAttribute("kills", characters_[c].kills);
        character->SetAttribute("back_row", characters_[c].back_row);
        character->SetAttribute("exp", characters_[c].exp);
        character->SetAttribute("exp_to_next", characters_[c].exp_to_next);

        // Stats.
        std::unique_ptr<TiXmlElement> character_stats(new TiXmlElement("Stats"));
        std::unique_ptr<TiXmlElement> character_stats_str(new TiXmlElement("Stat"));
        character_stats_str->SetAttribute("id", "str");
        character_stats_str->SetAttribute("base", characters_[c].str.base);
        character_stats_str->SetAttribute("bonus", characters_[c].str.extra);
        character_stats->LinkEndChild(character_stats_str.release());
        std::unique_ptr<TiXmlElement> character_stats_vit(new TiXmlElement("Stat"));
        character_stats_vit->SetAttribute("id", "vit");
        character_stats_vit->SetAttribute("base", characters_[c].vit.base);
        character_stats_vit->SetAttribute("bonus", characters_[c].vit.extra);
        character_stats->LinkEndChild(character_stats_vit.release());
        std::unique_ptr<TiXmlElement> character_stats_mag(new TiXmlElement("Stat"));
        character_stats_mag->SetAttribute("id", "mag");
        character_stats_mag->SetAttribute("base", characters_[c].mag.base);
        character_stats_mag->SetAttribute("bonus", characters_[c].mag.extra);
        character_stats->LinkEndChild(character_stats_mag.release());
        std::unique_ptr<TiXmlElement> character_stats_spr(new TiXmlElement("Stat"));
        character_stats_spr->SetAttribute("id", "spr");
        character_stats_spr->SetAttribute("base", characters_[c].spr.base);
        character_stats_spr->SetAttribute("bonus", characters_[c].spr.extra);
        character_stats->LinkEndChild(character_stats_spr.release());
        std::unique_ptr<TiXmlElement> character_stats_dex(new TiXmlElement("Stat"));
        character_stats_dex->SetAttribute("id", "dex");
        character_stats_dex->SetAttribute("base", characters_[c].dex.base);
        character_stats_dex->SetAttribute("bonus", characters_[c].dex.extra);
        character_stats->LinkEndChild(character_stats_dex.release());
        std::unique_ptr<TiXmlElement> character_stats_lck(new TiXmlElement("Stat"));
        character_stats_lck->SetAttribute("id", "lck");
        character_stats_lck->SetAttribute("base", characters_[c].lck.base);
        character_stats_lck->SetAttribute("bonus", characters_[c].lck.extra);
        character_stats->LinkEndChild(character_stats_lck.release());
        std::unique_ptr<TiXmlElement> character_stats_hp(new TiXmlElement("Stat"));
        character_stats_hp->SetAttribute("id", "hp");
        character_stats_hp->SetAttribute("base", characters_[c].hp.base);
        character_stats_hp->SetAttribute("current", characters_[c].hp.extra);
        character_stats->LinkEndChild(character_stats_hp.release());
        std::unique_ptr<TiXmlElement> character_stats_mp(new TiXmlElement("Stat"));
        character_stats_mp->SetAttribute("id", "mp");
        character_stats_mp->SetAttribute("base", characters_[c].mp.base);
        character_stats_mp->SetAttribute("current", characters_[c].mp.extra);
        character_stats->LinkEndChild(character_stats_mp.release());
        character->LinkEndChild(character_stats.release());

        // Limit information.
        std::unique_ptr<TiXmlElement> character_limits(new TiXmlElement("Limits"));
        character_limits->SetAttribute("current", characters_[c].limit_level);
        character_limits->SetAttribute("bar", characters_[c].limit_bar);
        for (int l = 0; l < MAX_LIMIT_LEVELS; l ++){
            std::unique_ptr<TiXmlElement> character_limits_level(new TiXmlElement("Level"));
            character_limits_level->SetAttribute("level", l);
            character_limits_level->SetAttribute("uses", characters_[c].limit_uses[l]);
            for (int t = 0; t < MAX_LIMIT_TECHNIQUES; t ++){
                if (characters_[c].limits_learned[l][t]){
                    std::unique_ptr<TiXmlElement> character_limits_level_technique(
                      new TiXmlElement("Technique")
                    );
                    character_limits_level_technique->SetAttribute("id", t);
                    character_limits_level_technique->SetAttribute("learned", true);
                    character_limits_level->LinkEndChild(
                      character_limits_level_technique.release()
                    );
                }
            }
            character_limits->LinkEndChild(character_limits_level.release());
        }
        character->LinkEndChild(character_limits.release());

        // Weapon.
        std::unique_ptr<TiXmlElement> character_weapon(new TiXmlElement("Weapon"));
        character_weapon->SetAttribute("id", characters_[c].weapon.id);
        std::unique_ptr<TiXmlElement> character_weapon_materia(new TiXmlElement("Materia"));
        for (int m = 0; m < MAX_EQUIP_SLOTS; m ++){
            if (characters_[c].weapon.materia[m].id >= 0){
                std::unique_ptr<TiXmlElement> materia(new TiXmlElement("Materia"));
                materia->SetAttribute("slot", m);
                materia->SetAttribute("id", characters_[c].weapon.materia[m].id);
                materia->SetAttribute("ap", characters_[c].weapon.materia[m].ap);
                if (characters_[c].weapon.materia[m].enemy_skill){
                    materia->SetAttribute("enemy_skill", true);
                    std::unique_ptr<TiXmlElement> character_weapon_enemy_skills(
                      new TiXmlElement("EnemySkill")
                    );
                    for (int s = 0; s < MAX_ENEMY_SKILLS; s ++){
                        if (characters_[c].weapon.materia[m].enemy_skill_learned[s]){
                            std::unique_ptr<TiXmlElement> enemy_skill(new TiXmlElement("Skill"));
                            enemy_skill->SetAttribute("id", s);
                            enemy_skill->SetAttribute("learned", true);
                            character_weapon_enemy_skills->LinkEndChild(enemy_skill.release());
                        }
                        materia->LinkEndChild(character_weapon_enemy_skills.release());
                    }
                }
                character_weapon_materia->LinkEndChild(materia.release());
            }
        }
        character_weapon->LinkEndChild(character_weapon_materia.release());
        character->LinkEndChild(character_weapon.release());

        // Armor.
        std::unique_ptr<TiXmlElement> character_armor(new TiXmlElement("Armor"));
        character_armor->SetAttribute("id", characters_[c].armor.id);
        std::unique_ptr<TiXmlElement> character_armor_materia(new TiXmlElement("Materia"));
        for (int m = 0; m < MAX_EQUIP_SLOTS; m ++){
            if (characters_[c].armor.materia[m].id >= 0){
                std::unique_ptr<TiXmlElement> materia(new TiXmlElement("Materia"));
                materia->SetAttribute("slot", m);
                materia->SetAttribute("id", characters_[c].armor.materia[m].id);
                materia->SetAttribute("ap", characters_[c].armor.materia[m].ap);
                if (characters_[c].armor.materia[m].enemy_skill){
                    materia->SetAttribute("enemy_skill", true);
                    std::unique_ptr<TiXmlElement> character_armor_enemy_skills(
                      new TiXmlElement("EnemySkill")
                    );
                    for (int s = 0; s < MAX_ENEMY_SKILLS; s ++){
                        if (characters_[c].armor.materia[m].enemy_skill_learned[s]){
                            std::unique_ptr<TiXmlElement> enemy_skill(new TiXmlElement("Skill"));
                            enemy_skill->SetAttribute("id", s);
                            enemy_skill->SetAttribute("learned", true);
                            character_armor_enemy_skills->LinkEndChild(enemy_skill.release());
                        }
                        materia->LinkEndChild(character_armor_enemy_skills.release());
                    }
                }
                character_armor_materia->LinkEndChild(materia.release());
            }
        }
        character_armor->LinkEndChild(character_armor_materia.release());
        character->LinkEndChild(character_armor.release());

        // Accessory.
        if (characters_[c].accessory >= 0){
            std::unique_ptr<TiXmlElement> character_accessory(new TiXmlElement("Accessory"));
            character_accessory->SetAttribute("id", characters_[c].accessory);
            character->LinkEndChild(character_accessory.release());
        }
        characters->LinkEndChild(character.release());

        // TODO: Statuses
    }
    container->LinkEndChild(characters.release());

    // Data banks.
    std::unique_ptr<TiXmlElement> banks(new TiXmlElement("DataBanks"));
    for (int b = 0; b < BANK_COUNT; b ++){
        std::unique_ptr<TiXmlElement> bank(new TiXmlElement("Bank"));
        bank->SetAttribute("id", b);
        for (int a = 0; a < BANK_ADDRESS_COUNT; a ++){
            if (data_[b][a] != 0){
                std::unique_ptr<TiXmlElement> address(new TiXmlElement("Address"));
                address->SetAttribute("address", a);
                address->SetAttribute("value", data_[b][a]);
                bank->LinkEndChild(address.release());
            }
        }
        banks->LinkEndChild(bank.release());
    }
    container->LinkEndChild(banks.release());

    // Save
    xml.LinkEndChild(container.release());
    xml.SaveFile(file_name);
    slot_ = slot;
    return;

}

void Savemap::Read(std::string file_name){
    // TODO: This need a try/catch, it's impossible to validate every value and type in the file.
    TiXmlDocument xml_file(file_name);
    if (xml_file.LoadFile() == false){
        empty_ = true;
        return;
    }

    TiXmlNode* node = xml_file.RootElement();
    if (node == nullptr || node->ValueStr() != "Savemap"){
        LOG_ERROR(
          "Savemap: " + file_name + " is not a valid savemap file! No <Savemap> in root."
        );
        empty_ = true;
        return;
    }

    slot_ = node->ToElement()->QueryIntAttribute("slot", &slot_);
    control_ = node->ToElement()->QueryStringAttribute("slot", &control_);
    // Make sure that control is not just a null character!
    // This would break many things,
    if (control_.size() < 2) control_ = "X";
    empty_ = false;

    node = node->FirstChild();
    while (node != nullptr){
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "WindowColours"){
            TiXmlNode* colour_node = node->FirstChild();
            while (colour_node != nullptr){
                if (
                  colour_node->Type() == TiXmlNode::TINYXML_ELEMENT
                  && colour_node->ValueStr() == "Corner"
                ){
                    std::string position;
                    TiXmlElement* colour_elm = colour_node->ToElement();
                    colour_elm->QueryStringAttribute("position", &position);
                    int colour;
                    if (position == "top_left"){
                        colour = -1;
                        colour_elm->QueryIntAttribute("red", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[0][0] = colour;
                        colour = -1;
                        colour_elm->QueryIntAttribute("green", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[0][1] = colour;
                        colour = -1;
                        colour_elm->QueryIntAttribute("blue", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[0][2] = colour;
                    }
                    else if (position == "top_right"){
                        colour = -1;
                        colour_elm->QueryIntAttribute("red", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[1][0] = colour;
                        colour = -1;
                        colour_elm->QueryIntAttribute("green", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[1][1] = colour;
                        colour = -1;
                        colour_elm->QueryIntAttribute("blue", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[1][2] = colour;
                    }
                    else if (position == "bottom_right"){
                        colour = -1;
                        colour_elm->QueryIntAttribute("red", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[2][0] = colour;
                        colour = -1;
                        colour_elm->QueryIntAttribute("green", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[2][1] = colour;
                        colour = -1;
                        colour_elm->QueryIntAttribute("blue", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[2][2] = colour;
                    }
                    else if (position == "bottol_left"){
                        colour = -1;
                        colour_elm->QueryIntAttribute("red", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[3][0] = colour;
                        colour = -1;
                        colour_elm->QueryIntAttribute("green", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[3][1] = colour;
                        colour = -1;
                        colour_elm->QueryIntAttribute("blue", &colour);
                        if (colour >= 0 && colour < MAX_COLOUR) window_colours_[3][2] = colour;
                    }
                }
                colour_node = colour_node->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "Money"){
            int money = -1;
            node->ToElement()->QueryIntAttribute("quantity", &money);
            if (money >= 0) money_ = money;
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "GameTime"){
            int seconds = -1;
            node->ToElement()->QueryIntAttribute("seconds", &seconds);
            if (seconds >= 0) seconds_ = seconds;
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "Countdown"){
            int seconds = -1;
            node->ToElement()->QueryIntAttribute("seconds", &seconds);
            if (seconds >= 0) countdown_ = seconds;
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "Location"){
            int val = -1;
            node->ToElement()->QueryIntAttribute("x", &val);
            if (val >= 0) location_.x = val;
            val = -1;
            node->ToElement()->QueryIntAttribute("y", &val);
            if (val >= 0) location_.y = val;
            val = -1;
            node->ToElement()->QueryIntAttribute("z", &val);
            if (val >= -1) location_.z = val;
            val = -1;
            node->ToElement()->QueryIntAttribute("triangle", &val);
            if (val >= 0) location_.triangle = val;
            val = -1;
            node->ToElement()->QueryIntAttribute("angle", &val);
            if (val >= -360 && val <=360) location_.angle = val;
            node->ToElement()->QueryStringAttribute("field", &location_.field);
            node->ToElement()->QueryStringAttribute("name", &location_.name);
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "Party"){
            TiXmlNode* party_node = node->FirstChild();
            while (party_node != nullptr){
                if (
                    party_node->Type() == TiXmlNode::TINYXML_ELEMENT
                  && party_node->ValueStr() == "Member"
                ){
                    int position = -1;
                    int member = -1;
                    TiXmlElement* party_elm = party_node->ToElement();
                    party_elm->QueryIntAttribute("position", &position);
                    party_elm->QueryIntAttribute("character_id", &member);
                    if (
                      position >= 0 && position < MAX_PARTY_MEMBERS
                      && member >= 0 && member < MAX_CHARACTERS
                    ){
                        party_[position] = member;
                    }
                }
                party_node = party_node->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "Items"){
            TiXmlNode* item_node = node->FirstChild();
            while (item_node != nullptr){
                if (
                  item_node->Type() == TiXmlNode::TINYXML_ELEMENT
                  && item_node->ValueStr() == "Item"
                ){
                    int slot = -1;
                    int id = -1;
                    int quantity = -1;
                    TiXmlElement* item_elm = item_node->ToElement();
                    item_elm->QueryIntAttribute("slot", &slot);
                    item_elm->QueryIntAttribute("id", &id);
                    item_elm->QueryIntAttribute("quantity", &quantity);
                    if (slot >= 0 && slot < MAX_ITEM_SLOTS && id >= 0 && quantity > 0){
                        items_[slot].id = id;
                        items_[slot].quantity = quantity;
                    }
                }
                item_node = item_node->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "KeyItems"){
            TiXmlNode* item_node = node->FirstChild();
            while (item_node != nullptr){
                if (
                  item_node->Type() == TiXmlNode::TINYXML_ELEMENT
                  && item_node->ValueStr() == "Item"
                ){
                    int id;
                    int owned;
                    TiXmlElement* item_elm = item_node->ToElement();
                    item_elm->QueryIntAttribute("id", &id);
                    item_elm->QueryIntAttribute("owned", &owned);
                    if (id >= 0 && id < MAX_KEY_ITEM_SLOTS && owned == 1) key_items_[id] = true;
                }
                item_node = item_node->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "Materia"){
            TiXmlNode* materia_node = node->FirstChild();
            while (materia_node != nullptr){
                if (
                  materia_node->Type() == TiXmlNode::TINYXML_ELEMENT
                  && materia_node->ValueStr() == "Materia"
                ){
                    int slot = -1;
                    int id = -2;
                    int ap = -1;
                    int eskill = 0;
                    TiXmlElement* materia_elm = materia_node->ToElement();
                    materia_elm->QueryIntAttribute("slot", &slot);
                    materia_elm->QueryIntAttribute("id", &id);
                    materia_elm->QueryIntAttribute("ap", &ap);
                    materia_elm->QueryIntAttribute("enemy_skill", &ap);
                    if (
                      slot >= 0 && slot < MAX_MATERIA_SLOTS && id >= -1
                      && ap >= 0 && (eskill == 0 | eskill == 1)
                    ){
                        materia_[slot].id = id;
                        materia_[slot].ap = ap;
                        materia_[slot].enemy_skill = (eskill == 1);
                        // TODO: For enemy skill, get learned.
                    }
                }
                materia_node = materia_node->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "MateriaStash"){
            TiXmlNode* materia_node = node->FirstChild();
            while (materia_node != nullptr){
                if (
                    materia_node->Type() == TiXmlNode::TINYXML_ELEMENT
                  && materia_node->ValueStr() == "Materia"
                ){
                    int slot = -1;
                    int id = -2;
                    int ap = -1;
                    int eskill = 0;
                    TiXmlElement* materia_elm = materia_node->ToElement();
                    materia_elm->QueryIntAttribute("slot", &slot);
                    materia_elm->QueryIntAttribute("id", &id);
                    materia_elm->QueryIntAttribute("ap", &ap);
                    materia_elm->QueryIntAttribute("enemy_skill", &ap);
                    if (
                      slot >= 0 && slot < MAX_STASH_SLOTS && id >= -1
                      && ap >= 0 && (eskill == 0 | eskill == 1)
                    ){
                        materia_stash_[slot].id = id;
                        materia_stash_[slot].ap = ap;
                        materia_stash_[slot].enemy_skill = (eskill == 1);
                        // TODO: For enemy skill, get learned.
                    }
                }
                materia_node = materia_node->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "Characters"){
            TiXmlNode* character_node = node->FirstChild();
            while (character_node != nullptr){
                if (
                    character_node->Type() == TiXmlNode::TINYXML_ELEMENT
                  && character_node->ValueStr() == "Character"
                ){
                    int id = -1;
                    TiXmlElement* character_elm = character_node->ToElement();
                    character_elm->QueryIntAttribute("id", &id);
                    if (id >= 0 && id < MAX_CHARACTERS){
                        characters_[id].id = id;
                        int val = -2;
                        character_elm->QueryIntAttribute("char_id", &val);
                        if (val >= -1) characters_[id].char_id = val;
                        character_elm->QueryStringAttribute("name", &characters_[id].name);
                        val = -1;
                        character_elm->QueryIntAttribute("enabled", &val);
                        if (val == 1) characters_[id].enabled = true;
                        else characters_[id].enabled = false;
                        val = -1;
                        character_elm->QueryIntAttribute("locked", &val);
                        characters_[id].locked = (val == 1);
                        val = -1;
                        character_elm->QueryIntAttribute("level", &val);
                        if (val > 0) characters_[id].level = val;
                        val = -1;
                        character_elm->QueryIntAttribute("kills", &val);
                        if (val >= 0) characters_[id].kills = val;
                        val = -1;
                        character_elm->QueryIntAttribute("back_row", &val);
                        characters_[id].back_row = (val == 1);
                        val = -1;
                        character_elm->QueryIntAttribute("exp", &val);
                        if (val >= 0) characters_[id].exp = val;
                        val = -1;
                        character_elm->QueryIntAttribute("exp_to_next", &val);
                        if (val >= 0) characters_[id].exp_to_next = val;
                        TiXmlNode* char_node = character_node->FirstChild();
                        while (char_node != nullptr){
                            if (
                              char_node->Type() == TiXmlNode::TINYXML_ELEMENT
                              && char_node->ValueStr() == "Stats"
                            ){
                                TiXmlNode* stat_node = char_node->FirstChild();
                                while (stat_node != nullptr){
                                    if (
                                      char_node->Type() == TiXmlNode::TINYXML_ELEMENT
                                      && char_node->ValueStr() == "Stat"
                                    ){
                                        std::string stat = "";
                                        int base = -1;
                                        int extra = -1;
                                        TiXmlElement* stat_elm = stat_node->ToElement();
                                        stat_elm->QueryStringAttribute("id", &stat);
                                        if (stat == "str"){
                                            stat_elm->QueryIntAttribute("base", &base);
                                            stat_elm->QueryIntAttribute("bonus", &extra);
                                            if (base >= 0 && extra >= 0){
                                                characters_[id].str.base = base;
                                                characters_[id].str.extra = extra;
                                            }
                                        }

                                        else if (stat == "vit"){
                                            stat_elm->QueryIntAttribute("base", &base);
                                            stat_elm->QueryIntAttribute("bonus", &extra);
                                            if (base >= 0 && extra >= 0){
                                                characters_[id].vit.base = base;
                                                characters_[id].vit.extra = extra;
                                            }
                                        }
                                        else if (stat == "mag"){
                                            stat_elm->QueryIntAttribute("base", &base);
                                            stat_elm->QueryIntAttribute("bonus", &extra);
                                            if (base >= 0 && extra >= 0){
                                                characters_[id].mag.base = base;
                                                characters_[id].mag.extra = extra;
                                            }
                                        }
                                        else if (stat == "spr"){
                                            stat_elm->QueryIntAttribute("base", &base);
                                            stat_elm->QueryIntAttribute("bonus", &extra);
                                            if (base >= 0 && extra >= 0){
                                                characters_[id].spr.base = base;
                                                characters_[id].spr.extra = extra;
                                            }
                                        }
                                        else if (stat == "dex"){
                                            stat_elm->QueryIntAttribute("base", &base);
                                            stat_elm->QueryIntAttribute("bonus", &extra);
                                            if (base >= 0 && extra >= 0){
                                                characters_[id].dex.base = base;
                                                characters_[id].dex.extra = extra;
                                            }
                                        }
                                        else if (stat == "lck"){
                                            stat_elm->QueryIntAttribute("base", &base);
                                            stat_elm->QueryIntAttribute("bonus", &extra);
                                            if (base >= 0 && extra >= 0){
                                                characters_[id].lck.base = base;
                                                characters_[id].lck.extra = extra;
                                            }
                                        }
                                        else if (stat == "hp"){
                                            stat_elm->QueryIntAttribute("base", &base);
                                            stat_elm->QueryIntAttribute("current", &extra);
                                            if (base > 0 && extra >= 0){
                                                characters_[id].hp.base = base;
                                                characters_[id].hp.extra = extra;
                                            }
                                        }
                                        else if (stat == "mp"){
                                            stat_elm->QueryIntAttribute("base", &base);
                                            stat_elm->QueryIntAttribute("current", &extra);
                                            if (base > 0 && extra >= 0){
                                                characters_[id].mp.base = base;
                                                characters_[id].mp.extra = extra;
                                            }
                                        }
                                    }
                                    stat_node = stat_node->NextSibling();
                                }
                            }
                            else if (
                              char_node->Type() == TiXmlNode::TINYXML_ELEMENT
                              && char_node->ValueStr() == "Limits"
                            ){
                                int val = -1;
                                char_node->ToElement()->QueryIntAttribute("current", &val);
                                if (val >= 0 && val < MAX_LIMIT_LEVELS)
                                    characters_[id].limit_level = val;
                                val = -1;
                                char_node->ToElement()->QueryIntAttribute("bar", &val);
                                if (val >= 0 && val <= MAX_LIMIT_BAR)
                                    characters_[id].limit_bar = val;
                                TiXmlNode* limits_node = char_node->FirstChild();
                                while (limits_node != nullptr){
                                    if (
                                        limits_node->Type() == TiXmlNode::TINYXML_ELEMENT
                                      && limits_node->ValueStr() == "Level"
                                    ){
                                        int level = -1;
                                        int uses = -1;
                                        limits_node->ToElement()->QueryIntAttribute(
                                          "level", &level
                                        );
                                        limits_node->ToElement()->QueryIntAttribute(
                                          "uses", &uses
                                        );
                                        if (level >= 0 && level < MAX_LIMIT_LEVELS){
                                            if (uses >= 0) characters_[id].limit_uses[level] = uses;
                                            TiXmlNode* tech_node = limits_node->FirstChild();
                                            while (tech_node != nullptr){
                                                if (
                                                  tech_node->Type() == TiXmlNode::TINYXML_ELEMENT
                                                  && tech_node->ValueStr() == "Technique"
                                                ){
                                                    int tech = -1;
                                                    int learned = -1;
                                                    tech_node->ToElement()->QueryIntAttribute(
                                                      "id", &tech
                                                    );
                                                    tech_node->ToElement()->QueryIntAttribute(
                                                      "learned", &learned
                                                    );
                                                    if (tech >= 0 && tech < MAX_LIMIT_TECHNIQUES){
                                                        characters_[id].limits_learned[level][tech]
                                                          = (learned == 1);
                                                    }
                                                }
                                                tech_node = tech_node->NextSibling();
                                            }
                                        }
                                    }
                                    limits_node = limits_node->NextSibling();
                                }
                            }
                            else if (
                              char_node->Type() == TiXmlNode::TINYXML_ELEMENT
                              && char_node->ValueStr() == "Weapon"
                            ){
                                int wpn = -1;
                                char_node->ToElement()->QueryIntAttribute("id", &wpn);
                                if (wpn >= 0) characters_[id].weapon.id = wpn;
                                TiXmlNode* mats_node = char_node->FirstChild();
                                if (
                                  mats_node->Type() == TiXmlNode::TINYXML_ELEMENT
                                  && mats_node->ValueStr() == "Materia"
                                ){
                                    TiXmlNode* mat_node = mats_node->FirstChild();
                                    while (mat_node != nullptr){
                                        if (
                                          mat_node->Type() == TiXmlNode::TINYXML_ELEMENT
                                          && mat_node->ValueStr() == "Materia"
                                        ){
                                            int slot = -1;
                                            int id = -2;
                                            int ap = -1;
                                            int eskill = 0;
                                            TiXmlElement* mat_elm = mat_node->ToElement();
                                            mat_elm->QueryIntAttribute("slot", &slot);
                                            mat_elm->QueryIntAttribute("id", &id);
                                            mat_elm->QueryIntAttribute("ap", &ap);
                                            mat_elm->QueryIntAttribute("enemy_skill", &ap);
                                            if (
                                              slot >= 0 && slot < MAX_EQUIP_SLOTS && id >= -1
                                              && ap >= 0 && (eskill == 0 | eskill == 1)
                                            ){
                                                characters_[id].weapon.materia[slot].id = id;
                                                characters_[id].weapon.materia[slot].ap = ap;
                                                characters_[id].weapon.materia[slot].enemy_skill
                                                  = (eskill == 1);
                                                // TODO: For enemy skill, get learned.
                                            }
                                        }
                                        mat_node = mat_node->NextSibling();
                                    }
                                }
                            }
                            else if (
                              char_node->Type() == TiXmlNode::TINYXML_ELEMENT
                              && char_node->ValueStr() == "Armor"
                            ){
                                int arm = -1;
                                char_node->ToElement()->QueryIntAttribute("id", &arm);
                                if (arm >= 0) characters_[id].armor.id = arm;
                                TiXmlNode* mats_node = char_node->FirstChild();
                                if (
                                  mats_node->Type() == TiXmlNode::TINYXML_ELEMENT
                                  && mats_node->ValueStr() == "Materia"
                                ){
                                    TiXmlNode* mat_node = mats_node->FirstChild();
                                    while (mat_node != nullptr){
                                        if (
                                          mat_node->Type() == TiXmlNode::TINYXML_ELEMENT
                                          && mat_node->ValueStr() == "Materia"
                                        ){
                                            int slot = -1;
                                            int id = -2;
                                            int ap = -1;
                                            int eskill = 0;
                                            TiXmlElement* mat_elm = mat_node->ToElement();
                                            mat_elm->QueryIntAttribute("slot", &slot);
                                            mat_elm->QueryIntAttribute("id", &id);
                                            mat_elm->QueryIntAttribute("ap", &ap);
                                            mat_elm->QueryIntAttribute("enemy_skill", &ap);
                                            if (
                                              slot >= 0 && slot < MAX_EQUIP_SLOTS && id >= -1
                                              && ap >= 0 && (eskill == 0 | eskill == 1)
                                            ){
                                                characters_[id].armor.materia[slot].id = id;
                                                characters_[id].armor.materia[slot].ap = ap;
                                                characters_[id].armor.materia[slot].enemy_skill
                                                  = (eskill == 1);
                                                // TODO: For enemy skill, get learned.
                                            }
                                        }
                                        mat_node = mat_node->NextSibling();
                                    }
                                }
                            }
                            else if (
                              char_node->Type() == TiXmlNode::TINYXML_ELEMENT
                              && char_node->ValueStr() == "Accessory"
                            ){
                                int acc = -2;
                                char_node->ToElement()->QueryIntAttribute("id", &acc);
                                if (acc >= -1) characters_[id].accessory = acc;
                            }
                            else if (
                              char_node->Type() == TiXmlNode::TINYXML_ELEMENT
                              && char_node->ValueStr() == "Status"
                            ){
                                  // TODO
                            }
                            char_node = char_node->NextSibling();
                        }
                    }
                }
                character_node = character_node->NextSibling();
            }
        }
        node = node->NextSibling();
    }
}
