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

#include <map>
#include <string>
#include <vector>
#include "XmlFile.h"

/**
 * Data of an attack, as stored in the attack XML files.
 */
struct AttackData{

    /**
     * Status effect applied by the attack.
     */
    struct Effect{

        /**
         * Status ID.
         */
        int id;

        /**
         * Chance to apply the effect, between 0 and 1.
         *
         * Negative values are the chance of the opposite (ie, to remove the effect).
         */
        float chance;

        /**
         * Effect mode: "add", "remove" or "toggle".
         */
        std::string mode;
    };

    int id = -1;
    std::string name;
    int mp = 0;
    bool reflectable = false;

    /**
     * Damage parameters ({@see XmlAttackFile} for the names): power, formula, modifier, accuracy,
     * affected_by_darkness, restore, damage_mp, drain_hp, drain_mp, ignore_status_defense,
     * always_critical.
     */
    std::map<std::string, float> damage;

    /**
     * Target parameters: selection_enabled, default_enemy, default_multiple, toggle_multiple,
     * fixed_row, short_range, all_rows, random, only_for_dead, no_retarget.
     */
    std::map<std::string, float> target;

    /**
     * Animation parameters: effect, impact_effect, sound, hurt_anim, camera_single,
     * camera_multiple.
     */
    std::map<std::string, float> animation;

    std::vector<Effect> effects;

    /**
     * Element indexes (0 is fire, 1 is ice, 2 is lightning...).
     */
    std::vector<int> elements;
};

/**
 * Reads an attack XML file.
 */
class XmlAttackFile : public XmlFile{

    public:

        /**
         * Constructor.
         *
         * @param[in] file Path to the attack XML file.
         */
        explicit XmlAttackFile(const Ogre::String& file);

        /**
         * Destructor.
         */
        virtual ~XmlAttackFile();

        /**
         * Parses the file.
         *
         * @param[out] attack The attack data.
         * @return True if the file was read, false if it's not valid.
         */
        bool LoadAttack(AttackData& attack);

        /**
         * Reads the index of attack files by attack ID.
         *
         * @param[in] file Path to the index XML file.
         * @return The file names (without path) by attack ID.
         */
        static std::map<int, std::string> LoadIndex(const Ogre::String& file);

    private:

        /**
         * Reads the Parameter children of a node into a map.
         */
        void ReadParameters(TiXmlNode* node, std::map<std::string, float>& parameters) const;

};
