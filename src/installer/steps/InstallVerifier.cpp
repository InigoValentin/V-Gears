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

#include "common/DataDirectory.h"
#include "InstallVerifier.h"
#include <boost/filesystem.hpp>
#include "common/InstallStamp.h"
#include "common/OgrePlugins.h"

/**
 * Directory with the Ogre plugins, found when V-Gears was built. Set by CMake (VGEARS_OGRE_PLUGIN_DIR).
 */
#ifndef VGEARS_OGRE_PLUGIN_DIR
#define VGEARS_OGRE_PLUGIN_DIR ""
#endif

namespace fs = boost::filesystem;

namespace{

    /**
     * Collects the results of the checks.
     */
    class Checker{

        public:

            Checker(const std::string& root, VerifyResult& result): root_(root), result_(result){
                if (!root_.empty() && root_.back() != '/') root_ += "/";
            }

            /**
             * A file must exist.
             */
            void File(const std::string& group, const std::string& path){
                ++ result_.checked;
                boost::system::error_code ec;
                if (!fs::is_regular_file(root_ + path, ec)) Fail(group, path + " is missing");
            }

            /**
             * The Ogre plugins listed in plugins.cfg must be found where the game looks for them.
             *
             * @param[in] group The group of the problems.
             * @param[in] search Where to look for them.
             */
            void OgrePlugins(const std::string& group, const VerifyPlugins& search){
                if (search.ogre_version.empty()) return;
                namespace plugins = VGears::OgrePlugins;
                const fs::path file(root_ + "plugins.cfg");
                boost::system::error_code ec;
                // A missing plugins.cfg is already reported.
                if (!fs::is_regular_file(file, ec)) return;
                ++ result_.checked;
                plugins::Config config;
                if (!plugins::ReadConfig(file, config)){
                    Fail(group, "plugins.cfg can't be read");
                    return;
                }
                const std::vector<fs::path> directories = plugins::SearchDirectories(
                  file.parent_path(), config.folder, fs::path(search.executable_dir), VGEARS_OGRE_PLUGIN_DIR
                );
                for (const std::string& name : config.plugins){
                    ++ result_.checked;
                    if (plugins::Find(name, directories, search.ogre_version).empty())
                        Fail(group, "the Ogre plugin " + name + " is not in the installation directory or the system");
                }
            }

            /**
             * A directory must have at least a number of files with an extension, searching in subdirectories.
             *
             * @param[in] extension Extension with the dot, or the full file name if it has none (like "map.xml").
             * An empty string matches every file.
             */
            void Files(
              const std::string& group, const std::string& dir, const std::string& extension, int minimum
            ){
                ++ result_.checked;
                int found = 0;
                boost::system::error_code ec;
                for (fs::recursive_directory_iterator it(root_ + dir, ec), end; !ec && it != end; it.increment(ec)){
                    if (!fs::is_regular_file(it->path(), ec)) continue;
                    const std::string name = it->path().filename().string();
                    if (
                      extension.empty() || (name.size() >= extension.size()
                        && name.compare(name.size() - extension.size(), extension.size(), extension) == 0)
                    ){
                        if (++ found >= minimum) return;
                    }
                }
                std::string wanted = extension.empty() ? "files" : extension + " files";
                Fail(
                  group, dir + " has " + std::to_string(found) + " " + wanted + ", at least "
                    + std::to_string(minimum) + " expected"
                );
            }

            void Fail(const std::string& group, const std::string& message){
                result_.problems.push_back({group, message});
            }

        private:

            std::string root_;
            VerifyResult& result_;
    };

}

