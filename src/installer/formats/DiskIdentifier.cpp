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
#include <cctype>
#include <iostream>
#include <memory>
#include <regex>
#include <vector>
#include <archive.h>
#include <archive_entry.h>
#include "DiskIdentifier.h"
#include "Iso9660Reader.h"

namespace{

    /**
     * Converts a string to upper case.
     */
    std::string ToUpper(std::string value){
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){
            return std::toupper(c);
        });
        return value;
    }

    /**
     * Reads the game ID from the entry of the SYSTEM.CNF file.
     */
    std::string ReadGameId(struct archive* image, struct archive_entry* entry){
        const size_t size = archive_entry_size(entry);
        if (size == 0) return "";
        std::vector<char> contents(size);
        const la_ssize_t read = archive_read_data(image, contents.data(), size);
        if (read <= 0) return "";
        return DiskIdentifier::FindGameId(std::string(contents.begin(), contents.begin() + read));
    }

    /**
     * Looks for the files that identify the PC disks, reading the ISO9660 folders directly.
     */
    const DiskReleases::Release* FindInFolders(const std::string& path){
        const DiskReleases::Release* release = nullptr;
        Iso9660Reader reader(path);
        reader.Walk(DiskIdentifier::NormalizePath, [&release](const Iso9660Reader::Entry& entry){
            release = DiskReleases::FindPcRelease(entry.path);
            return release == nullptr;
        });
        return release;
    }
}

namespace DiskIdentifier{

    Identification Identify(const std::string& path){
        Identification result;
        std::unique_ptr<struct archive, int(*)(struct archive*)> image(
          archive_read_new(), archive_read_free
        );
        archive_read_support_format_all(image.get());
        archive_read_support_filter_all(image.get());
        if (archive_read_open_filename(image.get(), path.c_str(), 2048) != ARCHIVE_OK){
            const char* error = archive_error_string(image.get());
            result.open_error = error ? error : "Unknown archive error";
            return result;
        }
        struct archive_entry* entry;
        bool has_entries = false;
        int status;
        while ((status = archive_read_next_header(image.get(), &entry)) != ARCHIVE_EOF){
            if (status != ARCHIVE_OK && status != ARCHIVE_WARN){
                std::cerr << "archive_read_next_header error: " << archive_error_string(image.get())
                  << " (code=" << status << ")" << std::endl;
                break;
            }
            if (status == ARCHIVE_WARN){
                std::cerr << "archive_read_next_header warning: "
                  << archive_error_string(image.get()) << std::endl;
            }
            has_entries = true;
            const std::string name = archive_entry_pathname(entry);
            // Look for known files of the PC disks.
            result.release = DiskReleases::FindPcRelease(NormalizePath(name));
            if (result.release != nullptr) return result;
            // PlayStation disks have their ID in SYSTEM.CNF.
            const std::string upper = ToUpper(name);
            if (
              upper == "SYSTEM.CNF" || upper == "/SYSTEM.CNF"
              || upper.find("SYSTEM.CNF;") != std::string::npos
            ){
                result.game_id = ReadGameId(image.get(), entry);
                result.release = DiskReleases::FindPlayStationRelease(result.game_id);
                return result;
            }
        }
        if (!has_entries && status == ARCHIVE_EOF) result.release = FindInFolders(path);
        return result;
    }

    std::string NormalizePath(std::string path){
        std::replace(path.begin(), path.end(), '\\', '/');
        while (!path.empty() && std::string("\n\r \t").find(path.back()) != std::string::npos)
            path.pop_back();
        while (!path.empty() && (path.front() == ' ' || path.front() == '\t')) path.erase(path.begin());
        path = RemoveVersion(path);
        if (!path.empty() && path.front() == '/') path.erase(path.begin());
        return ToUpper(path);
    }

    std::string RemoveVersion(std::string name){
        const size_t separator = name.find(';');
        if (
          separator != std::string::npos
          && std::all_of(name.begin() + separator + 1, name.end(), [](unsigned char c){
              return std::isdigit(c) != 0;
          })
        ) name.erase(separator);
        while (!name.empty() && name.back() == '.') name.pop_back();
        return name;
    }

    std::string FindGameId(const std::string& system_cnf){
        static const std::regex code(R"(([A-Z]{4})_(\d{3})\.(\d{2}))");
        std::smatch match;
        if (!std::regex_search(system_cnf, match, code)) return "";
        return match[1].str() + "-" + match[2].str() + match[3].str();
    }
}
