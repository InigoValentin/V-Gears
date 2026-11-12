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

#include "InstallLayout.h"

namespace InstallLayout{

    std::string DataDir(const std::string& root){return root + "data/";}

    std::string TempDir(const std::string& root){return root + "temp/";}

    const std::vector<std::string>& DataSubdirs(){
        static const std::vector<std::string> dirs = {
          "audio/musics", "audio/sounds", "fields", "fonts", "gamedata", "gamedata/enemy", "gamedata/attack",
          "gamedata/formation", "images/icons", "images/other", "images/characters", "images/fonts", "images/reels",
          "images/window", "screens", "scripts", "scripts/battle_models", "system", "texts", "models/fields",
          "models/battle/characters", "models/battle/scenes", "models/battle/enemies", "models/battle/attacks", "models/battle/effects",
          "models/world/terrain/0", "models/world/terrain/1", "models/world/terrain/2", "models/world/elements",
          "world/0", "world/1", "world/2", "wm"
        };
        return dirs;
    }

    const std::vector<std::string>& TempSubdirs(){
        static const std::vector<std::string> dirs = {"char", "battle_models", "world_models", "spell_models", "effects", "wm"};
        return dirs;
    }

    const std::vector<std::string>& KeptTempSubdirs(){
        static const std::vector<std::string> dirs = {"world_models"};
        return dirs;
    }

    std::vector<std::string> RemovableTempSubdirs(){
        std::vector<std::string> removable;
        for (const std::string& dir : TempSubdirs()){
            bool kept = false;
            for (const std::string& k : KeptTempSubdirs()) kept = kept || (k == dir);
            if (!kept) removable.push_back(dir);
        }
        return removable;
    }

    const std::vector<std::string>& GameResourceDirs(){
        static const std::vector<std::string> dirs = {
          "data/system", "data/audio", "data/fields", "data/fonts", "data/gamedata", "data/images", "data/models/fields",
          "data/models/world", "data/models/battle", "data/screens", "data/scripts", "data/texts", "data/wm",
          "temp/world_models"
        };
        return dirs;
    }

    std::string MigrateInstallDir(
      std::string legacy_data_dir, const std::string& legacy_config_dir, const std::string& default_dir
    ){
        while (!legacy_data_dir.empty() && legacy_data_dir.back() == '/') legacy_data_dir.pop_back();
        const std::string suffix = "/data";
        if (
          legacy_data_dir.size() >= suffix.size()
          && legacy_data_dir.compare(legacy_data_dir.size() - suffix.size(), suffix.size(), suffix) == 0
        ) legacy_data_dir.erase(legacy_data_dir.size() - suffix.size());
        if (!legacy_data_dir.empty()) return legacy_data_dir;
        if (!legacy_config_dir.empty()) return legacy_config_dir;
        return default_dir;
    }

}
