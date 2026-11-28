/*
 * V-Gears
 * Copyright (C) 2022 V-Gears Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Ogre.h>
#include "installer/extractors/BattleModelTools.h"
#include "formats/FF7NameLookup.h"
#include "formats/lgp/LGPArchive.h"
#include "formats/model/AFileManager.h"
#include "common/StringUtil.h"

std::vector<LgpEntry> ReadLgp(const std::string& path){
    File lgp_file(path);
    // Also, open it as a LGP archive.
    VGears::LGPArchive lgp(path, "LGP");
    lgp.open(path, true);
    lgp.load();
    std::vector<LgpEntry> entries;
    for (const VGears::LGPArchive::FileEntry& file : lgp.GetFiles()){
        if (file.data_offset + file.data_size > lgp_file.GetFileSize()) continue;
        entries.push_back(
          {File(&lgp_file, file.data_offset, file.data_size), file.file_name, file.folder}
        );
    }
    return entries;
}

void DecompileHrc(
  File compiled, const std::vector<std::string>& meshes, const std::string& path,
  const std::string& skeleton_name
){
    compiled.SetOffset(0);
    compiled.readU32LE();
    compiled.readU32LE();
    compiled.readU32LE();
    std::string decompiled(":HEADER_BLOCK 2\n:SKELETON " + skeleton_name + "\n");
    int bones = compiled.readU32LE();
    compiled.SetOffset(compiled.GetFileSize() - (bones * 12));
    decompiled += ":BONES " + std::to_string(bones) + "\n\n";
    size_t next_mesh = 0;
    for (int b = 0; b < bones; b ++){
        decompiled += "bone" + std::to_string(b) + "\n";
        int parent = compiled.readU32LE();
        if (parent == 0xFFFFFFFF) decompiled += "root\n";
        else decompiled += "bone" + std::to_string(parent) + "\n";
        union {
            float f;
            unsigned long ul;
            unsigned char b[4];
        } u;
        u.b[0] = compiled.readU8();
        u.b[1] = compiled.readU8();
        u.b[2] = compiled.readU8();
        u.b[3] = compiled.readU8();
        decompiled += std::to_string(u.f * -1) + "\n";
        int offset = compiled.readU32LE();
        if (offset == 0) decompiled += "0\n\n";
        else {
            decompiled += std::to_string(offset) + " ";
            if (next_mesh < meshes.size()) decompiled += meshes[next_mesh];
            next_mesh ++;
            decompiled += "\n\n";
        }
    }
    std::ofstream out(path);
    out << decompiled;
    out.close();
}

void AddAnimations(const Ogre::SkeletonPtr& skeleton, const std::vector<std::string>& animations){
    for (const std::string& animation : animations){
        VGears::AFileManager &afl_mgr(VGears::AFileManager::GetSingleton());
        Ogre::String base_name;
        VGears::StringUtil::splitBase(animation, base_name);
        VGears::AFilePtr a = afl_mgr.load(base_name + ".a", "FFVII").staticCast<VGears::AFile>();
        // Convert the FF7 name to a more readable name set in the meta data.
        a->AddTo(skeleton, VGears::NameLookup::Animation(base_name));
    }
}

void ReloadModelResources(Ogre::ResourceGroupManager* res_mgr, const std::string& directory){
    // TODO: Could this be done with res_mgr->declareResource() ? Probably faster
    res_mgr->removeResourceLocation(directory, "FFVII");
    res_mgr->addResourceLocation(directory, "FileSystem", "FFVII", true, true);
}
