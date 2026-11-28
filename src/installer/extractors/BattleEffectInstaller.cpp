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
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <cctype>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <OgreMesh.h>
#include "installer/tables/BattleCameraScripts.h"
#include "installer/tables/ExeEffectCameras.h"
#include "installer/tables/ExeEffectMeshes.h"
#include "installer/formats/TexFile.h"
#include "formats/model/HRCFileManager.h"
#include "installer/extractors/BattleEffectInstaller.h"

namespace{

/**
 * Makes a name that is safe to use in files and Ogre resources: lower case letters, numbers and
 * underscores.
 */
std::string EffectSafeName(std::string name){
    for (char& c : name){
        c = std::tolower(static_cast<unsigned char>(c));
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') c = '_';
    }
    return name;
}

/**
 * Gets the name without the extension, in lower case.
 */
std::string EffectStem(const std::string& file_name){
    std::string stem = file_name.substr(0, file_name.find_last_of('.'));
    std::transform(stem.begin(), stem.end(), stem.begin(), [](unsigned char c){return std::tolower(c);});
    return stem;
}

/**
 * Gets the name of a texture in a rsd file, without the extension, in lower case.
 */
std::string EffectRsdValue(const std::string& line){
    size_t equal = line.find('=');
    if (equal == std::string::npos) return "";
    std::string value = line.substr(equal + 1);
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) value.pop_back();
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) value.erase(0, 1);
    return EffectStem(value);
}

}

BattleEffectInstaller::BattleEffectInstaller(
  DiskImage& disk_image, const std::string& output_dir, Ogre::ResourceGroupManager* res_mgr,
  BattleMeshExporter& exporter, std::vector<LgpEntry>& files
):
  disk_image_(disk_image), output_dir_(output_dir), data_dir_(output_dir + "game/"),
  res_mgr_(res_mgr), exporter_(exporter), files_(files)
{}

