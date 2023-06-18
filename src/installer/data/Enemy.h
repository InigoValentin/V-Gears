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

struct Enemy{

    struct Element{

        /**
         * The element ID.
         */
        u8 id;

        /**
         * Damage modification factor when attacked by the element.
         *
         * 1 means normal damage taken.
         * 0 means no damage taken.
         * [0-1] means reduced damage.
         * [1-10) means extended damage.
         * (-10-0) means recovery
         * >10 means death.
         * <-10 means full recovery.
         */
        float factor;
    };

    /**
     * Enemy attack data.
     */
    struct Attack{

        /**
         * Attack ID.
         */
        u16 id;

        /**
         * Camera ID to use during the attack.
         *
         * -1 to not move the camera.
         */
        int camera;
    };

    /**
     * Data for drop and steal items.
     */
    struct Item{

        /**
         * Item ID.
         */
        unsigned int id;

        /**
         * Steal or drop rate.
         */
        float rate;
    };

    /**
     * Status immunity.
     */
    struct Immunity{

        /**
         * Status ID.
         */
        unsigned int status;

        /**
         * Immunity rate.
         *
         * 0 or lower means no immunity, 1 or higher completely immune. Values between 0 and 1
         * indicate resistance.
         */
        float rate;
    };

    /**
     * The enemy ID.
     */
    u32 id;

    /**
     * The enemy name.
     */
    std::string name;

    /**
     * The enemy level.
     */
    u8 level;

    /**
     * The enemy strength stat.
     */
    u8 str;

    /**
     * The enemy magic stat.
     */
    u8 mag;

    /**
     * The enemy defense stat.
     */
    u8 def;

    /**
     * The enemy magic defense stat.
     */
    u8 mdef;

    /**
     * The enemy speed stat.
     */
    u8 spd;

    /**
     * The enemy luck stat.
     */
    u8 lck;

    /**
     * The enemy evasion stat.
     */
    u8 eva;

    /**
     * The enemy HP.
     */
    u32 hp;

    /**
     * The enemy MP.
     */
    u32 mp;

    /**
     * Experience given upon defeat.
     */
    u32 exp;

    /**
     * AP given upon defeat.
     */
    u32 ap;

    /**
     * Money given upon defeat.
     */
    u32 money;

    /**
     * Back damage multiplier.
     */
    float back_damage;

    /**
     * List of animations.
     */
    std::vector<unsigned int> animations;

    /**
     * Elemental affinities.
     */
    std::vector<Element> elements;

    /**
     * Status immunities and resistances.
     */
    std::vector<Immunity> immunities;

    /**
     * Enemy attacks.
     */
    std::vector<Attack> attacks;

    /**
     * List of attacks that can be used while manipulated.
     *
     * The first one is also the one used in berserkr. If empty, it means the enemy can't be
     * manipulated or berserkd.
     */
    std::vector<u16> manipulate_attacks;

    /**
     * List of the items that can be stolen from the enemy.
     */
    std::vector<Item> steal;

    /**
     * List of the items that can be dropped from the enemy.
     */
    std::vector<Item> drop;

    /**
     * Two-letter model code.
     */
    std::string model;

    /**
     * ID of the item the enemy can be morphed into.
     *
     * If -1, it can't be morphed.
     */
    unsigned int morph;

    /**
     * Constructor.
     *
     * Initializes to default values.
     */
    Enemy():
      id(0), name(""), level(1), str(1), mag(1), def(1), mdef(1), spd(1), lck(1), eva(1), hp(1),
      mp(1), exp(0), ap(0), money(0), back_damage(1.0f), morph(-1)
    {
        animations.clear();
        elements.clear();
        immunities.clear();
        attacks.clear();
        manipulate_attacks.clear();
        steal.clear();
        drop.clear();
    }
};
