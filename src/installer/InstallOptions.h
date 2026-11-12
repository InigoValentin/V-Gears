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

#include <cstdint>
#include <string>
#include <vector>

/**
 * The options of an installation, and the rules between them.
 */
struct InstallOptions{
    bool battle_data = false;
    bool battle_models = false;
    bool kernel = false;
    bool images = false;
    bool sounds = false;
    bool music = false;
    bool videos = false;
    bool fields = false;
    bool field_models = false;
    bool wm = false;
    bool wm_models = false;
};

/**
 * Ready-made sets of options.
 */
enum class InstallPreset{EVERYTHING, FIELDS, WORLD_MAP, BATTLE, MEDIA, CUSTOM};

namespace InstallRules{

    /**
     * Gets the options of a preset.
     *
     * The custom preset has everything off.
     *
     * @param[in] preset The preset.
     * @param[in] videos_available True if the input disk has videos.
     */
    InstallOptions Preset(InstallPreset preset, bool videos_available);

    /**
     * Disables every option whose requirements are not met, so a disabled option is never active.
     *
     * - Field 3D models need field maps.
     * - World map 3D models need world map data.
     */
    void ApplyDependencies(InstallOptions& options);

    /**
     * Options that can currently be chosen, that is, whose requirements are met.
     */
    InstallOptions Selectable(const InstallOptions& options);

    /**
     * Describes the problems of an installation that can be detected from its options.
     *
     * @param[in] options The options.
     * @param[in] fields_installed True if the field maps are already installed in the installation directory.
     */
    std::vector<std::string> Problems(const InstallOptions& options, bool fields_installed);

    /**
     * Which external programs the options need.
     */
    struct ToolNeeds{

        /**
         * FFmpeg: converts the sounds and the music.
         */
        bool ffmpeg = false;

        /**
         * TiMidity: renders the MIDI music, and its output is converted by FFmpeg.
         */
        bool timidity = false;
    };

    /**
     * Gets the programs needed to install what the options choose.
     */
    ToolNeeds ToolsNeeded(const InstallOptions& options);

    /**
     * Describes what the installation will be missing if the programs can't be used.
     *
     * @param[in] options The options.
     * @param[in] ffmpeg_usable True if a working FFmpeg was found.
     * @param[in] timidity_usable True if a working TiMidity was found.
     * @return One sentence for each part that can't be installed. Empty if nothing will be missing.
     */
    std::vector<std::string> ToolConsequences(const InstallOptions& options, bool ffmpeg_usable, bool timidity_usable);

    /**
     * Estimates the disk space the options use, in MB.
     *
     * The values come from a full installation of the PC game: they are an orientation, not a promise. Videos are
     * not estimated because they depend on the disk.
     *
     * @param[in] options The options.
     * @param[out] temporary Extra space used while installing that is released at the end.
     * @return Space used by the installed data.
     */
    uint64_t EstimateMegabytes(const InstallOptions& options, uint64_t* temporary = nullptr);

}
