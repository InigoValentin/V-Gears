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

#include <algorithm>
#include <boost/filesystem.hpp>
#include "common/DataDirectory.h"
#include "common/InstallStamp.h"
#include "InstallLayout.h"

namespace InstallLayout{

    namespace{

        /**
         * Removes a file or a directory with its contents, and counts what was removed.
         */
        void Remove(const boost::filesystem::path& path, CleanResult& result){
            boost::system::error_code ec;
            const auto removed = boost::filesystem::remove_all(path, ec);
            if (ec) result.errors.push_back(path.string() + ": " + ec.message());
            else result.removed += removed;
        }

    }

    std::string OriginalDataDir(const std::string& root){return root + "original_data/";}

    const std::vector<std::string>& CleanInstallKeptGameDirs(){
        static const std::vector<std::string> dirs = {"video"};
        return dirs;
    }

    bool IsInstallation(const std::string& root){
        boost::system::error_code ec;
        return boost::filesystem::is_regular_file(root + "plugins.cfg", ec)
          && boost::filesystem::is_regular_file(root + "resources.cfg", ec);
    }

    CleanResult CleanInstallation(const std::string& root){
        namespace fs = boost::filesystem;
        CleanResult result;
        if (!IsInstallation(root)) return result;
        boost::system::error_code ec;
        // The saves, the configuration and the videos of an older version are moved out of the directories that
        // are deleted. Nothing is deleted if that fails.
        try{
            VGears::DataDirectory::MigrateLayout(root);
        }
        catch (const fs::filesystem_error& e){
            result.errors.push_back(e.what());
            return result;
        }
        // "data" is where older versions had the files that are now in "engine" and "game".
        for (const char* dir : {"original_data/", "temp/", "data/", "engine/", "defaults/"})
            if (fs::exists(root + dir, ec)) Remove(root + dir, result);
        // The record of the installations describes what was just deleted.
        if (fs::exists(root + VGears::InstallStamp::FILE_NAME, ec)) Remove(root + VGears::InstallStamp::FILE_NAME, result);
        const fs::path game(GameDir(root));
        if (fs::is_directory(game, ec)){
            const std::vector<std::string>& kept = CleanInstallKeptGameDirs();
            std::vector<fs::path> entries;
            for (fs::directory_iterator it(game, ec), end; !ec && it != end; it.increment(ec))
                entries.push_back(it->path());
            if (ec) result.errors.push_back(game.string() + ": " + ec.message());
            for (const fs::path& entry : entries){
                if (std::find(kept.begin(), kept.end(), entry.filename().string()) == kept.end())
                    Remove(entry, result);
            }
        }
        return result;
    }

    std::string GameDir(const std::string& root){return root + "game/";}

    std::string TempDir(const std::string& root){return root + "temp/";}

    const std::vector<std::string>& GameSubdirs(){
        static const std::vector<std::string> dirs = {
          "kernel", "field/maps", "field/models", "battle/enemies", "battle/attacks", "battle/formations",
          "battle/effects", "battle/scripts", "battle/images", "battle/models/characters", "battle/models/scenes",
          "battle/models/enemies", "battle/models/attacks", "battle/models/effects", "world/maps", "world/textures",
          "world/scripts", "world/models", "ui/images/icons", "ui/images/other",
          "ui/images/characters", "ui/images/fonts", "ui/images/reels", "ui/images/window", "ui/fonts",
          "audio/music", "audio/sounds"
        };
        return dirs;
    }

    const std::vector<std::string>& TempSubdirs(){
        static const std::vector<std::string> dirs = {"char", "battle_models", "world_models", "spell_models", "effects"};
        return dirs;
    }

    const std::vector<std::string>& GameResourceDirs(){
        static const std::vector<std::string> dirs = {
          "engine/system", "engine/fonts", "engine/images", "engine/ui", "engine/scripts", "engine/texts",
          "game/kernel", "game/field/maps", "game/field/models", "game/battle/models", "game/battle/images",
          "game/world/maps", "game/world/textures", "game/world/models", "game/ui/images", "game/ui/fonts",
          "game/audio"
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
