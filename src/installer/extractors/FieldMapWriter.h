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
#include <OgreVector3.h>
#include <tinyxml.h>
#include "decompiler/field/FieldDecompiler.h"
#include "formats/field/FLevelFile.h"
#include "FieldDataTypes.h"
#include "ModelsAndAnimationsDb.h"

/**
 * Writes the map file of a field (map.xml), which links the other files of the field and has its
 * entities, and the walkmesh (wm.xml).
 */
class FieldMapWriter{

    public:

        /**
         * Constructor.
         *
         * @param[in] data_dir The game data folder.
         * @param[in] map_list The names of the fields, by ID.
         * @param[in] spawn_points The spawn points of every field.
         * @param[in] scale_factors The scale factors of the fields.
         * @param[in,out] used_models The models that the fields use, and their animations. The
         * models of each field are added to it.
         * @param[in] log_output_line Function to write a line to the installation log.
         */
        FieldMapWriter(
          const std::string& data_dir, const MapList& map_list,
          const FieldSpawnPointsMap& spawn_points, const FieldScaleFactorMap& scale_factors,
          ModelsAndAnimationsDb& used_models, std::function<void(std::string)> log_output_line
        );

        /**
         * Writes the map file of a field.
         *
         * @param[in] field The field.
         * @param[in] decompiled The decompiled script of the field, with its lines and entities.
         */
        void WriteMap(const VGears::FLevelFilePtr& field, const FieldDecompiler::DecompiledScript& decompiled);

        /**
         * Writes the walkmesh of a field.
         *
         * @param[in] field The field.
         */
        void WriteWalkmesh(const VGears::FLevelFilePtr& field);

        /**
         * Finds the IDs of the music tracks of a field.
         *
         * Each "AKAO" string in the script section of the field is followed by the ID of a track.
         *
         * @param[in] raw_script The script section of the field.
         * @return The IDs of the tracks, in the order of the field.
         */
        static std::vector<int> ExtractMusicTrackIds(const std::vector<u8>& raw_script);

    private:

        /**
         * Factor of the lines of the scripts, before the scale of the field.
         */
        static const float LINE_SCALE_FACTOR;

        /**
         * Writes the elements that link the other files of the field.
         *
         * @param[in] field_name Name of the field.
         * @param[out] map The map element.
         */
        static void WriteFileLinks(const std::string& field_name, TiXmlElement& map);

        /**
         * Writes the random encounters of a field.
         *
         * The section has two tables of 24 bytes: enabled (1 byte), rate (1 byte), six standard
         * and four special encounters (16 bits each, battle ID in the lower 10 bits and
         * probability, out of 64, in the upper 6), and 2 bytes of padding.
         *
         * @param[in] raw_encounter The encounter section of the field.
         * @param[out] map The map element.
         */
        static void WriteEncounters(const std::vector<u8>& raw_encounter, TiXmlElement& map);

        /**
         * Writes the points where the player enters the field, from doors and scripts of other
         * fields.
         *
         * @param[in] field The field.
         * @param[out] map The map element.
         * @return The position of the first entrance that is not at the origin, or the origin.
         */
        Ogre::Vector3 WriteEntityPoints(const VGears::FLevelFilePtr& field, TiXmlElement& map);

        /**
         * Finds the walkmesh triangle of an entrance.
         *
         * The Z coordinate of the destination of an entrance is the index of the triangle. Some
         * map jumps of the original scripts have a triangle that is not in the walkmesh (pillar_3
         * to mds6_22): then the one under the position is used.
         *
         * @param[in] field The field.
         * @param[in] record The entrance.
         * @param[in] x X coordinate of the entrance, in the units of the walkmesh.
         * @param[in] y Y coordinate of the entrance, in the units of the walkmesh.
         * @return The index of the triangle. If the position is not on the walkmesh either, 0.
         */
        unsigned int FindEntranceTriangle(
          const VGears::FLevelFilePtr& field, const SpawnPointDb::Record& record,
          const float x, const float y
        );

        /**
         * Writes the lines, scripts and models of a field.
         *
         * They are written in the order of the original field: the engine starts their scripts in
         * that order, and some scripts depend on it (a script that clears the background areas
         * has to start before the doors that set them).
         *
         * @param[in] field The field.
         * @param[in] decompiled The decompiled script of the field.
         * @param[in] first_entity_point Where the player models are placed.
         * @param[out] map The map element.
         */
        void WriteEntities(
          const VGears::FLevelFilePtr& field, const FieldDecompiler::DecompiledScript& decompiled,
          const Ogre::Vector3& first_entity_point, TiXmlElement& map
        );

        /**
         * Creates the element of an entity with a model, and adds the model and its animations to
         * the used models.
         *
         * @param[in] entity The entity.
         * @param[in] model The model of the entity.
         * @param[in] first_entity_point Where the player models are placed.
         * @return The element of the entity.
         */
        std::unique_ptr<TiXmlElement> CreateModelEntity(
          const FieldDecompiler::FieldEntity& entity,
          const VGears::ModelListFile::ModelDescription& model, const Ogre::Vector3& first_entity_point
        );

        /**
         * Writes the lines of the gateways and of the background triggers in use.
         *
         * @param[in] field The field.
         * @param[out] map The map element.
         */
        void WriteTriggerLines(const VGears::FLevelFilePtr& field, TiXmlElement& map);

        /**
         * Writes the music tracks of a field.
         *
         * @param[in] field The field.
         * @param[out] map The map element.
         */
        static void WriteTracks(const VGears::FLevelFilePtr& field, TiXmlElement& map);

        /**
         * The game data folder.
         */
        std::string data_dir_;

        /**
         * The names of the fields, by ID.
         */
        const MapList& map_list_;

        /**
         * The spawn points of every field.
         */
        const FieldSpawnPointsMap& spawn_points_;

        /**
         * The scale factors of the fields.
         */
        const FieldScaleFactorMap& scale_factors_;

        /**
         * The models that the fields use, and their animations.
         */
        ModelsAndAnimationsDb& used_models_;

        /**
         * Function to write a line to the installation log.
         */
        std::function<void(std::string)> write_output_line_;
};
