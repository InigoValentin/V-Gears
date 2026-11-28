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
#include "installer/extractors/BattleEffectInstaller.h"
#include "installer/extractors/BattleMeshExporter.h"
#include "installer/extractors/BattleModelInstaller.h"
#include "installer/extractors/BattleSceneInstaller.h"
#include "installer/extractors/SpellModelInstaller.h"
#include "installer/formats/DiskImage.h"

/**
 * The battle data installer.
 *
 * It has an installer for each kind of battle data, and passes the calls on to them.
 */
class BattleDataInstaller{

    public:

        /**
         * Constructor
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] output_dir Path to the directory of the installation data.
         * @param[in] res_mgr The application resource manager..
         */
        BattleDataInstaller(
          const DiskImage& disk_image, const std::string output_dir, Ogre::ResourceGroupManager* res_mgr
        );

        /**
         * Destructor.
         */
        ~BattleDataInstaller();

        /**
         * Prepares the installer for scene processing.
         *
         * @return The total number of scenes to process.
         */
        unsigned int InitializeScenes();

        /**
         * Prepares the installer for battle 3D model processing.
         *
         * Reads the contents of the battle.lgp file.
         *
         * @return The total number of battle models to process.
         */
        unsigned int InitializeBattleModels();

        /**
         * Prepares the installer for spell 3D model processing.
         *
         * Reads the contents of the magic.lgp file.
         *
         * @return The total number of battle models to process.
         */
        unsigned int InitializeSpellModels();

        /**
         * Processes the next battle scene.
         *
         * @return The total number of processed scenes.
         */
        unsigned int ProcessScene();

        /**
         * Processes the next battle 3D model.
         *
         * Saves .hrc, .a and .p files for later conversion. .tex files are
         * converted directly to png.
         *
         * @return The total number of processed models.
         */
        unsigned int ProcessBattleModel();

        /**
         * Processes the next spell 3D model.
         *
         * Saves .hrc, .a and .p files for later conversion. .tex files are
         * converted directly to png.
         *
         * @return The total number of processed models.
         */
        unsigned int ProcessSpellModel();

        /**
         * Prepares the installer for battle model conversion.
         *
         * @return The total number of battle models to convert.
         */
        unsigned int ConvertBattleModelsInit();

        /**
         * Prepares the installer for spell model conversion.
         *
         * @return The total number of spell models to convert.
         */
        unsigned int ConvertSpellModelsInit();

        /**
         * Converts a model.
         *
         * @return The total number of converted models.
         */
        unsigned int ConvertBattleModel();

        /**
         * Converts a spell model.
         *
         * @return The total number of converted models.
         */
        unsigned int ConvertSpellModel();

        /**
         * Extracts the assets of the battle effects of magic.lgp: meshes, textures and sprite files.
         *
         * Effects are in folders of the archive, and files of different folders can have the same
         * name. Each file is saved with the folder as a prefix, so the names are unique, and an
         * effect model list is made with the meshes that can be converted.
         * {@see InitializeSpellModels} must be called before.
         *
         * @return The number of effect models to convert.
         */
        unsigned int InitializeEffectModels();

        /**
         * Converts an effect model to an Ogre mesh and its textures to PNG.
         *
         * @return The total number of converted effect models.
         */
        unsigned int ConvertEffectModel();

        /**
         * Writes the list of the converted effect models to battle/effects/effects.lua.
         */
        void WriteEffectModels();

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

        /**
         * Writes playable character data.
         */
        void WriteCharacterData();

        /**
         * Writes battle scene data.
         */
        void WriteSceneData();

    private:

        /**
         * The disk image to extract the data from.
         */
        DiskImage disk_image_;

        /**
         * Saves the meshes of all the models.
         */
        BattleMeshExporter exporter_;

        /**
         * The files of magic.lgp, shared by the spells and the effects.
         */
        std::vector<LgpEntry> magic_files_;

        /**
         * Scenes: enemies, attacks and formations.
         */
        BattleSceneInstaller scenes_;

        /**
         * Models of battle.lgp.
         */
        BattleModelInstaller models_;

        /**
         * Models of the spells.
         */
        SpellModelInstaller spells_;

        /**
         * Assets of the effects.
         */
        BattleEffectInstaller effects_;
};
