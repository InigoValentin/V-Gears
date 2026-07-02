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
#include <QtCore/qstring.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qglobal.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <sys/stat.h>
#include <vector>

Release::Release(){
    id = "Unknown ISO file";
    platform = PLATFORM_UNKNOWN;
    region = REGION_UNKNOWN;
    language = LANGUAGE_UNKNOWN;
    disk = DISK_UNKNOWN;
    valid = false;
    supported = false;
    error_message = "The selected file is not a valid Final Fantasy VII ISO file.";
    warning_message = "";
};

Release::Release(std::string iso_path) : Release() {
    struct archive* a = archive_read_new();
    struct archive_entry* entry;
    archive_read_support_format_iso9660(a);
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_filename(a, iso_path.c_str(), 2048) != ARCHIVE_OK) {
        const char* archive_error = archive_error_string(a);
        const std::string open_error = archive_error ? archive_error : "Unknown archive error";
        // Error reading iso
        std::cerr << "Failed to read ISO file: " << open_error << std::endl;
        error_message = "Failed to read ISO file: " + open_error;
        archive_read_free(a);
        return;
    }
    std::string game_id = "";

    auto to_upper = [](std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::toupper(c); });
        return value;
    };

    // Normalize a path from the ISO9660 filesystem, removing version suffixes and trailing dots.
    auto normalize_iso_path = [&](std::string value) {
        std::replace(value.begin(), value.end(), '\\', '/');
        while (
          !value.empty()
          && (value.back() == '\n' || value.back() == '\r' || value.back() == ' ' || value.back() == '\t')
        ) {
            value.pop_back();
        }
        while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
            value.erase(value.begin());
        }
        // ISO9660 listings often include version suffixes like ";1".
        std::size_t semi = value.find(';');
        if (semi != std::string::npos) {
            bool version_suffix = true;
            for (std::size_t i = semi + 1; i < value.size(); ++i) {
                if (!std::isdigit(static_cast<unsigned char>(value[i]))) {
                    version_suffix = false;
                    break;
                }
            }
            if (version_suffix) {
                value = value.substr(0, semi);
            }
        }
        // Some tools emit trailing dots for ISO9660 short names.
        while (!value.empty() && value.back() == '.') {
            value.pop_back();
        }
        if (!value.empty() && value.front() == '/') {
            value.erase(value.begin());
        }
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){
            return std::toupper(c);
        });
        return value;
    };

    auto apply_pc_release_from_path = [&](const std::string& normalized) {
        if (
          normalized == "FF7CONFIG.EXE" || normalized == "FF7CONFI.EXE"
          || normalized == "FF7CON~1.EXE" || normalized == "FF7INST.EXE"
        ) {
            this->iso_path = iso_path;
            id = "Final Fantasy VII (PC, 1998, USA) Install Disk";
            platform = PLATFORM_PC;
            region = REGION_NORTH_AMERICA;
            language = LANGUAGE_ENGLISH;
            disk = DISK_INSTALL;
            supported = true;
            valid = true;
            error_message = "";
            warning_message = "";
            return true;
        }
        if (normalized == "FF7/MOVIES/BIKE.AVI") {
            this->iso_path = iso_path;
            id = "Final Fantasy VII (PC, 1998, USA) Disk 1";
            platform = PLATFORM_PC;
            region = REGION_NORTH_AMERICA;
            language = LANGUAGE_ENGLISH;
            disk = DISK_1;
            supported = false;
            valid = false;
            error_message = "To install from a PC release disk, use the install disk instead.";
            warning_message = "";
            return true;
        }
        if (normalized == "FF7/MOVIES/BIGLIGHT.AVI") {
            this->iso_path = iso_path;
            id = "Final Fantasy VII (PC, 1998, USA) Disk 2";
            platform = PLATFORM_PC;
            region = REGION_NORTH_AMERICA;
            language = LANGUAGE_ENGLISH;
            disk = DISK_2;
            supported = false;
            valid = false;
            error_message = "To install from a PC release disk, use the install disk instead.";
            warning_message = "";
            return true;
        }
        if (normalized == "FF7/MOVIES/ENDING1.AVI") {
            this->iso_path = iso_path;
            id = "Final Fantasy VII (PC, 1998, USA) Disk 3";
            platform = PLATFORM_PC;
            region = REGION_NORTH_AMERICA;
            language = LANGUAGE_ENGLISH;
            disk = DISK_3;
            supported = false;
            valid = false;
            error_message = "To install from a PC release disk, use the install disk instead.";
            warning_message = "";
            return true;
        }
        return false;
    };

    bool pc_marker_found = false;
    int r = ARCHIVE_OK;
    bool saw_header = false;
    while (true) {
        std::cout << "Reading next header from ISO..." << std::endl;
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) {
            std::cout << "Reached end of ISO archive." << std::endl;
            break;
        }
        if (r != ARCHIVE_OK && r != ARCHIVE_WARN) {
            std::cerr << "archive_read_next_header error: "
              << archive_error_string(a) << " (code=" << r << ")" << std::endl;
            break;
        }
        if (r == ARCHIVE_WARN) {
            std::cerr << "archive_read_next_header warning: " << archive_error_string(a) << std::endl;
        }
        saw_header = true;

        std::string path = archive_entry_pathname(entry);
        std::cout << "Found file in ISO: " << path << std::endl;

        // Look for know files in the PC release disks.
        const std::string normalized = normalize_iso_path(path);
        if (apply_pc_release_from_path(normalized)) {
            pc_marker_found = true;
            break;
        }
        std::cout << "Finished checking PC releases." << std::endl;

        // If we haven't found the PC install marker, look for SYSTEM.CNF
        // to determine if it's a PSX release and extract the game ID.
        std::string upper_path = to_upper(path);
        if (
          upper_path == "SYSTEM.CNF" || upper_path == "/SYSTEM.CNF"
          || upper_path.find("SYSTEM.CNF;") != std::string::npos
        ) {
            std::cout << "SYSTEM.CNF FOUND." << std::endl;
            size_t size = archive_entry_size(entry);
            if (size > 0) {
                std::vector<char> buffer(size);
                la_ssize_t bytes_read = archive_read_data(a, buffer.data(), size);
                if (bytes_read > 0) {
                    std::string cnf_content(buffer.begin(), buffer.begin() + bytes_read);
                    std::cout << "SYSTEM.CNF content:\n" << cnf_content << std::endl;
                    std::regex code_regex(R"(([A-Z]{4})_(\d{3})\.(\d{2}))");
                    std::smatch match;
                    if (std::regex_search(cnf_content, match, code_regex)) {
                        game_id = match[1].str() + "-" + match[2].str() + match[3].str();
                        std::cout << "Detected game ID from SYSTEM.CNF: " << game_id << std::endl;
                        if (game_id == "SCUS-94163") {
                            id = "Final Fantasy VII (USA) Disc 1";
                            platform = PLATFORM_PS1;
                            region = REGION_NORTH_AMERICA;
                            language = LANGUAGE_ENGLISH;
                            disk = DISK_1;
                            supported = true;
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCUS-94164") {
                            id = "Final Fantasy VII (USA) Disc 2";
                            platform = PLATFORM_PS1;
                            region = REGION_NORTH_AMERICA;
                            language = LANGUAGE_ENGLISH;
                            disk = DISK_2;
                            supported = true;
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCUS-94165") {
                            id = "Final Fantasy VII (USA) Disc 3";
                            platform = PLATFORM_PS1;
                            region = REGION_NORTH_AMERICA;
                            language = LANGUAGE_ENGLISH;
                            disk = DISK_3;
                            supported = true;
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-00867") {
                            id = "Final Fantasy VII (Europe, English) Disc 1";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_ENGLISH;
                            disk = DISK_1;
                            supported = true;
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-10867") {
                            id = "Final Fantasy VII (Europe, English) Disc 2";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_ENGLISH;
                            disk = DISK_2;
                            supported = true;
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-20867") {
                            id = "Final Fantasy VII (Europe, English) Disc 3";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_ENGLISH;
                            disk = DISK_3;
                            supported = true;
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-00868") {
                            id = "Final Fantasy VII (Europe, French) Disc 1";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_FRENCH;
                            disk = DISK_1;
                            supported = false;
                            warning_message = "Spanish language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-10868") {
                            id = "Final Fantasy VII (Europe, French) Disc 2";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_FRENCH;
                            disk = DISK_2;
                            supported = false;
                            warning_message = "Spanish language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-20868") {
                            id = "Final Fantasy VII (Europe, French) Disc 3";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_FRENCH;
                            disk = DISK_3;
                            supported = false;
                            warning_message = "Spanish language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-00900") {
                            id = "Final Fantasy VII (Europe, Spanish) Disc 1";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_SPANISH;
                            disk = DISK_1;
                            supported = true;supported = false;
                            warning_message = "Spanish language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-10900") {
                            id = "Final Fantasy VII (Europe, Spanish) Disc 2";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_SPANISH;
                            disk = DISK_2;
                            supported = false;
                            warning_message = "Spanish language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-20900") {
                            id = "Final Fantasy VII (Europe, Spanish) Disc 3";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_SPANISH;
                            disk = DISK_3;
                            supported = false;
                            warning_message = "Spanish language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-00869") {
                            id = "Final Fantasy VII (Europe, German) Disc 1";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_GERMAN;
                            disk = DISK_1;
                            supported = true;supported = false;
                            warning_message = "German language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-10869") {
                            id = "Final Fantasy VII (Europe, German) Disc 2";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_GERMAN;
                            disk = DISK_2;
                            supported = false;
                            warning_message = "German language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-20869") {
                            id = "Final Fantasy VII (Europe, German) Disc 3";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_GERMAN;
                            disk = DISK_3;
                            supported = false;
                            warning_message = "German language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-00901") {
                            id = "Final Fantasy VII (Europe, Italian) Disc 1";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_ITALIAN;
                            disk = DISK_1;
                            supported = true;supported = false;
                            warning_message = "Italian language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-10901") {
                            id = "Final Fantasy VII (Europe, Italian) Disc 2";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_ITALIAN;
                            disk = DISK_2;
                            supported = false;
                            warning_message = "Italian language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else if (game_id == "SCES-20901") {
                            id = "Final Fantasy VII (Europe, Italian) Disc 3";
                            platform = PLATFORM_PS1;
                            region = REGION_EUROPE;
                            language = LANGUAGE_ITALIAN;
                            disk = DISK_3;
                            supported = false;
                            warning_message = "Italian language is not fully supported in V-Gears.";
                            valid = false;
                            error_message
                            = "Play Station releases are not yet supported in V-Gears."
                            "Use the PC release instead.";
                        }
                        else {
                            id = "Unknown Release";
                            platform = PLATFORM_UNKNOWN;
                            region = REGION_UNKNOWN;
                            language = LANGUAGE_UNKNOWN;
                            disk = DISK_UNKNOWN;
                            supported = false;
                            valid = false;
                            error_message = "Unrecognized game ID: " + game_id + ".";
                        }
                    }
                }
            }
            break;
        }
    }

    if (!saw_header && r == ARCHIVE_EOF) {
        std::cout << "No entries found by libarchive, trying isoinfo listing..." << std::endl;

        auto escape_shell = [](const std::string& value) {
            std::string out;
            for (char c : value) {
                if (c == '"' || c == '\\') {
                    out.push_back('\\');
                }
                out.push_back(c);
            }
            return out;
        };

        const std::string escaped_iso_path = escape_shell(iso_path);
        const std::vector<std::string> list_cmds = {
            "isoinfo -J -f -i \"" + escaped_iso_path + "\" 2>/dev/null",
            "isoinfo -R -J -f -i \"" + escaped_iso_path + "\" 2>/dev/null",
            "isoinfo -R -f -i \"" + escaped_iso_path + "\" 2>/dev/null",
            "isoinfo -f -i \"" + escaped_iso_path + "\" 2>/dev/null"
        };

        char buffer[4096];
        for (const auto& list_cmd : list_cmds) {
            FILE* list_pipe = popen(list_cmd.c_str(), "r");
            if (!list_pipe) {
                continue;
            }

            while (fgets(buffer, sizeof(buffer), list_pipe)) {
                std::string line(buffer);
                while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
                    line.pop_back();
                }
                if (line.empty()) {
                    continue;
                }
                if (apply_pc_release_from_path(normalize_iso_path(line))) {
                    pc_marker_found = true;
                    break;
                }
            }

            pclose(list_pipe);
            if (pc_marker_found) {
                break;
            }
        }
    }

    std::cout << "Finished checking releases." << std::endl;

    archive_read_close(a);
    archive_read_free(a);
    return;
};

