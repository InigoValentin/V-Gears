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
#include "BattleSceneFile.h"
#include "VGearsUtility.h"
#include "FF7Data.h"

BattleSceneFile::BattleSceneFile(const unsigned int id, File file): id_(id){Read(file);}

BattleSceneFile::~BattleSceneFile(){}

std::vector<Attack> BattleSceneFile::GetAttacks(){return attacks_;}

std::vector<Enemy> BattleSceneFile::GetEnemies(){return enemies_;}

std::vector<Formation> BattleSceneFile::GetFormations(){return formations_;}

void BattleSceneFile::Read(File file){
    file.SetOffset(0);
    scene_.enemy[0] = file.readU16LE();
    scene_.enemy[1] = file.readU16LE();
    scene_.enemy[2] = file.readU16LE();
    scene_.unused = file.readU16LE();
    for (int s = 0; s < 4; s ++){
        scene_.setup[s].location = file.readU16LE();
        scene_.setup[s].next_battle = file.readU16LE();
        scene_.setup[s].escape_counter = file.readU16LE();
        scene_.setup[s].unused = file.readU16LE();
        for (int a = 0; a < 4; a ++) scene_.setup[s].next_arena_candidates[a] = file.readU16LE();
        scene_.setup[s].flags = file.readU16LE();
        scene_.setup[s].layout = file.readU8();
        scene_.setup[s].camera = file.readU8();
    }
    for (int c = 0; c < 4; c ++){
        for (int p = 0; p < 3; p ++){
            // Camera positions are stored as 16 bit floats. Here, 16 bit are being read, then, the
            // sign is checked to fit it into -32766 and 32767 (0x7F9b). Then, the number is
            // divided by 255 (0xFF) to keep just the integer part.
            scene_.camera[c].camera[p].x = file.readU16LE();
            if (scene_.camera[c].camera[p].x > 0x7F9B) scene_.camera[c].camera[p].x -= 0xFFFF;
            scene_.camera[c].camera[p].x /= 0xFF;
            // Y and Z are inverted in original data.
            scene_.camera[c].camera[p].y = file.readU16LE();
            if (scene_.camera[c].camera[p].y > 0x7F9B) scene_.camera[c].camera[p].y -= 0xFFFF;
            scene_.camera[c].camera[p].y /= 0xFF;
            scene_.camera[c].camera[p].z = file.readU16LE();
            if (scene_.camera[c].camera[p].z > 0x7F9B) scene_.camera[c].camera[p].z -= 0xFFFF;
            scene_.camera[c].camera[p].z /= 0xFF;
            scene_.camera[c].camera[p].d_x = file.readU16LE();
            if (scene_.camera[c].camera[p].d_x > 0x7F9B) scene_.camera[c].camera[p].d_x -= 0xFFFF;
            scene_.camera[c].camera[p].d_x /= 0xFF;
            scene_.camera[c].camera[p].d_y = file.readU16LE();
            if (scene_.camera[c].camera[p].d_y > 0x7F9B) scene_.camera[c].camera[p].d_y -= 0xFFFF;
            scene_.camera[c].camera[p].d_y /= 0xFF;
            scene_.camera[c].camera[p].d_z = file.readU16LE();
            if (scene_.camera[c].camera[p].d_z > 0x7F9B) scene_.camera[c].camera[p].d_z -= 0xFFFF;
            scene_.camera[c].camera[p].d_z /= 0xFF;
        }
        for (int p = 0; p < 12; p ++) scene_.camera[c].unused[p] = file.readU8();
    }
    for (int f = 0; f < 4; f ++){
        for (int e = 0; e < 6; e ++){
            scene_.formation[f][e].id = file.readU16LE();
            // Enemy positions are stored as 16 bit floats. Here, 16 bit are being read, then, the
            // sign is checked to fit it into -32766 and 32767 (0x7F9b). Then, the number is
            // divided by 255 (0xFF) to keep just the integer part.
            scene_.formation[f][e].x = file.readU16LE();
            if (scene_.formation[f][e].x > 0x7F9B) scene_.formation[f][e].x -= 0xFFFF;
            scene_.formation[f][e].x /= 0xFF;
            // Y and Z are inverted in original data.
            scene_.formation[f][e].z = file.readU16LE();
            if (scene_.formation[f][e].z > 0x7F9B) scene_.formation[f][e].z -= 0xFFFF;
            scene_.formation[f][e].z /= 0xFF;
            scene_.formation[f][e].y = file.readU16LE();
            if (scene_.formation[f][e].y > 0x7F9B) scene_.formation[f][e].y -= 0xFFFF;
            scene_.formation[f][e].y /= 0xFF;
            scene_.formation[f][e].row = file.readU16LE();
            scene_.formation[f][e].cover_flags = file.readU16LE();
            scene_.formation[f][e].flags = file.readU32LE();
        }
    }
    for (int e = 0; e < 3; e ++){
        for (int n = 0; n < 32; n ++) scene_.enemy_data[e].name[n] = file.readU8();
        scene_.enemy_data[e].level = file.readU8();
        scene_.enemy_data[e].spd = file.readU8();
        scene_.enemy_data[e].lck = file.readU8();
        scene_.enemy_data[e].eva = file.readU8();
        scene_.enemy_data[e].str = file.readU8();
        scene_.enemy_data[e].def = file.readU8();
        scene_.enemy_data[e].mag = file.readU8();
        scene_.enemy_data[e].mdef = file.readU8();
        for (int l = 0; l < 8; l ++) scene_.enemy_data[e].elements[l] = file.readU8();
        for (int l = 0; l < 8; l ++) scene_.enemy_data[e].element_rates[l] = file.readU8();
        for (int a = 0; a < 16; a ++) scene_.enemy_data[e].animation_index[a] = file.readU8();
        for (int a = 0; a < 16; a ++) scene_.enemy_data[e].attacks[a] = file.readU16LE();
        for (int a = 0; a < 16; a ++) scene_.enemy_data[e].attack_camera[a] = file.readU16LE();
        for (int i = 0; i < 4; i ++) scene_.enemy_data[e].item_rate[i] = file.readU8();
        for (int i = 0; i < 4; i ++) scene_.enemy_data[e].item[i] = file.readU16LE();
        for (int m = 0; m < 3; m ++) scene_.enemy_data[e].maniputale_attacks[m] = file.readU16LE();
        scene_.enemy_data[e].unknown_0 = file.readU16LE();
        scene_.enemy_data[e].mp = file.readU16LE();
        scene_.enemy_data[e].ap = file.readU16LE();
        scene_.enemy_data[e].morph = file.readU16LE();
        scene_.enemy_data[e].back_damage = file.readU8();
        scene_.enemy_data[e].unused = file.readU8();
        scene_.enemy_data[e].hp = file.readU32LE();
        scene_.enemy_data[e].exp = file.readU32LE();
        scene_.enemy_data[e].money = file.readU32LE();
        scene_.enemy_data[e].immunities = file.readU32LE();
        scene_.enemy_data[e].unknown_1 = file.readU32LE();
    }
    for (int a = 0; a < 32; a ++){
        scene_.attack_data[a].accuracy = file.readU8();
        scene_.attack_data[a].impact_effect = file.readU8();
        scene_.attack_data[a].hurt_anim = file.readU8();
        scene_.attack_data[a].unknown = file.readU8();
        scene_.attack_data[a].mp = file.readU16LE();
        scene_.attack_data[a].sound = file.readU16LE();
        scene_.attack_data[a].camera_single = file.readU16LE();
        scene_.attack_data[a].camera_multiple = file.readU16LE();
        scene_.attack_data[a].target = file.readU8();
        scene_.attack_data[a].effect = file.readU8();
        scene_.attack_data[a].damage = file.readU8();
        scene_.attack_data[a].power = file.readU8();
        scene_.attack_data[a].condition = file.readU8();
        scene_.attack_data[a].status_change = file.readU8();
        scene_.attack_data[a].additional_effects = file.readU8();
        scene_.attack_data[a].additional_effects_mod = file.readU8();
        scene_.attack_data[a].status = file.readU32LE();
        scene_.attack_data[a].element = file.readU16LE();
        scene_.attack_data[a].special = file.readU16LE();
    }
    for (int a = 0; a < 32; a ++) scene_.attack_ids[a] = file.readU16LE();
    for (int a = 0; a < 32; a ++)
        for (int b = 0; b < 32; b ++) scene_.attack_names[a][b] = file.readU8();
    // TODO: Keep reading for AI scripts.

    // All the data is read, now format it.

    // Populate enemy list.
    for (int e = 0; e < 3; e ++){
        if (scene_.enemy[e] == 0xFFFF) continue;
        // If the enemy is already on the list, skip it.
        bool already_added = false;
        for (int t = 0; t < enemies_.size(); t ++){
            if (enemies_[t].id == scene_.enemy[e]){
                already_added = true;
                break;
            }
        }
        if (already_added) continue;
        // Create a new enemy
        Enemy enemy;
        enemy.id = scene_.enemy[e];
        enemy.model = FF7Data::GetEnemyModelFromEnemyId(enemy.id);
        enemy.name = VGears::Utility::DecodeString(scene_.enemy_data[e].name, 32);
        enemy.level = scene_.enemy_data[e].level;
        enemy.str = scene_.enemy_data[e].str;
        enemy.mag = scene_.enemy_data[e].mag;
        enemy.def = scene_.enemy_data[e].def;
        enemy.mdef = scene_.enemy_data[e].mdef;
        enemy.spd = scene_.enemy_data[e].spd;
        enemy.lck = scene_.enemy_data[e].lck;
        enemy.eva = scene_.enemy_data[e].eva;
        enemy.hp = scene_.enemy_data[e].hp;
        enemy.mp = scene_.enemy_data[e].mp;
        enemy.exp = scene_.enemy_data[e].exp;
        enemy.ap = scene_.enemy_data[e].ap;
        enemy.money = scene_.enemy_data[e].money;
        enemy.back_damage = scene_.enemy_data[e].back_damage / 8.0f;
        for (int a = 0; a < 16; a ++){
            if (scene_.enemy_data[e].animation_index[a] != 0xFF)
                enemy.animations.push_back(scene_.enemy_data[e].animation_index[a]);
        }
        for (int l = 0; l < 8; l ++){
            if (
              scene_.enemy_data[e].elements[l] != 0xFF
              && scene_.enemy_data[e].element_rates[l] != 0xFF
            ){
                Enemy::Element element;
                element.id = scene_.enemy_data[e].elements[l];
                switch (scene_.enemy_data[e].element_rates[l]){
                    case 0x00: element.factor =  11.0f; break; // Death.
                    case 0x02: element.factor =   2.0f; break; // Double damage.
                    case 0x04: element.factor =   0.5f; break; // Half damage.
                    case 0x05: element.factor =   0.0f; break; // Nullify;
                    case 0x06: element.factor =  -1.0f; break; // Absorb 100%;
                    case 0x07: element.factor = -11.0f; break; // Full cure;
                    default: element.factor = 1.0f; // Normal damage.
                }
                enemy.elements.push_back(element);
            }
        }
        if (scene_.enemy_data[e].immunities != 0xFFFFFFFF){
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000001)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::DEATH;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000002)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::NEAR_DEATH;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000004)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::SLEEP;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000008)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::POISONED;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000010)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::SADNESS;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000020)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::FURY;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000040)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::CONFU;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000080)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::SILENCE;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000100)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::HASTE;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000200)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::SLOW;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000400)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::STOP;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00000800)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::FROG;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00001000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::SMALL;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00002000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::SLOW_NUMB;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00004000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::PETRIFY;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00008000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::REGEN;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00010000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::BARRIER;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00020000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::M_BARRIER;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00040000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::REFLECT;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00080000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::DUAL;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00100000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::SHIELD;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00200000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::D_SENTENCE;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00400000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::MANIPULATE;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x00800000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::BERSERK;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x01000000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::PEERLESS;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x02000000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::PARALYSIS;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x04000000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::DARKNESS;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x08000000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::DUAL_DRAIN;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x10000000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::DEATH_FORCE;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x20000000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::RESIST;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x40000000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::LUCKY_GIRL;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
            if (scene_.enemy_data[e].immunities == (scene_.enemy_data[e].immunities | 0x80000000)){
                Enemy::Immunity immunity;
                immunity.status = GameData::STATUS::IMPRISONED;
                immunity.rate = 1.0f;
                enemy.immunities.push_back(immunity);
            }
        }
        for (int a = 0; a < 16; a ++){
            if (scene_.enemy_data[e].attacks[a] == 0xFFFF) continue;
            Enemy::Attack attack;
            attack.id = scene_.enemy_data[e].attacks[a];
            if (scene_.enemy_data[e].attack_camera[a] == 0xFFFF) attack.camera = -1;
            else attack.camera = scene_.enemy_data[e].attack_camera[a];
            enemy.attacks.push_back(attack);
        }
        for (int m = 0; m < 3; m ++){
            if (scene_.enemy_data[e].maniputale_attacks[m] == 0xFFFF) continue;
            enemy.manipulate_attacks.push_back(scene_.enemy_data[e].maniputale_attacks[m]);
        }
        for (int i = 0; i < 4; i ++){
            if (scene_.enemy_data[e].item[i] == 0xFF) continue;
            Enemy::Item item;
            item.id = scene_.enemy_data[e].item[i];
            if (scene_.enemy_data[e].item_rate[i] < 0x80){
                item.rate = scene_.enemy_data[e].item_rate[i] / 63.0f;
                enemy.drop.push_back(item);
            }
            else{
                item.rate = (scene_.enemy_data[e].item_rate[i] - 0x80) / 63.0f;
                enemy.steal.push_back(item);
            }
        }
        if (scene_.enemy_data[e].morph != 0xFFFF) enemy.morph = scene_.enemy_data[e].morph;

        //std::cout << "ENEMY " << enemy.id << ": " << enemy.name << "\n";

        enemies_.push_back(enemy);
    }

    // Populate attack list.
    for (int a = 0; a < 32; a ++){
        if (scene_.attack_ids[a] == 0xFFFF) continue;
        // If the attack is already on the list, skip it.
        bool already_added = false;
        for (int t = 0; t < attacks_.size(); t ++){
            if (attacks_[t].id == scene_.attack_ids[a]){
                already_added = true;
                break;
            }
        }
        if (already_added) continue;
        // Create a new attack.
        Attack attack;
        attack.id = scene_.attack_ids[a];
        attack.name = VGears::Utility::DecodeString(scene_.attack_names[a], 32);
        attack.damage.accuracy = scene_.attack_data[a].accuracy / 255.0f;
        if (scene_.attack_data[a].impact_effect != 0xFF)
            attack.animation.impact_effect = scene_.attack_data[a].impact_effect;
        if (scene_.attack_data[a].hurt_anim != 0xFF)
            attack.animation.hurt_anim = scene_.attack_data[a].hurt_anim;
        attack.mp = scene_.attack_data[a].mp;
        if (scene_.attack_data[a].sound != 0xFFFF)
            attack.animation.sound = scene_.attack_data[a].sound;
        if (scene_.attack_data[a].camera_single != 0xFFFF)
            attack.animation.camera_single = scene_.attack_data[a].camera_single;
        if (scene_.attack_data[a].camera_multiple != 0xFFFF)
            attack.animation.camera_multiple = scene_.attack_data[a].camera_multiple;
        attack.target.selection_enabled
          = (scene_.attack_data[a].target == (scene_.attack_data[a].target | 0x01));
        attack.target.default_enemy
          = (scene_.attack_data[a].target == (scene_.attack_data[a].target | 0x02));
        attack.target.default_multiple
          = (scene_.attack_data[a].target == (scene_.attack_data[a].target | 0x04));
        attack.target.toggle_multiple
          = (scene_.attack_data[a].target == (scene_.attack_data[a].target | 0x08));
        attack.target.fixed_row
          = (scene_.attack_data[a].target == (scene_.attack_data[a].target | 0x10));
        attack.target.short_range
          = (scene_.attack_data[a].target == (scene_.attack_data[a].target | 0x20));
        attack.target.all_rows
          = (scene_.attack_data[a].target == (scene_.attack_data[a].target | 0x40));
        attack.target.random
          = (scene_.attack_data[a].target == (scene_.attack_data[a].target | 0x80));
        if (scene_.attack_data[a].effect != 0xFFFF)
            attack.animation.effect = scene_.attack_data[a].effect;
        attack.damage.formula = scene_.attack_data[a].damage >> 4;
        attack.damage.modifier = scene_.attack_data[a].damage & 15;
        attack.damage.power = scene_.attack_data[a].power;
        switch(scene_.attack_data[a].condition){
            case 0: attack.damage.restore = GameData::RestoreType::HP; break;
            case 1: attack.damage.restore = GameData::RestoreType::MP; break;
            case 2: attack.damage.restore = GameData::RestoreType::STATUS; break;
            default: attack.damage.restore = GameData::RestoreType::NONE;
        }
        int status_chance_raw = 63 - (scene_.attack_data[a].status_change ^ 63); // Out of 63;
        int status_mode;
        if (
          scene_.attack_data[a].status_change == (scene_.attack_data[a].status_change | 0x00000080)
        ){
            status_mode = GameData::STATUS_EFFECT_MODE::TOGGLE;
        }
        else if (
          scene_.attack_data[a].status_change == (scene_.attack_data[a].status_change | 0x00000040)
        ){
            status_mode = GameData::STATUS_EFFECT_MODE::TOGGLE;
        }
        else status_mode = GameData::STATUS_EFFECT_MODE::TOGGLE;
        if (scene_.attack_data[a].status != 0xFFFFFFFF){
            float status_chance = status_chance_raw / 63.0f;
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000001)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::DEATH;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000002)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::NEAR_DEATH;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000004)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::SLEEP;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000008)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::POISONED;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000010)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::SADNESS;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000020)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::FURY;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000040)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::CONFU;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000080)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::SILENCE;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000100)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::HASTE;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000200)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::SLOW;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000400)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::STOP;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00000800)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::FROG;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00001000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::SMALL;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00002000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::SLOW_NUMB;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00004000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::PETRIFY;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00008000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::REGEN;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00010000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::BARRIER;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00020000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::M_BARRIER;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00040000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::REFLECT;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00080000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::DUAL;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00100000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::SHIELD;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00200000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::D_SENTENCE;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00400000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::MANIPULATE;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x00800000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::BERSERK;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x01000000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::PEERLESS;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x02000000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::PARALYSIS;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x04000000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::DARKNESS;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x08000000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::DUAL_DRAIN;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x10000000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::DEATH_FORCE;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x20000000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::RESIST;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x40000000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::LUCKY_GIRL;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
            if (scene_.attack_data[a].status == (scene_.attack_data[a].status | 0x80000000)){
                Attack::StatusEffect effect;
                effect.id = GameData::STATUS::IMPRISONED;
                effect.chance = status_chance;
                effect.type = status_mode;
                attack.status_effects.push_back(effect);
            }
        }
        if (scene_.attack_data[a].element >= 0 && scene_.attack_data[a].element <= 0x0F)
            attack.elements.push_back(scene_.attack_data[a].element);
        attack.damage.damage_mp
          = (scene_.attack_data[a].special != (scene_.attack_data[a].special | 0x0001));
        attack.damage.affected_by_darkness
          = (scene_.attack_data[a].special != (scene_.attack_data[a].special | 0x0004));
        if ((scene_.attack_data[a].special != (scene_.attack_data[a].special | 0x0010))){
            if (attack.damage.damage_mp) attack.damage.drain_mp = true;
            else attack.damage.drain_hp = true;
        }
        if ((scene_.attack_data[a].special != (scene_.attack_data[a].special | 0x0020))){
            attack.damage.drain_hp = true;
            attack.damage.drain_mp = true;
        }
        attack.damage.ignore_status_defense
          = (scene_.attack_data[a].special != (scene_.attack_data[a].special | 0x0080));
        attack.target.only_for_dead
          = (scene_.attack_data[a].special != (scene_.attack_data[a].special | 0x0100));
        attack.reflectable
          = (scene_.attack_data[a].special != (scene_.attack_data[a].special | 0x0200));
        attack.target.no_re_target
          = (scene_.attack_data[a].special != (scene_.attack_data[a].special | 0x0800));
        attack.damage.critical
          = (scene_.attack_data[a].special != (scene_.attack_data[a].special | 0x2000));

        // TODO: Parse scene_.attack_data[a].additional_effects with
        // scene_.attack_data[a].additional_effects_mod for special attack modifiers (23 total).
        // https://wiki.ffrtt.ru/index.php/FF7/Battle/Attack_Special_Effects

        //std::cout << "ATTACK " << ((int) attack.id) << ": " << attack.name << "\n";
        attacks_.push_back(attack);
    }

    // Formations
    for (int f = 0; f < 4; f ++){
        Formation formation;
        if (scene_.setup[f].next_battle != 0xFFFF)
            formation.next_formation = scene_.setup[f].next_battle;
        formation.escape_counter = (scene_.setup[f].escape_counter / 64.0f);
        // TODO: What is flag 0x01? Multilimb enemies?
        formation.escapable = (scene_.setup[f].flags == scene_.setup[f].flags | 0x02);
        formation.skip_victory_pose = (scene_.setup[f].flags != scene_.setup[f].flags | 0x04);
        formation.skip_spoils = (scene_.setup[f].flags != scene_.setup[f].flags | 0x08);
        formation.preemptive_disabled = (scene_.setup[f].flags != scene_.setup[f].flags | 0x10);
        // TODO: What is flag 0x20?
        // TODO: What is flag 0x40?
        // TODO: What is flag 0x80?
        switch(scene_.setup[f].layout){
            case 0x0: formation.layout = Formation::Layout::NORMAL; break;
            case 0x1: formation.layout = Formation::Layout::PREEMPTIVE; break;
            case 0x2: formation.layout = Formation::Layout::BACK_ATTACK; break;
            case 0x3: formation.layout = Formation::Layout::SIDE_ATTACK; break;
            case 0x4: formation.layout = Formation::Layout::PINCER_ATTACK; break;
            case 0x5: formation.layout = Formation::Layout::PINCER_ATTACK; break;
            case 0x6: formation.layout = Formation::Layout::SIDE_ATTACK; break;
            case 0x7: formation.layout = Formation::Layout::SIDE_ATTACK; break;
            case 0x8: formation.layout = Formation::Layout::LOCKED; break;
            default: formation.layout = Formation::Layout::NORMAL;
        }
        for (int e = 0; e < 6; e ++){
            if (scene_.formation[f][e].id != 0xFFFF){
                Formation::Enemy enemy;
                enemy.id = scene_.formation[f][e].id;
                enemy.x = scene_.formation[f][e].x;
                enemy.y = scene_.formation[f][e].y;
                enemy.z = scene_.formation[f][e].z;
                enemy.row = scene_.formation[f][e].row;
                enemy.cover[0] = (
                  scene_.formation[f][e].cover_flags == (scene_.formation[f][e].cover_flags | 0x01)
                  ? '1' : '0'
                );
                enemy.cover[1] = (
                  scene_.formation[f][e].cover_flags == (scene_.formation[f][e].cover_flags | 0x02)
                  ? '1' : '0'
                );
                enemy.cover[2] = (
                  scene_.formation[f][e].cover_flags == (scene_.formation[f][e].cover_flags | 0x04)
                  ? '1' : '0'
                );
                enemy.cover[3] = (
                  scene_.formation[f][e].cover_flags == (scene_.formation[f][e].cover_flags | 0x08)
                  ? '1' : '0'
                );
                enemy.cover[4] = (
                  scene_.formation[f][e].cover_flags == (scene_.formation[f][e].cover_flags | 0x10)
                  ? '1' : '0'
                );
                enemy.visible
                  = (scene_.formation[f][e].flags == scene_.formation[f][e].flags | 0x01);
                enemy.direction
                  = ((scene_.formation[f][e].flags == scene_.formation[f][e].flags | 0x02) ? 1 : 0);
                enemy.targeteable
                  = (scene_.formation[f][e].flags == scene_.formation[f][e].flags | 0x08);
                enemy.main_script_active
                  = (scene_.formation[f][e].flags == scene_.formation[f][e].flags | 0x10);
                formation.enemies.push_back(enemy);
            }
        }
        formation.location = scene_.setup[f].location;
        formation.location_name = LocationToString(scene_.setup[f].location);
        for (int c = 0; c < 3; c ++){
            Formation::Camera camera;
            camera.x = scene_.camera[f].camera[c].x;
            camera.y = scene_.camera[f].camera[c].y;
            camera.z = scene_.camera[f].camera[c].z;
            camera.d_x = scene_.camera[f].camera[c].d_x;
            camera.d_y = scene_.camera[f].camera[c].d_y;
            camera.d_z = scene_.camera[f].camera[c].d_z;
            formation.camera_positions.push_back(camera);
        }
        if (scene_.setup[f].camera < formation.camera_positions.size())
            formation.initial_camera_position = scene_.setup[f].camera;
        for (int a = 0; a < 4; a ++){
            if (scene_.setup[f].next_arena_candidates[a] != 999){
                formation.next_arena_formation_candidates.push_back(
                  scene_.setup[f].next_arena_candidates[a]
                );
                formation.is_arena_battle = true;
            }
        }
        formations_.push_back(formation);
    }

}

