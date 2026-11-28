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

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <tinyxml.h>
#include "common/File.h"
#include "common/TypeDefine.h"
#include "installer/formats/BattleSceneFile.h"
#include "installer/formats/BinGZipFile.h"
#include "installer/formats/DiskImage.h"

/**
 * Installs the data of the battle scenes (scene.bin): enemies, attacks and formations.
 */
class BattleSceneInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] data_dir Directory of the installed game data.
         */
        BattleSceneInstaller(DiskImage& disk_image, const std::string& data_dir);

        /**
         * Prepares the installer for scene processing.
         *
         * @return The total number of scenes to process.
         */
        unsigned int InitializeScenes();

        /**
         * Processes the next battle scene.
         *
         * @return The total number of processed scenes.
         */
        unsigned int ProcessScene();

        /**
         * Writes enemy data to files.
         */
        void WriteEnemies();

        /**
         * Writes attack data to files.
         */
        void WriteAttacks();

        /**
         * Writes formation data to files.
         */
        void WriteFormations();

    private:

        /**
         * Extract a gzipped file.
         *
         * @param[in] file Compressed file.
         * @return Uncompressed file. NULL if errors were encountered (they will
         * be logged to console).
         */
        std::unique_ptr<File> ExtractGZipScene(const File& file);

        /**
         * Constructs a file name for the XML file to save an enemy to.
         *
         * The filename will be "dame/enemy/[ID]_[NAME].xml" where [ID] is the
         * enemy ID, padded with '0' to a length of 4, and [NAME] is the enemy
         * name using only alphanumeric characters.
         *
         * @param[in] enemy The enemy to be saved.
         * @return The name for the enemy file.
         */
        std::string BuildEnemyFileName(Enemy enemy);

        /**
         * Constructs a file name for the XML file to save an attack to.
         *
         * The filename will be "dame/attack/[ID]_[NAME].xml" where [ID] is the
         * attack ID, padded with '0' to a length of 4, and [NAME] is the attack
         * name using only alphanumeric characters.
         *
         * @param[in] attack The attack to be saved.
         * @return The name for the attack file.
         */
        std::string BuildAttackFileName(Attack attack);

        /**
         * Adds the damage of an attack.
         *
         * @param[in] parent Element to add it to.
         * @param[in] attack The attack.
         */
        static void AddAttackDamage(TiXmlElement& parent, const Attack& attack);

        /**
         * Adds who an attack can target.
         *
         * @param[in] parent Element to add it to.
         * @param[in] attack The attack.
         */
        static void AddAttackTarget(TiXmlElement& parent, const Attack& attack);

        /**
         * Adds how an attack is shown.
         *
         * @param[in] parent Element to add it to.
         * @param[in] attack The attack.
         */
        static void AddAttackAnimation(TiXmlElement& parent, const Attack& attack);

        /**
         * Adds the stats of an enemy.
         *
         * @param[in] parent Element to add it to.
         * @param[in] enemy The enemy.
         */
        static void AddEnemyStats(TiXmlElement& parent, const Enemy& enemy);

        /**
         * Adds the elemental factors and the status immunities of an enemy.
         *
         * @param[in] parent Element to add it to.
         * @param[in] enemy The enemy.
         */
        static void AddEnemyResistances(TiXmlElement& parent, const Enemy& enemy);

        /**
         * Adds the attacks of an enemy, and the ones it has when manipulated.
         *
         * @param[in] parent Element to add it to.
         * @param[in] enemy The enemy.
         */
        static void AddEnemyAttacks(TiXmlElement& parent, const Enemy& enemy);

        /**
         * Adds a list of items of an enemy.
         *
         * @param[in] parent Element to add it to.
         * @param[in] name Name of the list.
         * @param[in] items The items.
         */
        static void AddEnemyItems(
          TiXmlElement& parent, const char* name, const std::vector<Enemy::Item>& items
        );

        /**
         * Adds the AI scripts of an enemy.
         *
         * @param[in] parent Element to add it to.
         * @param[in] enemy The enemy.
         */
        static void AddEnemyScripts(TiXmlElement& parent, const Enemy& enemy);

        /**
         * Directory of the installed game data.
         */
        std::string data_dir_;

        /**
         * Next scene file to process;
         */
        unsigned int next_scene_;

        /**
         * Number of scenes in SCENE.BIN;
         */
        unsigned int total_scenes_;

        /**
         * The original scene.bin file.
         */
        File scene_bin_;

        /**
         * List of uncompressed scenes.
         */
        std::vector<std::unique_ptr<File>> scenes_uncompressed_;

        /**
         * Processed scenes.
         */
        std::vector<BattleSceneFile> scenes_;

        /**
         * Enemy data read from the scenes;
         */
        std::vector<Enemy> enemies_;

        /**
         * Attack data read from the scenes;
         */
        std::vector<Attack> attacks_;

        /**
         * Map of attacks, ID and xml file path.
         */
        std::map<int, std::string> attack_map_;

        /**
         * Formations data read from the scenes;
         */
        std::vector<Formation> formations_;
};