unsigned int BattleEffectInstaller::InitializeEffectModels(){
    effect_models_.clear();
    next_effect_to_convert_ = 0;
    const std::string dir = output_dir_ + "temp/effects/";
    // Index the files by folder and name.
    std::map<std::pair<std::string, std::string>, size_t> index;
    for (size_t f = 0; f < files_.size(); f ++){
        std::string name = files_[f].name;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){return std::tolower(c);});
        index[{files_[f].folder, name}] = f;
    }
    // Folder key: the effect folder, "common" for the root.
    auto key_of = [](const std::string& folder){
        if (folder.empty()) return std::string("common");
        std::string key = folder;
        const std::string prefix = "ff7/data/battle/";
        if (key.compare(0, prefix.size(), prefix) == 0) key = key.substr(prefix.size());
        return EffectSafeName(key);
    };
    // Finds a file in the folder of the model, or in the archive root.
    auto find = [&](const std::string& folder, const std::string& name, std::string& found_folder){
        auto it = index.find({folder, name});
        if (it != index.end()){found_folder = folder; return it->second;}
        it = index.find({"", name});
        if (it != index.end()){found_folder = ""; return it->second;}
        return static_cast<size_t>(-1);
    };
    // The folders that have each file, for the textures of the models of the root.
    std::map<std::string, std::vector<std::string>> texture_folders;
    for (const auto& entry : index) if (!entry.first.first.empty()) texture_folders[entry.first.second].push_back(entry.first.first);
    std::set<std::string> written;
    auto write = [&](size_t f, const std::string& name){
        if (written.insert(name).second) files_[f].file.WriteFile(dir + name);
    };
    for (size_t f = 0; f < files_.size(); f ++){
        std::string rsd_name = files_[f].name;
        if (rsd_name.size() < 4 || EffectSafeName(rsd_name.substr(rsd_name.size() - 4)) != "_rsd") continue;
        const std::string& folder = files_[f].folder;
        // Parse the rsd.
        File& rsd_file = files_[f].file;
        rsd_file.SetOffset(0);
        std::string content;
        for (size_t b = 0; b < rsd_file.GetFileSize(); b ++) content += static_cast<char>(rsd_file.readU8());
        std::string ply;
        std::vector<std::string> tex;
        std::istringstream lines(content);
        std::string line;
        while (std::getline(lines, line)){
            std::string upper = line;
            std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c){return std::toupper(c);});
            if (upper.compare(0, 4, "PLY=") == 0) ply = EffectRsdValue(line);
            else if (upper.compare(0, 4, "TEX[") == 0) tex.push_back(EffectRsdValue(line));
        }
        if (ply.empty()) continue;
        std::string p_folder;
        size_t p_index = find(folder, ply + ".p", p_folder);
        if (p_index == static_cast<size_t>(-1)) continue;
        const std::string p_name = key_of(p_folder) + "_" + EffectSafeName(ply) + ".p";
        write(p_index, p_name);
        const std::string stem = EffectSafeName(EffectStem(rsd_name));
        // Tells if a texture is in a folder, or in the archive root.
        const auto has_texture = [&](const std::string& texture_folder, const std::string& t){
            std::string found;
            return find(texture_folder, t + ".tex", found) != static_cast<size_t>(-1);
        };
        // Writes a model with the textures of a folder. The textures keep their place in the list, so the
        // polygons get the texture they use even if one is missing.
        const auto add_model = [&](const std::string& name, const std::string& texture_folder){
            EffectModel model;
            model.name = name;
            std::string rsd = "@RSD940102\nPLY=" + p_name.substr(0, p_name.size() - 2) + ".PLY\n";
            rsd += "MAT=" + model.name + ".MAT\nGRP=" + model.name + ".GRP\n";
            std::vector<std::string> tex_names;
            for (const std::string& t : tex){
                std::string t_folder;
                const size_t t_index = find(texture_folder, t + ".tex", t_folder);
                if (t_index == static_cast<size_t>(-1)){
                    std::cerr << "[WARNING] The texture " << t << " of the effect model " << model.name
                      << " is not in magic.lgp." << std::endl;
                    tex_names.push_back(key_of(texture_folder) + "_" + EffectSafeName(t) + ".tex");
                    continue;
                }
                const std::string t_name = key_of(t_folder) + "_" + EffectSafeName(t) + ".tex";
                write(t_index, t_name);
                tex_names.push_back(t_name);
            }
            rsd += "NTEX=" + std::to_string(tex_names.size()) + "\n";
            for (size_t t = 0; t < tex_names.size(); t ++){
                rsd += "TEX[" + std::to_string(t) + "]=" + tex_names[t].substr(0, tex_names[t].size() - 4) + ".TIM\n";
            }
            model.textures = tex_names;
            std::ofstream(dir + model.name + ".rsd") << rsd;
            // A model with a single bone, with the mesh.
            std::ofstream(dir + model.name + ".hrc")
              << ":HEADER_BLOCK 2\n:SKELETON " << model.name << "\n:BONES 1\n\nbone0\nroot\n0.000000\n1 "
              << model.name << "\n\n";
            effect_models_.push_back(model);
        };
        bool complete = true;
        for (const std::string& t : tex) complete = complete && has_texture(folder, t);
        // The models of the archive root are shared by several effects, and each effect has its own version of
        // their textures in its folder (exp_1.tex is in five, all different). There is a model for each folder
        // that has all of them, named after the folder like the models of the folders.
        std::vector<std::string> variants;
        if (!complete && folder.empty()){
            std::set<std::string> candidates;
            for (const std::string& t : tex){
                const auto found = texture_folders.find(t + ".tex");
                if (found != texture_folders.end()) candidates.insert(found->second.begin(), found->second.end());
            }
            for (const std::string& candidate : candidates){
                bool all = true;
                for (const std::string& t : tex) all = all && has_texture(candidate, t);
                if (all) variants.push_back(candidate);
            }
        }
        if (variants.empty()) add_model(key_of(folder) + "_" + stem, folder);
        else for (const std::string& variant : variants) add_model(key_of(variant) + "_" + stem, variant);
    }
    ReloadModelResources(res_mgr_, dir);
    return effect_models_.size();
}

