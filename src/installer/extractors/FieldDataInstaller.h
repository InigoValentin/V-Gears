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
#include <memory>
#include <string>
#include <vector>
#include <OgreResourceGroupManager.h>
#include <tinyxml.h>
#include "formats/field/FLevelFile.h"
#include "installer/formats/DiskImage.h"
#include "FieldBackgroundInstaller.h"
#include "FieldDataTypes.h"
#include "FieldMapWriter.h"
#include "FieldModelInstaller.h"
#include "FieldScriptInstaller.h"
#include "ModelsAndAnimationsDb.h"

/**
 * Installs the fields: their scripts, texts, maps, backgrounds and walkmeshes, the list of fields
 * and the models they use.
 *
 * The work is done by {@see FieldScriptInstaller}, {@see FieldMapWriter},
 * {@see FieldBackgroundInstaller} and {@see FieldModelInstaller}. This class collects the data
 * that they need from every field, and runs them in steps.
 */
class FieldDataInstaller{

    public:

        /**
         * Installer constructor
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] output_dir Path to the directory to write generated data to.
         * @param[in] log_output_line Function to write a line to the installation log.
         */
        FieldDataInstaller(
          const DiskImage& disk_image, const std::string output_dir,
          std::function<void(std::string)> log_output_line
        );

        /**
         * Installer destructor.
         */
        ~FieldDataInstaller();

        /**
         * Fills the converted field list with the fields already installed in the data directory.
         *
         * Used when the fields are not converted in this run but other steps (like the world map
         * scripts) need them.
         */
        void LoadInstalledFields();

        /**
         * Retrieves the names of the fields, by ID.
         *
         * @return The names of the fields.
         */
        const MapList& GetMapList() const{return map_list_;}

        /**
         * Retrieves the scale factors of the fields.
         *
         * @return The scale factors, by field ID.
         */
        const FieldScaleFactorMap& GetFieldScaleFactors() const{return scale_factors_;}

        /**
         * Retrieves the names of the fields that have been converted.
         *
         * @return The names of the converted fields.
         */
        const MapList& GetConvertedMapList() const{return converted_map_list_;}

        /**
         * Initializer for {@see CollectSpawnAndScaleFactors}.
         *
         * Installation step 1.
         *
         * @param[in] res_mgr The application resource manager.
         * @return The total number of level files to process.
         */
        int CollectSpawnAndScaleFactorsInit(Ogre::ResourceGroupManager* res_mgr);

        /**
         * Reads spawn points and scale factors from flevel files.
         *
         * Installation step 2.
         *
         * @param[in] field_index Index of the field to collect from. Must be less than the value
         * returned by {@see CollectSpawnAndScaleFactorsInit}.
         */
        void CollectSpawnAndScaleFactors(int field_index);

        /**
         * Writes the coordinates of the exits to the world map (world/scripts/world_arrivals.lua).
         */
        void WriteWorldArrivals();

        /**
         * Converts FFVII PC fields to V-Gears format.
         *
         * @param[in] field_index Index of the field to convert. Must be less than the value
         * returned by {@see CollectSpawnAndScaleFactorsInit}.
         */
        void Convert(int field_index);

        /**
         * Initializer for {@see Write}.
         *
         * Installation step 3.
         *
         * @return The number of converted maps that need to be written.
         */
        int WriteInit();

        /**
         * Adds a converted field to the list of fields.
         *
         * Installation step 4.
         *
         * @param[in] field_index Index of the field. Must be less than the value returned by
         * {@see WriteInit}.
         */
        void Write(int field_index);

        /**
         * Saves the list of fields (field/maps/_fields.xml).
         *
         * Installation step 5.
         */
        void WriteEnd();

        /**
         * Initializer for {@see ConvertModels}.
         *
         * Installation step 6.
         *
         * @return List of names of models to convert.
         */
        std::vector<std::string> ConvertModelsInit();

        /**
         * Converts a field model to V-Gears format.
         *
         * Installation step 7 and final.
         *
         * @param[in] model_name Name of the model to convert.
         */
        void ConvertModels(std::string model_name);

    private:

        /**
         * Checks if a file is a field file.
         *
         * To verify it, it just checks that the extension is not '.tex', '.tut' or '.siz' and
         * that the name is not 'maplist'.
         *
         * @param[in] resource_name The name of the file to test.
         * @return False if the file is surely not a field file, true otherwise.
         */
        static bool IsAFieldFile(const Ogre::String& resource_name);

        /**
         * Collects the spawn points that the gateways and the map jumps of a field add to other
         * fields.
         *
         * @param[in] field The field to collect from.
         * @return False if the script can't be decompiled or the field has no triggers.
         */
        bool CollectSpawnPoints(const VGears::FLevelFilePtr& field);

        /**
         * Converts a FFVII PC field to a V-Gears field.
         *
         * @param[in] field The field map to convert.
         */
        void PcFieldToVGearsField(const VGears::FLevelFilePtr& field);

        /**
         * Path to the directory to write generated data to.
         */
        std::string output_dir_;

        /**
         * The game data folder.
         */
        std::string data_dir_;

        /**
         * Function to write a line to the installation log.
         */
        std::function<void(std::string)> write_output_line_;

        /**
         * The files of the field archive.
         */
        Ogre::StringVectorPtr flevel_file_list_;

        /**
         * The names of the fields, by ID.
         */
        MapList map_list_;

        /**
         * The fields whose data could not be collected. They are not converted.
         */
        std::vector<std::string> failed_fields_;

        /**
         * The spawn points of every field.
         */
        FieldSpawnPointsMap spawn_points_;

        /**
         * The scale factors of the fields.
         */
        FieldScaleFactorMap scale_factors_;

        /**
         * The models that the fields use, and their animations.
         */
        ModelsAndAnimationsDb used_models_and_anims_;

        /**
         * The names of the converted fields.
         */
        MapList converted_map_list_;

        /**
         * The document of the list of fields.
         */
        std::unique_ptr<TiXmlDocument> doc_;

        /**
         * The root element of the list of fields.
         */
        std::unique_ptr<TiXmlElement> element_;

        /**
         * Writes the scripts and the texts of the fields.
         */
        FieldScriptInstaller script_installer_;

        /**
         * Writes the maps and the walkmeshes of the fields.
         */
        FieldMapWriter map_writer_;

        /**
         * Writes the backgrounds of the fields.
         */
        FieldBackgroundInstaller background_installer_;

        /**
         * Converts the models of the fields.
         */
        FieldModelInstaller model_installer_;
};
