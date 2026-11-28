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

#include <functional>
#include <string>
#include <vector>
#include <OgreMesh.h>
#include "installer/formats/DiskImage.h"
#include "ModelsAndAnimationsDb.h"

/**
 * Converts the models that the fields use, with their animations and textures.
 */
class FieldModelInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] disk_image The image of the game disk.
         * @param[in] output_dir The installation folder.
         * @param[in] data_dir The game data folder.
         * @param[in] used_models The models that the fields use, and their animations.
         * @param[in] log_output_line Function to write a line to the installation log.
         */
        FieldModelInstaller(
          const DiskImage& disk_image, const std::string& output_dir, const std::string& data_dir,
          ModelsAndAnimationsDb& used_models, std::function<void(std::string)> log_output_line
        );

        /**
         * Extracts the files of the models and lists the models to convert.
         *
         * @return The names of the models that the fields use.
         */
        std::vector<std::string> Initialize();

        /**
         * Converts a model and the animations that the fields use.
         *
         * Errors are written to the log.
         *
         * @param[in] model_name Name of the model, as returned by {@see Initialize}.
         */
        void ConvertModel(const std::string& model_name);

    private:

        /**
         * Extracts the files of char.lgp to the temporary folder of the models.
         */
        void ExtractModelFiles();

        /**
         * Makes the bounds of a mesh hold the positions the animations move its root to.
         *
         * @param[in,out] mesh The mesh.
         * @param[in] bind_bounds The bounds of the mesh, without animations.
         * @param[in] translation_bounds The bounds of the root translations of the animations.
         */
        static void ExtendBounds(
          const Ogre::MeshPtr& mesh, const Ogre::AxisAlignedBox& bind_bounds,
          const Ogre::AxisAlignedBox& translation_bounds
        );

        /**
         * Exports a mesh, with its skeleton, materials and textures.
         *
         * @param[in] outdir Folder to export to.
         * @param[in] mesh The mesh.
         */
        void ExportMesh(const std::string& outdir, const Ogre::MeshPtr& mesh);

        /**
         * Converts the texture of a texture unit and makes the unit use the converted one.
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
         * The image of the game disk.
         */
        DiskImage disk_image_;

        /**
         * The installation folder.
         */
        std::string output_dir_;

        /**
         * The game data folder.
         */
        std::string data_dir_;

        /**
         * The models that the fields use, and their animations.
         */
        ModelsAndAnimationsDb& used_models_;

        /**
         * Function to write a line to the installation log.
         */
        std::function<void(std::string)> write_output_line_;

        /**
         * The materials already exported.
         */
        std::vector<std::string> materials_;
};