VerifyResult VerifyInstallation(
  const std::string& root, const std::string& reference_dir, const VerifyPlugins& plugins
){
    VerifyResult result;
    Checker check(root, result);

    // The record of the installations: whether the last one finished, and whether a part is outdated.
    const std::string installation = "Installation";
    ++ result.checked;
    bool recorded = false;
    for (const std::string& problem : VGears::InstallStamp::Problems(fs::path(root), &recorded))
        result.problems.push_back({installation, problem});
    if (!recorded){
        result.problems.push_back({
          installation, "there is no record of the installation: it was made by an older installer, whose data may "
          "be outdated. Install the data again."
        });
    }

    // Configuration, plugins and the files of the repository, which the game always needs.
    const std::string config = "Configuration";
    check.File(config, "plugins.cfg");
    check.File(config, "resources.cfg");
    // The plugins don't have to be in the installation directory: the game also looks for them next to its
    // executable and in the system.
    check.OgrePlugins(config, plugins);
    const std::string game = "Game files";
    for (const char* file : {
      "user/config.cfg", "engine/system/system.fontdef", "engine/system/system.lua", "engine/system/system.material",
      "engine/fonts/_fonts.xml", "engine/ui/_screens.xml", "engine/scripts/_scripts.xml", "engine/texts/_texts.xml",
      "engine/metadata/field_models_and_animation_metadata.xml"
    }) check.File(game, file);

    // Everything in the repository's data folder.
    if (!reference_dir.empty()){
        std::string reference = reference_dir;
        if (reference.back() != '/') reference += "/";
        boost::system::error_code ec;
        std::string installed = root;
        if (installed.empty() || installed.back() != '/') installed += "/";
        int missing = 0;
        std::string first;
        for (fs::recursive_directory_iterator it(reference, ec), end; !ec && it != end; it.increment(ec)){
            if (!fs::is_regular_file(it->path(), ec)) continue;
            const std::string relative = it->path().string().substr(reference.size());
            if (VGears::DataDirectory::IsPlayerPath(relative)) continue;
            ++ result.checked;
            if (!fs::exists(installed + relative, ec)){
                if (missing ++ == 0) first = relative;
            }
        }
        if (missing > 0){
            check.Fail(
              "Copied data files",
              std::to_string(missing) + " file(s) of the V-Gears data folder are missing, for example " + first
            );
        }
    }

    // Converted game data.
    const std::string kernel = "Kernel data";
    for (const char* file : {
      "accessories", "armors", "attacks", "characters", "commands", "growth", "initial_savemap", "items", "key_items",
      "materia", "summons", "weapons"
    }) check.File(kernel, std::string("game/kernel/") + file + ".lua");

    const std::string battle = "Battle data";
    check.File(battle, "game/battle/attacks/_index.xml");
    check.Files(battle, "game/battle/enemies", ".xml", 1);
    check.Files(battle, "game/battle/formations", ".xml", 1);

    const std::string battle_models = "Battle models";
    check.File(battle_models, "game/battle/models/characters.xml");
    check.File(battle_models, "game/battle/models/scenes.xml");
    check.Files(battle_models, "game/battle/models", ".mesh", 1);

    const std::string fields = "Field maps";
    check.File(fields, "game/field/maps/_fields.xml");
    check.File(fields, "game/field/maps/md1stin/map.xml");
    check.Files(fields, "game/field/maps", "map.xml", 500);

    const std::string field_models = "Field models";
    for (const char* file : {"cloud.mesh", "cloud.skeleton", "cloud.material"})
        check.File(field_models, std::string("game/field/models/") + file);
    check.Files(field_models, "game/field/models", ".mesh", 100);

    const std::string world = "World map";
    for (const char* file : {"WM0.MAP", "WM2.MAP", "WM3.MAP"}) check.File(world, std::string("game/world/maps/") + file);
    // The scripts of the maps are decompiled by the installer, the other ones are copied from the
    // V-Gears data folder.
    for (const char* file : {"wm0.lua", "wm2.lua", "wm3.lua"})
        check.File(world, std::string("game/world/scripts/") + file);
    for (const char* file : {"triangle_triggers.lua", "vehicles.lua", "script_stubs.lua"})
        check.File(world, std::string("engine/scripts/world/") + file);
    check.File(world, "game/world/world_encounters.xml");
    check.Files(world, "game/world/models/terrain/0", ".mesh", 1);
    check.Files(world, "game/world/textures", ".tex", 1);

    const std::string world_models = "World map models";
    check.Files(world_models, "game/world/models", ".skeleton", 1);

    // The sounds and the music are converted with FFmpeg and TiMidity: without them there are no ogg files.
    const std::string sounds = "Sound effects";
    check.File(sounds, "game/audio/sounds/_sounds.xml");
    check.Files(sounds, "game/audio/sounds", ".ogg", 100);
    const std::string music = "Music";
    check.File(music, "game/audio/music/_musics.xml");
    check.Files(music, "game/audio/music", ".ogg", 50);
    const std::string images = "Images";
    check.Files(images, "game/ui/images/icons", "", 1);
    // Fonts, built from WINDOW.BIN. They are installed even when images are not.
    const std::string fonts = "Fonts";
    for (const char* file : {
      "game/ui/fonts/ffvii_en.png", "game/ui/fonts/ffvii_menu.png", "game/ui/fonts/ffvii_digits.png", "game/ui/fonts/ffvii_timer.png"
    }) check.File(fonts, file);
    return result;
}
