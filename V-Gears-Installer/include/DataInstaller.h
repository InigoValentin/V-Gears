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
#include <vector>
#include <iostream>
#include "common/VGearsApplication.h"
#include "common/FinalFantasy7/FF7NameLookup.h"
#include "data/VGearsTriggersFile.h"
#include "data/VGearsFLevelFile.h"
#include "FieldTextWriter.h"
#include "KernelDataInstaller.h"
#include "ModelsAndAnimationsDb.h"
#include "ScopedLgp.h"
#include "SpawnPointDb.h"

// TODO: Separate classes in files.
// TODO: Move implementations to cpp file.

typedef std::set<std::string> MapCollection;

typedef std::map<u16, SpawnPointDb> FieldSpawnPointsMap;

typedef std::map<u16, float> FieldScaleFactorMap;


/**
 * The data installer.
 */
class DataInstaller{

    public:

        /**
         * The scale factor for line point coordinates.
         *
         * When a LINE opcode is found in the game scripts, the line points X and Y coordinates
         * must be scaled down by this factor.
         */
        static float LINE_SCALE_FACTOR;

        /**
         * Installer constructor
         *
         * @param[in] input_dir Path to the directory containing the original
         * data to parse.
         * @param[in] exe_path Path to the ff7.exe file. It's optional and it may be empty.
         * @param[in] output_dir Path to the directory to write generated data
         * to.
         * @param[in] write_output_line Pointer to function to write output.
         */
        DataInstaller(
          const std::string input_dir, const std::string exe_path, const std::string output_dir,
          std::function<void(std::string)> write_output_line,
          std::function<void(std::string)> set_progress_label
        );

        /**
         * Installer destructor.
         */
        ~DataInstaller();

        /**
         * Handle the installation progress.
         *
         * @return Installation progress [0-100].
         */
        int Progress();

    private:

        /**
         * Calculates the installation progress.
         *
         * @return Installation progress [0-100]
         */
        const int CalcProgress();

        /**
         * Creates a directory in the outputh path.
         *
         * @param[in] dir Path of the directory to create, relative to the
         * output path..
         * @throws std::runtime_error If the directory can't be created.
         */
        void CreateDir(const std::string& dir);

        /**
         * Converts a tex file to png.
         *
         * The image is saved in the model data directory.
         *
         * @param[in] name Path to the tex file to convert.
         */
        void TexToPng(const std::string name);

        /**
         * Exports a mesh to a file.
         *
         * The file will have the mesh name.
         *
         * @param[in] outdir Path to the directory where the file will be saved.
         * @param[in] mesh The mesh to export.
         */
        void ExportMesh(const std::string outdir, const Ogre::MeshPtr &mesh);

        /**
         * Initializer for {@see CollectionFieldSpawnAndScaleFactors}.
         *
         * Installation step 1.
         */
        void InitCollectSpawnAndScaleFactors();

        /**
         * Reads spawn points and scale factors from flevel files.
         *
         * Installation step 2.
         */
        void CollectionFieldSpawnAndScaleFactors();

        /**
         * Converts FFVII PC fields to V-Gears format.
         */
        void ConvertFieldsIteration();

        /**
         * Initializer for {@see WriteMapsXmlIteration}.
         *
         * Installation step 3.
         */
        void WriteMapsXmlBegin();

        /**
         * Saves the game maps to XML files.
         *
         * Installation step 4.
         */
        void WriteMapsXmlIteration();

        /**
         * Cleans up after {@see WriteMapsXmlIteration}.
         *
         * Installation step 5.
         */
        void EndWriteMapsXml();

        /**
         * Initializer for {@see ConvertFieldModelsIteration}.
         *
         * Installation step 6.
         */
        void ConvertFieldModelsBegin();

        /**
         * Converts the field models to V-Gears format.
         *
         * Installation step 7 and final.
         */
        void ConvertFieldModelsIteration();

        /**
         * Installation steps.
         */
        enum InstallationSteps{

            /**
             * Installation not started.
             */
            IDLE = 0,

