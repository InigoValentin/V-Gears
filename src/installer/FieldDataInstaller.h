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

#include <vector>
#include <string>
#include <OgreMesh.h>
#include <tinyxml.h>
#include "data/VGearsFLevelFile.h"
#include "ScopedLgp.h"
#include "SpawnPointDb.h"
#include "ModelsAndAnimationsDb.h"
#include "FieldTextWriter.h"
#include "decompiler/field/FieldScriptFormatter.h"
#include "decompiler/field/FieldDecompiler.h"
#include "common/FinalFantasy7/FF7NameLookup.h"

typedef std::vector<std::string> MapList;

typedef std::map<u16, SpawnPointDb> FieldSpawnPointsMap;

typedef std::map<u16, float> FieldScaleFactorMap;

class BaseFF7FieldScriptFormatter : public FieldScriptFormatter{

    public:

        /**
         * Constructor.
         *
         * @param[in] field_name The field name.
         * @param[in] field_id_to_name_lookup Field name lookup table to use.
         * @param[in] spawn_points The list of spawn points.
         */
        BaseFF7FieldScriptFormatter(
          const std::string& field_name, const std::vector<std::string>& field_id_to_name_lookup,
          FieldSpawnPointsMap& spawn_points
        ) :
          field_name_(field_name), field_id_to_name_lookup_(field_id_to_name_lookup),
          spawn_points_(spawn_points)
        {}

        /**
         * Composes a spawn point name
         *
         * The name will have the format
         * {FIELD_NAME}_{ENTITY}_{FUNCTION_NAME}_addr_{ADDRESS}
         *
         * @param[in] map_id Unused.
         * @param[in] entity Entity name.
         * @param[in] function_name Name of the function.
         * @param[in] address @todo Understand and document.
         * @return The composed spawn point name.
         */
        virtual std::string GetSpawnPointName(
          unsigned int map_id, const std::string& entity,
          const std::string& function_name, unsigned int address
        ) override{
            return
              field_name_ + "_" + entity + "_" + function_name + "_addr_" + std::to_string(address);
        }

        /**
         * Retrieves a map name from it's ID.
         *
         * @param[in] map_id The map ID.
         * @return The map name.
         */
        virtual std::string GetMapName(unsigned int map_id) override{
            return field_id_to_name_lookup_[map_id];
        }

        /**
         * Retrieves a user friendly variable name.
         *
         * @param[in] bank Variable bank.
         * @param[in] addr Variable address.
         * @return Friendly name for the variable, or an empty string if it
         * doesn't have one assigned.
         */
        virtual std::string GetFriendlyVarName(
          unsigned int bank, unsigned int addr
        ) override{
            return VGears::NameLookup::FieldScriptVarName(bank, addr);
        }

        /**
         * Retrieves a user friendly entity name.
         *
         * @param[in] entity Entity name.
         * @return Friendly name for the entity, or the current name if it
         * doesn't have one assigned.
         */
        virtual std::string GetFriendlyEntityName(const std::string& entity) override{
            return VGears::NameLookup::FieldScriptEntityName(entity);
        }

        /**
         * Retrieves a user friendly character name.
         *
         * @param[in] char_id Character ID.
         * @return Friendly name for the entity, or an empty string if it
         * doesn't have one assigned.
         */
        virtual std::string GetFriendlyCharName(int char_id) override{
            return VGears::NameLookup::CharName(char_id);
        }

        /**
         * Retrieves a user friendly function name.
         *
         * @param[in] entity The name of the entity the function belongs to.
         * @param[in] function_name Function name.
         * @return Friendly name for the function, or the current name if it
         * doesn't have one assigned.
         */
        virtual std::string GetFriendlyFunctionName(
          const std::string& entity, const std::string& function_name
        ) override{
            return VGears::NameLookup::FieldScriptFunctionName(
              field_name_, entity, function_name
            );
        }

        /**
         * Retrieves the header comment for a function in an entity.
         *
         * @param[in] entity The name of the entity the function belongs to.
         * @param[in] function_name Function name.
         * @return Friendly comment for the function, or an empty string if it
         * doesn't have one assigned.
         */
        virtual std::string GetFunctionComment(
          const std::string& entity, const std::string& function_name
        ) override{
            return VGears::NameLookup::FieldScriptFunctionComment(
              field_name_, entity, function_name
            );
        }

