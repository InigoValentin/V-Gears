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
#include "FieldDataInstaller.h"
#include "KernelDataInstaller.h"
#include "MediaDataInstaller.h"
#include "BattleDataInstaller.h"
#include "WorldInstaller.h"
#include "ModelsAndAnimationsDb.h"

/**
 * The data installer.
 */
class DataInstaller{

    public:

        /**
         * Advanced options available in the installer.
         */
        struct AdvancedOptions{

            /**
             * Option to skip battle scenes installation.
             */
            bool skip_battle_data;

            /**
             * Option to skip battle models installation.
             */
            bool skip_battle_models;

            /**
             * Option to skip kernel data installation.
             */
            bool skip_kernel;

            /**
             * Option to skip image installation.
             */
            bool skip_images;

            /**
             * Option to skip sound effects installation.
             */
            bool skip_sounds;

            /**
             * Option to skip music tracks installation.
             */
            bool skip_music;

            /**
             * Option to skip field maps installation.
             */
            bool skip_fields;

            /**
             * Option to skip field models installation.
             */
            bool skip_field_models;

            /**
             * Option to skip world map data installation.
             */
            bool skip_wm;

            /**
             * Option to skip world map models installation.
             */
            bool skip_wm_models;

            /**
             * Option to keep original data after installation.
             */
            bool keep_originals;

            /**
             * Option to avoid calls to the ffmpeg executable.
             */
            bool no_ffmpeg;

