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
#include <OgreResourceGroupManager.h>
#include "common/File.h"
#include "installer/extractors/BattleMeshExporter.h"
#include "installer/extractors/BattleModelFiles.h"
#include "installer/extractors/BattleModelTools.h"
#include "installer/formats/DiskImage.h"

/**
 * Installs the 3D models of battle.lgp: the party, the enemies and the battle scenes.
 */
class BattleModelInstaller{

    public:

        /**
         * Constructor.
         *
         * @param[in] disk_image Disk image to extract the data from.
         * @param[in] output_dir Path to the directory of the installation data.
         * @param[in] res_mgr The application resource manager.
         * @param[in] exporter Saves the converted meshes.
         */
        BattleModelInstaller(
          DiskImage& disk_image, const std::string& output_dir, Ogre::ResourceGroupManager* res_mgr,
          BattleMeshExporter& exporter
        );

        /**
         * Prepares the installer for battle 3D model processing.
         *
         * Reads the contents of the battle.lgp file.
         *
         * @return The total number of battle models to process.
         */
        unsigned int InitializeBattleModels();

        /**
         * Processes the next battle 3D model.
         *
         * Saves .hrc, .a and .p files for later conversion. .tex files are
         * converted directly to png.
         *
         * @return The total number of processed models.
         */
        unsigned int ProcessBattleModel();

        /**
         * Prepares the installer for battle model conversion.
         *
         * @return The total number of battle models to convert.
         */
        unsigned int ConvertBattleModelsInit();

        /**
         * Converts a model.
         *
         * @return The total number of converted models.
         */
        unsigned int ConvertBattleModel();

        /**
         * Writes playable character data.
         */
        void WriteCharacterData();

        /**
         * Writes battle scene data.
         */
        void WriteSceneData();

    private:

        /**
         * Models for playable characters.
         */
        struct CharacterModel{

            /**
             * Descriptive name for the model.
             */
            std::string name;

            /**
             * Model file.
             */
            std::string model;
        };

        /**
         * Models for battle scenes.
         */
        struct SceneModel{

            /**
             * Scene ID.
             */
            int id;

            /**
             * Model file.
             */
            std::string model;

            /**
             * Scene description.
             */
            std::string description;
        };

        /**
         * Playable character model info.
         */
        struct CharacterData{

            /**
             * Character ID.
             */
            unsigned int id;

            /**
             * List of models for the character.
             */
            std::vector<CharacterModel> models;
        };

        /**
         * Converts a texture of a model to PNG, and notes the character or scene it belongs to.
         *
         * @param[in] id Identifier of the model.
         * @param[in] type The two last letters of the name of the file.
         * @param[in] file The texture file.
         */
        void ProcessTexture(const std::string& id, const std::string& type, const File& file);

        /**
         * Adds a model to the list of models of a playable character.
         *
         * @param[in] character_id The character.
         * @param[in] name Name of the model.
         * @param[in] mesh Name of the mesh file.
         */
        void AddCharacterModel(
          const unsigned int character_id, const std::string& name, const std::string& mesh
        );

        /**
         * Adds a model to the list of battle scenes, if it is not there.
         *
         * @param[in] scene_id The scene.
         * @param[in] description Description of the scene.
         * @param[in] mesh Name of the mesh file.
         */
        void AddSceneModel(const int scene_id, const std::string& description, const std::string& mesh);

        /**
         * Adds to a model of the party the files with the animations of its limit breaks.
         *
         * @param[in,out] model The model.
         */
        void AddLimitAnimations(BattleModel& model) const;

        /**
         * Converts the animation scripts of a model, and writes the data for its cameras.
         *
         * @param[in] model The model.
         * @param[in] full_name Name of the converted model.
         * @param[in] is_player True if it is a model of the party.
         * @param[in] is_scene True if it is a battle scene.
         */
        void ConvertScripts(
          const BattleModel& model, const std::string& full_name, const bool is_player,
          const bool is_scene
        );

        /**
         * Writes the skeleton of each weapon of a model.
         *
         * @param[in] model The model.
         * @param[in] full_name Name of the converted model.
         * @return The names of the weapon models.
         */
        std::vector<std::string> WriteWeaponSkeletons(
          const BattleModel& model, const std::string& full_name
        ) const;

        /**
         * Converts and saves the weapon models.
         *
         * @param[in] weapons Names of the weapon models.
         * @param[in] path Folder of the battle models where they are saved.
         */
        void ExportWeapons(const std::vector<std::string>& weapons, const std::string& path);

        /**
         * Generates all required .rsd models for a model.
         *
         * To be used for enemy, playable character and scene models (anything
         * in battle.lgp).
         *
         * @param[in] model Model information.
         * @param[in] path Output path.
         */
        void WriteRsdFiles(BattleModel model, const std::string& path);

        /**
         * Writes the data of a model that the cameras of the battles use, as a Lua script.
         *
         * @param[in] script The file with the animation scripts of the model.
         * @param[in] skeleton The skeleton of the model.
         * @param[in] model_name Name of the model, like rt_cloud.
         */
        void WriteModelCamera(File script, File skeleton, const std::string& model_name);

        /**
         * Decompiles a compiled HRC file.
         *
         * To be used only for battle background HRC files, since they don't
         * have a defined bone hierarchy, and every bone is assumed to be a
         * child of the previous one. For enemy and playable character models,
         * use {@see DecompileHrc}. Note that this is not a general
         * purpose HRC decompiler, it only works for files in battle.lgp.
         *
         * @param[in] compiled The compiles HRC file.
         * @param[in] model The model, can be a battle model or an spell model.
         * @param[in] path Path to save the decompiled file to.
         * @param[in] skeleton_name Name for the skeleton.
         */
        void DecompileSceneModelHrc(File compiled, BattleModel model, std::string path, std::string skeleton_name);

        /**
         * The disk image to extract the data from.
         */
        DiskImage& disk_image_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

        /**
         * Directory where the game data is installed (<install dir>/data/).
         */
        std::string data_dir_;

        /**
         * Pointer to the application resource manager.
         */
        Ogre::ResourceGroupManager* res_mgr_;

        /**
         * Saves the converted meshes.
         */
        BattleMeshExporter& exporter_;

        /**
         * The files in the original battle.lgp file.
         */
        std::vector<LgpEntry> files_;

        /**
         * List of the models found in battle.lgp.
         */
        std::vector<BattleModel> models_;

        /**
         * List of models for playable character;
         */
        std::vector<CharacterData> character_data_;

        /**
         * List of models for battle scenes;
         */
        std::vector<SceneModel> scene_data_;

        /**
         * Next file to process.
         */
        unsigned int next_model_to_process_ = 0;

        /**
         * Next model to convert.
         */
        unsigned int next_model_to_convert_ = 0;
};