    protected:

        /**
         * The field name.
         */
        std::string field_name_;

        /**
         * The list of spawn points.
         */
        FieldSpawnPointsMap& spawn_points_;

        /**
         * Look up table relatingfield IDs and names.
         */
        const std::vector<std::string>& field_id_to_name_lookup_;
};

/**
 * Handles the script gateway colection.
 */
class FF7FieldScriptGatewayCollector : public BaseFF7FieldScriptFormatter{

    // TODO: Move declaration to a .h file.

    public:

        /**
         * Cosntructor.
         */
        FF7FieldScriptGatewayCollector(
          const std::string& field_name, const std::vector<std::string>& field_id_to_name_lookup,
          FieldSpawnPointsMap& spawn_points, size_t this_field_id
        ) :
          BaseFF7FieldScriptFormatter(field_name,field_id_to_name_lookup, spawn_points),
          field_id_(this_field_id)
        {}

        /**
         * Adds an spawn point.
         *
         * If the spawn is new, a new record will be added to the spawn point
         * database. If it was already there, update the record to add the
         * origin.
         *
         * @param[in] map_id The target map ID.
         * @param[in] entity The entity that acts as the spawn point.
         * @param[in] function_name The spawn function name.
         * @param[in] address @todo Understand and document.
         * @param[in] x X coordinate of the field target at which to spawn.
         * @param[in] y Y coordinate of the field target at which to spawn.
         * @param[in] z Z coordinate of the field target at which to spawn.
         * @param[in] angle Orientation at which to spawn.
         */
        virtual void AddSpawnPoint(
          unsigned int map_id, const std::string& entity, const std::string& function_name,
          unsigned int address, int x, int y, int z, int angle
        ) override {
            auto it = spawn_points_.find(map_id);
            VGears::TriggersFile::Gateway gw = {};
            gw.destination_field_id = map_id;
            gw.destination.x = x;
            gw.destination.y = y;
            gw.destination.z = z;
            gw.dir = angle;
            if (it != std::end(spawn_points_)){
                // Add to the list of gateways that link to destination_field_id.
                SpawnPointDb::Record rec;
                rec.from_script = true;
                rec.field_id = field_id_;
                rec.gateway = gw;
                rec.entity_name = entity;
                rec.script_function_name = function_name;
                rec.gateway_index_or_map_jump_address = address;
                it->second.gateways_to_this_field.push_back(rec);
            }
            else{
                // Create a new record for destination_field_id
                SpawnPointDb db;
                db.target_field_id = map_id;
                SpawnPointDb::Record rec;
                rec.from_script = true;
                rec.field_id = field_id_;
                rec.gateway = gw;
                rec.entity_name = entity;
                rec.script_function_name = function_name;
                rec.gateway_index_or_map_jump_address = address;
                db.gateways_to_this_field.push_back(rec);
                spawn_points_.insert(std::make_pair(db.target_field_id, db));
            }
        }

    private:

        /**
         * The field map ID.
         */
        size_t field_id_;

};

/**
 * Handles the formatting of field scripts.
 */
class FF7FieldScriptFormatter : public BaseFF7FieldScriptFormatter{

    public:

        /**
         * Constructor.
         *
         * @param[in] field_name The field name.
         * @param[in] models The list of field models.
         * @param[in] field_id_to_name_lookup Field name lookup table to use.
         * @param[in] spawn_points The list of spawn points.
         */
        FF7FieldScriptFormatter(
          const std::string& field_name, const VGears::ModelListFilePtr& models,
          const std::vector<std::string>& field_id_to_name_lookup,
          FieldSpawnPointsMap& spawn_points
        ) :
          BaseFF7FieldScriptFormatter(field_name, field_id_to_name_lookup, spawn_points),
          model_loader_(models)
        {}

