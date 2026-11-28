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
#include <map>
#include <set>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <tinyxml.h>
#include <OgreMesh.h>
#include "installer/formats/TexFile.h"
#include "formats/model/HRCFileManager.h"
#include "formats/model/AFileManager.h"
#include "installer/formats/AbFile.h"
#include "installer/formats/DaFile.h"
#include "installer/tables/FF7Data.h"
#include "common/StringUtil.h"
#include "formats/FF7NameLookup.h"
#include "installer/extractors/BattleModelInstaller.h"

namespace{

/**
 * Files of magic.lgp (without extension) with the animations of the limit breaks of each model of the party.
 * The animation files have the format of the animations of the models, with 8 animations for the skeleton of
 * the character (the animations 44 to 51 of the model, which are empty in battle.lgp) and 8 for its weapon.
 * The files are in the order of the techniques of the character (limit level 1 first and second, level 2, ...):
 * the table that ff7.exe uses to load them (at 0x7b7b58) has them in groups of 7 by character.
 */
const std::map<std::string, std::vector<std::string>>& LimitAnimationFiles(){
    static const std::vector<std::string> cloud = {"blaver", "kyou", "limcl2", "limcl6", "limcl3", "limcl7", "limcl4"};
    static const std::vector<std::string> barret = {"hvshot", "limbr5", "limbr2", "limbr6", "limbr3", "limbr7", "limbr4"};
    static const std::map<std::string, std::vector<std::string>> files = {
        {"rt", cloud}, {"si", cloud},
        {"ru", {"limfast"}}, // Tifa.
        {"rv", {"iyash", "limea5", "limea2", "limea6", "limea3", "kodo", "limea4"}}, // Aerith.
        {"rw", {"limsled", "limrd5", "limsled", "limrd6", "limrd3", "limrd7", "limrd4"}}, // Red XIII.
        {"rx", {"limyf1", "limyf5", "limyf2", "limyf6", "limyf3", "limyf7", "limyf4"}}, // Yuffie.
        {"ry", {"dice"}}, // Cait Sith.
        {"rz", {"limcd1", "limcd5", "limcd2", "limcd6", "limcd3", "limcd7", "limcd4"}}, // Cid.
        {"sb", barret}, {"sc", barret}, {"sd", barret}, {"se", barret},
    };
    return files;
}

}

BattleModelInstaller::BattleModelInstaller(
  DiskImage& disk_image, const std::string& output_dir, Ogre::ResourceGroupManager* res_mgr,
  BattleMeshExporter& exporter
):
  disk_image_(disk_image), output_dir_(output_dir), data_dir_(output_dir + "game/"),
  res_mgr_(res_mgr), exporter_(exporter)
{}

unsigned int BattleModelInstaller::InitializeBattleModels(){
    next_model_to_process_ = 0;
    next_model_to_convert_ = 0;
    models_.clear();
    files_ = ReadLgp(*disk_image_.filePath("data/battle/battle.lgp"));
    return files_.size();
}

unsigned int BattleModelInstaller::ProcessBattleModel(){
    if (next_model_to_process_ >= files_.size()) return files_.size();
    // TODO: Do something with the files.
    // A little explanation. Each file has a 4 letter name, for example: 1234
    // 12 is the model identifier.
    // 34 if the type of file:
    //  - aa: Skeleton file (.hrc)
    //  - ab: Animation scripts
    //  - ac - al: Textures (.tex)
    //  - am - cz: Polygon files (.p)
    //  - da: Animations (.anim)
    const std::string name = files_[next_model_to_process_].name;
    File& file = files_[next_model_to_process_].file;
    const std::string id = name.substr(0, 2);
    const std::string type = name.substr(2, 2);
    const std::string temp = output_dir_ + "temp/battle_models/" + name;
    if (type == "aa"){
        // HRC file.
        file.WriteFile(temp + ".hrcbin");
        FindOrAddModel(models_, id).hrc = name + ".hrc";
    }
    else if (type == "ab"){
        file.WriteFile(temp + ".script");
        FindOrAddModel(models_, id).script = name + ".script";
    }
    else if (type == "da"){
        // .da Animation file.
        file.WriteFile(temp + ".anim");
        FindOrAddModel(models_, id).anim = name + ".anim";
    }
    else if (type >= "ac" && type <= "al"){
        // .tex texture files. Save directly to their directory.
        file.WriteFile(temp + ".tex");
        ProcessTexture(id, type, file);
    }
    else{
        // .p polygon file.
        file.WriteFile(temp + ".p");
        FindOrAddModel(models_, id).p.push_back(name + ".p");
    }
    next_model_to_process_ ++;
    return next_model_to_process_;
}

