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
#include <common/File.h>
#include <data/Attack.h>
#include <data/Enemy.h>
#include <data/Formation.h>

class BattleSceneFile{

    public:

        /**
         * Constructor.
         *
         * Reads data from a file.
         *
         * @param[in] id ID of the file, the position at which it is found.
         * @param[in] file File to read from.
         */
        BattleSceneFile(const unsigned int id, File file);

        /**
         * Destructor.
         */
        ~BattleSceneFile();

        /**
         * Retrieves the attacks found in the scene.
         *
         * @return The list of attacks.
         */
        std::vector<Attack> GetAttacks();

        /**
         * Retrieves the enemies found in the scene.
         *
         * @return The list of enemies.
         */
        std::vector<Enemy> GetEnemies();

        /**
         * Retrieves the formations found in the scene.
         *
         * @return The list of formations.
         */
        std::vector<Formation> GetFormations();

    private:

        /**
         * A battle scene structure.
         */
        struct Scene{

            /**
             * Scene battle setup data (20 bytes).
             */
            struct Setup{

                /**
                 * Battle location.
                 */
                u16 location;

                /**
                 * ID of the battle that start upon defeating all enemies.
                 *
                 * If set, a new set of enemies will be loaded when all the current ones are
                 * defeated, without ending the battle scene.
                 */
                u16 next_battle;

                /**
                 * @todo Understand and document.
                 */
                u16 escape_counter;

                /**
                 * Unused data.
                 */
                u16 unused;

                /**
                 * In arena battles, candidate enemy formation IDs for next round.
                 */
                u16 next_arena_candidates[4];

                /**
                 * Battle flags.
                 *
                 * Includes the escapable flag.
                 */
                u16 flags;

                /**
                 * Battle layout.
                 *
                 * 00 - Normal figh.t
                 * 01 - Preemptive.
                 * 02 - Back attack.
                 * 03 - Side attack.
                 * 04 - Attacked from both sides (pincer attack, reverse side attack).
                 * 05 - Another attack from both sides battle (different maybe?).
                 * 06 - Another side attack.
                 * 07 - A third side attack.
                 * 08 - Normal battle that locks player in the front row, change command disabled.
                 */
                u8 layout;

                /**
                 * Indexed Pre-Battle Camera position .
                 */
                u8 camera;
            };

            /**
             * Scene camera placement data.
             */
            struct Camera{

                /**
                 * Position for a camera scene.
                 */
                struct Position{

                    /**
                     * X position.
                     */
                    u16 x;

                    /**
                     * Y position.
                     */
                    u16 y;

                    /**
                     * z position.
                     */
                    u16 z;

                    /**
                     * X direction.
                     */
                    u16 d_x;

                    /**
                     * Y direction.
                     */
                    u16 d_y;

                    /**
                     * z direction.
                     */
                    u16 d_z;
                };

                /**
                 * Camera positions.
                 *
                 * Index 0 is primary, indexes 1 and 2 are secondary positions.
                 */
                Position camera[3];

                /**
                 * Unused data.
                 */
                u8 unused[12];
            };

            /**
             * A scene formation structure.
             */
            struct Formation{

                /**
                 * Enemy ID,
                 */
                u16 id;

                /**
                 * Enemy X position.
                 */
                u16 x;

                /**
                 * Enemy Y position.
                 */
                u16 y;

                /**
                 * Enemy Z position.
                 */
                u16 z;

                /**
                 * Enemy row.
                 */
                u16 row;

                /**
                 * Binary "Cover Flags".
                 *
                 * These flags are used in conjunction with row to determine if a target can be
                 * selected as the target of a short-range attack. The determination of this is
                 * worked out in this way: An enemy exists in row 1 and another in row 2. If the
                 * enemy in row 1 shares a cover flag with the enemy in row 2 then the enemy in row
                 * 2 cannot be targeted until all enemies in row 1 that share a cover flag with the
                 * row 2 enemy is defeated. It works like this. Two active enemies exist, A and B.
                 *
                 * If ((B's row > A's row) and (B's cover flags AND A's cover flags) > 0) then
                 * enemy B cannot be targeted by short-range attacks.
                 */
                u16 cover_flags;

