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

#include "common/BinGZipFile.h"
#include "common/TypeDefine.h"
#include "data/BattleSceneFile.h"
#include "data/VGearsLGPArchive.h"

/**
 * The battle data installer.
 */
class BattleDataInstaller{


    public:

        /**
         * Constructor
         *
         * @param[in] input_dir Path to the directory containing the original data to parse.
         * @param[in] output_dir Path to the directory of the installation data.
         * @param[in] res_mgr The application resource manager..
         */
        BattleDataInstaller(
          const std::string input_dir, const std::string output_dir,
          Ogre::ResourceGroupManager* res_mgr
        );

        /**
         * Destructor.
         */
        ~BattleDataInstaller();

        /**
         * Prepares the installer for scene processing.
         *
         * @return The total number of scenes to process.
         */
        unsigned int InitializeScenes();

        /**
         * Prepares the installer for battle 3D model processing.
         *
         * Reads the contents of the battle.lgp file.
         *
         * @return The total number of battle models to process.
         */
        unsigned int InitializeBattleModels();

        /**
         * Prepares the installer for spell 3D model processing.
         *
         * Reads the contents of the magic.lgp file.
         *
         * @return The total number of battle models to process.
         */
        unsigned int InitializeSpellModels();

        /**
         * Processes the next battle scene.
         *
         * @return The total number of processed scenes.
         */
        unsigned int ProcessScene();

        /**
         * Processes the next battle 3D model.
         *
         * Saves .hrc, .a and .p files for later conversion. .tex files are converted directly to
         * png.
         *
         * @return The total number of processed models.
         */
        unsigned int ProcessBattleModel();

        /**
         * Processes the next spell 3D model.
         *
         * Saves .hrc, .a and .p files for later conversion. .tex files are converted directly to
         * png.
         *
         * @return The total number of processed models.
         */
        unsigned int ProcessSpellModel();

        /**
         * Prepares the installer for battle model conversion.
         *
         * @return The total number of battle models to convert.
         */
        unsigned int ConvertBattleModelsInit();

        /**
         * Prepares the installer for spell model conversion.
         *
         * @return The total number of spell models to convert.
         */
        unsigned int ConvertSpellModelsInit();

        /**
         * Converts a model.
         *
         * @return The total number of converted models.
         */
        unsigned int ConvertBattleModel();

        /**
         * Converts a spell model.
         *
         * @return The total number of converted models.
         */
        unsigned int ConvertSpellModel();

        /**
         * Writes enemy data to files.
         */
        void WriteEnemies();

        /**
         * Writes attack data to files.
         */
        void WriteAttacks();

        /**
         * Writes formation data to files.
         */
        void WriteFormations();

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
         * A model.
         *
         * It can be a battle model or an spell model.
         */
        struct Model{

            /**
             * Model identifier.
             *
             * For battle models, a two letters string. For spell models, anything.
             */
            std::string id;

            /**
             * Name of the model .hrc file.
             *
             * For battle models, it will be the *aa file. For spell models, the *.d file.
             */
            std::string hrc;

            /**
             * Name of the animation file.
             *
             * For battle models, it will be the *da file. For spell models, the *.b file.
             */
            std::string anim;

            /**
             * List of .p polygon files associated to the model
             */
            std::vector<std::string> p;

            Model(): id(""), hrc(""), anim(""){
                p.clear();
            }

        };

        /**
         * Files to compose a battle model.
         */
        struct BattleModel: Model{

            /**
             * Name of the "ab" scripts file.
             */
            std::string script;

            /**
             * List of .tex texture files associated to the model
             */
            std::vector<std::string> tex;

            /**
             * List of .a animation files associated to the model
             */
            std::vector<std::string> a;

            /**
             * Constructor, initializes default values.
             */
            BattleModel(): script(""){
                tex.clear();
                a.clear();
            }

        };

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
         * Files to compose a spell model.
         */
        struct SpellModel: Model{

            /**
             * List of .a files associated to the model
             */
            std::vector<std::string> a;

            /**
             * List of .a files associated to the model
             */
            std::vector<std::string> s;

            /**
             * Constructor, initializes default values.
             */
            SpellModel(){
                a.clear();
                s.clear();
            }
        };

        /**
         * Extract a gzipped file.
         *
         * @param[in] file Compressed file.
         * @return Uncompressed file. NULL if errors were encountered (they will be logged to
         * console).
         */
        File* ExtractGZipScene(File file);

        /**
         * Constructs a file name for the XML file to save an enemy to.
         *
         * The filename will be "dame/enemy/[ID]_[NAME].xml" where [ID] is the enemy ID, padded
         * with '0' to a length of 4, and [NAME] is the enemy name using only alphanumeric
         * characters.
         *
         * @param[in] enemy The enemy to be saved.
         * @return The name for the enemy file.
         */
        std::string BuildEnemyFileName(Enemy enemy);

        /**
         * Constructs a file name for the XML file to save an attack to.
         *
         * The filename will be "dame/attack/[ID]_[NAME].xml" where [ID] is the attack ID, padded
         * with '0' to a length of 4, and [NAME] is the attack name using only alphanumeric
         * characters.
         *
         * @param[in] attack The attack to be saved.
         * @return The name for the attack file.
         */
        std::string BuildAttackFileName(Attack attack);

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
         * Generates all required .rsd models for a model.
         *
         * To be used for enemy, playable character and scene models (anything in battle.lgp).
         *
         * @param[in] model Model information.
         * @param[in] path Output path.
         */
        void GenerateRsdFiles(BattleModel model, std::string path);