void BattleModelInstaller::ProcessTexture(
  const std::string& id, const std::string& type, const File& file
){
    const FF7Data::BattleModelInfo info = FF7Data::GetBattleModelInfo(id);
    TexFile tex(file);
    std::string path = data_dir_ + "battle/models/";
    if (info.is_player){
        path += "characters/";
        AddCharacterModel(info.numeric_id, info.name, info.alphanumeric_id + "_" + info.name_normal + ".mesh");
    }
    else if (info.is_scene){
        path += "scenes/";
        AddSceneModel(info.numeric_id, info.name, info.alphanumeric_id + "_" + info.name_normal + ".mesh");
    }
    else path += "enemies/";
    const std::string png = id + type + "_" + info.name_normal + ".png";
    tex.SavePng(path + png);
    res_mgr_->declareResource(png, "Texture", "FFVIITextures");
    FindOrAddModel(models_, id).tex.push_back(png);
}

void BattleModelInstaller::AddCharacterModel(
  const unsigned int character_id, const std::string& name, const std::string& mesh
){
    // Every texture of a character model reaches this point, but the model is listed once.
    auto character = std::find_if(
      character_data_.begin(), character_data_.end(),
      [&](const CharacterData& data){return data.id == character_id;}
    );
    if (character == character_data_.end())
        character = character_data_.insert(character, {character_id, {}});
    for (const CharacterModel& model : character->models)
        if (model.model == mesh) return;
    character->models.push_back({name, mesh});
}

void BattleModelInstaller::AddSceneModel(
  const int scene_id, const std::string& description, const std::string& mesh
){
    // Every texture of a scene model reaches this point, but the map needs one entry.
    if (std::any_of(scene_data_.begin(), scene_data_.end(), [&](const SceneModel& scene){
        return scene.id == scene_id && scene.model == mesh;
    })) return;
    scene_data_.push_back({scene_id, mesh, description});
}

unsigned int BattleModelInstaller::ConvertBattleModelsInit(){
    next_model_to_convert_ = 0;
    // The animations of the limit breaks are in magic.lgp. Take the ones of the party to the folder of the battle
    // models, so they are converted with them.
    try{
        std::set<std::string> wanted;
        for (const auto& entry : LimitAnimationFiles()){
            for (const std::string& tag : entry.second) wanted.insert(tag + ".a00");
        }
        for (LgpEntry& entry : ReadLgp(*disk_image_.filePath("data/battle/magic.lgp"))){
            std::string name = entry.name;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            if (wanted.count(name) == 0) continue;
            entry.file.WriteFile(output_dir_ + "temp/battle_models/limit_" + name);
        }
    }
    catch (const std::exception& ex){
        std::cerr << "[ERROR] Exception reading the limit animations: " << ex.what() << std::endl;
    }
    return models_.size();
}

