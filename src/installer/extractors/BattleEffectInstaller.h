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
#include <OgreResourceGroupManager.h>
#include "installer/extractors/BattleMeshExporter.h"
#include "installer/extractors/BattleModelFiles.h"
#include "installer/extractors/BattleModelTools.h"
#include "installer/formats/DiskImage.h"

/**
 * Installs the assets of the battle effects: the models and sprites of magic.lgp, and the meshes
 * and cameras that are in ff7.exe.
 */
class BattleEffectInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] output_dir Path to the directory of the installation data.
         * @param[in] res_mgr The application resource manager.
         * @param[in] exporter Saves the converted meshes.
         * @param[in] files The files of magic.lgp, read before the effects are installed.
         */
        BattleEffectInstaller(
          DiskImage& disk_image, const std::string& output_dir, Ogre::ResourceGroupManager* res_mgr,
          BattleMeshExporter& exporter, std::vector<LgpEntry>& files
        );

        /**
         * Extracts the assets of the battle effects of magic.lgp: meshes, textures and sprite files.
         *
         * Effects are in folders of the archive, and files of different folders can have the same
         * name. Each file is saved with the folder as a prefix, so the names are unique, and an
         * effect model list is made with the meshes that can be converted.
         * The files of magic.lgp must have been read before.
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

    private:

        /**
         * A static mesh of a battle effect.
         */
        struct EffectModel{

            /**
             * Name of the model, unique: folder and name of the original rsd file.
             */
            std::string name;

            /**
             * Names of the textures, with the same prefix as the model.
             */
            std::vector<std::string> textures;

            /**
             * True if the model was converted.
             */
            bool converted = false;
        };

        /**
         * Converts the textures of the effects and writes the sprite frame lists to
         * battle/effects/effect_sprites.lua. Called by {@see WriteEffectModels}.
         */
        void WriteEffectSprites();

        /**
         * Reads the camera scripts of the battle effects from ff7.exe and writes them to
         * battle/effects/effect_cameras.lua. Called by {@see WriteEffectModels}.
         */
        void WriteEffectCameras();

        /**
         * Reads the camera scripts of the battles (camdat0-2.bin) and writes them to
         * battle/battle_cameras.lua. Called by {@see WriteEffectModels}.
         */
        void WriteBattleCameras();

        /**
         * Reads the meshes of the battle effects that are in the data of ff7.exe (the ground of Ifrit,
         * the mountain of Ramuh...) and writes them to battle/effects/effect_meshes.lua. Called by
         * {@see WriteEffectModels}.
         */
        void WriteEffectMeshes();

        /**
         * The disk image to extract the data from.
         */
        DiskImage& disk_image_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

        /**
         * Directory where the game data is installed (<install dir>/data/).
         */
        std::string data_dir_;

        /**
         * Pointer to the application resource manager.
         */
        Ogre::ResourceGroupManager* res_mgr_;

        /**
         * Saves the converted meshes.
         */
        BattleMeshExporter& exporter_;

        /**
         * The files in the original magic.lgp file.
         */
        std::vector<LgpEntry>& files_;

        /**
         * The effect models.
         */
        std::vector<EffectModel> effect_models_;

        /**
         * The next effect model to convert.
         */
        size_t next_effect_to_convert_ = 0;
};
