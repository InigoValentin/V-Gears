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

#include <algorithm>
#include <iterator>
#include "installer/support/Utility.h"
#include "installer/tables/FF7Data.h"
#include "BattleSceneConverter.h"

namespace{

    /**
     * The names of the battle locations, by ID.
     */
    const char* const LOCATION_NAMES[] = {
      "blank",
      "bizarro_battle_center",
      "grassland",
      "mt_nibel",
      "forest",
      "beach",
      "desert",
      "snow",
      "swamp",
      "sector_1_train_station",
      "reactor_1",
      "reactor_1_core",
      "reactor_1_entrance",
      "sector_4_subway",
      "nibel_caves",
      "shinra_hq",
      "midgar_raid_subway",
      "hojo_lab",
      "shinra_elevators",
      "shinra_roof",
      "midgar_highway",
      "wutai_pagoda",
      "church",
      "coral_valley",
      "midgar_slums",
      "sector_4_corridors",
      "sector_4_gantries",
      "sector_7_pillar_stairway",
      "sector_7pillar_top",
      "sector_8",
      "sewers",
      "mythril_mines",
      "crater_floating_platforms",
      "corel_mountain_path",
      "junon_beach",
      "junon_cargo_ship",
      "corel_prison",
      "battle_square",
      "da_chao_rapps_battle",
      "cids_backyard",
      "final_descent_to_sephiroth",
      "reactor_5_entrance",
      "ancients_escher_room",
      "shinra_mansion",
      "junon_airship_dock",
      "whirlwind_maze",
      "junon_underwater_reactor",
      "gongaga_reactor",
      "gelnika",
      "train_graveyard",
      "ice_caves",
      "sister_ray",
      "sister_ray_base",
      "forgotten_city_altar",
      "crater_descent",
      "crater_hatchery",
      "crater_water_area",
      "safer_battle",
      "kalm_flashback_dragon",
      "junon_pipe",
      "blank",
      "railway_canyon",
      "whirlwind_maze_crater",
      "railway_rollercoaster",
      "wooden_bridge",
      "da_chao",
      "fort_condor",
      "dirt_wasteland",
      "bizarro_battle_right",
      "bizarro_battle_left",
      "jenova_synthesis",
      "corel_train_battle",
      "cosmo_canyon",
      "caverns_of_the_gi",
      "nibelheim_mansion_basement",
      "ancients_demons_gate",
      "ancients_mural_room",
      "ancients_clock_passage",
      "final_battle",
      "jungle",
      "ultimate_weapon_highwind",
      "corel_reactor",
      "unused",
      "don_corneos_mansion",
      "emerald_weapon_battle",
      "reactor_5",
      "shinra_hq_-_escape",
      "ultimate_weapon_gongaga",
      "corel_prison_dyne",
      "ultimate_weapon_forest"
    };

    /**
     * Checks if the list has an element with an ID.
     */
    template<typename Element>
    bool Contains(const std::vector<Element>& elements, const unsigned int id){
        return std::any_of(elements.begin(), elements.end(), [id](const Element& element){
            return element.id == id;
        });
    }

    /**
     * Checks if some bits are all set in a value.
     */
    bool Has(const unsigned int value, const unsigned int bits){return value == (value | bits);}

    /**
     * Adds the attacks that an enemy can use. Each of the 16 slots has an attack ID, the
     * animation script of the model to play and the camera to use.
     */
    void AddAttacks(const BattleSceneData::EnemyData& data, Enemy& enemy){
        for (int a = 0; a < 16; a ++){
            if (data.attacks[a] == 0xFFFF) continue;
            Enemy::Attack attack;
            attack.id = data.attacks[a];
            attack.animation = (data.animation_index[a] != 0xFF) ? data.animation_index[a] : -1;
            attack.camera = (data.attack_camera[a] != 0xFFFF) ? data.attack_camera[a] : -1;
            enemy.attacks.push_back(attack);
        }
    }

    /**
     * Adds the elemental defenses of an enemy.
     */
    void AddElements(const BattleSceneData::EnemyData& data, Enemy& enemy){
        for (int l = 0; l < 8; l ++){
            if (data.elements[l] == 0xFF || data.element_rates[l] == 0xFF) continue;
            Enemy::Element element;
            element.id = data.elements[l];
            element.factor = BattleSceneConverter::ElementFactor(data.element_rates[l]);
            enemy.elements.push_back(element);
        }
    }

