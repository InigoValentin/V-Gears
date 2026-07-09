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
#include "DiskImage.h"
#include <QtCore/qstring.h>
#include <QtCore/qdir.h>
#include <QtCore/qdiriterator.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qglobal.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <regex>
#include <functional>
#include <stdexcept>
#include <sys/stat.h>
#include <vector>

DiskImage::DiskImage(){
    id = "Unknown image file";
    platform = PLATFORM_UNKNOWN;
    region = REGION_UNKNOWN;
    language = LANGUAGE_UNKNOWN;
    disk = DISK_UNKNOWN;
    valid = false;
    supported = false;
    error_message = "The selected file is not a valid Final Fantasy VII image file.";
    warning_message = "";
    is_extracted = false;
};

DiskImage::DiskImage(std::string path) : DiskImage() {
    struct archive* a = archive_read_new();
    struct archive_entry* entry;
    archive_read_support_format_iso9660(a);
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_filename(a, path.c_str(), 2048) != ARCHIVE_OK) {
        const char* archive_error = archive_error_string(a);
        const std::string open_error = archive_error ? archive_error : "Unknown archive error";
        // Error reading image
        error_message = "Failed to read image file: " + open_error;
        archive_read_free(a);
        return;
    }
    std::string game_id = "";

    auto to_upper = [](std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return std::toupper(c); });
        return value;
    };

    // Normalize a path from the ISO9660 filesystem, removing version suffixes and trailing dots.
    auto normalize_path = [&](std::string value) {
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
            this->path = path;
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
            this->path = path;
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
            this->path = path;
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
            this->path = path;
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
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) {
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

        // Look for know files in the PC release disks.
        const std::string normalized = normalize_path(path);
        if (apply_pc_release_from_path(normalized)) {
            pc_marker_found = true;
            break;
        }

        // If the PC install marker hasn't been found, look for SYSTEM.CNF
        // to determine if it's a PSX release and extract the game ID.
        std::string upper_path = to_upper(path);
        if (
          upper_path == "SYSTEM.CNF" || upper_path == "/SYSTEM.CNF"
          || upper_path.find("SYSTEM.CNF;") != std::string::npos
        ) {
            size_t size = archive_entry_size(entry);
            if (size > 0) {
                std::vector<char> buffer(size);
                la_ssize_t bytes_read = archive_read_data(a, buffer.data(), size);
                if (bytes_read > 0) {
                    std::string cnf_content(buffer.begin(), buffer.begin() + bytes_read);
                    std::regex code_regex(R"(([A-Z]{4})_(\d{3})\.(\d{2}))");
                    std::smatch match;
                    if (std::regex_search(cnf_content, match, code_regex)) {
                        game_id = match[1].str() + "-" + match[2].str() + match[3].str();
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
                            id = "Unknown DiskImage";
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
        //std::cout << "libarchive returned no entries, falling back to direct ISO9660 parsing..." << std::endl;
        auto read_le32 = [](const uint8_t* bytes) -> uint32_t {
            return static_cast<uint32_t>(bytes[0])
                | (static_cast<uint32_t>(bytes[1]) << 8)
                | (static_cast<uint32_t>(bytes[2]) << 16)
                | (static_cast<uint32_t>(bytes[3]) << 24);
        };

        std::ifstream file(path, std::ios::binary);
        if (file) {
            const std::size_t sector_size = 2048;

            auto read_bytes = [&](std::streamoff offset, std::size_t size) {
                std::vector<uint8_t> bytes(size);
                file.seekg(offset, std::ios::beg);
                file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
                const std::size_t bytes_read = static_cast<std::size_t>(file.gcount());
                bytes.resize(bytes_read);
                return bytes;
            };

            const std::vector<uint8_t> pvd = read_bytes(static_cast<std::streamoff>(16) * sector_size, sector_size);
            const bool pvd_valid =
                pvd.size() == sector_size
                && pvd[0] == 1
                && pvd[1] == 'C' && pvd[2] == 'D'
                && pvd[3] == '0' && pvd[4] == '0' && pvd[5] == '1';

            if (pvd_valid) {
                const uint32_t root_lba = read_le32(&pvd[156 + 2]);
                const uint32_t root_size = read_le32(&pvd[156 + 10]);

                std::function<void(uint32_t, uint32_t, const std::string&)> walk_directory;
                walk_directory = [&](uint32_t extent_lba, uint32_t extent_size, const std::string& prefix) {
                    if (pc_marker_found || extent_size == 0) {
                        return;
                    }

                    const std::vector<uint8_t> dir_data = read_bytes(
                        static_cast<std::streamoff>(extent_lba) * sector_size,
                        extent_size
                    );

                    std::size_t offset = 0;
                    while (offset < dir_data.size()) {
                        const uint8_t record_size = dir_data[offset];
                        if (record_size == 0) {
                            offset = ((offset / sector_size) + 1) * sector_size;
                            continue;
                        }
                        if (offset + record_size > dir_data.size() || offset + 33 > dir_data.size()) {
                            break;
                        }

                        const uint32_t entry_lba = read_le32(&dir_data[offset + 2]);
                        const uint32_t entry_size = read_le32(&dir_data[offset + 10]);
                        const uint8_t entry_flags = dir_data[offset + 25];
                        const uint8_t entry_name_len = dir_data[offset + 32];

                        if (entry_name_len > 0 && offset + 33 + entry_name_len <= dir_data.size()) {
                            const bool is_dot_entry =
                                entry_name_len == 1
                                && (dir_data[offset + 33] == 0x00 || dir_data[offset + 33] == 0x01);

                            if (!is_dot_entry) {
                                const std::string raw_name(
                                    reinterpret_cast<const char*>(&dir_data[offset + 33]),
                                    reinterpret_cast<const char*>(&dir_data[offset + 33 + entry_name_len])
                                );
                                const std::string normalized_name = normalize_path(raw_name);
                                const std::string full_path
                                  = prefix.empty() ? normalized_name : prefix + "/" + normalized_name;
                                if (apply_pc_release_from_path(full_path)) {
                                    pc_marker_found = true;
                                    return;
                                }
                                if ((entry_flags & 0x02) != 0) {
                                    walk_directory(entry_lba, entry_size, full_path);
                                    if (pc_marker_found) {
                                        return;
                                    }
                                }
                            }
                        }
                        offset += record_size;
                    }
                };
                walk_directory(root_lba, root_size, "");
            }
        }
    }
    archive_read_close(a);
    archive_read_free(a);
    return;
};

DiskImage::~DiskImage(){};

bool DiskImage::extractImage(std::string installation_path){
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

    // Open the image file
    if ((r = archive_read_open_filename(a, path.c_str(), 10240))) {
        std::cerr << "Error: Could not open image file: " << archive_error_string(a) << std::endl;
        return false;
    }

    // Read through the archive entries
    bool saw_header = false;
    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK || r == ARCHIVE_WARN) {
        saw_header = true;
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
            // Copy data from the image to the disk
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

    if (!saw_header && r == ARCHIVE_EOF) {
        archive_read_close(a);
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);

        auto read_le32 = [](const uint8_t* bytes) -> uint32_t {
            return static_cast<uint32_t>(bytes[0])
                | (static_cast<uint32_t>(bytes[1]) << 8)
                | (static_cast<uint32_t>(bytes[2]) << 16)
                | (static_cast<uint32_t>(bytes[3]) << 24);
        };

        auto normalize_image_name = [](std::string value) {
            std::replace(value.begin(), value.end(), '\\', '/');
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
            while (!value.empty() && value.back() == '.') {
                value.pop_back();
            }
            return value;
        };

        std::ifstream image_file(path, std::ios::binary);
        if (!image_file) {
            error_message = "Failed to open image file for direct ISO9660 extraction.";
            return false;
        }

        const std::size_t sector_size = 2048;
        bool extraction_ok = true;

        auto read_bytes = [&](std::streamoff offset, std::size_t size) {
            std::vector<uint8_t> bytes(size);
            image_file.seekg(offset, std::ios::beg);
            image_file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
            const std::size_t bytes_read = static_cast<std::size_t>(image_file.gcount());
            bytes.resize(bytes_read);
            return bytes;
        };

        const std::vector<uint8_t> pvd = read_bytes(static_cast<std::streamoff>(16) * sector_size, sector_size);
        const bool pvd_valid =
            pvd.size() == sector_size
            && pvd[0] == 1
            && pvd[1] == 'C' && pvd[2] == 'D'
            && pvd[3] == '0' && pvd[4] == '0' && pvd[5] == '1';

        if (!pvd_valid) {
            error_message = "Direct ISO9660 fallback failed: invalid primary volume descriptor.";
            return false;
        }

        const uint32_t root_lba = read_le32(&pvd[156 + 2]);
        const uint32_t root_size = read_le32(&pvd[156 + 10]);

        std::function<void(uint32_t, uint32_t, const std::string&)> extract_directory;
        extract_directory = [&](uint32_t extent_lba, uint32_t extent_size, const std::string& prefix) {
            if (!extraction_ok || extent_size == 0) {
                return;
            }

            const std::vector<uint8_t> dir_data = read_bytes(
                static_cast<std::streamoff>(extent_lba) * sector_size,
                extent_size
            );
            if (dir_data.empty() && extent_size > 0) {
                extraction_ok = false;
                return;
            }

            std::size_t offset = 0;
            while (offset < dir_data.size()) {
                const uint8_t record_size = dir_data[offset];
                if (record_size == 0) {
                    offset = ((offset / sector_size) + 1) * sector_size;
                    continue;
                }
                if (offset + record_size > dir_data.size() || offset + 33 > dir_data.size()) {
                    break;
                }

                const uint32_t entry_lba = read_le32(&dir_data[offset + 2]);
                const uint32_t entry_size = read_le32(&dir_data[offset + 10]);
                const uint8_t entry_flags = dir_data[offset + 25];
                const uint8_t entry_name_len = dir_data[offset + 32];

                if (entry_name_len > 0 && offset + 33 + entry_name_len <= dir_data.size()) {
                    const bool is_dot_entry =
                        entry_name_len == 1
                        && (dir_data[offset + 33] == 0x00 || dir_data[offset + 33] == 0x01);
                    if (!is_dot_entry) {
                        std::string raw_name(
                            reinterpret_cast<const char*>(&dir_data[offset + 33]),
                            reinterpret_cast<const char*>(&dir_data[offset + 33 + entry_name_len])
                        );
                        raw_name = normalize_image_name(raw_name);
                        const std::string relative_path = prefix.empty() ? raw_name : prefix + "/" + raw_name;
                        const std::string absolute_path = data_dir + "/" + relative_path;

                        if ((entry_flags & 0x02) != 0) {
                            QDir mkdir_dir;
                            if (!mkdir_dir.mkpath(QString::fromStdString(absolute_path))) {
                                extraction_ok = false;
                                return;
                            }
                            extract_directory(entry_lba, entry_size, relative_path);
                            if (!extraction_ok) {
                                return;
                            }
                        }
                        else {
                            QString abs_qpath = QString::fromStdString(absolute_path);
                            QString abs_parent = QFileInfo(abs_qpath).absolutePath();
                            QDir parent_dir(abs_parent);
                            if (!parent_dir.exists() && !parent_dir.mkpath(".")) {
                                extraction_ok = false;
                                return;
                            }

                            std::ofstream out_file(absolute_path, std::ios::binary);
                            if (!out_file) {
                                extraction_ok = false;
                                return;
                            }

                            const std::streamoff file_offset = static_cast<std::streamoff>(entry_lba) * sector_size;
                            image_file.seekg(file_offset, std::ios::beg);
                            std::vector<char> chunk(64 * 1024);
                            uint32_t remaining = entry_size;
                            while (remaining > 0) {
                                const std::size_t to_read = std::min<std::size_t>(chunk.size(), remaining);
                                image_file.read(chunk.data(), static_cast<std::streamsize>(to_read));
                                const std::streamsize got = image_file.gcount();
                                if (got <= 0) {
                                    extraction_ok = false;
                                    return;
                                }
                                out_file.write(chunk.data(), got);
                                remaining -= static_cast<uint32_t>(got);
                            }
                        }
                    }
                }
                offset += record_size;
            }
        };

        extract_directory(root_lba, root_size, "");
        if (!extraction_ok) {
            error_message = "Direct ISO9660 fallback failed while extracting files.";
            return false;
        }

        content_path = data_dir;
        is_extracted = true;
        return true;
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
    is_extracted = true;
    return true;
}

std::unique_ptr<std::string> DiskImage::fileExists(std::string file_path) {
    if (!is_extracted) {
        std::cerr << "Error: Attempted to check for file existence before extracting the disk image." << std::endl;
        return nullptr;
    }
    auto normalize_segment = [](QString segment) {
        segment = segment.trimmed();
        while (!segment.isEmpty() && (segment.endsWith('.') || segment.endsWith(' '))) {
            segment.chop(1);
        }
        int semi = segment.indexOf(';');
        if (semi >= 0) {
            bool numeric_suffix = true;
            for (int i = semi + 1; i < segment.size(); ++i) {
                if (!segment.at(i).isDigit()) {
                    numeric_suffix = false;
                    break;
                }
            }
            if (numeric_suffix) {
                segment = segment.left(semi);
            }
        }
        return segment.toUpper();
    };
    auto split_segments = [&](const QString& path) {
        QString normalized = path;
        normalized.replace('\\', '/');
        while (normalized.startsWith('/')) {
            normalized.remove(0, 1);
        }
        normalized = QDir::cleanPath(normalized);
        QStringList parts = normalized.split('/', Qt::SkipEmptyParts);
        QStringList out;
        for (const QString& part : parts) {
            if (part == ".") {
                continue;
            }
            out.push_back(normalize_segment(part));
        }
        return out;
    };
    auto split_base_ext = [](const QString& name) {
        const int dot = name.lastIndexOf('.');
        if (dot <= 0 || dot == name.size() - 1) {
            return qMakePair(name, QString());
        }
        return qMakePair(name.left(dot), name.mid(dot + 1));
    };
    auto segment_matches = [&](const QString& requested, const QString& actual) {
        if (requested == actual) {
            return true;
        }

        const auto req = split_base_ext(requested);
        const auto act = split_base_ext(actual);
        if (req.second != act.second) {
            return false;
        }

        QString req_base = req.first;
        QString act_base = act.first;

        const int req_tilde = req_base.indexOf('~');
        const int act_tilde = act_base.indexOf('~');
        if (req_tilde >= 0) {
            req_base = req_base.left(req_tilde);
        }
        if (act_tilde >= 0) {
            act_base = act_base.left(act_tilde);
        }

        if (req_base.isEmpty() || act_base.isEmpty()) {
            return false;
        }

        return req_base.startsWith(act_base) || act_base.startsWith(req_base);
    };
    const QString base_dir = QDir::cleanPath(QString::fromStdString(content_path).replace('\\', '/'));
    const QString requested_path = QString::fromStdString(file_path);
    const QString direct_candidate = QDir(base_dir).filePath(requested_path);
    QFileInfo direct_file(direct_candidate);
    if (direct_file.exists() && direct_file.isFile()) {
        return std::make_unique<std::string>(direct_file.absoluteFilePath().toStdString());
    }
    const QStringList requested_segments = split_segments(requested_path);
    if (requested_segments.isEmpty()) {
        return nullptr;
    }
    QDirIterator it(base_dir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString file_full_path = it.next();
        const QString relative_path = QDir(base_dir).relativeFilePath(file_full_path);
        const QStringList actual_segments = split_segments(relative_path);
        if (actual_segments.size() < requested_segments.size()) {
            continue;
        }

        bool all_segments_match = true;
        const int offset = actual_segments.size() - requested_segments.size();
        for (int i = 0; i < requested_segments.size(); ++i) {
            if (!segment_matches(requested_segments[i], actual_segments[offset + i])) {
                all_segments_match = false;
                break;
            }
        }
        if (all_segments_match) {
            return std::make_unique<std::string>(QFileInfo(file_full_path).absoluteFilePath().toStdString());
        }
    }
    return nullptr;
}

const std::string DiskImage::getPath(){
    return path;
}

const std::string DiskImage::getId(){
    return id;
};

const DiskImage::Platform DiskImage::getPlatform(){
    return platform;
};

const DiskImage::Region DiskImage::getRegion(){
    return region;
};

const DiskImage::Language DiskImage::getLanguage(){
    return language;
};

const DiskImage::Disk DiskImage::getDisk(){
    return disk;
};

const bool DiskImage::isValid(){
    return valid;
};

const bool DiskImage::isSupported(){
    return supported;
};

const std::string DiskImage::getErrorMessage(){
    return error_message;
};

const std::string DiskImage::getWarningMessage(){
    return warning_message;
};

std::string const DiskImage::getContentPath() {
    return content_path;
}