unsigned int BattleEffectInstaller::ConvertEffectModel(){
    if (next_effect_to_convert_ >= effect_models_.size()) return effect_models_.size();
    EffectModel& model = effect_models_[next_effect_to_convert_];
    try{
        VGears::HRCFileManager::GetSingleton().load(model.name + ".hrc", "FFVII");
        Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(model.name + ".mesh", "FFVII"));
        exporter_.Export("effects/", mesh);
        for (const std::string& texture : model.textures){
            const std::string path = data_dir_ + "battle/models/effects/"
              + texture.substr(0, texture.size() - 4) + ".png";
            if (boost::filesystem::exists(path)) continue;
            // A texture that is not in magic.lgp is already reported.
            if (!boost::filesystem::exists(output_dir_ + "temp/effects/" + texture)) continue;
            try{
                TexFile tex(output_dir_ + "temp/effects/" + texture);
                tex.SavePng(path);
            }
            catch (const std::exception& ex){
                // Many of the original textures are dummies.
                std::cerr << "[WARNING] Error converting effect texture " << texture << ": " << ex.what() << std::endl;
            }
        }
        model.converted = true;
    }
    catch (const Ogre::Exception& ex){
        std::cerr << "[WARNING] Could not convert effect model " << model.name << ": " << ex.what() << std::endl;
    }
    catch (const std::exception& ex){
        std::cerr << "[WARNING] Could not convert effect model " << model.name << ": " << ex.what() << std::endl;
    }
    next_effect_to_convert_ ++;
    return next_effect_to_convert_;
}

void BattleEffectInstaller::WriteEffectModels(){
    std::ofstream file(data_dir_ + "battle/effects/effects.lua");
    file << "-- Meshes of the battle effects, extracted from magic.lgp. Names are the folder of the effect\n"
         << "-- and the original file, for example magic_faira_1. Meshes are in game/battle/models/effects.\n"
         << "Game.EffectModels = {\n";
    for (const EffectModel& model : effect_models_){
        if (model.converted) file << "    \"" << model.name << "\",\n";
    }
    file << "}\n";
    WriteEffectSprites();
    WriteEffectCameras();
    WriteEffectMeshes();
    WriteBattleCameras();
}

void BattleEffectInstaller::WriteBattleCameras(){
    // The scripts of the cameras of the battles (see BattleCameraScripts.h).
    std::map<int, BattleCameraScripts::Set> sets;
    for (int i = 0; i < 3; ++ i){
        const std::string name = "data/battle/camdat" + std::to_string(i) + ".bin";
        auto path = disk_image_.filePath(name);
        BattleCameraScripts::Set set;
        if (path != nullptr && BattleCameraScripts::ReadFile(*path, set)) sets[i] = set;
        else std::cerr << "[WARNING] Can't read " << name << "." << std::endl;
    }
    std::ofstream file(data_dir_ + "battle/battle_cameras.lua");
    BattleCameraScripts::WriteLua(file, sets);
    // The ones of the start of the battles are in the executable.
    BattleCameraScripts::Set intro;
    auto exe_path = disk_image_.filePath("ff7.exe");
    if (exe_path != nullptr && BattleCameraScripts::ReadIntro(*exe_path, intro)){
        BattleCameraScripts::WriteIntroLua(file, intro);
    }
    else std::cerr << "[WARNING] Can't read the cameras of the start of the battles from ff7.exe." << std::endl;
}

void BattleEffectInstaller::WriteEffectMeshes(){
    // The meshes of the effects that are in the data of ff7.exe, like the ground of Ifrit and the
    // mountain of Ramuh (see ExeEffectMeshes.h). Names are the address of the block in the executable.
    std::map<uint32_t, ExeEffectMeshes::Mesh> meshes;
    auto exe_path = disk_image_.filePath("ff7.exe");
    if (exe_path == nullptr || !ExeEffectMeshes::Extract(*exe_path, meshes)){
        std::cerr << "[WARNING] Can't read the meshes of the effects from ff7.exe." << std::endl;
    }
    std::ofstream file(data_dir_ + "battle/effects/effect_meshes.lua");
    ExeEffectMeshes::WriteLua(file, meshes);
}

