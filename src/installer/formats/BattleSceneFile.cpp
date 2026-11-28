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

#include "BattleSceneConverter.h"
#include "BattleSceneFile.h"
#include "BattleSceneReader.h"

BattleSceneFile::BattleSceneFile(File file){
    const BattleSceneData data = BattleSceneReader::Read(file);
    enemies_ = BattleSceneConverter::Enemies(data);
    attacks_ = BattleSceneConverter::Attacks(data);
    formations_ = BattleSceneConverter::Formations(data);
}

std::vector<Attack> BattleSceneFile::GetAttacks(){return attacks_;}

std::vector<Enemy> BattleSceneFile::GetEnemies(){return enemies_;}

std::vector<Formation> BattleSceneFile::GetFormations(){return formations_;}