        /**
         * Generates all required .rsd models for a model.
         *
         * To be used for spell models (anything in magic.lgp).
         *
         * @param[in] model Model information.
         * @param[in] path Output path.
         */
        void GenerateRsdFiles(SpellModel model, std::string path);

        /**
         * Decompiles a compiled HRC file.
         *
         * To be used for battle enemy and playable character models, since they have a
         * well-defined bone hierarchy. For battle background models, use
         * {@see DecompileSceneSceneHrc}, and for spell models, use {@see DecompileSpellModelHrc}.
         * Note that this is not a general purpose HRC decompiler, it only works for files in
         * battle.lgp.
         *
         * @param[in] compiled The compiles HRC file.
         * @param[in] model The model, it can be a battle model or an spell model.
         * @param[in] path Path to save the decompiled file to.
         * @param[in] skeleton_name Name for the skeleton.
         */
        void DecompileBattleModelHrc(
          File compiled, BattleModel model, std::string path, std::string skeleton_name
        );

        /**
         * Decompiles a compiled HRC file.
         *
         * To be used for spell models, since they have a well-defined bone hierarchy. For battle
         * background models, use {@see DecompileSceneSceneHrc}, and for enemy and playable
         * character models, use {@see DecompileBattleModelHrc}. Note that this is not a general
         * purpose HRC decompiler, it only works for files in magic.lgp.
         *
         * @param[in] compiled The compiles HRC file.
         * @param[in] model The model, it can be a battle model or an spell model.
         * @param[in] path Path to save the decompiled file to.
         * @param[in] skeleton_name Name for the skeleton.
         */
        void DecompileSpellModelHrc(
          File compiled, SpellModel model, std::string path, std::string skeleton_name
        );

        /**
         * Decompiles a compiled HRC file.
         *
         * To be used only for battle background HRC files, since they don't have a defined bone
         * hierarchy, and every bone is assumed to be a child of the previous one. For enemy and
         * playable character models, use {@see DecompileBattleModelHrc}. Note that this is not a
         * general purpose HRC decompiler, it only works for files in battle.lgp.
         *
         * @param[in] compiled The compiles HRC file.
         * @param[in] model The model, it can be a battle model or an spell model.
         * @param[in] path Path to save the decompiled file to.
         * @param[in] skeleton_name Name for the skeleton.
         */
        void DecompileSceneModelHrc(
          File compiled, BattleModel model, std::string path, std::string skeleton_name
        );

        /**
         * The path to the directory from which to read the PC game data.
         */
        std::string input_dir_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

        /**
         * Pointer to the application resource manager.
         */
        Ogre::ResourceGroupManager* res_mgr_;

        /**
         * Function used to print text to the log output, line by line.
         */
        std::function<void(std::string)> write_output_line_;

        /**
         * Next scene file to process;
         */
        unsigned int next_scene_;

        /**
         * Number of scenes in SCENE.BIN;
         */
        unsigned int total_scenes_;

        /**
         * The original scene.bin file.
         */
        File scene_bin_;

        /**
         * List of uncompressed scenes.
         */
        std::vector<File*> scenes_uncompressed_;

        /**
         * Processed scenes.
         */
        std::vector<BattleSceneFile> scenes_;

        /**
         * Enemy data read from the scenes;
         */
        std::vector<Enemy> enemies_;

        /**
         * List of models for playable character;
         */
        std::vector<CharacterData> character_data_;

        /**
         * List of models for battle scenes;
         */
        std::vector<SceneModel> scene_data_;

        /**
         * Map of enemies, ID and xml file path.
         */
        std::map<int, std::string> enemy_map_;

        /**
         * Attack data read from the scenes;
         */
        std::vector<Attack> attacks_;

        /**
         * Map of attacks, ID and xml file path.
         */
        std::map<int, std::string> attack_map_;

        /**
         * Formations data read from the scenes;
         */
        std::vector<Formation> formations_;

        /**
         * Map of formations, ID and xml file path.
         */
        std::map<int, std::string> formation_map_;

        /**
         * The files in the original battle.lgp file.
         */
        std::vector<File> battle_lgp_files_;

        /**
         * File names for the files in {@see battle_lgp_files_}.
         */
        std::vector<std::string> battle_lgp_file_names_;

        /**
         * The files in the original magic.lgp file.
         */
        std::vector<File> magic_lgp_files_;

        /**
         * File names for the files in {@see magic_lgp_files_}.
         */
        std::vector<std::string> magic_lgp_file_names_;

        /**
         * Next model to process;
         */
        unsigned int next_model_to_process_;

        /**
         * Next model to convert.
         */
        unsigned int next_model_to_convert_;

        /**
         * List of battle models files found in battle.lgp
         */
        std::vector<BattleModel> battle_models_;

        /**
         * List of battle model materials;
         */
        std::vector<std::string> battle_materials_;

        /**
         * List of spell models files found in magic.lgp
         */
        std::vector<SpellModel> spell_models_;

        /**
         * List of spell model materials;
         */
        std::vector<std::string> spell_materials_;


};
