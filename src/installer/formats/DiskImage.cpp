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
#include "DiskExtractor.h"
#include "DiskIdentifier.h"
#include "DiskImage.h"
#include "DiskPaths.h"
#include "DiskReleases.h"
#include "DiskVideoExtractor.h"

DiskImage::DiskImage():
  id("Unknown image file"), platform(PLATFORM_UNKNOWN), region(REGION_UNKNOWN),
  language(LANGUAGE_UNKNOWN), disk(DISK_UNKNOWN), valid(false), supported(false),
  error_message("The selected file is not a valid Final Fantasy VII image file."),
  is_extracted(false)
{}

DiskImage::DiskImage(std::string path): DiskImage(){
    const DiskIdentifier::Identification found = DiskIdentifier::Identify(path);
    if (!found.open_error.empty()) error_message = "Failed to read image file: " + found.open_error;
    else if (found.release != nullptr) SetRelease(*found.release, path);
    else if (!found.game_id.empty()){
        id = "Unknown DiskImage";
        error_message = "Unrecognized game ID: " + found.game_id + ".";
    }
}

DiskImage::~DiskImage(){}

void DiskImage::SetRelease(const DiskReleases::Release& release, const std::string& image_path){
    id = release.name;
    platform = release.platform;
    region = release.region;
    language = release.language;
    disk = release.disk;
    supported = release.supported;
    warning_message = release.warning;
    // Only PC disks can be installed.
    valid = platform == PLATFORM_PC;
    if (valid){
        path = image_path;
        error_message = "";
    }
    else{
        error_message
          = "Play Station releases are not yet supported in V-Gears. Use the PC release instead.";
    }
}

bool DiskImage::extractImage(std::string installation_path, const std::function<void(float)>& progress){
    const std::string dir = installation_path + "/original_data/" + (id.empty() ? "ff7" : id);
    DiskExtractor extractor(path, progress);
    std::string error;
    if (!extractor.Extract(dir, error)){
        std::cerr << "Error: " << error << std::endl;
        error_message = error;
        return false;
    }
    content_path = dir;
    is_extracted = true;
    return true;
}

std::unique_ptr<std::string> DiskImage::filePath(std::string file_path){
    if (!is_extracted){
        std::cerr << "Error: Attempted to check for file existence before extracting the disk image."
          << std::endl;
        return nullptr;
    }
    return DiskPaths::Find(content_path, file_path);
}

const std::string DiskImage::getPath(){
    return path;
}

const std::string DiskImage::getId(){
    return id;
}

const bool DiskImage::isValid(){
    return valid;
}

const bool DiskImage::isSupported(){
    return supported;
}

const std::string DiskImage::getErrorMessage(){
    return error_message;
}

const std::string DiskImage::getWarningMessage(){
    return warning_message;
}

bool const DiskImage::isVideoDisk(){
    return valid && platform == PLATFORM_PC && (disk == DISK_1 || disk == DISK_2 || disk == DISK_3);
}

bool DiskImage::extractVideos(
  const std::string& output_dir, const std::function<void(float)>& progress,
  const std::function<void(const std::string&)>& log, int* extracted, int* skipped
){
    DiskVideoExtractor extractor(path, progress, log);
    const bool ok = extractor.Extract(output_dir + "video/");
    if (extracted) *extracted = extractor.Extracted();
    if (skipped) *skipped = extractor.Skipped();
    return ok;
}

std::string const DiskImage::getContentPath() {
    return content_path;
}
