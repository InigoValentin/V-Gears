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

#pragma once

#include <vector>
#include "common/TypeDefine.h"

/*
 * Types of the kernel data that several kinds of data use.
 */
namespace KernelData{

    enum ELEMENT{

        /**
         * Fire element.
         */
        FIRE = 0,


        /**
         * Ice element.
         */
        ICE,


        /**
         * Bolt element.
         */
        BOLT,


        /**
         * Earth element.
         */
        EARTH,


        /**
         * Poison element.
         */
        POISON,


        /**
         * Gravity element.
         */
        GRAVITY,


        /**
         * Water element.
         */
        WATER,


        /**
         * Wind element.
         */
        WIND,


        /**
         * Holy element.
         */
        HOLY,


        /**
         * Restorative element.
         */
        RESTORATIVE,


        /**
         * Cut element.
         */
        CUT,


        /**
         * Hit element.
         */
        HIT,


        /**
         * Punch element.
         */
        PUNCH,


        /**
         * Shoot element.
         */
        SHOOT,


        /**
         * Shout element.
         */
        SHOUT,


        /**
         * Hidden element.
         */
        HIDDEN,
    };

    /**
     * List of target selection options.
     */
    struct Target{

        /**
         * The target can be selected.
         *
         * If the target of the attack is selectable, at least between some
         * subset of possible targets, this must be true.
         */
        bool selection_enabled;

        /**
         * The default target is an enemy.
         *
         * When true, the cursor starts in an enemy row. When false, it
         * starts in the ally row. It may or may not be possible to change
         * it.
         */
        bool default_enemy;

        /**
         * The default target is a group.
         *
         * When true, the cursor starts will all the members of a row
         * selected. When false, it starts with only one target selected. It
         * may or may not be possible to change it.
         */
        bool default_multiple;

        /**
         * The selection can change between groups and individuals.
         *
         * When true, the target can be changed between groups and
         * individuals. When false, only {@see default_multiple} determines
         * if the target is an individual or a group.
         */
        bool toggle_multiple;

        /**
         * The target group can be changed.
         *
         * When true, the target cannot be changed between enemies and
         * allies. In this case {@see default_enemy} will determine the
         * group that can be targeted.
         */
        bool fixed_row;

        /**
         * The command is short ranged.
         *
         * When true, if the target or the caster is not in the front of
         * their row, the target will take half damage. Also, when true, an
         * enemy covered by another enemy can't be targeted.
         */
        bool short_range;

        /**
         * Targets all, enemies and allies.
         *
         * When true, all allies and enemies are targets, and the selection
         * can't be changed.
         */
        bool all_rows;

        /**
         * The target is random.
         *
         * When multiple targets are selected, one will be selected at
         * random to be the receiving target. Cursor will cycle among all
         * viable targets.
         */
        bool random;

    };

    /**
     * Determines what an item affects on when used from the menu.
     */
    enum RESTORE_TYPE{

        /**
         * The item affects a party member or group HP.
         */
        RESTORE_HP = 0,

        /**
         * The item affects a party member or group MP.
         */
        RESTORE_MP = 1,

        /**
         * The item affects a party member or group status.
         */
        RESTORE_STATUS = 2,

        /**
         * The item is not restorative.
         */
        RESTORE_NONE = 3
    };

    /**
     * List of altered status.
     */
    enum STATUS{

        /**
         * The character is dead.
         */
        DEATH = 0,

        /**
         * The character has critical HP.
         */
        NEAR_DEATH,

        /**
         * The character is asleep.
         */
        SLEEP,

        /**
         * The character is poisoned.
         */
        POISONED,

        /**
         * The character is sad.
         */
        SADNESS,

        /**
         * The character is furious.
         */
        FURY,

        /**
         * The character is confused.
         */
        CONFU,

        /**
         * The character is silenced.
         */
        SILENCE,

        /**
         * The character is slow.
         */
        HASTE,

        /**
         * The character is
         */
        SLOW,

        /**
         * The character is stopped.
         */
        STOP,

        /**
         * The character is a frog.
         */
        FROG,

        /**
         * The character is small.
         */
        SMALL,

        /**
         * The character is going to be petrified.
         */
        SLOW_NUMB,

        /**
         * The character is petrified.
         */
        PETRIFY,

        /**
         * The character is regenerating health.
         */
        REGEN,

        /**
         * The character has a physical barrier.
         */
        BARRIER,

        /**
         * The character has a magical barrier
         */
        M_BARRIER,

        /**
         * The character reflects magical attacks.
         */
        REFLECT,

        /**
         * The character has dual status.
         *
         * @todo What does it do.
         */
        DUAL,

        /**
         * The character has a shield.
         */
        SHIELD,

        /**
         * The character has a death sentence.
         */
        D_SENTENCE,

        /**
         * The character is being manipulated.
         */
        MANIPULATE,

        /**
         * The character is berserk.
         */
        BERSERK,

        /**
         * The character is invincible.
         */
        PEERLESS,

        /**
         * The character is paralyzed.
         */
        PARALYSIS,

        /**
         * The character is blinded.
         */
        DARKNESS,

        /**
         * The character has dual drain.
         */
        DUAL_DRAIN,

        /**
         * The character has deathforce.
         */
        DEATH_FORCE,

        /**
         * The character has resist.
         */
        RESIST,

        /**
         * Cait' Sith lucky girl mode.
         */
        LUCKY_GIRL,

        /**
         * The character is imprisioned.
         */
        IMPRISONED
    };

    /**
     * How a status effect modifier attack or item can act.
     */
    enum STATUS_EFFECT{

        /**
         * Inflicts status changes.
         */
        INFLICT = 0,

        /**
         * Cures status changes.
         */
        CURE = 1,

        /**
         * Toggles status changes.
         *
         * If the target doesn't have the status change, inflict it. If the
         * target already has the status change, cure it.
         */
        TOGGLE = 2
    };

    struct StatusEffect{

        /**
         * The list of status to inflict or cure.
         */
        std::vector<int> status;

        /**
         * The chance of landing a status effect (out of 63).
         */
        u16 chance;

        /**
         * The status change mode.
         */
        STATUS_EFFECT mode;

    };

    /**
     * Materia slot types.
     */
    enum SLOT_TYPE{

        /**
         * A non linked slot.
         */
        SLOT_UNLINKED = 0,

        /**
         * A linked slot.
         *
         * If the slot is in an even position, it's linked to the one in the
         * right. If it's in an odd position, it's linked to the one in the
         * left. Remember that slot positions are 0-index.
         */
        SLOT_LINKED = 1
    };

    /**
     * Bonus in stat givven by weapons or armor.
     */
    struct StatBonus{

        /**
         * ID of the raised stat.
         */
        u8 stat;

        /**
         * Stat value raise.
         */
        u8 bonus;
    };
}