    /**
     * Adds the status immunities of an enemy.
     *
     * They are a bitmask: the bit position is the status ID, as in the original game (0 death,
     * 1 near death, 2 sleep, 3 poison, 4 sadness, 5 fury, 6 confusion, 7 silence...).
     */
    void AddImmunities(const BattleSceneData::EnemyData& data, Enemy& enemy){
        if (data.immunities == 0xFFFFFFFF) return;
        for (int bit = 0; bit < 32; bit ++){
            if ((data.immunities & (1u << bit)) == 0) continue;
            Enemy::Immunity immunity;
            immunity.status = bit;
            immunity.rate = 1.0f;
            enemy.immunities.push_back(immunity);
        }
    }

    /**
     * Adds the items that an enemy drops and the ones that can be stolen from it.
     */
    void AddItems(const BattleSceneData::EnemyData& data, Enemy& enemy){
        for (int i = 0; i < 4; i ++){
            if (data.item[i] == 0xFFFF) continue;
            Enemy::Item item;
            item.id = data.item[i];
            // The highest bit of the rate marks the items to steal.
            if (data.item_rate[i] < 0x80){
                item.rate = data.item_rate[i] / 63.0f;
                enemy.drop.push_back(item);
            }
            else{
                item.rate = (data.item_rate[i] - 0x80) / 63.0f;
                enemy.steal.push_back(item);
            }
        }
    }

    /**
     * Converts the data of an enemy.
     */
    Enemy ConvertEnemy(const BattleSceneData& scene, const int index){
        const BattleSceneData::EnemyData& data = scene.enemy_data[index];
        Enemy enemy;
        enemy.id = scene.enemy[index];
        enemy.model = FF7Data::GetEnemyModelFromEnemyId(enemy.id);
        enemy.name = VGears::Utility::DecodeString(data.name, 32);
        enemy.level = data.level;
        enemy.str = data.str;
        enemy.mag = data.mag;
        enemy.def = data.def;
        enemy.mdef = data.mdef;
        enemy.spd = data.spd;
        enemy.lck = data.lck;
        enemy.eva = data.eva;
        enemy.hp = data.hp;
        enemy.mp = data.mp;
        enemy.exp = data.exp;
        enemy.ap = data.ap;
        enemy.money = data.money;
        enemy.back_damage = data.back_damage / 8.0f;
        for (int a = 0; a < 16; a ++){
            if (data.animation_index[a] != 0xFF) enemy.animations.push_back(data.animation_index[a]);
        }
        enemy.ai_scripts = scene.ai_scripts[index];
        AddAttacks(data, enemy);
        AddElements(data, enemy);
        AddImmunities(data, enemy);
        for (int m = 0; m < 3; m ++){
            if (data.maniputale_attacks[m] != 0xFFFF)
                enemy.manipulate_attacks.push_back(data.maniputale_attacks[m]);
        }
        AddItems(data, enemy);
        if (data.morph != 0xFFFF) enemy.morph = data.morph;
        return enemy;
    }

    /**
     * Sets the animations, sounds and cameras of an attack.
     */
    void SetAnimation(const BattleSceneData::AttackData& data, Attack& attack){
        if (data.impact_effect != 0xFF) attack.animation.impact_effect = data.impact_effect;
        if (data.hurt_anim != 0xFF) attack.animation.hurt_anim = data.hurt_anim;
        if (data.sound != 0xFFFF) attack.animation.sound = data.sound;
        if (data.camera_single != 0xFFFF) attack.animation.camera_single = data.camera_single;
        if (data.camera_multiple != 0xFFFF) attack.animation.camera_multiple = data.camera_multiple;
        if (data.effect != 0xFF) attack.animation.effect = data.effect;
    }

    /**
     * Sets how an attack chooses its targets.
     */
    void SetTarget(const BattleSceneData::AttackData& data, Attack& attack){
        attack.target.selection_enabled = Has(data.target, 0x01);
        attack.target.default_enemy = Has(data.target, 0x02);
        attack.target.default_multiple = Has(data.target, 0x04);
        attack.target.toggle_multiple = Has(data.target, 0x08);
        attack.target.fixed_row = Has(data.target, 0x10);
        attack.target.short_range = Has(data.target, 0x20);
        attack.target.all_rows = Has(data.target, 0x40);
        attack.target.random = Has(data.target, 0x80);
    }

    /**
     * Sets the damage of an attack.
     */
    void SetDamage(const BattleSceneData::AttackData& data, Attack& attack){
        attack.damage.accuracy = data.accuracy / 255.0f;
        attack.damage.formula = data.damage >> 4;
        attack.damage.modifier = data.damage & 15;
        attack.damage.power = data.power;
        switch(data.condition){
            case 0: attack.damage.restore = GameData::RestoreType::HP; break;
            case 1: attack.damage.restore = GameData::RestoreType::MP; break;
            case 2: attack.damage.restore = GameData::RestoreType::STATUS; break;
            default: attack.damage.restore = GameData::RestoreType::NONE;
        }
    }