                /**
                 * Initial condition flags. Only last 5 bits are considered.
                 * 0x0001  Visible.
                 * 0x0002  Indicates initial direction facing if players get a side attack.
                 * 0x0004  Unknown.
                 * 0x0008  Targetable.
                 * 0x0010  Main Script Active.
                 */
                u32 flags;
            };

            struct EnemyData{

                /**
                 * Enemy's name (completed by FFh bytes).
                 */
                u8 name[32];

                /**
                 * Enemy's level.
                 */
                u8 level;

                /**
                 * Enemy's speed.
                 */
                u8 spd;

                /**
                 * Enemy's luck.
                 */
                u8 lck;

                /**
                 * Enemy's evade stat.
                 */
                u8 eva;

                /**
                 * Enemy's strength.
                 */
                u8 str;

                /**
                 * Enemy's defense.
                 */
                u8 def;

                /**
                 * Enemy's magic.
                 */
                u8 mag;

                /**
                 * Enemy's magic defense.
                 */
                u8 mdef;

                /**
                 * Elements of the enemy.
                 */
                u8 elements[8];

                /**
                 * Element rates for @{see elements}, respectively.
                 */
                u8 element_rates[8];

                /**
                 * Animation index for each @{see attacks}.
                 */
                u8 animation_index[16];

                /**
                 * Enemy attack IDs.
                 */
                u16 attacks[16];

                /**
                 * Enemy Attacks Camera Movement Id for single and multiple targets (2 bytes each).
                 *
                 * If set this will overwrite camera movement set in attack itself.
                 */
                u16 attack_camera[16];

                /**
                 * Item drop/steal rates.
                 *
                 * These are chances to get items listed in @{see item}. 1 byte per item. If the
                 * rate is lower than 80h, for e.g. 08h - then this is a drop item and has 8/63 [63
                 * is max] chance for drop. But if rate is higher than 80h, let's say... A0h, then
                 * this is an item for steal, and chances for successful steal is
                 * A0h - 80h = 20h = 32/63.
                 */
                u8 item_rate[4];

                /**
                 * IDs of items dropped/stolen from the enemy.
                 *
                 * 0xFFFF means no item.
                 */
                u8 item[4];

                /**
                 * IDs of attacks that can be performed while manipulated.
                 *
                 * The first one is the attack to use in berserk mode.
                 */
                u16 maniputale_attacks[3];

                /**
                 * Unknown data.
                 */
                u16 unknown_0;

                /**
                 * Enemy's MP.
                 */
                u16 mp;

                /**
                 * AP given upon enemy defeat.
                 */
                u16 ap;

                /**
                 * ID of the item the enemy can be morphed into.
                 *
                 * If 0xFFFF, the enemy can't be morphed
                 */
                u16 morph;

                /**
                 * Back damage multiplier.
                 *
                 * damage = damage * back_damage / 8.
                 */
                u8 back_damage;

                /**
                 * Unused data (align).
                 */
                u8 unused;

                /**
                 * Enemy's HP.
                 */
                u32 hp;

                /**
                 * Experience points given upon defeat.
                 */
                u32 exp;

                /**
                 * Money given upon defeat.
                 */
                u32 money;

                /**
                 * Status immunities.
                 *
                 * @todo: a bit set is an immunity?
                 */
                u32 immunities;

                /**
                 * Unknown data.
                 */
                u32 unknown_1;
            };

            /**
             * A battle scene attack data structure.
             */
            struct AttackData{

                /**
                 * Probability of the attack to land. 1 byte.
                 */
                u8 accuracy;

                /**
                 * Impact effect. 1 byte.
                 *
                 * The sprite shown when the attack hits.
                 */
                u8 impact_effect;

                /**
                 * Index of the animation played by the target when hit. 1 byte.
                 */
                u8 hurt_anim;

                /**
                 * Unknown data. 1 bytes.
                 */
                u8 unknown;

                /**
                 * MP cost. 2 bytes.
                 *
                 * Determines the camera movement when the command is executed over a single target.
                 */
                u16 mp;

