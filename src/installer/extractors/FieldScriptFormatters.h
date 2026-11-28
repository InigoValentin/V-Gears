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

#include <iostream>
#include <string>
#include <vector>
#include "decompiler/field/FieldScriptFormatter.h"
#include "formats/FF7NameLookup.h"
#include "formats/model/FF7ModelListFile.h"
#include "FieldDataTypes.h"

/**
 * Names of the fields, entities, variables and functions shared by the field script formatters.
 */
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
        ) : field_name_(field_name), field_id_to_name_lookup_(field_id_to_name_lookup), spawn_points_(spawn_points)
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
          unsigned int map_id, const std::string& entity, const std::string& function_name, unsigned int address
        ) override{
            return field_name_ + "_" + entity + "_" + function_name + "_addr_" + std::to_string(address);
        }

        /**
         * Retrieves a map name from it's ID.
         *
         * @param[in] map_id The map ID.
         * @return The map name.
         */
        virtual std::string GetMapName(unsigned int map_id) override{return field_id_to_name_lookup_[map_id];}

        /**
         * Retrieves a user friendly variable name.
         *
         * @param[in] bank Variable bank.
         * @param[in] addr Variable address.
         * @return Friendly name for the variable, or an empty string if it
         * doesn't have one assigned.
         */
        virtual std::string GetFriendlyVarName(unsigned int bank, unsigned int addr) override{
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
        virtual std::string GetFriendlyCharName(int char_id) override{return VGears::NameLookup::CharName(char_id);}

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
            return VGears::NameLookup::FieldScriptFunctionName(field_name_, entity, function_name);
        }

        /**
         * Retrieves the header comment for a function in an entity.
         *
         * @param[in] entity The name of the entity the function belongs to.
         * @param[in] function_name Function name.
         * @return Friendly comment for the function, or an empty string if it
         * doesn't have one assigned.
         */
        virtual std::string GetFunctionComment(const std::string& entity, const std::string& function_name) override{
            return VGears::NameLookup::FieldScriptFunctionComment(field_name_, entity, function_name);
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
 * Handles the script gateway collection.
 */
class FF7FieldScriptGatewayCollector : public BaseFF7FieldScriptFormatter{

    public:

        /**
         * Constructor.
         */
        FF7FieldScriptGatewayCollector(
          const std::string& field_name, const std::vector<std::string>& field_id_to_name_lookup,
          FieldSpawnPointsMap& spawn_points, size_t this_field_id
        ) : BaseFF7FieldScriptFormatter(field_name,field_id_to_name_lookup, spawn_points), field_id_(this_field_id)
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
            SpawnPointDb::Record record;
            record.from_script = true;
            record.field_id = static_cast<u16>(field_id_);
            record.gateway = {};
            record.gateway.destination_field_id = static_cast<u16>(map_id);
            record.gateway.destination.x = x;
            record.gateway.destination.y = y;
            record.gateway.destination.z = z;
            record.gateway.dir = angle;
            record.entity_name = entity;
            record.script_function_name = function_name;
            record.gateway_index_or_map_jump_address = address;
            FieldData::AddSpawnPoint(spawn_points_, record);
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
          const std::vector<std::string>& field_id_to_name_lookup, FieldSpawnPointsMap& spawn_points
        ) : BaseFF7FieldScriptFormatter(field_name, field_id_to_name_lookup, spawn_points), model_loader_(models)
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
                std::cerr << "FF7FieldScriptFormatter::AnimationName ERROR:" << "In field " << field_name_
                  << " the model " << model_info.name << " animation with ID " << id << " is out of bounds ("
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

        /**
         * Retrieves the animation a model must play when its script doesn't set a default one.
         *
         * In the original game a model without a requested animation plays the first one of its
         * list.
         *
         * @param[in] char_id ID of the model, as given to the CHAR opcode.
         * @return Name of the first animation of the model. An empty string if the model already
         * has an animation called "Idle", or it has no animations.
         */
        virtual std::string GetDefaultAnimationName(int char_id) override{
            if (static_cast<unsigned int>(char_id) >= model_loader_->GetModels().size()) return "";
            const auto& animations = model_loader_->GetModels().at(char_id).animations;
            if (animations.empty()) return "";
            std::string first;
            for (size_t i = 0; i < animations.size(); ++ i){
                Ogre::String base_name;
                VGears::StringUtil::splitBase(animations[i].name, base_name);
                VGears::StringUtil::toLowerCase(base_name);
                const std::string friendly = VGears::NameLookup::Animation(base_name);
                if (friendly == "Idle") return "";
                if (i == 0) first = friendly;
            }
            return first;
        }

        /**
         * Retrieves the animation a model plays when it walks or runs on its own.
         *
         * @param[in] char_id ID of the model, as given to the CHAR opcode.
         * @param[in] index 1 for the walk animation, 2 for the run animation.
         * @return Name of the animation, or an empty string if the model hasn't got it.
         */
        virtual std::string GetMoveAnimationName(int char_id, int index) override{
            if (char_id < 0 || static_cast<unsigned int>(char_id) >= model_loader_->GetModels().size())
                return "";
            const auto& animations = model_loader_->GetModels().at(char_id).animations;
            if (static_cast<unsigned int>(index) >= animations.size()) return "";
            Ogre::String base_name;
            VGears::StringUtil::splitBase(animations[index].name, base_name);
            VGears::StringUtil::toLowerCase(base_name);
            return VGears::NameLookup::Animation(base_name);
        }

    private:

        /**
         * The field model loader.
         */
        const VGears::ModelListFilePtr& model_loader_;
};