void BattleEffectInstaller::WriteEffectCameras(){
    // The scripts of the cameras are in the data of ff7.exe, and are found from the code that
    // uses them. Names are the address of the script in memory.
    std::map<uint32_t, std::vector<int16_t>> scripts;
    auto exe_path = disk_image_.filePath("ff7.exe");
    if (exe_path == nullptr || !ExeEffectCameras::Extract(*exe_path, scripts)){
        std::cerr << "[WARNING] Can't read the camera scripts of the effects from ff7.exe." << std::endl;
    }
    std::ofstream file(data_dir_ + "battle/effects/effect_cameras.lua");
    file << "-- Camera scripts of the battle effects, read from ff7.exe. Game.EffectCameras[name] is a list\n"
         << "-- of 16 bit words, with the address of the script in the executable as the name (like\n"
         << "-- 0x8ae268). See engine/scripts/battle/effect_camera.lua for the format.\n"
         << "Game.EffectCameras = Game.EffectCameras or {}\n";
    for (const auto& script : scripts){
        file << "Game.EffectCameras[\"0x" << std::hex << script.first << std::dec << "\"] = {";
        for (size_t i = 0; i < script.second.size(); ++ i){
            if (i % 16 == 0) file << "\n    ";
            file << script.second[i] << ", ";
        }
        file << "\n}\n";
    }
}