                /**
                 * Impact sound. 2 bytes.
                 *
                 * Determines the camera movement when the command is executed over multiple targets.
                 */
                u16 sound;

                /**
                 * Camera movement ID for single target. 2 bytes.
                 *
                 * Determines the camera movement when the command is executed over a single target.
                 */
                u16 camera_single;

                /**
                 * Camera movement ID for multiple target. 2 bytes.
                 *
                 * Determines the camera movement when the command is executed over multiple
                 * targets.
                 */
                u16 camera_multiple;

                /**
                 * Targeting mode. 1 byte.
                 */
                u8 target;

                /**
                 * Atack effect ID. 1 byte.
                 *
                 * Used for animation in battle.
                 */
                u8 effect;

                /**
                 * Damage formula. 1 byte.
                 *
                 * This byte is divided into two nybbles (four bits). Upper nybble determines what
                 * considerations are made in calculating damage such as physical/magical, allowing
                 * criticals, how to calculate accuracy, etc. There are also three sets of known
                 * formulae that are paired with the upper nybble values. These are selected in the
                 * Lower Nybble and determine how to calculate pre-defense damage
                 */
                u8 damage;

                /**
                 * The attack power. 1 byte.
                 *
                 * Used for damage/healing calculation.
                 */
                u8 power;

                /**
                 * The restore type.
                 */
                u8 condition;

                /**
                 * Information about status change mode and chance. 1 bytes.
                 */
                u8 status_change;

                /**
                 * Special flags. 1 byte.
                 *
                 * @todo Document and retrieve with info from
                 * https://wiki.ffrtt.ru/index.php?title=FF7/Battle/Attack_Special_Effects
                 */
                u8 additional_effects;

                /**
                 * Special flags. 1 byte.
                 *
                 * @todo Document and retrieve with info from
                 * https://wiki.ffrtt.ru/index.php?title=FF7/Item_data
                 */
                u8 additional_effects_mod;

                /**
                 * Information about what status can be inflicted or cured. 4 bytes.
                 *
                 * The first six bits are the chance of inflicting/curing (out of 63). If the
                 * seventh bit is set, the status is cured instead of inflicted. If the eighth bit
                 * is set, then the status is toggled (cured if present, inflicted if not present).
                 */
                u32 status;

                /**
                 * Information about the item elements. 2 bytes.
                 */
                u16 element;

                /**
                 * Special flags. 2 bytes.
                 *
                 * @todo document and parse with info from
                 * https://wiki.ffrtt.ru/index.php?title=FF7/Battle/Special_Attack_Flags
                 */
                u16 special;

            };

            /**
             * Enemy IDs.
             */
            u16 enemy[3];

            /**
             * Unused data (padding).
             */
            u16 unused;

            /**
             * Battle scene set-ups.
             */
            Setup setup[4];

            /**
             * Camera placement data.
             */
            Camera camera[4];

            /**
             * Battle formation data.
             */
            Formation formation[4][6];

            /**
             * Enemy data.
             */
            EnemyData enemy_data[3];

            /**
             * Attack data.
             */
            AttackData attack_data[32];

            /**
             * Attack IDs.
             */
            u16 attack_ids[32];

            /**
             * Attack names.
             */
            u8 attack_names[32][32];

            // TOOD: AI
        };

        /**
         * Retrieves a location name from it's ID.
         *
         * @param[in] id Location ID.
         * @return The location name, or "UNKNOWN Location" if the ID is invalid.
         */
        static std::string LocationToString(const int id);

        /**
         * Reads data from a file.
         *
         * @param[in] file File to read from.
         */
        void Read(File file);

        /**
         * The file ID.
         */
        unsigned int id_;

        /**
         * The scene data.
         */
        Scene scene_;

        /**
         * Attacks found in the scene.
         */
        std::vector<Attack> attacks_;

        /**
         * Enemies found in the scene.
         */
        std::vector<Enemy> enemies_;

        /**
         * Formations found in the scene.
         */
        std::vector<Formation> formations_;

};
