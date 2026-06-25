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

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <archive.h>
#include <archive_entry.h>
#include "Release.h"

Release::Release(){
    id = "";
    platform = PLATFORM_UNKNOWN;
    region = REGION_UNKNOWN;
    language = LANGUAGE_UNKNOWN;
    disk = DISK_UNKNOWN;
    valid = false;
    supported = false;
    error_message = "";
    warning_message = "";
};



Release::Release(std::string iso_path){
    Release();
    struct archive* a = archive_read_new();
    struct archive_entry* entry;
    archive_read_support_format_iso9660(a);
    archive_read_support_format_all(a); // Fail-safe fallback

    if (archive_read_open_filename(a, iso_path.c_str(), 10240) != ARCHIVE_OK) {
        archive_read_free(a);
        // Error readding iso
        error_message = "Failed to read ISO file: " + std::string(archive_error_string(a));
        return;
    }
    std::string game_id = "";

    // Scan headers without extracting data to disk
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string path = archive_entry_pathname(entry);
        
        // Convert to uppercase to handle varied ISO naming conventions
        for (char &c : path) c = std::toupper(c);

        // We only care about SYSTEM.CNF at the root
        if (path == "SYSTEM.CNF" || path == "/SYSTEM.CNF" || path.find("SYSTEM.CNF;") != std::string::npos) {
            size_t size = archive_entry_size(entry);
            if (size > 0) {
                std::vector<char> buffer(size);
                archive_read_data(a, buffer.data(), size);
                
                std::string cnf_content(buffer.begin(), buffer.end());
                // Get the game ID from the SYSTEM.CNF content using regex
                // Looks for patterns like SLUS_123.45, SCUS_944.44, SLES_001.23, etc.
                std::regex code_regex(R"(([A-Z]{4})_(\d{3})\.(\d{2}))");
                std::smatch match;

                if (std::regex_search(cnf_content, match, code_regex)) {
                    // match[1] = ABCD, match[2] = 123, match[3] = 45
                    // Standardize it to the common "ABCD-12345" format
                    game_id = match[1].str() + "-" + match[2].str() + match[3].str();
                }
                else game_id = "";
            }
            break; // Found a PC release
            std::string path = archive_entry_pathname(entry);
            if (path.find("diski.x") != std::string::npos) {
                id = "Final Fantasy VII (PC) Disc 1";
                platform = PLATFORM_PC;
                region = REGION_UNKNOWN;
                language = LANGUAGE_UNKNOWN;
                disk = DISK_1;
                supported = true;
                valid = true;
            }
            else if (path.find("diskii.x") != std::string::npos) {
                id = "Final Fantasy VII (PC) Disc 3";
                platform = PLATFORM_PC;
                region = REGION_UNKNOWN;
                language = LANGUAGE_UNKNOWN;
                disk = DISK_3;
                supported = true;
                valid = true;
            }
            else if (path.find("diskiii.x") != std::string::npos) {
                id = "Final Fantasy VII (PC) Disc 3";
                platform = PLATFORM_PC;
                region = REGION_UNKNOWN;
                language = LANGUAGE_UNKNOWN;
                disk = DISK_3;
                supported = true;
                valid = true;
            }
            return;
        }
    }
    archive_read_close(a);
    archive_read_free(a);

    // If it's a PC release, no need to check the game ID further
    if (platform == PLATFORM_PC) {
        return;
    }

    if (game_id.empty()) {
        error_message = "Game ID not found in ISO file: " + iso_path;
        return;
    }
    
    if (game_id == "SCUS-94163") {
        id = "Final Fantasy VII (USA) Disc 1";
        platform = PLATFORM_PS1;
        region = REGION_NORTH_AMERICA;
        language = LANGUAGE_ENGLISH;
        disk = DISK_1;
        supported = true;
        valid = true;
    }
    else if (game_id == "SCUS-94164") {
        id = "Final Fantasy VII (USA) Disc 2";
        platform = PLATFORM_PS1;
        region = REGION_NORTH_AMERICA;
        language = LANGUAGE_ENGLISH;
        disk = DISK_2;
        supported = true;
        valid = true;
    }
    else if (game_id == "SCUS-94165") {
        id = "Final Fantasy VII (USA) Disc 3";
        platform = PLATFORM_PS1;
        region = REGION_NORTH_AMERICA;
        language = LANGUAGE_ENGLISH;
        disk = DISK_3;
        supported = true;
        valid = true;
    }
    else if (game_id == "SCES-00867") {
        id = "Final Fantasy VII (Europe, English) Disc 1";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_ENGLISH;
        disk = DISK_1;
        supported = true;
        valid = true;
    }
    else if (game_id == "SCES-10867") {
        id = "Final Fantasy VII (Europe, English) Disc 2";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_ENGLISH;
        disk = DISK_2;
        supported = true;
        valid = true;
    }
    else if (game_id == "SCES-20867") {
        id = "Final Fantasy VII (Europe, English) Disc 3";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_ENGLISH;
        disk = DISK_3;
        supported = true;
        valid = true;
    }
    else if (game_id == "SCES-00868") {
        id = "Final Fantasy VII (Europe, French) Disc 1";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_FRENCH;
        disk = DISK_1;
        supported = false;
        warning_message = "Spanish language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-10868") {
        id = "Final Fantasy VII (Europe, French) Disc 2";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_FRENCH;
        disk = DISK_2;
        supported = false;
        warning_message = "Spanish language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-20868") {
        id = "Final Fantasy VII (Europe, French) Disc 3";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_FRENCH;
        disk = DISK_3;
        supported = false;
        warning_message = "Spanish language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-00900") {
        id = "Final Fantasy VII (Europe, Spanish) Disc 1";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_SPANISH;
        disk = DISK_1;
        supported = true;supported = false;
        warning_message = "Spanish language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-10900") {
        id = "Final Fantasy VII (Europe, Spanish) Disc 2";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_SPANISH;
        disk = DISK_2;
        supported = false;
        warning_message = "Spanish language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-20900") {
        id = "Final Fantasy VII (Europe, Spanish) Disc 3";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_SPANISH;
        disk = DISK_3;
        supported = false;
        warning_message = "Spanish language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-00869") {
        id = "Final Fantasy VII (Europe, German) Disc 1";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_GERMAN;
        disk = DISK_1;
        supported = true;supported = false;
        warning_message = "German language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-10869") {
        id = "Final Fantasy VII (Europe, German) Disc 2";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_GERMAN;
        disk = DISK_2;
        supported = false;
        warning_message = "German language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-20869") {
        id = "Final Fantasy VII (Europe, German) Disc 3";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_GERMAN;
        disk = DISK_3;
        supported = false;
        warning_message = "German language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-00901") {
        id = "Final Fantasy VII (Europe, Italian) Disc 1";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_ITALIAN;
        disk = DISK_1;
        supported = true;supported = false;
        warning_message = "Italian language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-10901") {
        id = "Final Fantasy VII (Europe, Italian) Disc 2";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_ITALIAN;
        disk = DISK_2;
        supported = false;
        warning_message = "Italian language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    else if (game_id == "SCES-20901") {
        id = "Final Fantasy VII (Europe, Italian) Disc 3";
        platform = PLATFORM_PS1;
        region = REGION_EUROPE;
        language = LANGUAGE_ITALIAN;
        disk = DISK_3;
        supported = false;
        warning_message = "Italian language is not fully supported in V-Gears. Some features may not work as expected.";
        valid = true;
    }
    return;
};

Release::~Release(){};

std::string Release::getId(){
    return id;
};

Release::Platform Release::getPlatform(){
    return platform;
};

Release::Region Release::getRegion(){
    return region;
};

Release::Language Release::getLanguage(){
    return language;
};

Release::Disk Release::getDisk(){
    return disk;
};

bool Release::isValid(){
    return valid;
};

bool Release::isSupported(){
    return supported;
};

std::string Release::getErrorMessage(){
    return error_message;
};

std::string Release::getWarningMessage(){
    return warning_message;
};
