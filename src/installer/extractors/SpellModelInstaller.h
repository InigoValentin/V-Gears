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
 * Installs the 3D models of the spells, from magic.lgp.
 */
class SpellModelInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] output_dir Path to the directory of the installation data.
         * @param[in] res_mgr The application resource manager.
         * @param[in] exporter Saves the converted meshes.
         * @param[out] files The files of magic.lgp are read here.
         */
        SpellModelInstaller(
          DiskImage& disk_image, const std::string& output_dir, Ogre::ResourceGroupManager* res_mgr,
          BattleMeshExporter& exporter, std::vector<LgpEntry>& files
        );

        /**
         * Prepares the installer for spell 3D model processing.
         *
         * Reads the contents of the magic.lgp file.
         *
         * @return The total number of battle models to process.
         */
        unsigned int InitializeSpellModels();

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
         * Prepares the installer for spell model conversion.
         *
         * @return The total number of spell models to convert.
         */
        unsigned int ConvertSpellModelsInit();

        /**
         * Converts a spell model.
         *
         * @return The total number of converted models.
         */
        unsigned int ConvertSpellModel();

    private:

        /**
         * Generates all required .rsd models for a model.
         *
         * To be used for spell models (anything in magic.lgp).
         *
         * @param[in] model Model information.
         * @param[in] path Output path.
         */
        void WriteRsdFiles(const SpellModel& model, const std::string& path);

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
         * List of the models found in magic.lgp.
         */
        std::vector<SpellModel> models_;

        /**
         * Next file to process.
         */
        unsigned int next_model_to_process_ = 0;

        /**
         * Next model to convert.
         */
        unsigned int next_model_to_convert_ = 0;
};