unsigned int BattleModelInstaller::ConvertBattleModel(){
    if (next_model_to_convert_ >= models_.size()) return models_.size();
    BattleModel model = models_[next_model_to_convert_];
    try{
        const std::string temp = output_dir_ + "temp/battle_models/";
        const FF7Data::BattleModelInfo info = FF7Data::GetBattleModelInfo(model.id);
        const std::string full_name = model.id + "_" + info.name_normal;
        std::string path = "";
        if (info.is_player) path += "characters/";
        else if (info.is_scene) path += "scenes/";
        else path += "enemies/";
        const File compiled(temp + model.hrc + "bin");
        if (info.is_scene)
            DecompileSceneModelHrc(compiled, model, temp + full_name + ".hrc", full_name);
        else{
            std::vector<std::string> sorted = model.p;
            std::sort(sorted.begin(), sorted.end());
            std::vector<std::string> meshes;
            for (const std::string& p : sorted) meshes.push_back(p.substr(0, 4));
            DecompileHrc(compiled, meshes, temp + full_name + ".hrc", full_name);
        }
        WriteRsdFiles(model, temp);

        if ("" != model.anim){ // Skip for non-animated, ie battle backgrounds
            DaFile da(File(temp + model.anim));
            for (const std::string& file_name : da.GenerateAFiles(model.id, temp))
                model.a.push_back(file_name);
        }
        if (info.is_player) AddLimitAnimations(model);
        // Skip non scripted models.
        if ("" != model.script) ConvertScripts(model, full_name, info.is_player, info.is_scene);
        std::vector<std::string> weapons;
        if (info.is_player) weapons = WriteWeaponSkeletons(model, full_name);

        // Reload the resources for the newly created hrc and rsd.
        ReloadModelResources(res_mgr_, temp);

        VGears::HRCFileManager::GetSingleton().load(full_name + ".hrc", "FFVII");
        Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(full_name + ".mesh", "FFVII"));
        AddAnimations(mesh->getSkeleton(), model.a);
        exporter_.Export(path, mesh);
        ExportWeapons(weapons, path);
    }
    catch (const Ogre::Exception& ex){
        std::cerr << "[ERROR] Ogre exception converting battle model " << model.hrc <<": " << ex.what() << std::endl;
    }
    catch (const std::exception& ex){
        std::cerr << "[ERROR] Exception converting battle model " << model.hrc << ": " << ex.what() << std::endl;
    }
    next_model_to_convert_ ++;
    return next_model_to_convert_;
}

void BattleModelInstaller::AddLimitAnimations(BattleModel& model) const{
    // Limit breaks of the party: their animations are in files of magic.lgp. Each one is added to
    // the model with the names <model>_<file>_00 to _07, which replace the empty animations 44 to
    // 51 for that limit.
    auto limits = LimitAnimationFiles().find(model.id);
    if (limits == LimitAnimationFiles().end()) return;
    std::set<std::string> converted;
    for (const std::string& tag : limits->second){
        if (!converted.insert(tag).second) continue;
        std::string limit_path = output_dir_ + "temp/battle_models/limit_" + tag + ".a00";
        if (!std::ifstream(limit_path).good()) continue;
        try{
            File limit_file(limit_path);
            DaFile limit_da(limit_file);
            std::vector<std::string> limit_files = limit_da.GenerateAFiles(
              model.id + "_" + tag, output_dir_ + "temp/battle_models/"
            );
            for (const std::string& file_name : limit_files){
                // Only the animations of the skeleton (0 to 7), not the ones of the weapon.
                size_t dot = file_name.find_last_of('.');
                int number = std::atoi(file_name.substr(dot - 2, 2).c_str());
                if (number < 8) model.a.push_back(file_name);
            }
        }
        catch (const std::exception& ex){
            std::cerr << "[ERROR] Exception reading the limit animation " << tag << ": " << ex.what() << std::endl;
        }
    }
}

void BattleModelInstaller::ConvertScripts(
  const BattleModel& model, const std::string& full_name, const bool is_player, const bool is_scene
){
    const std::string temp = output_dir_ + "temp/battle_models/";
    // Only the models of the party have the 74 scripts (the limits are the ones from 60 up). The
    // other ones (enemies and the rest of the models with scripts) have 32.
    AbFile ab(File(temp + model.script), !is_player);
    ab.GenerateScripts(full_name, data_dir_ + "battle/scripts/");
    if (!is_scene && "" != model.hrc){
        WriteModelCamera(File(temp + model.script), File(temp + model.hrc + "bin"), full_name);
    }
}

