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

#include <string>
#include <vector>
#include "BattleSceneData.h"
#include "installer/formats/Attack.h"
#include "installer/formats/Enemy.h"
#include "installer/formats/Formation.h"

/**
 * Converts the data of a battle scene to enemies, attacks and formations.
 */
namespace BattleSceneConverter{

    /**
     * Retrieves the enemies of a scene.
     *
     * @param[in] data The scene.
     * @return The enemies, each one once.
     */
    std::vector<Enemy> Enemies(const BattleSceneData& data);

    /**
     * Retrieves the attacks of a scene.
     *
     * @param[in] data The scene.
     * @return The attacks, each one once.
     */
    std::vector<Attack> Attacks(const BattleSceneData& data);

    /**
     * Retrieves the formations of a scene.
     *
     * @param[in] data The scene.
     * @return The 4 formations.
     */
    std::vector<Formation> Formations(const BattleSceneData& data);

    /**
     * Converts the elemental rate of an enemy to the factor of the damage.
     *
     * @param[in] rate The rate of the original game.
     * @return The factor.
     */
    float ElementFactor(const u8 rate);

    /**
     * Converts the layout of a formation.
     *
     * @param[in] layout The layout of the original game.
     * @return The layout. Normal if it's unknown.
     */
    Formation::Layout Layout(const u8 layout);

    /**
     * Retrieves the name of a battle location.
     *
     * @param[in] id ID of the location.
     * @return The name.
     */
    std::string LocationName(const unsigned int id);
}