void BattleEffectInstaller::WriteEffectSprites(){
    // The sprite files (.s) are lists of frames, and every frame a list of quads of a sprite sheet (see
    // the layout below). They are written twice: all the quads one after the other (the first format,
    // used by the Cure effect) and the frames with their quads (effect_animations.lua).
    // Textures are converted too, with the name of the effect folder as a prefix (common_ for the
    // archive root), to battle/images/battle_fx, so they can be used as images of sprites.
    const std::string key_prefix = "ff7/data/battle/";
    auto key_of = [&](std::string folder){
        if (folder.empty()) return std::string("common");
        if (folder.compare(0, key_prefix.size(), key_prefix) == 0) folder = folder.substr(key_prefix.size());
        return EffectSafeName(folder);
    };
    std::ofstream file(data_dir_ + "battle/effects/effect_sprites.lua");
    file << "-- Frames of the sprite files of magic.lgp. Game.EffectSprites[name] is a list of frames with\n"
         << "-- the position (u, v) and size (w, h) in the texture, in pixels, the offset to draw it\n"
         << "-- (ox, oy) and the flags. The names are the folder and the file, like common_iyashi.\n"
         << "-- Textures are in game/battle/images/battle_fx, like common_iyashi00.png.\n"
         << "Game.EffectSprites = Game.EffectSprites or {}\n";
    // (The list has the quads of all the frames in order, with the position of the corner as ox, oy.)
    std::ofstream animations(data_dir_ + "battle/effects/effect_animations.lua");
    animations << "-- Frames of the sprite files (.SPT) of magic.lgp. Game.EffectAnimations[name] is a list of frames and\n"
               << "-- every frame a list of quads: {x, y, u, v, w, h}, with the position of the corner and the size of the\n"
               << "-- quad in units of 8 of the effect, and the place in the texture in pixels. The names are the folder\n"
               << "-- and the file, like common_houden_2.\n"
               << "Game.EffectAnimations = Game.EffectAnimations or {}\n";
    const std::string dir = output_dir_ + "temp/effects/";
    boost::filesystem::create_directories(data_dir_ + "battle/images/battle_fx/");
    for (size_t f = 0; f < files_.size(); f ++){
        const std::string& name = files_[f].name;
        if (name.size() < 3) continue;
        std::string ext = name.substr(name.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){return std::tolower(c);});
        const std::string key = key_of(files_[f].folder);
        if (ext == "tex"){
            const std::string stem = EffectSafeName(EffectStem(name));
            const std::string path = data_dir_ + "battle/images/battle_fx/" + key + "_" + stem + ".png";
            if (boost::filesystem::exists(path)) continue;
            try{
                files_[f].file.WriteFile(dir + key + "_" + stem + ".tex");
                TexFile tex(dir + key + "_" + stem + ".tex");
                tex.SavePng(path);
            }
            catch (const std::exception& ex){
                // Many of the original textures are dummies.
            }
        }
        else if (ext == "s"){
            File& sprite_file = files_[f].file;
            const size_t size = sprite_file.GetFileSize();
            if (size < 8) continue;
            sprite_file.SetOffset(0);
            std::vector<unsigned char> bytes(size);
            for (size_t b = 0; b < size; b ++) bytes[b] = sprite_file.readU8();
            // Only the files with the header of sprite lists.
            if (bytes[0] != 0x23 || bytes[1] != 0x20) continue;
            // After the header (the two characters, and the number of frames as a 32 bit value, at 4)
            // there are the frames. Each one has two 16 bit values, the second is the number of quads,
            // and then 20 bytes for every quad: flags (4 bytes), the position of the corner (x and y,
            // 16 bits), the place in the texture (u and v, 16 bits), 4 more bytes, and the width
            // (8 bits, twice) and the height (8 bits, twice). Positions and sizes are in units of 8 of
            // the effect. Layout found in the code that draws them (0x598a7a), and all the files of
            // magic.lgp end exactly where their last frame ends.
            struct Quad{int x, y, u, v, w, h;};
            std::vector<std::vector<Quad>> frames;
            const uint32_t frame_count = bytes[4] | (bytes[5] << 8) | (bytes[6] << 16) | (static_cast<uint32_t>(bytes[7]) << 24);
            size_t p = 8;
            bool valid = true;
            for (uint32_t fr = 0; fr < frame_count && valid; ++ fr){
                if (p + 4 > size){valid = false; break;}
                const int count = static_cast<int16_t>(bytes[p + 2] | (bytes[p + 3] << 8));
                p += 4;
                if (count < 0 || p + static_cast<size_t>(count) * 20 > size){valid = false; break;}
                std::vector<Quad> quads;
                for (int q = 0; q < count; ++ q, p += 20){
                    Quad quad;
                    quad.x = static_cast<int16_t>(bytes[p + 4] | (bytes[p + 5] << 8));
                    quad.y = static_cast<int16_t>(bytes[p + 6] | (bytes[p + 7] << 8));
                    quad.u = static_cast<int16_t>(bytes[p + 8] | (bytes[p + 9] << 8));
                    quad.v = static_cast<int16_t>(bytes[p + 10] | (bytes[p + 11] << 8));
                    quad.w = bytes[p + 16];
                    quad.h = bytes[p + 18];
                    quads.push_back(quad);
                }
                frames.push_back(quads);
            }
            if (!valid || p != size || frames.empty()) continue;
            const std::string sprite_name = key + "_" + EffectSafeName(EffectStem(name));
            // All the quads one after the other, for the effects that pick them by number.
            file << "Game.EffectSprites[\"" << sprite_name << "\"] = {\n";
            for (const std::vector<Quad>& quads : frames){
                for (const Quad& quad : quads){
                    file << "    {u = " << quad.u << ", v = " << quad.v << ", w = " << quad.w << ", h = " << quad.h
                         << ", ox = " << quad.x << ", oy = " << quad.y << "},\n";
                }
            }
            file << "}\n";
            // The frames, with their quads.
            animations << "Game.EffectAnimations[\"" << sprite_name << "\"] = {\n";
            for (const std::vector<Quad>& quads : frames){
                animations << "    {";
                for (size_t q = 0; q < quads.size(); ++ q){
                    animations << (q > 0 ? ", " : "") << "{" << quads[q].x << ", " << quads[q].y << ", " << quads[q].u << ", "
                               << quads[q].v << ", " << quads[q].w << ", " << quads[q].h << "}";
                }
                animations << "},\n";
            }
            animations << "}\n";
        }
    }
}
