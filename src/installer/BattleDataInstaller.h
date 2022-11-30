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

#include "common/BinGZipFile.h"
#include "common/TypeDefine.h"
#include "data/BattleSceneFile.h"

/**
 * The battle data installer.
 */
class BattleDataInstaller{


    public:

        /**
         * Constructor
         *
         * @param[in] input_dir Path to the directory containing the original data to parse.
         * @param[in] output_dir Path to the directory of the installation data.
         */
        BattleDataInstaller(const std::string input_dir, const std::string output_dir);

        /**
         * Destructor.
         */
        ~BattleDataInstaller();

        /**
         * Prepares the installer.
         *
         * @return The total number of scenes.
         */
        unsigned int Initialize();

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
         * @return Uncompressed file. NULL if errors were encountered (they will be logged to
         * console).
         */
        File* ExtractGZipScene(File file);

        /**
         * Constructs a file name for the XML file to save an enemy to.
         *
         * The filename will be "dame/enemy/[ID]_[NAME].xml" where [ID] is the enemy ID, padded
         * with '0' to a length of 4, and [NAME] is the enemy name using only alphanumeric
         * characters.
         *
         * @param[in] enemy The enemy to be saved.
         * @return The name for the enemy file.
         */
        std::string BuildEnemyFileName(Enemy enemy);

        /**
         * Constructs a file name for the XML file to save an attack to.
         *
         * The filename will be "dame/attack/[ID]_[NAME].xml" where [ID] is the attack ID, padded
         * with '0' to a length of 4, and [NAME] is the attack name using only alphanumeric
         * characters.
         *
         * @param[in] attack The attack to be saved.
         * @return The name for the attack file.
         */
        std::string BuildAttackFileName(Attack attack);

        /**
         * The path to the directory from which to read the PC game data.
         */
        std::string input_dir_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

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
        std::vector<File*> scenes_uncompressed_;

        /**
         * Processed scenes.
         */
        std::vector<BattleSceneFile> scenes_;

        /**
         * Enemy data read from the scenes;
         */
        std::vector<Enemy> enemies_;

        /**
         * Map of enemies, ID and xml file path.
         */
        std::map<int, std::string> enemy_map_;

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

        /**
         * Map of formations, ID and xml file path.
         */
        std::map<int, std::string> formation_map_;

};
