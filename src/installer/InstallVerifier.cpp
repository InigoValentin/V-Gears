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

#include "InstallVerifier.h"
#include <boost/filesystem.hpp>

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
             * A file whose name starts with a prefix (a library with a version number) must exist in a directory.
             */
            void FileWithPrefix(const std::string& group, const std::string& dir, const std::string& prefix){
                ++ result_.checked;
                boost::system::error_code ec;
                for (fs::directory_iterator it(root_ + dir, ec), end; !ec && it != end; it.increment(ec)){
                    if (it->path().filename().string().compare(0, prefix.size(), prefix) == 0) return;
                }
                Fail(group, "no " + prefix + "* file in " + (dir.empty() ? "the installation directory" : dir));
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

VerifyResult VerifyInstallation(const std::string& root, const std::string& reference_dir){
    VerifyResult result;
    Checker check(root, result);

    // Configuration, plugins and the files of the repository, which the game always needs.
    const std::string config = "Configuration";
    check.File(config, "plugins.cfg");
    check.File(config, "resources.cfg");
    check.FileWithPrefix(config, "", "RenderSystem_GL");
    check.FileWithPrefix(config, "", "Codec_STBI");
    const std::string game = "Game files";
    for (const char* file : {
      "data/config/config.cfg", "data/system/system.fontdef", "data/system/system.lua", "data/system/system.material",
      "data/fonts/_fonts.xml", "data/screens/_screens.xml", "data/scripts/_scripts.xml", "data/texts/_texts.xml"
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
            ++ result.checked;
            const std::string relative = it->path().string().substr(reference.size());
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
    }) check.File(kernel, std::string("data/gamedata/") + file + ".lua");

    const std::string battle = "Battle data";
    check.File(battle, "data/gamedata/attack/_index.xml");
    check.Files(battle, "data/gamedata/enemy", ".xml", 1);
    check.Files(battle, "data/gamedata/formation", ".xml", 1);

    const std::string battle_models = "Battle models";
    check.File(battle_models, "data/models/battle/characters.xml");
    check.File(battle_models, "data/models/battle/scenes.xml");
    check.Files(battle_models, "data/models/battle", ".mesh", 1);

    const std::string fields = "Field maps";
    check.File(fields, "data/fields/_fields.xml");
    check.File(fields, "data/fields/md1stin/map.xml");
    check.Files(fields, "data/fields", "map.xml", 500);

    const std::string field_models = "Field models";
    for (const char* file : {"cloud.mesh", "cloud.skeleton", "cloud.material"})
        check.File(field_models, std::string("data/models/fields/") + file);
    check.Files(field_models, "data/models/fields", ".mesh", 100);

    const std::string world = "World map";
    for (const char* file : {"WM0.MAP", "WM2.MAP", "WM3.MAP"}) check.File(world, std::string("data/wm/") + file);
    check.File(world, "data/scripts/worldmap/wm0.lua");
    check.File(world, "data/gamedata/world_encounters.xml");
    check.Files(world, "data/models/world/terrain/0", ".mesh", 1);

    const std::string world_models = "World map models";
    check.Files(world_models, "temp/world_models", "", 1);
    check.Files(world_models, "data/models/world", ".skeleton", 1);

    // The sounds and the music are converted with FFmpeg and TiMidity: without them there are no ogg files.
    const std::string sounds = "Sound effects";
    check.File(sounds, "data/audio/sounds/_sounds.xml");
    check.Files(sounds, "data/audio/sounds", ".ogg", 100);
    const std::string music = "Music";
    check.File(music, "data/audio/musics/_musics.xml");
    check.Files(music, "data/audio/musics", ".ogg", 50);
    const std::string images = "Images";
    check.Files(images, "data/images/icons", "", 1);
    return result;
}