Release::~Release(){};

bool Release::extractIso(std::string installation_path){
    struct archive* a;
    struct archive* ext;
    struct archive_entry* entry;
    int flags;
    int r;

    // Attributes to restore
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    a = archive_read_new();
    archive_read_support_format_iso9660(a);
    archive_read_support_format_all(a); // Backup to support UDF ISOs
    archive_read_support_filter_all(a); // Support compressed/filtered streams
    
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    // Create the required output directory if it doesn't exist
    std::string data_dir = installation_path + "/original_data/" + (id.empty() ? "ff7" : id);
    QString target = QString::fromStdString(data_dir);
    QDir dir(target);
    QString parent = QFileInfo(target).absolutePath();
    QDir parent_dir(parent);
    if (!parent_dir.exists()) {
        if (!parent_dir.mkpath(".")) {
            std::cerr << "Failed to create extraction parent directory: " << parent.toStdString() << std::endl;
            return false;
        }
    }
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            std::cerr << "Failed to create extraction directory: " << data_dir << std::endl;
            return false;
        }
    }

    // Open the ISO file
    if ((r = archive_read_open_filename(a, iso_path.c_str(), 10240))) {
        std::cerr << "Error: Could not open ISO file: " << archive_error_string(a) << std::endl;
        return false;
    }

    // Read through the archive entries
    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK || r == ARCHIVE_WARN) {
        if (r == ARCHIVE_WARN) {
            std::cerr << "archive_read_next_header warning during extraction: " << archive_error_string(a) << std::endl;
        }
        if (!entry) {
            std::cerr << "Skipping null archive entry" << std::endl;
            continue;
        }
        const char* entry_path = archive_entry_pathname(entry);
        if (!entry_path) {
            std::cerr << "Skipping entry without a pathname" << std::endl;
            archive_write_finish_entry(ext);
            continue;
        }
        // Prepend destination directory to the current entry path
        std::string current_file_path = entry_path;
        std::string full_output_path = data_dir + "/" + current_file_path;
        archive_entry_set_pathname(entry, full_output_path.c_str());

        // Write the header
        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK) {
            std::cerr << "Warning (Header): " << archive_error_string(ext) << std::endl;
        }
        else if (archive_entry_size(entry) > 0) {
            // Copy data from the ISO to the disk
            const void* buff;
            size_t size;
            la_int64_t offset;

            while (true) {
                r = archive_read_data_block(a, &buff, &size, &offset);
                if (r == ARCHIVE_EOF) break;
                if (r < ARCHIVE_OK) {
                    std::cerr << "Error reading data block: " << archive_error_string(a) << std::endl;
                    break;
                }
                r = archive_write_data_block(ext, buff, size, offset);
                if (r < ARCHIVE_OK) {
                    std::cerr << "Error writing data block: " << archive_error_string(ext) << std::endl;
                    break;
                }
            }
        }
        archive_write_finish_entry(ext);
    }
    if (r != ARCHIVE_EOF && r != ARCHIVE_OK && r != ARCHIVE_WARN) {
        std::cerr << "archive_read_next_header error during extraction: " << archive_error_string(a) << " (code=" << r << ")" << std::endl;
        // continue cleanup and return failure
        archive_read_close(a);
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);
        return false;
    }

    // Clean up
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);

    content_path = data_dir;
    return true;
}

const std::string Release::getId(){
    return id;
};

const Release::Platform Release::getPlatform(){
    return platform;
};

const Release::Region Release::getRegion(){
    return region;
};

const Release::Language Release::getLanguage(){
    return language;
};

const Release::Disk Release::getDisk(){
    return disk;
};

const bool Release::isValid(){
    return valid;
};

const bool Release::isSupported(){
    return supported;
};

const std::string Release::getErrorMessage(){
    return error_message;
};

const std::string Release::getWarningMessage(){
    return warning_message;
};

std::string const Release::getContentPath() {
    return content_path;
}
