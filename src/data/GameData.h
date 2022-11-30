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

class GameData{

    public:

        class ELEMENT{

            public:

                /**
                 * Fire element.
                 */
                static const unsigned int FIRE = 0;

                /**
                 * Ice element.
                 */
                static const unsigned int ICE = 1;

                /**
                 * Bolt element.
                 */
                static const unsigned int BOLT = 2;

                /**
                 * Earth element.
                 */
                static const unsigned int EARTH = 3;

                /**
                 * Poison element.
                 */
                static const unsigned int POISON = 4;

                /**
                 * Gravity element.
                 */
                static const unsigned int GRAVITY = 5;

                /**
                 * Water element.
                 */
                static const unsigned int WATER = 6;

                /**
                 * Wind element.
                 */
                static const unsigned int WIND = 7;

                /**
                 * Holy element.
                 */
                static const unsigned int HOLY = 8;

                /**
                 * Restorative element.
                 */
                static const unsigned int RESTORATIVE = 9;

                /**
                 * Cut element.
                 */
                static const unsigned int CUT = 10;

                /**
                 * Hit element.
                 */
                static const unsigned int HIT = 11;

                /**
                 * Punch element.
                 */
                static const unsigned int PUNCH = 12;

                /**
                 * Shoot element.
                 */
                static const unsigned int SHOOT = 13;

                /**
                 * Shout element.
                 */
                static const unsigned int SHOUT = 14;

                /**
                 * Hidden element.
                 */
                static const unsigned int HIDDEN = 15;
        };

        /**
         * List of target selection options.
         */
        class TARGET_MODES{

            public:

                /**
                 * The target can be selected.
                 *
                 * If the target of the attack is selectable, at least between some subset of
                 * possible targets, this must be true.
                 */
                static const unsigned int SELECTION_ENABLED = 0;

                /**
                 * The default target is an enemy.
                 *
                 * When true, the cursor starts in an enemy row. When false, it starts in the ally
                 * row. It may or may not be possible to change it.
                 */
                static const unsigned int DEFAULT_ENEMY = 1;

                /**
                 * The default target is a group.
                 *
                 * When true, the cursor starts will all the members of a row selected. When false,
                 * it starts with only one target selected. It may or may not be possible to change
                 * it.
                 */
                static const unsigned int DEFAULT_MULTIPLE = 2;

                /**
                 * The selection can change between groups and individuals.
                 *
                 * When true, the target can be changed between groups and individuals When false,
                 * only {@see default_multiple} determines if the target is an individual or a
                 * group.
                 */
                static const unsigned int TOGGLE_MULTIPLE = 3;

                /**
                 * The target group can be changed.
                 *
                 * When true, the target cannot be changed between enemies and allies. In this case
                 * {@see default_enemy} will determine the group that can be targeted.
                 */
                static const unsigned int FIXED_ROW = 4;

                /**
                 * The command is short ranged.
                 *
                 * When true, if the target or the caster is not in the front of their row, the
                 * target will take half damage. Also, when true, an enemy covered by another enemy
                 * can't be targeted.
                 */
                static const unsigned int SHORT_RANGE = 5;

                /**
                 * Targets all, enemies and allies.
                 *
                 * When true, all allies and enemies are targets, and the selection can't be changed.
                 */
                static const unsigned int ALL_ROWS = 6;

                /**
                 * The target is random.
                 *
                 * When multiple targets are selected, one will be selected at random to be the
                 * receiving target. Cursor will cycle among all viable targets.
                 */
                static const unsigned int RANDOM = 7;

                /**
                 * Retrieves a descriptive string from an option ID.
                 *
                 * @param[in] id Option ID.
                 * @return Option lowercase description. "unknown_target_option" for unknown
                 * options.
                 */
                static std::string to_string(const unsigned int id){
                    switch(id){
                        case SELECTION_ENABLED: return "selection_enabled";
                        case DEFAULT_ENEMY: return "default_enemy";
                        case DEFAULT_MULTIPLE: return "default_multiple";
                        case TOGGLE_MULTIPLE: return "toggle_multiple";
                        case FIXED_ROW: return "fixed_row";
                        case SHORT_RANGE: return "short_range";
                        case ALL_ROWS: return "all_rows";
                        case RANDOM: return "random";
                        default: return "unknown_target_option";
                    }
                }
        };

        /**
         * Determines what an attack (or item) restores.
         */
        enum class RestoreType{

            /**
             * The attack affects a party member or group HP.
             */
            HP = 0,

