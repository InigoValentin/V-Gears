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
#include "engine/save/Savemap.h"
#include "common/Logger.h"
#include "common/TypeDefine.h"

constexpr unsigned int SavemapData::MAX_CHARACTERS;
constexpr unsigned int SavemapData::MAX_ITEM_SLOTS;
constexpr unsigned int SavemapData::MAX_KEY_ITEM_SLOTS;
constexpr unsigned int SavemapData::MAX_MATERIA_SLOTS;
constexpr unsigned int SavemapData::MAX_STASH_SLOTS;
constexpr unsigned int SavemapData::MAX_EQUIP_SLOTS;
constexpr unsigned int SavemapData::MAX_ENEMY_SKILLS;
constexpr unsigned int SavemapData::MAX_LIMIT_LEVELS;
constexpr unsigned int SavemapData::MAX_LIMIT_TECHNIQUES;
constexpr unsigned int SavemapData::MAX_LIMIT_BAR;
constexpr unsigned int SavemapData::MAX_PARTY_MEMBERS;
constexpr unsigned int SavemapData::BANK_COUNT;
constexpr unsigned int SavemapData::BANK_ADDRESS_COUNT;
constexpr unsigned int SavemapData::MAX_COLOUR;

const char* const SavemapData::CORNER_NAMES[4]
  = {"top_left", "top_right", "bottom_right", "bottom_left"};

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
    location_.x = 0.0f;
    location_.y = 0.0f;
    location_.z = -1.0f;
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