std::string BattleSceneFile::LocationToString(const int id){
    switch(id){
        case 0x0000: return "blank";
        case 0x0001: return "bizarro_battle_center";
        case 0x0002: return "grassland";
        case 0x0003: return "mt_nibel";
        case 0x0004: return "forest";
        case 0x0005: return "beach";
        case 0x0006: return "desert";
        case 0x0007: return "snow";
        case 0x0008: return "swamp";
        case 0x0009: return "sector_1_train_station";
        case 0x000a: return "reactor_1";
        case 0x000b: return "reactor_1_core";
        case 0x000c: return "reactor_1_entrance";
        case 0x000d: return "sector_4_subway";
        case 0x000e: return "nibel_caves";
        case 0x000f: return "shinra_hq";
        case 0x0010: return "midgar_raid_subway";
        case 0x0011: return "hojo_lab";
        case 0x0012: return "shinra_elevators";
        case 0x0013: return "shinra_roof";
        case 0x0014: return "midgar_highway";
        case 0x0015: return "wutai_pagoda";
        case 0x0016: return "church";
        case 0x0017: return "coral_valley";
        case 0x0018: return "midgar_slums";
        case 0x0019: return "sector_4_corridors";
        case 0x001a: return "sector_4_gantries";
        case 0x001b: return "sector_7_pillar_stairway";
        case 0x001c: return "sector_7pillar_top";
        case 0x001d: return "sector_8";
        case 0x001e: return "sewers";
        case 0x001f: return "mythril_mines";
        case 0x0020: return "crater_floating_platforms";
        case 0x0021: return "corel_mountain_path";
        case 0x0022: return "junon_beach";
        case 0x0023: return "junon_cargo_ship";
        case 0x0024: return "corel_prison";
        case 0x0025: return "battle_square";
        case 0x0026: return "da_chao_rapps_battle";
        case 0x0027: return "cids_backyard";
        case 0x0028: return "final_descent_to_sephiroth";
        case 0x0029: return "reactor_5_entrance";
        case 0x002a: return "ancients_escher_room";
        case 0x002b: return "shinra_mansion";
        case 0x002c: return "junon_airship_dock";
        case 0x002d: return "whirlwind_maze";
        case 0x002e: return "junon_underwater_reactor";
        case 0x002f: return "gongaga_reactor";
        case 0x0030: return "gelnika";
        case 0x0031: return "train_graveyard";
        case 0x0032: return "ice_caves";
        case 0x0033: return "sister_ray";
        case 0x0034: return "sister_ray_base";
        case 0x0035: return "forgotten_city_altar";
        case 0x0036: return "crater_descent";
        case 0x0037: return "crater_hatchery";
        case 0x0038: return "crater_water_area";
        case 0x0039: return "safer_battle";
        case 0x003a: return "kalm_flashback_dragon";
        case 0x003b: return "junon_pipe";
        case 0x003c: return "blank";
        case 0x003d: return "railway_canyon";
        case 0x003e: return "whirlwind_maze_crater";
        case 0x003f: return "railway_rollercoaster";
        case 0x0040: return "wooden_bridge";
        case 0x0041: return "da_chao";
        case 0x0042: return "fort_condor";
        case 0x0043: return "dirt_wasteland";
        case 0x0044: return "bizarro_battle_right";
        case 0x0045: return "bizarro_battle_left";
        case 0x0046: return "jenova_synthesis";
        case 0x0047: return "corel_train_battle";
        case 0x0048: return "cosmo_canyon";
        case 0x0049: return "caverns_of_the_gi";
        case 0x004a: return "nibelheim_mansion_basement";
        case 0x004b: return "ancients_demons_gate";
        case 0x004c: return "ancients_mural_room";
        case 0x004d: return "ancients_clock_passage";
        case 0x004e: return "final_battle";
        case 0x004f: return "jungle";
        case 0x0050: return "ultimate_weapon_highwind";
        case 0x0051: return "corel_reactor";
        case 0x0052: return "unused";
        case 0x0053: return "don_corneos_mansion";
        case 0x0054: return "emerald_weapon_battle";
        case 0x0055: return "reactor_5";
        case 0x0056: return "shinra_hq_-_escape";
        case 0x0057: return "ultimate_weapon_gongaga";
        case 0x0058: return "corel_prison_dyne";
        case 0x0059: return "ultimate_weapon_forest";
        default: return "unknonw_location";
    }
}