std::vector<std::string> BattleModelInstaller::WriteWeaponSkeletons(
  const BattleModel& model, const std::string& full_name
) const{
    // Playable characters hold their weapons in battle (but Red XIII, who has none). Each one is a
    // polygon file by itself, named <id>c<k..z>, so make a model for each with a single bone.
    std::vector<std::string> weapons;
    if (model.id == "rw") return weapons;
    for (const std::string& p : model.p){
        if (p.length() < 4 || p[2] != 'c' || p[3] < 'k' || p[3] > 'z') continue;
        char number[8];
        std::snprintf(number, sizeof(number), "%02d", p[3] - 'k');
        const std::string weapon_name = full_name + "_weapon_" + number;
        std::ofstream hrc(output_dir_ + "temp/battle_models/" + weapon_name + ".hrc");
        hrc << ":HEADER_BLOCK 2\n:SKELETON " << weapon_name << "\n:BONES 1\n\nbone0\nroot\n0.000000\n1 "
            << p.substr(0, 4) << "\n\n";
        hrc.close();
        weapons.push_back(weapon_name);
    }
    return weapons;
}

void BattleModelInstaller::ExportWeapons(
  const std::vector<std::string>& weapons, const std::string& path
){
    for (const std::string& weapon : weapons){
        try{
            VGears::HRCFileManager::GetSingleton().load(weapon + ".hrc", "FFVII");
            Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(weapon + ".mesh", "FFVII"));
            exporter_.Export(path, mesh);
        }
        catch (const Ogre::Exception& ex){
            std::cerr << "[ERROR] Ogre exception converting weapon " << weapon << ": " << ex.what() << std::endl;
        }
    }
}

void BattleModelInstaller::WriteCharacterData(){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("characters"));
    for (CharacterData data : character_data_){
        std::unique_ptr<TiXmlElement> xml_character(new TiXmlElement("character"));
        xml_character->SetAttribute("id", data.id);
        for (CharacterModel model : data.models){
            std::unique_ptr<TiXmlElement> xml_model(new TiXmlElement("model"));
            xml_model->SetAttribute("name", model.name);
            xml_model->SetAttribute("file", model.model);
            xml_character->LinkEndChild(xml_model.release());
        }
        container->LinkEndChild(xml_character.release());
    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(data_dir_ + "battle/models/characters.xml");
}

void BattleModelInstaller::WriteSceneData(){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("scenes"));
    for (SceneModel data : scene_data_){
        std::unique_ptr<TiXmlElement> xml_scene(new TiXmlElement("scene"));
        xml_scene->SetAttribute("id", data.id);
        xml_scene->SetAttribute("file", data.model);
        xml_scene->SetAttribute("description", data.description);
        container->LinkEndChild(xml_scene.release());
    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(data_dir_ + "battle/models/scenes.xml");
}

void BattleModelInstaller::WriteRsdFiles(BattleModel model, const std::string& path){
    std::sort(model.tex.begin(), model.tex.end());
    // For each p file, generate a rsd file.
    for (std::string p : model.p){
        std::string base_name = p.substr(0, 4);
        std::string content = "@RSD940102\n";
        content += "PLY=" + base_name + ".PLY\n";
        content += "MAT=" + base_name + ".MAT\n";
        content += "GRP=" + base_name + ".GRP\n";
        content += "NTEX=" + std::to_string(model.tex.size()) + "\n";
        for (int t = 0; t < model.tex.size(); t ++) content += "TEX[" + std::to_string(t) + "]=" + model.tex[t] + "\n";
        content += "\n";
        std::ofstream out(path + base_name + ".rsd");
        out << content;
        out.close();
    }
}

