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

#include <boost/filesystem.hpp>
#include "WorldInstaller.h"

WorldInstaller::WorldInstaller(
  const DiskImage& disk_image, const std::string output_dir, Ogre::ResourceGroupManager* res_mgr
):
  disk_image_(disk_image), data_dir_(output_dir + "game/"), script_installer_(data_dir_),
  encounter_installer_(data_dir_), model_installer_(output_dir, data_dir_, res_mgr)
{}

WorldInstaller::~WorldInstaller(){}

void WorldInstaller::InstallMaps(){
    CopyMaps();
    ExtractTextures();
}

bool WorldInstaller::GenerateScripts(
  const std::vector<std::string>& field_names, const std::map<u16, float>& field_scale_factors,
  const std::vector<std::string>& converted_fields, std::string& error_message
){
    error_message.clear();
    if (field_names.empty() || converted_fields.empty()){
        error_message = "Field maps were not installed; enable field map extraction.";
        return false;
    }
    const std::string path = ArchivePath();
    if (path.empty()){
        error_message = "WORLD_US.LGP was not found on the game disc.";
        return false;
    }
    WorldArchive archive(path);
    for (const VGears::LGPArchive::FileEntry& entry : archive.Entries()){
        if (WorldArchive::LowerName(entry) == "enc_w.bin")
            encounter_installer_.Install(archive.Read(entry));
    }
    return script_installer_.Install(
      archive, field_names, field_scale_factors, converted_fields, error_message
    );
}

void WorldInstaller::ProcessModels(){
    WorldArchive archive(ArchivePath());
    model_installer_.Install(archive);
}

void WorldInstaller::CopyMaps(){
    boost::filesystem::create_directories(data_dir_ + "world/maps/");
    for (const char* map : {"WM0.MAP", "WM2.MAP", "WM3.MAP"}){
        boost::filesystem::copy_file(
          *disk_image_.filePath(std::string("data/wm/") + map), data_dir_ + "world/maps/" + map,
          boost::filesystem::copy_options::overwrite_existing
        );
    }
}

void WorldInstaller::ExtractTextures(){
    boost::filesystem::create_directories(data_dir_ + "world/textures/");
    WorldArchive archive(ArchivePath());
    for (const VGears::LGPArchive::FileEntry& entry : archive.Entries()){
        if (!archive.Contains(entry)) continue;
        const std::string name = WorldArchive::LowerName(entry);
        if (name.size() < 4 || name.compare(name.size() - 4, 4, ".tex") != 0) continue;
        archive.Open(entry).WriteFile(data_dir_ + "world/textures/" + name);
    }
}

std::string WorldInstaller::ArchivePath(){
    const auto path = disk_image_.filePath("data/wm/world_us.lgp");
    return path ? *path : std::string();
}
