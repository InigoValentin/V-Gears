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
#include <OgreMesh.h>

/**
 * Saves the meshes of the battle models, with their skeletons and materials.
 */
class BattleMeshExporter{

    public:

        /**
         * Constructor.
         *
         * @param[in] data_dir Directory of the installed game data.
         */
        explicit BattleMeshExporter(const std::string& data_dir);

        /**
         * Exports a mesh to a file.
         *
         * The file will have the mesh name.
         *
         * @param[in] outdir Folder of the battle models where the file is saved, like "enemies/".
         * @param[in] mesh The mesh to export.
         */
        void Export(const std::string outdir, const Ogre::MeshPtr &mesh);

    private:

        /**
         * Directory of the installed game data.
         */
        std::string data_dir_;

        /**
         * Materials already saved. Each one is saved only once.
         */
        std::vector<std::string> materials_;
};