void BattleModelInstaller::WriteModelCamera(
  File script, File skeleton, const std::string& model_name
){
    // The scripts of a model start with a header of 0x68 bytes, that ff7.exe copies to the data the
    // cameras use (0x42b6a0): 16 points of the model at 0x12, the factors of the size at 0x44, two
    // views of the model (a camera position and a point to look at) at 0x4C and the height of the point
    // the camera looks at, at 0x64.
    if (script.GetFileSize() < 0x68 || skeleton.GetFileSize() < 16) return;
    skeleton.SetOffset(12);
    const unsigned int bones = skeleton.readU32LE();
    if (bones == 0 || skeleton.GetFileSize() < 16 + bones * 12) return;
    std::ofstream file(data_dir_ + "battle/scripts/" + model_name + "_camera.lua");
    if (!file) return;
    file << "-- Data of the model for the cameras of the battles. A point is the end of the bone with the\n"
      << "-- number of the point minus one (0 is the root).\n"
      << "BattleModelCameras = BattleModelCameras or {}\n"
      << "BattleModelCameras[\"" << model_name << "\"] = {\n    points = {";
    script.SetOffset(0x12);
    for (int i = 0; i < 16; i ++) file << static_cast<int>(script.readU8()) << ", ";
    file << "},\n    size = {";
    script.SetOffset(0x44);
    for (int i = 0; i < 3; i ++) file << static_cast<int>(static_cast<int16_t>(script.readU16LE())) << ", ";
    // The two views, one after the other.
    script.SetOffset(0x4C);
    file << "},\n    views = {";
    for (int v = 0; v < 2; v ++){
        for (int p = 0; p < 2; p ++){
            file << (p == 0 ? "{pos = {" : "look = {");
            for (int i = 0; i < 3; i ++) file << static_cast<int>(static_cast<int16_t>(script.readU16LE())) << ", ";
            file << (p == 0 ? "}, " : "}}, ");
        }
    }
    script.SetOffset(0x64);
    file << "},\n    look_height = " << static_cast<int>(static_cast<int16_t>(script.readU16LE()))
      << ",\n    lengths = {";
    // The bones are the last data of the skeleton: parent, length (negative) and polygons.
    skeleton.SetOffset(skeleton.GetFileSize() - bones * 12);
    for (unsigned int b = 0; b < bones; b ++){
        skeleton.readU32LE();
        const u32 raw = skeleton.readU32LE();
        float length = 0.0f;
        std::memcpy(&length, &raw, sizeof(length));
        skeleton.readU32LE();
        file << "[" << b << "] = " << -length << ", ";
    }
    file << "},\n}\n";
}

void BattleModelInstaller::DecompileSceneModelHrc(
  File compiled, BattleModel model, std::string path, std::string skeleton_name
){
    compiled.SetOffset(0);
    compiled.readU32LE(); // Skip 4 bytes
    compiled.readU32LE(); // Skip 4 bytes
    compiled.readU32LE(); // Skip 4 bytes
    compiled.readU32LE(); // Supposed to be the number of bones, but for scenes is always 0.
    std::sort(model.p.begin(), model.p.end());
    std::string decompiled(":HEADER_BLOCK 2\n:SKELETON " + skeleton_name + "\n");
    int bones = model.p.size();
    decompiled += ":BONES " + std::to_string(bones) + "\n\n";
    int p_file_index = 0;
    for (int b = 0; b < bones; b ++){
        decompiled += "bone" + std::to_string(b) + "\n";
        if (b == 0) decompiled += "root\n";
        else decompiled += ("bone" + std::to_string(b - 1) + "\n");
        // The pieces share the coordinates of the scene. A length would move each one along the depth, away
        // from the previous one.
        decompiled += "0.000000\n";
        decompiled += "1 " + model.p[b].substr(0, 4) + "\n\n";
    }
    std::ofstream out(path);
    out << decompiled;
    out.close();
}
