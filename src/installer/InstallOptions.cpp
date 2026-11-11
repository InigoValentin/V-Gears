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

#include "InstallOptions.h"

namespace InstallRules{

    InstallOptions Preset(InstallPreset preset, bool videos_available){
        InstallOptions o;
        switch (preset){
            case InstallPreset::EVERYTHING:
                o.battle_data = o.battle_models = o.kernel = o.images = o.sounds = o.music = true;
                o.fields = o.field_models = o.wm = o.wm_models = true;
                o.videos = videos_available;
                break;
            case InstallPreset::FIELDS:
                o.kernel = o.fields = o.field_models = true;
                break;
            case InstallPreset::WORLD_MAP:
                o.wm = o.wm_models = true;
                break;
            case InstallPreset::BATTLE:
                o.kernel = o.battle_data = o.battle_models = true;
                break;
            case InstallPreset::MEDIA:
                o.images = o.sounds = o.music = true;
                o.videos = videos_available;
                break;
            case InstallPreset::CUSTOM:
                break;
        }
        ApplyDependencies(o);
        return o;
    }

    InstallOptions Selectable(const InstallOptions& options){
        InstallOptions s;
        s.battle_data = s.battle_models = s.kernel = s.images = s.sounds = s.music = s.videos = true;
        s.fields = s.wm = true;
        s.field_models = options.fields;
        s.wm_models = options.wm;
        return s;
    }

    void ApplyDependencies(InstallOptions& o){
        // Applied until nothing changes, in case a rule depends on the result of another.
        bool changed = true;
        while (changed){
            const InstallOptions before = o;
            const InstallOptions s = Selectable(o);
            o.field_models = o.field_models && s.field_models;
            o.wm_models = o.wm_models && s.wm_models;
            changed = o.field_models != before.field_models || o.wm_models != before.wm_models;
        }
    }

    std::vector<std::string> Problems(const InstallOptions& options, bool fields_installed){
        std::vector<std::string> problems;
        if (options.wm && !options.fields && !fields_installed){
            problems.push_back(
              "The world map scripts need the field maps: select 'Extract field maps', or install them first."
            );
        }
        return problems;
    }

    ToolNeeds ToolsNeeded(const InstallOptions& o){
        ToolNeeds needs;
        needs.ffmpeg = o.sounds || o.music || o.videos;
        needs.timidity = o.music;
        return needs;
    }

    std::vector<std::string> ToolConsequences(const InstallOptions& o, bool ffmpeg_usable, bool timidity_usable){
        std::vector<std::string> consequences;
        const ToolNeeds needs = ToolsNeeded(o);
        if (needs.ffmpeg && !ffmpeg_usable){
            if (o.sounds) consequences.push_back("The sound effects will not be installed: FFmpeg is needed to convert them.");
            if (o.music) consequences.push_back("The music will not be installed: FFmpeg is needed to convert it.");
        }
        else if (needs.timidity && !timidity_usable){
            consequences.push_back("The music will not be installed: TiMidity is needed to play the MIDI files.");
        }
        return consequences;
    }

    uint64_t EstimateMegabytes(const InstallOptions& o, uint64_t* temporary){
        uint64_t installed = 0;
        uint64_t temp = 0;
        if (o.kernel) installed += 1;
        if (o.battle_data) installed += 10;
        if (o.battle_models){
            installed += 220;
            temp += 225;
        }
        if (o.images) installed += 2;
        if (o.sounds) installed += 100;
        if (o.music) installed += 130;
        if (o.fields) installed += 380;
        if (o.field_models){
            installed += 190;
            temp += 85;
        }
        if (o.wm){
            installed += 30;
            temp += 6;
        }
        if (o.wm_models){
            installed += 7;
            temp += 6;
        }
        if (temporary) *temporary = temp;
        return installed;
    }

}
