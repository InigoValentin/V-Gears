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

#include "VideoFiles.h"
#include <algorithm>
#include <cctype>

namespace VideoFiles{

    std::string NormalizePath(std::string path){
        std::replace(path.begin(), path.end(), '\\', '/');
        const std::size_t semicolon = path.find(';');
        if (semicolon != std::string::npos){
            bool version = semicolon + 1 < path.size();
            for (std::size_t i = semicolon + 1; i < path.size(); ++ i)
                version = version && std::isdigit(static_cast<unsigned char>(path[i]));
            if (version) path.erase(semicolon);
        }
        while (!path.empty() && path.back() == '.') path.pop_back();
        while (!path.empty() && path.front() == '/') path.erase(path.begin());
        std::transform(path.begin(), path.end(), path.begin(), [](unsigned char c){return std::tolower(c);});
        return path;
    }

    bool IsMovie(const std::string& path, std::string* name){
        static const std::string folder = "ff7/movies/";
        const std::string normalized = NormalizePath(path);
        if (normalized.size() <= folder.size() || normalized.compare(0, folder.size(), folder) != 0) return false;
        const std::string file = normalized.substr(folder.size());
        // Files in subdirectories, and directories (which end with a slash), are not movies.
        if (file.find('/') != std::string::npos) return false;
        if (name) *name = file;
        return true;
    }

}
