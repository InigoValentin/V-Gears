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
         */
        BattleDataInstaller(const std::string input_dir, const std::string output_dir);

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
         * Prepares the installer for 3D model processing.
         *
         * @return The total number of models to process.
         */
        unsigned int InitializeModels();

        /**
         * Processes the next battle scene.
         *
         * @return The total number of processed scenes.
         */
        unsigned int ProcessScene();

        /**
         * Processes the 3D model.
         *
         * Saves .hrc, .a and .p files for later conversion. .tex files are converted directly to
         * png.
         *
         * @return The total number of processed models.
         */
        unsigned int ProcessModel();

        /**
         * Prepres the installer for model conversion.
         *
         * @return The total number of models to convert.
         */
        unsigned int ConvertModelsInit();

        /**
         * Converts a model.
         *
         * @return The total number of converted models.
         */
        unsigned int ConvertModel();

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

    private:

        /**
         * Files to compose a model.
         */
        struct Model{

            /**
             * Model identifier, two letters.
             */
            std::string id;

            /**
             * Name of the model .hrc file
             */
            std::string hrc;

            /**
             * Name of the "da" animation file.
             */
            std::string anim;

            /**
             * List of .p polygon files associated to the model
             */
            std::vector<std::string> p;

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
            Model(): id(""), hrc(""), anim(""){
                p.clear();
                tex.clear();
                a.clear();
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
         */
        void GenerateRsdFiles(Model model, std::string path);

        /**
         * Decompiles a compiled HRC file.
         *
         * Note that this is not a general purpose HRC decompiler, it only works for files in
         * battle.lgp.
         *
         * @param[in] compiled The compiles HRC file.
         * @param[in] model The model.
         * @param[in] path PAth to save the decompiled file to.
         */
        void DecompileHrc(File compiled, Model model, std::string path);

        void ExtractAFilesFromDAFile(File da, Model* model, std::string path);

        /**
         * The path to the directory from which to read the PC game data.
         */
        std::string input_dir_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

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
         * Next model to process;
         */
        unsigned int next_model_to_process_;

        /**
         * Next model to convert.
         */
        unsigned int next_model_to_convert_;

        /**
         * List of models files found in battle.lgp
         */
        std::vector<Model> models_;

        /**
         * List of model materials;
         */
        std::vector<std::string> materials_;


};
