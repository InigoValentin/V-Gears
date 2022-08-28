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
#include "common/make_unique.h"
#include "common/QGearsApplication.h"
#include "common/FinalFantasy7/FF7NameLookup.h"
#include "data/QGearsTriggersFile.h"
#include "common/QGearsStringUtil.h"
#include "data/QGearsFLevelFile.h"
#include "ff7FieldTextWriter.h"

// TODO: Separate classes in files.
// TODO: Move implementations to cpp file.

/**
 * Handles a scoped LGP archive.
 */
class ScopedLgp{

    public:

        /**
         * Constructor.
         *
         * Don't use directly.
         */
        ScopedLgp(const ScopedLgp&) = delete;

        /**
         * Copy constructor.
         *
         * Don't use directly.
         */
        ScopedLgp& operator = (const ScopedLgp&) = delete;

        /**
         * Constructor.
         */
        ScopedLgp(
          Ogre::Root* root, std::string full_path,
          std::string type, std::string group
        ) : root_(root), full_path_(full_path), group_(group) {
            if (root_){
                std::cout << "[RESOURCE] " << full_path_ << ", "
                  << type << ", " << group_ << std::endl;
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                  full_path_, type, group_
                );
            }
        }

        /**
         * Destructor.
         */
        ~ScopedLgp(){
            if (root_){
                Ogre::ResourceGroupManager::getSingleton()
                  .removeResourceLocation(full_path_, group_);
            }
        }

    private:

        /**
         * The Ogre root system.
         */
        Ogre::Root* root_;

        /**
         * Full path to the LGP archive.
         */
        std::string full_path_;

        /**
         * Group for the resource.
         */
        std::string group_;
};


typedef std::set<std::string> MapCollection;

/**
 * A database of spawn points.
 */
class SpawnPointDb{

    public:
        /**
         * Field IDs.
         *
         * ID of the field the gateways records from N other number of fields
         * are linking to.
         */
        u16 target_field_id = 0;

        /**
         * A spawn point database record.
         */
        class Record{

            public:

                /**
                 * Field that links to {@see SpawnPointDb::target_field_id}.
                 */
                u16 field_id = 0;

                /**
                 * Index of the gateway in {@see field_id}.
                 */
                u32 gateway_index_or_map_jump_address = 0;

                /**
                 * Gateway data.
                 */
                QGears::TriggersFile::Gateway gateway;

                /**
                 * Indicates if the gateway is a map jump from a script.
                 */
                bool from_script = false;

                /**
                 * @todo Understand and document.
                 *
                 * Only used from script calls.
                 */
                std::string entity_name;

                /**
                 * @todo Understand and document.
                 *
                 * Only used from script calls.
                 */
                std::string script_function_name;
        };

        /**
         * List of gateways to this field.
         */
        std::vector<Record> gateways_to_this_field;
};

typedef std::map<u16, SpawnPointDb> FieldSpawnPointsMap;

typedef std::map<u16, float> FieldScaleFactorMap;

typedef std::map<std::string, std::set<std::string>> ModelAnimationMap;

/**
 * Database of model animations.
 */
class ModelsAndAnimationsDb{

    public:

        /**
         * Normalizes an animation name.
         *
         * Generates a normalized file name, without path, lowercase, and an
         * '.a' extension.
         *
         * @param name[in] Current name.
         * @return The normalized name.
         */
        std::string NormalizeAnimationName(const std::string& name){
            Ogre::String base_name;
            QGears::StringUtil::splitBase(name, base_name);
            std::transform(
              base_name.begin(), base_name.end(), base_name.begin(), ::tolower
            );
            return base_name + ".a";
        }

        /**
         * @todo Understand and document.
         *
         * @param model[in] @todo Understand and document.
         * @return @todo Understand and document.
         */
        std::set<std::string>& ModelAnimations(const std::string model){
            // HACK FIX LGP READING
            std::string model_lower = model;
            std::transform(
              model_lower.begin(), model_lower.end(),
              model_lower.begin(), ::tolower
            );
            return map[model_lower];
        }

        /**
         * Retrieves the metadata file name associated to a model.
         *
         * @param model_name[in] The model name.
         * @return Name of the metadata file.
         */
        std::string ModelMetaDataName(const std::string& model_name){
            // If not in meta data then just replace .hrc with .mesh.
            Ogre::String base_name;
            QGears::StringUtil::splitBase(model_name, base_name);
            return QGears::FF7::NameLookup::model(base_name) + ".mesh";
        }

        //private:
        ModelAnimationMap map;
};

/**
 * The data installer.
 */
class FF7DataInstaller{
    public:
        /**
         * Installer constructor
         *
         * @param input_dir[in] Path to the directory containing the original
         * data to parse.
         * @param output_dir[in] Path to the directory to write generated data
         * to.
         * @param write_output_line Pointer to function to write output.
         */
        FF7DataInstaller(
          std::string input_dir, std::string output_dir,
          std::function<void(std::string)> write_output_line
        );

        /**
         * Installer destructor.
         */
        ~FF7DataInstaller();

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
        int CalcProgress();

        /**
         * Creates a directory in the outputh path.
         *
         * @param dir[in] Path of the directory to create, relative to the
         * output path..
         * @throws std::runtime_error If the directory can't be created.
         */
        void CreateDir(const std::string& dir);

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
         * The path to the directory where to sve the V-Gears data.
         */
        std::string output_dir_;

        /**
         * The installer application.
         *
         * It's a singleton so it can't be recreated. It will crash the second
         * time round.
         */
        QGears::Application application_;

        /**
         * LGP archive with field data.
         */
        std::unique_ptr<ScopedLgp> fields_lgp_;

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
         * Field currently being processed.
         */
        QGears::FLevelFilePtr field_;

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
         * Function used to print text.
         */
        std::function<void(std::string)> write_output_line;

        /**
         * Field text writer.
         */
        FF7FieldTextWriter field_text_writer_;

        /**
         * Written materials.
         */
        std::vector<std::string> materials_;
};