            /**
             * The attack affects a party member or group MP.
             */
            MP = 1,

            /**
             * The attack affects a party member or group status.
             */
            STATUS = 2,

            /**
             * The attack is not restorative.
             */
            NONE = 3
        };

        /**
         * List of altered status.
         */
        class STATUS{

            public:

                /**
                 * The character is dead.
                 */
                static const unsigned int DEATH = 0;

                /**
                 * The character has critical HP.
                 */
                static const unsigned int NEAR_DEATH = 1;

                /**
                 * The character is asleep.
                 */
                static const unsigned int SLEEP = 2;

                /**
                 * The character is poisoned.
                 */
                static const unsigned int POISONED = 3;

                /**
                 * The character is sad.
                 */
                static const unsigned int SADNESS = 4;

                /**
                 * The character is furious.
                 */
                static const unsigned int FURY = 5;

                /**
                 * The character is confused.
                 */
                static const unsigned int CONFU = 5;

                /**
                 * The character is silenced.
                 */
                static const unsigned int SILENCE = 6;

                /**
                 * The character is slow.
                 */
                static const unsigned int HASTE = 7;

                /**
                 * The character is
                 */
                static const unsigned int SLOW = 8;

                /**
                 * The character is stopped.
                 */
                static const unsigned int STOP = 9;

                /**
                 * The character is a frog.
                 */
                static const unsigned int FROG = 10;

                /**
                 * The character is small.
                 */
                static const unsigned int SMALL = 11;

                /**
                 * The character is going to be petrified.
                 */
                static const unsigned int SLOW_NUMB = 12;

                /**
                 * The character is petrified.
                 */
                static const unsigned int PETRIFY = 13;

                /**
                 * The character is regenerating health.
                 */
                static const unsigned int REGEN = 14;

                /**
                 * The character has a physical barrier.
                 */
                static const unsigned int BARRIER = 15;

                /**
                 * The character has a magical barrier
                 */
                static const unsigned int M_BARRIER = 16;

                /**
                 * The character reflects magical attacks.
                 */
                static const unsigned int REFLECT = 17;

                /**
                 * The character has dual status.
                 *
                 * @todo What does it do.
                 */
                static const unsigned int DUAL = 18;

                /**
                 * The character has a shield.
                 */
                static const unsigned int SHIELD = 19;

                /**
                 * The character has a death sentence.
                 */
                static const unsigned int D_SENTENCE = 20;

                /**
                 * The character is being manipulated.
                 */
                static const unsigned int MANIPULATE = 21;

                /**
                 * The character is berserk.
                 */
                static const unsigned int BERSERK = 22;

                /**
                 * The character is invincible.
                 */
                static const unsigned int PEERLESS = 23;

                /**
                 * The character is paralyzed.
                 */
                static const unsigned int PARALYSIS = 24;

                /**
                 * The character is blinded.
                 */
                static const unsigned int DARKNESS = 25;

                /**
                 * The character has dual drain.
                 */
                static const unsigned int DUAL_DRAIN = 26;

                /**
                 * The character has deathforce.
                 */
                static const unsigned int DEATH_FORCE = 27;

                /**
                 * The character has resist.
                 */
                static const unsigned int RESIST = 28;

                /**
                 * Cait' Sith lucky girl mode.
                 */
                static const unsigned int LUCKY_GIRL = 29;

                /**
                 * The character is imprisioned.
                 */
                static const unsigned int IMPRISONED = 30;
        };

        /**
         * How a status effect modifier attack or item can act.
         */
        class STATUS_EFFECT_MODE{

            public:

                /**
                 * Inflicts status changes.
                 */
                static const unsigned int INFLICT = 0;

                /**
                 * Cures status changes.
                 */
                static const unsigned int CURE = 1;

                /**
                 * Toggles status changes.
                 *
                 * If the target doesn't have the status change, inflict it. If the target already
                 * has the status change, cure it.
                 */
                static const unsigned int TOGGLE = 2;

                /**
                 * Retrieves a descriptive string from an mode ID.
                 *
                 * @param[in] id Mode ID.
                 * @return Mode lowercase description. "unknown_effect_mode" for unknown modes.
                 */
                static std::string to_string(const unsigned int id){
                    switch(id){
                        case INFLICT: return "inflict";
                        case CURE: return "cure";
                        case TOGGLE: return "toggle";
                        default: return "unknown_effect_mode";
                    }
                }
        };
};
