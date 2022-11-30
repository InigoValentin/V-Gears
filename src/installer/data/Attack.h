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

#include <string>
#include "common/TypeDefine.h"
#include "data/GameData.h"

/**
 * An attack structure.
 */
struct Attack{

    /**
     * Attack parameters related to the damage done.
     */
    struct Damage{

        /**
         * The attack power.
         *
         * Used for damage/healing calculation.
         */
        u16 power;

        /**
         * Indicates physical damage.
         */
        bool physical;

        /**
         * Indicates magical damage.
         */
        bool magical;

        /**
         * The damage formula.
         */
        int formula;

        /**
         * The damage modifier.
         */
        int modifier;

        /**
         * Probability of the attack to land.
         */
        float accuracy;

        /**
         * Indicates if the accuracy is affected by darkness.
         */
        bool affected_by_darkness;

        /**
         * The restore type.
         */
        GameData::RestoreType restore;

        /**
         * Indicates if the attack damages MP instead of HP.
         */
        bool damage_mp;

        /**
         * Indicates if the attack drains HP from the target.
         */
        bool drain_hp;

        /**
         * Indicates if the attack drains MP from the target.
         */
        bool drain_mp;

        /**
         * Indicates if the attack ignores status defense.
         */
        bool ignore_status_defense;

        /**
         * If true, the attack will always be critical.
         */
        bool critical;

    };

    /**
     * Status effect information.
     */
    struct StatusEffect{

        /**
         * ID of the status effect.
         */
        u8 id;

        /**
         * Chance of inflicting
         */
        float chance;

        /**
         * Inflict/cure/toggle mode.
         */
        unsigned int type;
    };

    /**
     * Targeting parameters.
     */
    struct Target{

        /**
         * The target can be selected.
         *
         * If the target of the attack is selectable, at least between some subset of
         * possible targets, this must be true.
         */
        bool selection_enabled;

        /**
         * The default target is an enemy.
         *
         * When true, the cursor starts in an enemy row. When false, it starts in the ally
         * row. It may or may not be possible to change it.
         */
        bool default_enemy;

        /**
         * The default target is a group.
         *
         * When true, the cursor starts will all the members of a row selected. When false,
         * it starts with only one target selected. It may or may not be possible to change
         * it.
         */
        bool default_multiple;

        /**
         * The selection can change between groups and individuals.
         *
         * When true, the target can be changed between groups and individuals When false,
         * only {@see default_multiple} determines if the target is an individual or a
         * group.
         */
        bool toggle_multiple;

        /**
         * The target group can be changed.
         *
         * When true, the target cannot be changed between enemies and allies. In this case
         * {@see default_enemy} will determine the group that can be targeted.
         */
        bool fixed_row;

        /**
         * The command is short ranged.
         *
         * When true, if the target or the caster is not in the front of their row, the
         * target will take half damage. Also, when true, an enemy covered by another enemy
         * can't be targeted.
         */
        bool short_range;

        /**
         * Targets all, enemies and allies.
         *
         * When true, all allies and enemies are targets, and the selection can't be changed.
         */
        bool all_rows;

        /**
         * The target is random.
         *
         * When multiple targets are selected, one will be selected at random to be the
         * receiving target. Cursor will cycle among all viable targets.
         */
        bool random;

        /**
         * If true, the attack will miss unless the target is dead.
         */
        bool only_for_dead;

        /**
         * The attack can be re-targeted if the target is dead.
         */
        bool no_re_target;
    };

    /**
     * Animation data related to the attack;
     */
    struct Animation{

        /**
         * Atack effect ID.
         *
         * Used for animation in battle. -1 if the attack has no animation.
         */
        int effect;

        /**
         * Impact effect.
         *
         * The sprite shown when the attack hits. -1 to not show any.
         */
        int impact_effect;

        /**
         * Index of the animation played by the target when hit. -1 to not show any.
         */
        int hurt_anim;

        /**
         * Impact sound.
         *
         * Determines the camera movement when the command is executed over multiple targets. -1 to not
         * play any.
         */
        int sound;

        /**
         * Camera movement ID for single target.
         *
         * Determines the camera movement when the command is executed over a single target.  -1 to not
         * move the camera.
         */
        int camera_single;

        /**
         * Camera movement ID for multiple target.
         *
         * Determines the camera movement when the command is executed over multiple
         * targets. -1 to not move the camera.
         */
        int camera_multiple;
    };

    /**
     * Attack ID.
     */
    u32 id;

    /**
     * Attack name.
     */
    std::string name;

    /**
     * Damage parameters.
     */
    Damage damage;

    /**
     * MP cost.
     *
     * Determines the MP const of the attack.
     */
    u16 mp;

    /**
     * Targeting options.
     */
    Target target;

    /**
     * The attack can be reflected.
     */
    bool reflectable;

    /**
     * Information about status change mode and chance.
     */
    std::vector<StatusEffect> status_effects;

    /**
     * Information about the item elements. 2 bytes.
     */
    std::vector<unsigned int> elements;

    /**
     * Animation data.
     */
    Animation animation;

    /**
     * Constructor.
     *
     * Initializes data to default values.
     */
    Attack(): id(0), name(""), mp(0), reflectable(false){
        damage.physical = false;
        damage.magical = false;
        damage.power = 0;
        damage.formula = -1;
        damage.modifier = -1;
        damage.accuracy = 1.0f;
        damage.affected_by_darkness = false;
        damage.restore = GameData::RestoreType::NONE;
        damage.damage_mp = false;
        damage.drain_hp = false;
        damage.drain_mp = false;
        damage.ignore_status_defense = false;
        damage.critical = false;
        target.selection_enabled = false;
        target.default_enemy = false;
        target.default_multiple = false;
        target.toggle_multiple = false;
        target.fixed_row = false;
        target.short_range = false;
        target.all_rows = false;
        target.random = false;
        target.only_for_dead = false;
        target.no_re_target = false;
        animation.effect = -1;
        animation.impact_effect = -1;
        animation.hurt_anim = -1;
        animation.sound = -1;
        animation.camera_single = -1;
        animation.camera_multiple = -1;
        status_effects.clear();
        elements.clear();
    }
};