            /**
             * Option to avoid calls to the timidity executable
             */
            bool no_timidity;
        };

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
         * @param[in] input_dir Path to the directory containing the original data to parse.
         * @param[in] output_dir Path to the directory to write generated data to.
         * @param[in] options Adavanced options for the installer.
         * @param[in] write_output_line Pointer to function to write output.
         */
        DataInstaller(
          const std::string input_dir, const std::string output_dir, AdvancedOptions options,
          std::function<void(std::string, int, bool)> write_output_line
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
        float Progress();

    private:

        /**
         * Calculates the installation progress.
         *
         * @return Installation progress [0-100]
         */
        const float CalcProgress();

        /**
         * Creates a directory in the outputh path.
         *
         * @param[in] dir Path of the directory to create, relative to the output path.
         * @throws std::runtime_error If the directory can't be created.
         */
        void CreateDir(const std::string& dir);

        /**
         * Creates all directories required for the installation.
         *
         * It also adds certain location to the resource group manager.
         */
        void CreateDirectories();

        /**
         * Cleans up after the installation is complete.
         */
        void CleanInstall();

        /**
         * Installation steps.
         */
        enum InstallationSteps{

            /**
             * Installation not started.
             */
            IDLE = 0,

            /**
             * Create all directories required for the installation.
             */
            CREATE_DIRECTORIES,

            /**
             * Initialize data for installation.
             */
            INITIALIZE,

            /**
             * Initializes the battle installer for scene processing.
             */
            BATTLE_SCENES_INIT,

            /**
             * Extracts battle scenes.
             */
            BATTLE_SCENES_PROCESS,

            /**
             * Writes attack data.
             */
            BATTLE_SCENES_WRITE_ATTACKS,

            /**
             * Writes enemy data.
             */
            BATTLE_SCENES_WRITE_ENEMIES,

            /**
             * Writes enemy formation data.
             */
            BATTLE_SCENES_WRITE_FORMATIONS,

            /**
             * Initializes the battle installer for 3D model processing.
             */
            BATTLE_MODELS_INIT,

            /**
             * Extracts 3D battle models.
             */
            BATTLE_MODELS_PROCESS,

            /**
             * Prepares for 3D battle model conversion.
             */
            BATTLE_MODELS_CONVERT_INIT,

            /**
             * Converts 3D battle models.
             */
            BATTLE_MODELS_CONVERT,

            /**
             * Writes playable character model info.
             */
            BATTLE_MODELS_WRITE_CHARACTERS,

            /**
             * Writes battle scene model info.
             */
            BATTLE_MODELS_WRITE_SCENES,

            /**
             * Initializes the battle installer for 3D spell processing.
             */
            SPELL_MODELS_INIT,

            /**
             * Extracts 3D spell models.
             */
            SPELL_MODELS_PROCESS,

            /**
             * Prepares for 3D spell model conversion.
             */
            SPELL_MODELS_CONVERT_INIT,

            /**
             * Converts 3D spell models.
             */
            SPELL_MODELS_CONVERT,

            /**
             * Parses item and materia prices from ff7.exe.
             */
            KERNEL_PRICES,

            /**
             * Parses command data from KERNEL.BIN.
             */
            KERNEL_COMMANDS,

            /**
             * Parses attack data from KERNEL.BIN.
             */
            KERNEL_ATTACKS,

            /**
             * Parses characters data from KERNEL.BIN.
             */
            KERNEL_CHARACTERS,

            /**
             * Parses item data from KERNEL.BIN.
             */
            KERNEL_ITEMS,

            /**
             * Parses growth data from KERNEL.BIN.
             */
            KERNEL_GROWTH,

            /**
             * Parses weapon data from KERNEL.BIN.
             */
            KERNEL_WEAPONS,

            /**
             * Parses armor data from KERNEL.BIN.
             */
            KERNEL_ARMORS,

            /**
             * Parses accessory data from KERNEL.BIN.
             */
            KERNEL_ACCESSORIES,

            /**
             * Parses materia data from KERNEL.BIN.
             */
            KERNEL_MATERIA,

            /**
             * Parses key item data data from KERNEL.BIN.
             */
            KERNEL_KEY_ITEMS,

            /**
             * Parses summon names data from KERNEL.BIN.
             */
            KERNEL_SUMMON_NAMES,

            /**
             * Parses the initial savemap from KERNEL.BIN.
             */
            KERNEL_SAVEMAP,

            /**
             * Extract game images and icons from menu.lgp.
             */
            MEDIA_IMAGES,

            /**
             * Prepares the installer for sound extraction.
             */
            MEDIA_SOUNDS_INIT,

            /**
             * Extract game sounds.
             */
            MEDIA_SOUNDS,

            /**
             * Build sounds index.
             */
            MEDIA_SOUNDS_INDEX,

            /**
             * Prepares the installer for music extraction.
             */
            MEDIA_MUSICS_INIT,

            /**
             * Extracts the game music.
             */
            MEDIA_MUSICS,

            /**
             * Extracts the game high quality music.
             */
            MEDIA_MUSICS_HQ,

            /**
             * Build the music index.
             */
            MEDIA_MUSICS_INDEX,

            /**
             * Initializer for {@see SPAWN_POINTS_AND_SCALE_FACTORS}.
             */
            FIELD_SPAWN_POINTS_AND_SCALE_FACTORS_INIT,

            /**
             * Step that collects spawn points and sclae factors.
             */
            FIELD_SPAWN_POINTS_AND_SCALE_FACTORS,

            /**
             * Step that convets fields to V-Gears format.
             */
            FIELD_CONVERT,

            /**
             * Initializer for {@see WRITE_MAPS}.
             */
            FIELD_WRITE_INIT,

            /**
             * Step that writes XML maps.
             */
            FIELD_WRITE,

            /**
             * Clean up after {@see WRITE_MAPS}.
             */
            FIELD_WRITE_END,

            /**
             * Initializer for {@see CONVERT_FIELD_MODELS}.
             */
            FIELD_CONVERT_MODELS_INIT,

            /**
             * Step that converts field models.
             */
            FIELD_CONVERT_MODELS,

            /**
             * Prepares the installer for world map data installation.
             */
            WM_INIT,

            /**
             * Generates the materials for the world map terrain.
             */
            WM_MATERIALS,

            /**
             * Step that extracts world map terrain data.
             */
            WM_MAPS,

            /**
             * Step that extracts world map models.
             */
            WM_MODELS,

            /**
             * Cleans up after the installation.
             */
            CLEAN,

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
         * Substeps in the current installation step.
         */
        int substeps_;

        /**
         * Current substep in the current installation step.
         */
        int cur_substep_;

        /**
         * Weight of each installation step.
         *
         * Used to calculate current progress.
         */
        int step_weight_[STATE_COUNT];

        /**
         * List of field model names.
         */
        std::vector<std::string> field_model_names_;

        /**
         * The path to the directory from which to read the PC game data.
         */
        std::string input_dir_;

        /**
         * The path to the directory where to save the V-Gears data.
         */
        std::string output_dir_;

        /**
         * Advanced options.
         */
        AdvancedOptions options_;

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
         * The installer for fiel maps data.
         */
        std::unique_ptr<FieldDataInstaller> field_installer_;

        /**
         * The installer for kernel data.
         */
        std::unique_ptr<KernelDataInstaller> kernel_installer_;

        /**
         * The installer for media files.
         */
        std::unique_ptr<MediaDataInstaller> media_installer_;

        /**
         * The installer for battle data.
         */
        std::unique_ptr<BattleDataInstaller> battle_installer_;

        /**
         * The installer for world map data.
         */
        std::unique_ptr<WorldInstaller> world_installer_;

        /**
         * Function used to print text to the log output, line by line.
         *
         * @param[in] msg the message to write.
         * @param[in] level Log level for the message.
         * @param[in] as_progress If true, the text will also be set as the current text behind the
         * progress bar.
         */
        std::function<void(std::string msg, int level, bool progress)> write_output_line_;

};