        // Names an animation, can't return empty
        /**
         * Retreieves an animation name from iths ID.
         *
         * @param[in] char_id ID of the character the animation belogs to.
         * @param[in] id Animation ID.
         * @return The animation name. If there is no name assigned, or it
         * can't be found, a string with the animation ID.
         */
        virtual std::string GetFriendlyAnimationName(int char_id, int id) override{
            // Get the animation file name, then look up the friendly name of
            // the "raw" animation.
            if (static_cast<unsigned int>(char_id) >= model_loader_->GetModels().size()){
                std::cerr << "FF7FieldScriptFormatter::AnimationName ERROR:"
                  << "Char ID " << char_id << " out of bounds" << std::endl;
                return std::to_string(id);
            }
            const auto& model_info = model_loader_->GetModels().at(char_id);
            if (static_cast<unsigned int>(id) >= model_info.animations.size()){
                std::cerr << "FF7FieldScriptFormatter::AnimationName ERROR:"
                  << "In field " << field_name_ << " the model " << model_info.name
                  << " animation with ID " << id << " is out of bounds ("
                  << model_info.animations.size() << ")" << std::endl;
                return std::to_string(id);
            }
            const auto raw_name = model_info.animations.at(id).name;
            // Trim off ".yos" or whatever extension the model loader adds in.
            Ogre::String base_name;
            VGears::StringUtil::splitBase(raw_name, base_name);
            VGears::StringUtil::toLowerCase(base_name);
            return VGears::NameLookup::Animation(base_name);
        }

    private:

        /**
         * The field model loader.
         */
        const VGears::ModelListFilePtr& model_loader_;
};

class FieldDataInstaller{

    public:

        /**
         * Installer constructor
         *
         * @param[in] input_dir Path to the directory containing the original
         * data to parse.
         * @param[in] output_dir Path to the directory to write generated data
         * to.
         */
        FieldDataInstaller(const std::string input_dir, const std::string output_dir);

        /**
         * Installer destructor.
         */
        ~FieldDataInstaller();

        /**
         * Initializer for {@see CollectionFieldSpawnAndScaleFactors}.
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
         * Converts FFVII PC fields to V-Gears format.
         *
         * @param[in] field_index Index of the field to collect from. Must be less than the value
         * returned by {@see CollectSpawnAndScaleFactorsInit}.
         */
        void Convert(int field_index);

        /**
         * Initializer for {@see WriteMapsXmlIteration}.
         *
         * Installation step 3.
         *
         * @return The number of converted maps that need to be written.
         */
        int WriteInit();

        /**
         * Saves the game maps to XML files.
         *
         * Installation step 4.
         *
         * @param[in] field_index Index of the field to collect from. Must be less than the value
         * returned by {@see WriteInit}.
         */
        void Write(int field_index);

        /**
         * Cleans up after {@see WriteMapsXmlIteration}.
         *
         * Installation step 5.
         */
        void WriteEnd();

        /**
         * Initializer for {@see ConvertFieldModelsIteration}.
         *
         * Installation step 6.
         *
         * @return List of names of models to convert.
         */
        std::vector<std::string> ConvertModelsInit();

        /**
         * Converts the field models to V-Gears format.
         *
         * Installation step 7 and final.
         *
         * @param[in] model_name Name of the model to convert.
         */
        void ConvertModels(std::string model_name);

    private:

        /**
         * Creates a gateway script.
         *
         * @param[in] gateway_entity_name Name of the gateway entity.
         * @param[in] target_map_name Name of the target map.
         * @param[in] source_spawn_point_name The spawn point name.
         * @return Gateway LUA script.
         */
        static std::string CreateGateWayScript(
          const std::string& gateway_entity_name, const std::string& target_map_name,
          const std::string& source_spawn_point_name
        );

        /**
         * Retrieves a field ID from a name.
         *
         * @param[in] name The name of the field.
         * @param[in] field_id_to_name_lookup Lookup table to use.
         * @return The field ID for the specified name.
         * @throws std::runtime_error If there is no ID for the field name.
         */
        static size_t GetFieldId(
          const std::string& name, const std::vector<std::string>& field_id_to_name_lookup
        );

