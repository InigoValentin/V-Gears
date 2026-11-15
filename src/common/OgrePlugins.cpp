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

#include "common/OgrePlugins.h"
#include <algorithm>
#include <fstream>

namespace VGears{

    namespace OgrePlugins{

        namespace bfs = boost::filesystem;

        namespace{

            std::string Trim(const std::string& text){
                const char* blanks = " \t\r\n";
                const size_t first = text.find_first_not_of(blanks);
                if (first == std::string::npos) return "";
                return text.substr(first, text.find_last_not_of(blanks) - first + 1);
            }

            /**
             * Usual directories of the Ogre plugins installed by the system.
             */
            const std::vector<bfs::path> SYSTEM_DIRECTORIES = {
#ifndef _WIN32
              "/usr/lib/x86_64-linux-gnu/OGRE", "/usr/lib/aarch64-linux-gnu/OGRE", "/usr/lib/i386-linux-gnu/OGRE",
              "/usr/lib64/OGRE", "/usr/lib/OGRE", "/usr/local/lib/OGRE", "/usr/local/lib64/OGRE"
#endif
            };
        }

        bool ReadConfig(const bfs::path& file, Config& config){
            config = Config();
            std::ifstream in(file.string());
            if (!in.good()) return false;
            std::string line;
            while (std::getline(in, line)){
                line = Trim(line);
                if (line.empty() || line[0] == '#') continue;
                const size_t equals = line.find('=');
                if (equals == std::string::npos) continue;
                const std::string key = Trim(line.substr(0, equals));
                const std::string value = Trim(line.substr(equals + 1));
                if (key == "PluginFolder") config.folder = value;
                else if (key == "Plugin" && !value.empty()) config.plugins.push_back(value);
            }
            return true;
        }

        std::vector<bfs::path> SearchDirectories(
          const bfs::path& config_directory, const std::string& config_folder,
          const bfs::path& executable_directory, const std::string& build_directory
        ){
            std::vector<bfs::path> directories;
            auto add = [&directories](const bfs::path& directory){
                if (directory.empty()) return;
                const bfs::path normal = directory.lexically_normal();
                if (std::find(directories.begin(), directories.end(), normal) == directories.end())
                    directories.push_back(normal);
            };
            if (!config_folder.empty()) add(bfs::absolute(bfs::path(config_folder), config_directory));
            add(config_directory);
            if (!executable_directory.empty()){
                add(executable_directory);
                add(executable_directory / ".." / "lib" / "OGRE");
                add(executable_directory / ".." / "lib");
            }
            if (!build_directory.empty()) add(bfs::path(build_directory));
            for (const bfs::path& directory : SYSTEM_DIRECTORIES) add(directory);
            return directories;
        }

        std::vector<std::string> FileNames(const std::string& name, const std::string& ogre_version){
#if defined(_WIN32)
            (void)ogre_version;
            return {name + ".dll"};
#elif defined(__APPLE__)
            (void)ogre_version;
            return {name + ".dylib", name + ".framework"};
#else
            return {name + ".so." + ogre_version};
#endif
        }

        bfs::path Find(const std::string& name, const std::vector<bfs::path>& directories, const std::string& ogre_version){
            boost::system::error_code ec;
            const bfs::path as_path(name);
            if (as_path.is_absolute()) return bfs::exists(as_path, ec) ? as_path : bfs::path();
            const std::vector<std::string> names = FileNames(name, ogre_version);
            for (const bfs::path& directory : directories){
                for (const std::string& file_name : names){
                    const bfs::path file = directory / file_name;
                    if (bfs::exists(file, ec)) return file;
                }
            }
            return bfs::path();
        }
    }
}
