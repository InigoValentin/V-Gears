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
#include <cstdlib>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

namespace VGears{

    namespace DataDirectory{

        namespace bfs = boost::filesystem;

        const char* ENV_DATA_DIR("VGEARS_DATA_DIR");

        const char* ENV_TEMPLATE_DIR("VGEARS_TEMPLATE_DIR");

        namespace{

            /**
             * Checks if two paths are the same directory, without failing if one of them doesn't exist.
             */
            bool SameDirectory(const bfs::path& a, const bfs::path& b){
                if (a.empty() || b.empty()) return false;
                boost::system::error_code ec;
                const bool same = bfs::equivalent(a, b, ec);
                if (!ec) return same;
                return a.lexically_normal() == b.lexically_normal();
            }

            /**
             * Checks if a directory exists and has something inside.
             */
            bool IsNonEmptyDirectory(const bfs::path& directory){
                boost::system::error_code ec;
                if (!bfs::is_directory(directory, ec)) return false;
                return bfs::directory_iterator(directory, ec) != bfs::directory_iterator();
            }

            /**
             * Checks if a directory has the layout of older versions: plugins.cfg and a "data" subdirectory.
             */
            bool IsLegacyDataDirectory(const bfs::path& directory){
                boost::system::error_code ec;
                return bfs::is_regular_file(directory / "plugins.cfg", ec) && bfs::is_directory(directory / "data", ec);
            }

            /**
             * Moves a file or a directory without overwriting anything. Directories that exist in both places are
             * merged, and removed from the origin if they end up empty.
             *
             * @return The number of files and directories moved.
             */
            unsigned int MoveMissing(const bfs::path& from, const bfs::path& to){
                if (!bfs::exists(from)) return 0;
                if (!bfs::exists(to)){
                    bfs::create_directories(to.parent_path());
                    bfs::rename(from, to);
                    return 1;
                }
                if (!bfs::is_directory(from) || !bfs::is_directory(to)) return 0;
                std::vector<bfs::path> entries;
                for (bfs::directory_iterator it(from), end; it != end; ++ it) entries.push_back(it->path());
                unsigned int moved = 0;
                for (const bfs::path& entry : entries) moved += MoveMissing(entry, to / entry.filename());
                if (bfs::is_empty(from)) bfs::remove(from);
                return moved;
            }

            /**
             * Gets the home directory of the user, or an empty path.
             */
            bfs::path HomeDirectory(){
#ifdef _WIN32
                if (const char* profile = std::getenv("USERPROFILE")) if (*profile) return bfs::path(profile);
                const char* drive = std::getenv("HOMEDRIVE");
                const char* path = std::getenv("HOMEPATH");
                if (drive && path) return bfs::path(std::string(drive) + path);
                return bfs::path();
#else
                if (const char* home = std::getenv("HOME")) if (*home) return bfs::path(home);
                if (const passwd* user = getpwuid(getuid())) if (user->pw_dir) return bfs::path(user->pw_dir);
                return bfs::path();
#endif
            }
        }

        bool IsDataDirectory(const bfs::path& directory){
            boost::system::error_code ec;
            return bfs::is_regular_file(directory / "plugins.cfg", ec) && bfs::is_directory(directory / "engine", ec);
        }

        bfs::path DefaultDirectory(){
            const bfs::path home = HomeDirectory();
            if (home.empty()) return bfs::path();
#ifdef _WIN32
            return home / "v-gears";
#else
            return home / ".v-gears";
#endif
        }

        bfs::path ExecutableDirectory(const char* argv0){
            boost::system::error_code ec;
#ifdef _WIN32
            wchar_t buffer[MAX_PATH * 4];
            const DWORD length = GetModuleFileNameW(nullptr, buffer, sizeof(buffer) / sizeof(buffer[0]));
            if (length > 0 && length < sizeof(buffer) / sizeof(buffer[0]))
                return bfs::path(std::wstring(buffer, length)).parent_path();
#else
            const bfs::path self = bfs::read_symlink("/proc/self/exe", ec);
            if (!ec && !self.empty()) return self.parent_path();
#endif
            if (argv0 == nullptr || *argv0 == '\0') return bfs::current_path(ec);
            return bfs::absolute(bfs::path(argv0)).lexically_normal().parent_path();
        }

        std::vector<bfs::path> TemplateCandidates(const bfs::path& executable_directory){
            std::vector<bfs::path> candidates;
            if (const char* env = std::getenv(ENV_TEMPLATE_DIR)) if (*env) candidates.emplace_back(env);
            const bfs::path& exe = executable_directory;
            // Portable folder: <dir>/v-gears(.exe) and <dir>/data/plugins.cfg.
            candidates.push_back(exe / "data");
            // Installed package: <prefix>/bin or <prefix>/games, and <prefix>/share/v-gears.
            candidates.push_back((exe / ".." / "share" / "v-gears").lexically_normal());
            // Build tree: <repository>/build/bin and <repository>/data. Also <repository>/build/<type>/bin.
            candidates.push_back((exe / ".." / "data").lexically_normal());
            candidates.push_back((exe / ".." / ".." / "data").lexically_normal());
            candidates.push_back((exe / ".." / ".." / ".." / "data").lexically_normal());
#ifndef _WIN32
            candidates.push_back("/usr/share/v-gears");
            candidates.push_back("/usr/local/share/v-gears");
#endif
            return candidates;
        }

