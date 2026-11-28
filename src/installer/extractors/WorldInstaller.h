/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#pragma once

#include <map>
#include <string>
#include <vector>
#include <OgreResourceGroupManager.h>
#include "common/TypeDefine.h"
#include "installer/formats/DiskImage.h"
#include "WorldEncounterInstaller.h"
#include "WorldModelInstaller.h"
#include "WorldScriptInstaller.h"

/**
 * The world map data installer.
 *
 * It copies the maps and their textures, which the game reads as they are. The rest of the work
 * is done by {@see WorldScriptInstaller}, {@see WorldEncounterInstaller} and
 * {@see WorldModelInstaller}.
 */
class WorldInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] output_dir Path to the directory of the installation data.
         * @param[in] res_mgr The application resource manager.
         */
        WorldInstaller(
          const DiskImage& disk_image, const std::string output_dir, Ogre::ResourceGroupManager* res_mgr
        );

        /**
         * Destructor.
         */
        ~WorldInstaller();

        /**
         * Copies the map files and the textures of the terrain.
         */
        void InstallMaps();

        /**
         * Exports the scripts of the world maps and the random encounters.
         *
         * @param[in] field_names The names of the fields, by ID.
         * @param[in] field_scale_factors The scale factors of the fields.
         * @param[in] converted_fields The names of the installed fields.
         * @param[out] error_message Why the scripts could not be written.
         * @return True if the scripts were written.
         */
        bool GenerateScripts(
          const std::vector<std::string>& field_names,
          const std::map<u16, float>& field_scale_factors,
          const std::vector<std::string>& converted_fields, std::string& error_message
        );

        /**
         * Converts the models of the world map.
         */
        void ProcessModels();

    private:

        /**
         * Copies the map files to "world/maps".
         */
        void CopyMaps();

        /**
         * Copies the textures of the world map archive to "world/textures". The game uses them for
         * the terrain.
         */
        void ExtractTextures();

        /**
         * Finds the world map archive in the disk.
         *
         * @return The path of the archive, or an empty string if it's not in the disk.
         */
        std::string ArchivePath();

        /**
         * The disk image to extract the PC game data from.
         */
        DiskImage disk_image_;

        /**
         * Directory where the game data is installed.
         */
        std::string data_dir_;

        /**
         * Writes the scripts of the maps.
         */
        WorldScriptInstaller script_installer_;

        /**
         * Writes the random encounters.
         */
        WorldEncounterInstaller encounter_installer_;

        /**
         * Converts the models.
         */
        WorldModelInstaller model_installer_;
};
