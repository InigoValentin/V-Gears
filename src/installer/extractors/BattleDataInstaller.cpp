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

#include "installer/extractors/BattleDataInstaller.h"

BattleDataInstaller::BattleDataInstaller(
  const DiskImage& disk_image, const std::string output_dir, Ogre::ResourceGroupManager* res_mgr
):
  disk_image_(disk_image),
  exporter_(output_dir + "game/"),
  scenes_(disk_image_, output_dir + "game/"),
  models_(disk_image_, output_dir, res_mgr, exporter_),
  spells_(disk_image_, output_dir, res_mgr, exporter_, magic_files_),
  effects_(disk_image_, output_dir, res_mgr, exporter_, magic_files_)
{}

BattleDataInstaller::~BattleDataInstaller(){}

unsigned int BattleDataInstaller::InitializeScenes(){return scenes_.InitializeScenes();}

unsigned int BattleDataInstaller::InitializeBattleModels(){return models_.InitializeBattleModels();}

unsigned int BattleDataInstaller::InitializeSpellModels(){return spells_.InitializeSpellModels();}

unsigned int BattleDataInstaller::ProcessScene(){return scenes_.ProcessScene();}

unsigned int BattleDataInstaller::ProcessBattleModel(){return models_.ProcessBattleModel();}

unsigned int BattleDataInstaller::ProcessSpellModel(){return spells_.ProcessSpellModel();}

unsigned int BattleDataInstaller::ConvertBattleModelsInit(){
    return models_.ConvertBattleModelsInit();
}

unsigned int BattleDataInstaller::ConvertSpellModelsInit(){return spells_.ConvertSpellModelsInit();}

unsigned int BattleDataInstaller::ConvertBattleModel(){return models_.ConvertBattleModel();}

unsigned int BattleDataInstaller::ConvertSpellModel(){return spells_.ConvertSpellModel();}

unsigned int BattleDataInstaller::InitializeEffectModels(){
    return effects_.InitializeEffectModels();
}

unsigned int BattleDataInstaller::ConvertEffectModel(){return effects_.ConvertEffectModel();}

void BattleDataInstaller::WriteEffectModels(){effects_.WriteEffectModels();}

void BattleDataInstaller::WriteEnemies(){scenes_.WriteEnemies();}

void BattleDataInstaller::WriteAttacks(){scenes_.WriteAttacks();}

void BattleDataInstaller::WriteFormations(){scenes_.WriteFormations();}

void BattleDataInstaller::WriteCharacterData(){models_.WriteCharacterData();}

void BattleDataInstaller::WriteSceneData(){models_.WriteSceneData();}
