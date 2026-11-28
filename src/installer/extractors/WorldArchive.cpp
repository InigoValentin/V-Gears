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

#include "common/StringUtil.h"
#include "WorldArchive.h"

WorldArchive::WorldArchive(const std::string& path): file_(path){
    VGears::LGPArchive archive(path, "LGP");
    archive.open(path, true);
    archive.load();
    entries_ = archive.GetFiles();
}

const VGears::LGPArchive::FileList& WorldArchive::Entries() const{return entries_;}

bool WorldArchive::Contains(const VGears::LGPArchive::FileEntry& entry) const{
    return entry.data_offset + entry.data_size <= file_.GetFileSize();
}

File WorldArchive::Open(const VGears::LGPArchive::FileEntry& entry){
    return File(&file_, entry.data_offset, entry.data_size);
}

std::vector<u8> WorldArchive::Read(const VGears::LGPArchive::FileEntry& entry){
    std::vector<u8> data(entry.data_size);
    if (!data.empty()) Open(entry).GetFileBuffer(data.data(), 0, entry.data_size);
    return data;
}

std::string WorldArchive::LowerName(const VGears::LGPArchive::FileEntry& entry){
    std::string name = entry.file_name;
    VGears::StringUtil::toLowerCase(name);
    return name;
}