    /**
     * Adds the statuses that an attack causes or removes.
     *
     * The lower 6 bits of the status change byte are the chance out of 63, and the upper 2 bits
     * the mode (0x80 toggles the statuses, 0x40 cures them, otherwise they are inflicted). The
     * status bitmask has the status IDs as bit positions, as in the original game.
     */
    void AddStatusEffects(const BattleSceneData::AttackData& data, Attack& attack){
        if (data.status == 0xFFFFFFFF) return;
        int mode = GameData::STATUS_EFFECT_MODE::INFLICT;
        if ((data.status_change & 0x80) != 0) mode = GameData::STATUS_EFFECT_MODE::TOGGLE;
        else if ((data.status_change & 0x40) != 0) mode = GameData::STATUS_EFFECT_MODE::CURE;
        const float chance = (data.status_change & 0x3F) / 63.0f;
        for (int bit = 0; bit < 32; bit ++){
            if ((data.status & (1u << bit)) == 0) continue;
            Attack::StatusEffect effect;
            effect.id = bit;
            effect.chance = chance;
            effect.type = mode;
            attack.status_effects.push_back(effect);
        }
    }

    /**
     * Sets the special properties of an attack. Each one is on when its bit is not set.
     */
    void SetSpecialFlags(const BattleSceneData::AttackData& data, Attack& attack){
        const auto on = [&data](const unsigned int bit){return !Has(data.special, bit);};
        attack.damage.damage_mp = on(0x0001);
        attack.damage.affected_by_darkness = on(0x0004);
        if (on(0x0010)){
            if (attack.damage.damage_mp) attack.damage.drain_mp = true;
            else attack.damage.drain_hp = true;
        }
        if (on(0x0020)){
            attack.damage.drain_hp = true;
            attack.damage.drain_mp = true;
        }
        attack.damage.ignore_status_defense = on(0x0080);
        attack.target.only_for_dead = on(0x0100);
        attack.reflectable = on(0x0200);
        attack.target.no_re_target = on(0x0800);
        attack.damage.critical = on(0x2000);
    }

    /**
     * Converts the data of an attack.
     */
    Attack ConvertAttack(const BattleSceneData& scene, const int index){
        const BattleSceneData::AttackData& data = scene.attack_data[index];
        Attack attack;
        attack.id = scene.attack_ids[index];
        attack.name = VGears::Utility::DecodeString(scene.attack_names[index], 32);
        attack.mp = data.mp;
        SetAnimation(data, attack);
        SetTarget(data, attack);
        SetDamage(data, attack);
        AddStatusEffects(data, attack);
        // Attack elements are a bitmask (0x0001 is fire, 0x0002 ice...), while enemies elements
        // are indexes (0 is fire, 1 is ice...). Elements are stored as indexes.
        if (data.element != 0xFFFF){
            for (int bit = 0; bit < 16; bit ++){
                if ((data.element & (1 << bit)) != 0) attack.elements.push_back(bit);
            }
        }
        SetSpecialFlags(data, attack);
        // TODO: Parse additional_effects with additional_effects_mod for special attack
        // modifiers (23 total). https://wiki.ffrtt.ru/index.php/FF7/Battle/Attack_Special_Effects
        return attack;
    }

    /**
     * Converts an enemy of a formation.
     */
    Formation::Enemy ConvertFormationEnemy(const BattleSceneData::Formation& data){
        Formation::Enemy enemy;
        enemy.id = data.id;
        enemy.x = data.x;
        enemy.y = data.y;
        enemy.z = data.z;
        enemy.row = data.row;
        for (int cover = 0; cover < 5; cover ++)
            enemy.cover[cover] = Has(data.cover_flags, 1u << cover) ? '1' : '0';
        // TODO: These are always true: the comparisons are done before the bits are added.
        enemy.visible = (data.flags == data.flags | 0x01);
        enemy.direction = ((data.flags == data.flags | 0x02) ? 1 : 0);
        enemy.targeteable = (data.flags == data.flags | 0x08);
        enemy.main_script_active = (data.flags == data.flags | 0x10);
        return enemy;
    }

    /**
     * Converts a camera position of a formation.
     */
    Formation::Camera ConvertCamera(const BattleSceneData::Camera::Position& data){
        // The original data has x to the side, y down and z in depth (the enemies are in the
        // negative side). The scene has the same x, the depth in y and z up, in units 255 times
        // bigger, like the positions of the enemies.
        Formation::Camera camera;
        camera.x = data.x / 255.0f;
        camera.y = data.z / 255.0f;
        camera.z = -data.y / 255.0f;
        camera.d_x = data.d_x / 255.0f;
        camera.d_y = data.d_z / 255.0f;
        camera.d_z = -data.d_y / 255.0f;
        return camera;
    }

