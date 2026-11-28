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

#include <string>
#include <OgreMesh.h>
#include <OgreResourceGroupManager.h>
#include "WorldArchive.h"

/**
 * Converts the models of the world map, with their animations and textures.
 */
class WorldModelInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] output_dir The installation folder.
         * @param[in] data_dir The game data folder.
         * @param[in] res_mgr The application resource manager.
         */
        WorldModelInstaller(
          const std::string& output_dir, const std::string& data_dir,
          Ogre::ResourceGroupManager* res_mgr
        );

        /**
         * Converts the models of the world map archive.
         *
         * Errors are written to the log.
         *
         * @param[in] archive The world map archive.
         */
        void Install(WorldArchive& archive);

    private:

        /**
         * Extracts all the files of the archive to the temporary folder of the models.
         *
         * @param[in] archive The world map archive.
         * @return The names of the model (HRC) files.
         */
        std::vector<std::string> ExtractFiles(WorldArchive& archive);

        /**
         * Converts a model.
         *
         * @param[in] file_name Name of the HRC file of the model.
         */
        void ConvertModel(const std::string& file_name);

        /**
         * Adds the animations of a model to its skeleton.
         *
         * @param[in] model_name Name of the model, without extension.
         * @param[in,out] skeleton The skeleton.
         */
        static void AddAnimations(const std::string& model_name, Ogre::SkeletonPtr& skeleton);

        /**
         * Exports a mesh, with its skeleton, materials and textures.
         *
         * @param[in] mesh The mesh.
         */
        void ExportMesh(const Ogre::MeshPtr& mesh);

        /**
         * Converts the texture of a texture unit and makes the unit use the converted one.
         *
         * Errors are written to the log.
         *
         * @param[in,out] unit The texture unit.
         * @param[in] base_mesh_name Name of the mesh, without extension.
         */
        void ConvertTexture(Ogre::TextureUnitState& unit, const std::string& base_mesh_name);

        /**
         * Folder where the files of the models are extracted.
         */
        std::string TempDir() const;

        /**
         * Folder of the converted models.
         */
        std::string ModelsDir() const;

        /**
         * The installation folder.
         */
        std::string output_dir_;

        /**
         * The game data folder.
         */
        std::string data_dir_;

        /**
         * The application resource manager.
         */
        Ogre::ResourceGroupManager* res_mgr_;
};
