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

#include <vector>
#include "common/File.h"
#include "installer/formats/Attack.h"
#include "installer/formats/Enemy.h"
#include "installer/formats/Formation.h"

/**
 * A battle scene.
 */
class BattleSceneFile{

    public:

        /**
         * Constructor.
         *
         * Reads a scene, with {@see BattleSceneReader}, and converts it, with
         * {@see BattleSceneConverter}.
         *
         * @param[in] file File to read from.
         */
        explicit BattleSceneFile(File file);

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
