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
#include <ctime>
#include <fstream>
#include <set>
#include <sstream>
#include "common/InstallStamp.h"

namespace VGears{

    namespace InstallStamp{

        namespace bfs = boost::filesystem;

        const char* const FILE_NAME = "install.info";

        namespace{

            /**
             * Joins a list with commas.
             */
            std::string Join(const std::vector<std::string>& items){
                std::string joined;
                for (const std::string& item : items) joined += (joined.empty() ? "" : ",") + item;
                return joined;
            }

            /**
             * Splits a list separated by commas, skipping empty items.
             */
            std::vector<std::string> Split(const std::string& list){
                std::vector<std::string> items;
                std::stringstream stream(list);
                std::string item;
                while (std::getline(stream, item, ',')) if (!item.empty()) items.push_back(item);
                return items;
            }

            /**
             * Removes the spaces at both ends.
             */
            std::string Trim(std::string text){
                const auto space = [](unsigned char c){return std::isspace(c) != 0;};
                while (!text.empty() && space(text.back())) text.pop_back();
                while (!text.empty() && space(text.front())) text.erase(0, 1);
                return text;
            }
        }

        bool Read(const bfs::path& directory, Stamp& stamp){
            stamp = Stamp();
            std::ifstream file((directory / FILE_NAME).string());
            if (!file.is_open()) return false;
            std::string line;
            while (std::getline(file, line)){
                line = Trim(line);
                if (line.empty() || line[0] == '#') continue;
                const size_t equal = line.find('=');
                if (equal == std::string::npos) continue;
                const std::string key = Trim(line.substr(0, equal));
                const std::string value = Trim(line.substr(equal + 1));
                if (key == "complete") stamp.complete = value == "1";
                else if (key == "installer") stamp.installer = value;
                else if (key == "date") stamp.date = value;
                else if (key == "disk") stamp.disk = value;
                else if (key == "installing") stamp.installing = Split(value);
                else if (key.compare(0, 5, "part.") == 0 && key.size() > 5){
                    int version = 0;
                    try{version = std::stoi(value);}
                    catch (...){version = 0;}
                    stamp.parts[key.substr(5)] = version;
                }
            }
            return true;
        }

        bool Write(const bfs::path& directory, const Stamp& stamp){
            std::ofstream file((directory / FILE_NAME).string(), std::ios::out | std::ios::trunc);
            if (!file.is_open()) return false;
            file << "# Record of the installations of the game data, written by v-gears-installer.\n"
                 << "# The game and the verifier read it to know if the data must be installed again.\n"
                 << "complete=" << (stamp.complete ? 1 : 0) << "\n"
                 << "installer=" << stamp.installer << "\n"
                 << "date=" << stamp.date << "\n"
                 << "disk=" << stamp.disk << "\n";
            if (!stamp.installing.empty()) file << "installing=" << Join(stamp.installing) << "\n";
            for (const auto& part : stamp.parts) file << "part." << part.first << "=" << part.second << "\n";
            return static_cast<bool>(file);
        }

        void Start(
          Stamp& stamp, const std::vector<std::string>& parts, const std::string& installer,
          const std::string& date, const std::string& disk
        ){
            // The parts of an installation that didn't finish, and are not installed again now, are not usable.
            if (!stamp.complete){
                for (const std::string& part : stamp.installing){
                    if (std::find(parts.begin(), parts.end(), part) == parts.end()) stamp.parts[part] = 0;
                }
            }
            stamp.complete = false;
            stamp.installing = parts;
            stamp.installer = installer;
            stamp.date = date;
            stamp.disk = disk;
        }

        void Finish(Stamp& stamp){
            for (const std::string& part : stamp.installing) stamp.parts[part] = DATA_VERSION;
            stamp.installing.clear();
            stamp.complete = true;
        }

        std::vector<std::string> Outdated(const Stamp& stamp){
            std::set<std::string> outdated;
            for (const auto& part : stamp.parts) if (part.second < DATA_VERSION) outdated.insert(part.first);
            if (!stamp.complete) outdated.insert(stamp.installing.begin(), stamp.installing.end());
            return std::vector<std::string>(outdated.begin(), outdated.end());
        }

        std::vector<std::string> Problems(const bfs::path& directory, bool* found){
            std::vector<std::string> problems;
            Stamp stamp;
            const bool read = Read(directory, stamp);
            if (found != nullptr) *found = read;
            if (!read) return problems;
            if (!stamp.complete){
                problems.push_back(
                  "The last installation (" + stamp.date + ") didn't finish: it was interrupted or it failed. "
                  "Install the data again."
                );
            }
            std::vector<std::string> old;
            for (const auto& part : stamp.parts){
                const bool interrupted = std::find(stamp.installing.begin(), stamp.installing.end(), part.first)
                  != stamp.installing.end();
                if (part.second < DATA_VERSION && !(interrupted && !stamp.complete)) old.push_back(part.first);
            }
            if (!old.empty()){
                problems.push_back(
                  "These parts were installed by an older installer, or their installation didn't finish, and must be "
                  "installed again: " + Join(old) + "."
                );
            }
            return problems;
        }

        std::string Now(){
            const std::time_t now = std::time(nullptr);
            char text[32];
            std::strftime(text, sizeof(text), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
            return text;
        }
    }
}