        /**
         * Checks if a file is a field file.
         *
         * To verify it, it just checks that the extension is not '.tex', '.tut' or
         * '.siz' and that the name is not 'maplist'.
         *
         * @param[in] resource_name The name of the file to test.
         * @return False if the file is surely not a field file, true otherwise.
         */
        static bool IsAFieldFile(const Ogre::String& resource_name);

        /**
         * Checks if a map cause fatal crash bugs.
         *
         * It uses a hardcoded list of maps that are known to cause errors. This can be
         * removed when whatever is causing the crash(s) is fixed.
         *
         * NOTE: Even so, conversion of all models will fail with a bone index out of
         * bounds.
         *
         * @param[in] resource_name The name of the map file to test.
         * @return True if the map will crash, false otherwise.
         */
        static bool WillCrash(const Ogre::String& resource_name);

        /**
         * Checks if a map is a test field.
         *
         * It uses a hardcoded list of maps that are known to be test maps. Unused maps
         * are considered to be test maps.
         *
         * @param[in] resource_name The name of the map file to test.
         * @return True if the map is a test map, false otherwise.
         */
        static bool IsTestField(const Ogre::String& resource_name);

        /**
         * Collects the scale factor from a map.
         *
         * @param[in] field The field to collect from.
         * @param[out] scale_factors The scale factor for the map will be set here, in
         * the appropiate map entry.
         * @param[in] field_id_to_name_lookup Lookup table to relate field IDs and
         * names.
         */
        static void CollectFieldScaleFactors(
          VGears::FLevelFilePtr& field, FieldScaleFactorMap& scale_factors,
          const std::vector<std::string>& field_id_to_name_lookup
        );

        /**
         * Collects the spawn point from a field.
         *
         * If a spawn is new, a new record will be added to the spawn point database.
         * If it was already there, update the record to add the origin.
         *
         * @param[in] field The field to collect from.
         * @param[in] field_id_to_name_lookup Lookup table to relate field IDs and
         * names.
         * @param[out] spawn_points Spawn points will be added sequentially to the end
         * of this map.
         */
        static void CollectSpawnPoints(
          VGears::FLevelFilePtr& field, const std::vector<std::string>& field_id_to_name_lookup,
          FieldSpawnPointsMap& spawn_points
        );

        /**
         * The scale factor for line point coordinates.
         *
         * When a LINE opcode is found in the game scripts, the line points X and Y coordinates
         * must be scaled down by this factor.
         */
        static float LINE_SCALE_FACTOR;

        /**
         * Path to the field models directory.
         */
        static std::string FIELD_MODELS_DIR;

        /**
         * Path to the field maps directory.
         */
        static std::string FIELD_MAPS_DIR;

        /**
         * Gateways to this map ID are considered to be inactive.
         */
        static int INACTIVE_GATEWAY_ID;

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
         * Reads the music track IDs from a specific field.
         *
         * @param[in] field The field to extract tracks from.
         * @return The list of tracks. It can be empty.
         */
        std::vector<int> ExtractMusicTrackIds(VGears::FLevelFilePtr& field);

        /**
         * Converts a FFVII PC field to a V-Gears field.
         *
         * @param[in] field The field map to convert.
         */
        void PcFieldToVGearsField(VGears::FLevelFilePtr& field);

        /**
         * Retrieves a field scale factor.
         *
         * @param[in] field_id The id (name) of the field.
         * @return The scale factor for the map.
         * @throws std::runtime_error If there is no ID for the field name.
         */
        float GetFieldScaleFactor(size_t field_id);

        /**
         * Creates a directory in the outputh path.
         *
         * @param[in] dir Path of the directory to create, relative to the
         * output path..
         * @throws std::runtime_error If the directory can't be created.
         */
        void CreateDir(const std::string& dir);

        /**
         * The path to the directory from which to read the PC game data.
         */
        std::string input_dir_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

        /**
         * Helper variable to indicate internal progress of installation steps.
         */
        size_t conversion_step_;

        /**
         * Helper variable to indicate internal progress of installation steps.
         */
        size_t progress_step_num_elements_;

        /**
         * Iterator counter.
         */
        size_t iterator_counter_;

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
        MapList converted_map_list_;

        /**
         * Iterator for {@see converted_map_list}.
         */
        MapList::iterator converted_map_list_iterator_;

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
