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
#include "BattleSceneData.h"

/**
 * Reads the data of a battle scene, as it is in the file.
 */
namespace BattleSceneReader{

    /**
     * Reads a scene.
     *
     * @param[in] file The file of the scene.
     * @return The data.
     */
    BattleSceneData Read(File file);

    /**
     * Reads the AI scripts of the enemies of a scene.
     *
     * Blocks and scripts that go past the end of the scene are left out.
     *
     * @param[in] bytes The whole scene.
     * @param[in,out] data The scripts are set in it.
     */
    void ReadAiScripts(const std::vector<u8>& bytes, BattleSceneData& data);
}