            /**
             * Initializer for {@see SPAWN_POINTS_AND_SCALE_FACTORS}.
             */
            SPAWN_POINTS_AND_SCALE_FACTORS_INIT = 1,

            /**
             * Step that collects spawn points and sclae factors.
             */
            SPAWN_POINTS_AND_SCALE_FACTORS = 2,

            /**
             * Step that convets fields to V-Gears format.
             */
            CONVERT_FIELDS = 3,

            /**
             * Initializer for {@see WRITE_MAPS}.
             */
            WRITE_MAPS_INIT = 4,

            /**
             * Step that writes XML maps.
             */
            WRITE_MAPS = 5,

            /**
             * Clean up after {@see WRITE_MAPS}.
             */
            WRITE_MAPS_CLEAN = 6,

            /**
             * Initializer for {@see CONVERT_FIELD_MODELS}.
             */
            CONVERT_FIELD_MODELS_INIT = 7,

            /**
             * Step that converts field models.
             */
            CONVERT_FIELD_MODELS = 8,

            /**
             * Number of installation steps.
             */
            STATE_COUNT,

            /**
             * Installation finished.
             */
            DONE = 100,
        };

        /**
         * Current installation status.
         */
        InstallationSteps installation_state_ = IDLE;

        /**
         * The path to the directory from which to read the PC game data.
         */
        std::string input_dir_;

        /**
         * The path to original PC game executable.
         */
        std::string exe_path_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

        /**
         * Iterator counter.
         */
        size_t iterator_counter_;

        /**
         * Helper variable to indicate internal progress of installation steps.
         */
        size_t conversion_step_;

        /**
         * Helper variable to indicate internal progress of installation steps.
         */
        size_t progress_step_num_elements_;

        /**
         * The installer application.
         *
         * It's a singleton so it can't be recreated. It will crash the second
         * time round.
         */
        VGears::Application application_;

        /**
         * LGP archive with field data.
         */
        std::unique_ptr<ScopedLgp> fields_lgp_;

        /**
         * The installer for kernel data.
         */
        std::unique_ptr<KernelDataInstaller> kernel_installer_;

        /**
         * LGP archive with texture data.
         */
        std::unique_ptr<ScopedLgp> textures_lgp_;

        /**
         * LGP archive with field model data.
         */
        std::unique_ptr<ScopedLgp> field_models_lgp_;


        /**
         * List of flevel files.
         */
        Ogre::StringVectorPtr flevel_file_list_;

        /**
         * The list of maps.
         */
        std::vector<std::string> map_list_;

        /**
         * Map of the collected spawn points.
         */
        FieldSpawnPointsMap spawn_points_;

        /**
         * Map of th collected scale factors.
         */
        FieldScaleFactorMap scale_factors_;

        /**
         * ModelsAndAnimationsUsedByConvertedFields
         */
        ModelsAndAnimationsDb used_models_and_anims_;

        /**
         * List of converted maps.
         */
        MapCollection converted_map_list_;

        /**
         * Iterator for {@see converted_map_list}.
         */
        MapCollection::iterator converted_map_list_iterator_;

        /**
         * Field currently being processed.
         */
        VGears::FLevelFilePtr field_;

        /**
         * List of model files.
         */
        Ogre::StringVectorPtr field_model_file_list_;

        /**
         * An XML document.
         *
         * Internally used during some installation steps.
         */
        std::unique_ptr<TiXmlDocument> doc_;

        /**
         * An XML element.
         *
         * Internally used during some installation steps.
         */
        std::unique_ptr<TiXmlElement> element_;

        /**
         * Iterator for {@see used_models_and_anims_}.
         */
        ModelAnimationMap::iterator model_animation_map_iterator_;

        /**
         * Function used to print text to the log output, line by line.
         */
        std::function<void(std::string)> write_output_line_;

        /**
         * Function used to print set the current installation progress text.
         */
        std::function<void(std::string)> set_progress_label_;

        /**
         * Field text writer.
         */
        FieldTextWriter field_text_writer_;

        /**
         * Written materials.
         */
        std::vector<std::string> materials_;
};