    /**
     * Converts the data of a formation.
     */
    Formation ConvertFormation(const BattleSceneData& scene, const int index){
        const BattleSceneData::Setup& setup = scene.setup[index];
        Formation formation;
        if (setup.next_battle != 0xFFFF) formation.next_formation = setup.next_battle;
        formation.escape_counter = (setup.escape_counter / 64.0f);
        // Observed in the original scene.bin (1024 setups): the upper byte is always 0xFF and
        // bit 0x02 is clear in most random battles, so it means "can't escape" (inverted).
        // TODO: What is flag 0x01?
        formation.escapable = ((setup.flags & 0x02) == 0);
        // TODO: Bits 0x04, 0x08 and 0x10 are set in almost every setup, so they can't be read as
        // positive "skip victory pose", "skip spoils" and "preemptive disabled" flags. The
        // defaults are used until their meaning is verified against known battles.
        formation.skip_victory_pose = false;
        formation.skip_spoils = false;
        formation.preemptive_disabled = false;
        // TODO: What are flags 0x20, 0x40 and 0x80?
        formation.layout = BattleSceneConverter::Layout(setup.layout);
        for (const BattleSceneData::Formation& enemy : scene.formation[index]){
            if (enemy.id != 0xFFFF) formation.enemies.push_back(ConvertFormationEnemy(enemy));
        }
        formation.location = setup.location;
        formation.location_name = BattleSceneConverter::LocationName(setup.location);
        for (const BattleSceneData::Camera::Position& camera : scene.camera[index].camera)
            formation.camera_positions.push_back(ConvertCamera(camera));
        // The camera byte of the formation is the script of the camera at the start of the
        // battle, which ends at the first camera of the formation unless it chooses another one.
        formation.intro_camera = setup.camera;
        for (const u16 candidate : setup.next_arena_candidates){
            if (candidate == 999) continue;
            formation.next_arena_formation_candidates.push_back(candidate);
            formation.is_arena_battle = true;
        }
        return formation;
    }
}

namespace BattleSceneConverter{

    std::vector<Enemy> Enemies(const BattleSceneData& data){
        std::vector<Enemy> enemies;
        for (int e = 0; e < 3; e ++){
            if (data.enemy[e] != 0xFFFF && !Contains(enemies, data.enemy[e]))
                enemies.push_back(ConvertEnemy(data, e));
        }
        return enemies;
    }

    std::vector<Attack> Attacks(const BattleSceneData& data){
        std::vector<Attack> attacks;
        for (int a = 0; a < 32; a ++){
            if (data.attack_ids[a] != 0xFFFF && !Contains(attacks, data.attack_ids[a]))
                attacks.push_back(ConvertAttack(data, a));
        }
        return attacks;
    }

    std::vector<Formation> Formations(const BattleSceneData& data){
        std::vector<Formation> formations;
        for (int f = 0; f < 4; f ++) formations.push_back(ConvertFormation(data, f));
        return formations;
    }

    float ElementFactor(const u8 rate){
        switch (rate){
            case 0x00: return 11.0f; // Death.
            case 0x02: return 2.0f; // Double damage.
            case 0x04: return 0.5f; // Half damage.
            case 0x05: return 0.0f; // Nullify.
            case 0x06: return -1.0f; // Absorb 100%.
            case 0x07: return -11.0f; // Full cure.
            default: return 1.0f; // Normal damage.
        }
    }

    Formation::Layout Layout(const u8 layout){
        static const Formation::Layout LAYOUTS[] = {
          Formation::Layout::NORMAL, Formation::Layout::PREEMPTIVE,
          Formation::Layout::BACK_ATTACK, Formation::Layout::SIDE_ATTACK,
          Formation::Layout::PINCER_ATTACK, Formation::Layout::PINCER_ATTACK,
          Formation::Layout::SIDE_ATTACK, Formation::Layout::SIDE_ATTACK,
          Formation::Layout::LOCKED
        };
        if (layout >= std::end(LAYOUTS) - std::begin(LAYOUTS)) return Formation::Layout::NORMAL;
        return LAYOUTS[layout];
    }

    std::string LocationName(const unsigned int id){
        if (id >= std::end(LOCATION_NAMES) - std::begin(LOCATION_NAMES)) return "unknonw_location";
        return LOCATION_NAMES[id];
    }
}