        bfs::path FindTemplate(const std::vector<bfs::path>& candidates, const bfs::path& exclude){
            for (const bfs::path& candidate : candidates){
                if (SameDirectory(candidate, exclude)) continue;
                if (IsDataDirectory(candidate)) return candidate;
            }
            return bfs::path();
        }

        unsigned int CopyMissing(const bfs::path& from, const bfs::path& to){
            unsigned int copied = 0;
            bfs::create_directories(to);
            for (bfs::recursive_directory_iterator it(from), end; it != end; ++ it){
                const bfs::path relative = it->path().lexically_relative(from);
                const bfs::path target = to / relative;
                if (IsPlayerPath(relative)) continue;
                if (bfs::is_directory(it->status())) bfs::create_directories(target);
                else if (bfs::is_regular_file(it->status()) && !bfs::exists(target)){
                    bfs::create_directories(target.parent_path());
                    bfs::copy_file(it->path(), target);
                    ++ copied;
                }
            }
            return copied;
        }

        bool IsPlayerPath(const bfs::path& relative){
            if (relative.empty()) return false;
            const std::string first = relative.begin()->string();
            return first == "user" || first == "logs";
        }

        unsigned int MigrateLayout(const bfs::path& directory){
            if (!bfs::is_regular_file(directory / "plugins.cfg")) return 0;
            const bfs::path user = directory / "user";
            unsigned int moved = 0;
            moved += MoveMissing(directory / "save", user / "saves");
            moved += MoveMissing(directory / "controls.cfg", user / "controls.cfg");
            moved += MoveMissing(directory / "console_history.txt", user / "console_history.txt");
            moved += MoveMissing(directory / "data" / "config" / "config.cfg", user / "config.cfg");
            moved += MoveMissing(directory / "data" / "video", directory / "game" / "video");
            std::vector<bfs::path> screenshots;
            for (bfs::directory_iterator it(directory), end; it != end; ++ it){
                const std::string name = it->path().filename().string();
                if (name.compare(0, 11, "screenshot_") == 0 && it->path().extension() == ".png")
                    screenshots.push_back(it->path());
            }
            for (const bfs::path& file : screenshots)
                moved += MoveMissing(file, user / "screenshots" / file.filename());
            return moved;
        }

        void PrepareDirectory(const bfs::path& directory){
            for (const char* dir : {
              "user/saves", "user/screenshots", "logs", "game/kernel", "game/field/maps", "game/field/models",
              "game/battle/models", "game/battle/images", "game/world/maps", "game/world/textures", "game/world/models",
              "game/ui/images", "game/ui/fonts", "game/audio"
            }) bfs::create_directories(directory / dir);
            const bfs::path defaults = directory / "defaults" / "config.cfg";
            const bfs::path config = directory / "user" / "config.cfg";
            if (bfs::exists(defaults) && !bfs::exists(config)) bfs::copy_file(defaults, config);
        }

        Resolution Resolve(
          const std::string& requested, const bfs::path& current, const bfs::path& default_directory,
          const std::vector<bfs::path>& template_candidates
        ){
            Resolution result;
            bfs::path directory;
            if (!requested.empty()) directory = bfs::absolute(bfs::path(requested), current);
            else if (IsDataDirectory(current) || IsLegacyDataDirectory(current)) directory = current;
            else directory = default_directory;
            if (directory.empty()){
                result.error = "The home directory can't be found. Use --data-dir or set "
                  + std::string(ENV_DATA_DIR) + " to choose the data directory.";
                return result;
            }
            directory = directory.lexically_normal();

            try{
                if (bfs::is_directory(directory)) result.migrated_files = MigrateLayout(directory);
            }
            catch (const bfs::filesystem_error& e){
                result.error = "The files of an older version could not be moved in " + directory.string() + ": "
                  + e.what();
                return result;
            }

            if (!IsDataDirectory(directory)){
                if (
                  IsNonEmptyDirectory(directory) && !bfs::is_directory(directory / "engine")
                  && !bfs::is_directory(directory / "game") && !bfs::is_directory(directory / "data")
                ){
                    result.error = "The directory " + directory.string() + " is not empty and has no V-Gears files, "
                      "so they are not copied there. Choose an empty or V-Gears directory.";
                    return result;
                }
                const bfs::path source = FindTemplate(template_candidates, directory);
                if (source.empty()){
                    result.error = "The directory " + directory.string() + " has no V-Gears files, and the files "
                      "shipped with V-Gears were not found to copy them. Reinstall V-Gears, or set "
                      + std::string(ENV_TEMPLATE_DIR) + " to the folder that contains plugins.cfg and engine/.";
                    return result;
                }
                try{
                    result.copied_files = CopyMissing(source, directory);
                }
                catch (const bfs::filesystem_error& e){
                    result.error = "The V-Gears files could not be copied from " + source.string() + " to "
                      + directory.string() + ": " + e.what();
                    return result;
                }
                result.seeded = true;
                result.template_directory = source;
                if (!IsDataDirectory(directory)){
                    result.error = "The V-Gears files were copied to " + directory.string()
                      + ", but it's still missing plugins.cfg or the engine folder.";
                    return result;
                }
            }
            try{
                PrepareDirectory(directory);
            }
            catch (const bfs::filesystem_error& e){
                result.error = "The directories of " + directory.string() + " could not be prepared: " + e.what();
                return result;
            }
            result.directory = directory;
            return result;
        }
    }
}
