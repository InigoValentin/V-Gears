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

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <zlib.h>
#include <tinyxml.h>
#include <boost/filesystem.hpp>
#include <OgreMesh.h>
#include "BattleDataInstaller.h"
#include "BattleCameraScripts.h"
#include "ExeEffectCameras.h"
#include "ExeEffectMeshes.h"
#include "ModelMaterials.h"
#include "TexFile.h"
#include "formats/model/VGearsHRCFileManager.h"
#include "formats/model/VGearsAFileManager.h"
#include "data/AbFile.h"
#include "data/DaFile.h"
#include "data/FF7Data.h"
#include "common/VGearsStringUtil.h"
#include "formats/FF7NameLookup.h"

BattleDataInstaller::BattleDataInstaller(
  const DiskImage& disk_image, const std::string output_dir, Ogre::ResourceGroupManager* res_mgr
):
  disk_image_(disk_image), output_dir_(output_dir), data_dir_(output_dir + "data/"),
  scene_bin_(*const_cast<DiskImage&>(disk_image).filePath("data/battle/scene.bin")), total_scenes_(0), next_scene_(0),
  res_mgr_(res_mgr)
{}

BattleDataInstaller::~BattleDataInstaller(){}

unsigned int BattleDataInstaller::InitializeScenes(){
    next_scene_ = 0;
    scenes_.clear();
    enemies_.clear();
    attacks_.clear();
    scenes_uncompressed_.clear();
    total_scenes_ = 0;
    // scene.bin is a sequence of 0x2000 bytes blocks (33 in the original game). Each block starts
    // with up to 16 pointers (in 4-byte units) to gzipped scenes, terminated by 0xFFFFFFFF. A scene
    // ends where the next one starts, or at the end of the block. There are 256 scenes in total.
    // Scene indexes are positional: every slot must be kept, even if it can't be extracted,
    // because battle IDs (scene * 4 + setup) are derived from them.
    const u32 block_size = 0x2000;
    const u32 blocks = scene_bin_.GetFileSize() / block_size;
    for (u32 i = 0; i < blocks; i ++){
        File scene_bin_block(&scene_bin_, i * block_size, block_size);
        u32 offsets[16];
        u32 count = 0;
        for (; count < 16; count ++){
            scene_bin_block.SetOffset(4 * count);
            u32 pointer = scene_bin_block.readU32LE();
            if (pointer == 0xFFFFFFFF || pointer * 4 >= block_size) break;
            offsets[count] = pointer * 4;
        }
        for (u32 o = 0; o < count; o ++){
            u32 start_offset = offsets[o];
            u32 end_offset = (o + 1 < count) ? offsets[o + 1] : block_size;
            if (end_offset <= start_offset) end_offset = block_size;
            File scene_bin_fragment(&scene_bin_block, start_offset, end_offset - start_offset);
            // NULL if it can't be extracted. Kept to preserve scene indexes.
            scenes_uncompressed_.push_back(ExtractGZipScene(scene_bin_fragment));
            total_scenes_ ++;
        }
    }
    return total_scenes_;
}

unsigned int BattleDataInstaller::InitializeBattleModels(){
    next_model_to_process_ = 0;
    next_model_to_convert_ = 0;
    battle_lgp_files_.clear();
    battle_lgp_file_names_.clear();
    battle_models_.clear();
    // Open battle.lgp
    File battle_lgp_file(*disk_image_.filePath("data/battle/battle.lgp"));
    // Also, open it as a LGP archive.
    VGears::LGPArchive battle_lgp(*disk_image_.filePath("data/battle/battle.lgp"), "LGP");
    battle_lgp.open(*disk_image_.filePath("data/battle/battle.lgp"), true);
    battle_lgp.load();
    VGears::LGPArchive::FileList file_list = battle_lgp.GetFiles();
    for (int i = 0; i < file_list.size(); i ++){
        VGears::LGPArchive::FileEntry f = file_list.at(i);
        if (f.data_offset + f.data_size <= battle_lgp_file.GetFileSize()){
            File b_lgp_file(&battle_lgp_file, f.data_offset, f.data_size);
            battle_lgp_files_.push_back(b_lgp_file);
            battle_lgp_file_names_.push_back(f.file_name);
        }
    }
    return battle_lgp_files_.size();
}

unsigned int BattleDataInstaller::InitializeSpellModels(){
    next_model_to_process_ = 0;
    next_model_to_convert_ = 0;
    battle_lgp_files_.clear();
    battle_lgp_file_names_.clear();
    battle_models_.clear();
    magic_lgp_files_.clear();
    magic_lgp_file_names_.clear();
    magic_lgp_file_folders_.clear();
    // Open magic.lgp
    File magic_lgp_file(*disk_image_.filePath("data/battle/magic.lgp"));
    // Also, open it as a LGP archive.
    VGears::LGPArchive magic_lgp(*disk_image_.filePath("data/battle/magic.lgp"), "LGP");
    magic_lgp.open(*disk_image_.filePath("data/battle/magic.lgp"), true);
    magic_lgp.load();
    VGears::LGPArchive::FileList file_list = magic_lgp.GetFiles();
    for (int i = 0; i < file_list.size(); i ++){
        VGears::LGPArchive::FileEntry f = file_list.at(i);
        if (f.data_offset + f.data_size <= magic_lgp_file.GetFileSize()){
            File m_lgp_file(&magic_lgp_file, f.data_offset, f.data_size);
            magic_lgp_files_.push_back(m_lgp_file);
            magic_lgp_file_names_.push_back(f.file_name);
            magic_lgp_file_folders_.push_back(f.folder);
        }
    }
    return magic_lgp_files_.size();
}

unsigned int BattleDataInstaller::ProcessScene(){
    if (next_scene_ >= scenes_uncompressed_.size()) return total_scenes_;
    File* scene_file = scenes_uncompressed_[next_scene_].get();
    if (scene_file == NULL){
        // Keep battle IDs stable: an unreadable scene still takes its 4 battle slots.
        std::cout << "Warning: scene " << next_scene_ << " could not be extracted." << std::endl;
        for (int f = 0; f < 4; f ++){
            Formation empty;
            empty.id = next_scene_ * 4 + f;
            formations_.push_back(empty);
        }
        next_scene_ ++;
        return next_scene_;
    }
    BattleSceneFile scene(next_scene_, *scene_file);
    scenes_.push_back(scene);
    // Add new enemies to the list.
    std::vector<Enemy> enemies = scene.GetEnemies();
    for (int e = 0; e < enemies.size(); e ++){
        if (std::any_of(
          enemies_.begin(), enemies_.end(), [&](const Enemy& existing){ return existing.id == enemies[e].id; }
        )) continue;
        enemies_.push_back(enemies[e]);
    }
    // Add new attacks to the list.
    std::vector<Attack> attacks = scene.GetAttacks();
    for (int a = 0; a < attacks.size(); a ++){
        if (std::any_of(
          attacks_.begin(), attacks_.end(), [&](const Attack& existing){ return existing.id == attacks[a].id; }
        )) continue;
        attacks_.push_back(attacks[a]);
    }
    // Battle ID = scene index * 4 + setup index, as referenced by field scripts and encounters.
    std::vector<Formation> formations = scene.GetFormations();
    for (int f = 0; f < formations.size(); f ++){
        formations[f].id = next_scene_ * 4 + f;
        formations_.push_back(formations[f]);
    }
    next_scene_ ++;
    return next_scene_;
}

unsigned int BattleDataInstaller::ProcessBattleModel(){
    if (next_model_to_process_ >= battle_lgp_files_.size()) return battle_lgp_files_.size();
    // TODO: Do something with the files.
    // A little explanation. Each file has a 4 letter name, for example: 1234
    // 12 is the model identifier.
    // 34 if the type of file:
    //  - aa: Skeleton file (.hrc)
    //  - ab: Animation scripts
    //  - ac - al: Textures (.tex)
    //  - am - cz: Polygon files (.p)
    //  - da: Animations (.anim)
    std::string id = battle_lgp_file_names_[next_model_to_process_].substr(0, 2);
    /*if (id != "rt"){ // TODO DEBUG REMOVE
        next_model_to_process_ ++;
        return next_model_to_process_;
    }*/
    std::string type = battle_lgp_file_names_[next_model_to_process_].substr(2, 2);
    FF7Data::BattleModelInfo info = FF7Data::GetBattleModelInfo(id);
    if (type == "aa"){
        // HRC file.
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/"
          + battle_lgp_file_names_[next_model_to_process_] + ".hrcbin"
        );
        bool found = false;
        for (int m = 0; m < battle_models_.size(); m ++){
            if (battle_models_[m].id == id){
                battle_models_[m].hrc = battle_lgp_file_names_[next_model_to_process_] + ".hrc";
                found = true;
                break;
            }
        }
        if (found == false){
            BattleModel model;
            model.id = id;
            model.hrc = battle_lgp_file_names_[next_model_to_process_] + ".hrc";
            battle_models_.push_back(model);
        }
    }
    else if (type == "ab"){
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/"
          + battle_lgp_file_names_[next_model_to_process_] + ".script"
        );
        auto model_it = std::find_if(
            battle_models_.begin(), battle_models_.end(),
            [&](const BattleModel& model){ return model.id == id; }
        );
        if (model_it != battle_models_.end()){
                model_it->script = battle_lgp_file_names_[next_model_to_process_] + ".script";
        }
        else{
            BattleModel model;
            model.id = id;
            model.script = battle_lgp_file_names_[next_model_to_process_] + ".script";
            battle_models_.push_back(model);
        }
    }
    else if (type == "da"){
        // .da Animation file.
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/" + battle_lgp_file_names_[next_model_to_process_] + ".anim"
        );
        bool found = false;
        for (int m = 0; m < battle_models_.size(); m ++){
            if (battle_models_[m].id == id){
                battle_models_[m].anim = battle_lgp_file_names_[next_model_to_process_] + ".anim";
                found = true;
                break;
            }
        }
        if (found == false){
            BattleModel model;
            model.id = id;
            model.anim = battle_lgp_file_names_[next_model_to_process_] + ".anim";
            battle_models_.push_back(model);
        }
    }
    else if (
      type == "ac" || type == "ad" || type == "ae" || type == "af" || type == "ag"
      || type == "ah"|| type == "ai" || type == "aj" || type == "ak" || type == "al"
    ){
        // .tex texture files. Save directly to their directory.
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/" + battle_lgp_file_names_[next_model_to_process_] + ".tex"
        );
        TexFile tex(battle_lgp_files_[next_model_to_process_]);
        std::string path = data_dir_ + "models/battle/";
        if (info.is_player){
            path += "characters/";
            // Add or update on character_data_
            bool found = false;
            for (auto data: character_data_){
                if (data.id == info.numeric_id){
                    // Check if the model already exists for this character
                    bool model_found = false;
                    for (auto mod: data.models){
                        if (mod.model == info.alphanumeric_id + "_" + info.name_normal + ".mesh"){
                            model_found = true;
                            break;
                        }
                    }
                    if (model_found) break;
                    CharacterModel model;
                    model.name = info.name;
                    model.model = info.alphanumeric_id + "_" + info.name_normal + ".mesh";
                    data.models.push_back(model);
                    character_data_.push_back(data);
                    found = true;
                    break;
                }
            }
            if (!found){
                CharacterData character_data;
                character_data.id = info.numeric_id;
                CharacterModel model;
                model.name = info.name;
                model.model = info.alphanumeric_id + "_" + info.name_normal + ".mesh";
                character_data.models.push_back(model);
                character_data_.push_back(character_data);
            }
        }
        else if (info.is_scene){
            SceneModel scene_data;
            scene_data.id = info.numeric_id;
            scene_data.model = info.alphanumeric_id + "_" + info.name_normal + ".mesh";
            scene_data.description = info.name;
            // Every texture of a scene model reaches this point, but the map needs one entry.
            if (std::none_of(scene_data_.begin(), scene_data_.end(), [&](const SceneModel& s){
                return s.id == scene_data.id && s.model == scene_data.model;
            })) scene_data_.push_back(scene_data);
            path += "scenes/";
        }
        else path += "enemies/";
        path += (id + type + "_" + info.name_normal + ".png");
        tex.SavePng(path);
        res_mgr_->declareResource(
          id + type + "_" + info.name_normal + ".png", "Texture", "FFVIITextures"
        );
        bool found = false;
        for (int m = 0; m < battle_models_.size(); m ++){
            if (battle_models_[m].id == id){
                battle_models_[m].tex.push_back(id + type+ "_" + info.name_normal + ".png");
                found = true;
                break;
            }
        }
        if (found == false){
            BattleModel model;
            model.id = id;
            model.tex.push_back(id + type + "_" + info.name_normal + ".png");
            battle_models_.push_back(model);
        }
    }
    else{
        // .p polygon file.
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/" + battle_lgp_file_names_[next_model_to_process_] + ".p"
        );
        bool found = false;
        for (int m = 0; m < battle_models_.size(); m ++){
            if (battle_models_[m].id == id){
                battle_models_[m].p.push_back(battle_lgp_file_names_[next_model_to_process_] + ".p");
                found = true;
                break;
            }
        }
        if (found == false){
            BattleModel model;
            model.id = id;
            model.p.push_back(battle_lgp_file_names_[next_model_to_process_] + ".p");
            battle_models_.push_back(model);
        }
    }
    next_model_to_process_ ++;
    return next_model_to_process_;
}

unsigned int BattleDataInstaller::ProcessSpellModel(){
    if (next_model_to_process_ >= magic_lgp_files_.size()) return magic_lgp_files_.size();
    // TODO: Do something with the files.
    // A little explanation. There are no patterns on file names, and there are multiple extensions.
    //  - d: Skeleton file, similar to aa from battle models.
    //  - b: Unknown.
    //  - p*: Polygon files, for each bone.
    //  - tex: Texture files, TIM format.
    //  - rsd: Resource files
    //  - s: Unknown.
    //  - a00: Animation files, similar to da from battle models.
    //std::cout << "SPELL MODEL FILE: " << magic_lgp_file_names_[next_model_to_process_] << std::endl;
    std::string fname = magic_lgp_file_names_[next_model_to_process_];
    std::string id = fname.substr(0, fname.find_last_of("."));
    std::string ext = fname.substr(fname.find_last_of(".") + 1);


    FF7Data::SpellModelInfo info = FF7Data::GetSpellModelInfo(id);
    //magic_lgp_files_[next_model_to_process_].WriteFile(output_dir_ + "temp/spell_models/" + fname);

    if (ext == "d"){
        // Skeleton file.
        magic_lgp_files_[next_model_to_process_].WriteFile(output_dir_ + "temp/spell_models/" + fname);
        bool found = false;
        for (int m = 0; m < spell_models_.size(); m ++){
            if (spell_models_[m].id == id){
                spell_models_[m].hrc = fname;
                found = true;
                break;
            }
        }
        if (found == false){
            SpellModel model;
            model.id = id;
            model.hrc = fname;
            spell_models_.push_back(model);
        }
    }
    else if (ext.at(0) == 'p'){
        // Bone file.
        // If the extension is just ".p", use the file name.
        std::string p_name;
        if (ext == "p") p_name = fname;
        // If the extension is ".pXX", save it with altered name.
        else p_name =
          fname.substr(0, fname.length() - 4) + "_" + fname.substr(fname.length() - 2, 2) + ".p";
        magic_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/spell_models/" + p_name
        );
        bool found = false;
        for (int m = 0; m < spell_models_.size(); m ++){
            if (spell_models_[m].id == id){
                spell_models_[m].p.push_back(p_name);
                found = true;
                break;
            }
        }
        if (found == false){
            SpellModel model;
            model.id = id;
            model.p.push_back(p_name);
            spell_models_.push_back(model);
        }
    }
    if (ext == "tex"){
        // Texture files. They can be shared among various spells, so just convert them and don't
        // add them to a particular model.
        magic_lgp_files_[next_model_to_process_].WriteFile(output_dir_ + "temp/spell_models/" + fname);
        TexFile tex(output_dir_ + "temp/spell_models/" + fname);
        std::string path = data_dir_ + "models/battle/attacks/" + id + ".png";
        // Many of these tex files are wrong and dummied out in the OG, so try/catch.
        try{
            tex.SavePng(path);
        }
        catch(const std::exception& e){
            std::cerr << "[WARNING] Error converting texture " << (output_dir_ + "temp/spell_models/" + fname)
              << " to png: " << e.what() << std::endl;
        }
    }
    if (ext == "a00"){
        // Animation file.
        magic_lgp_files_[next_model_to_process_].WriteFile(output_dir_ + "temp/spell_models/" + fname);
        bool found = false;
        for (int m = 0; m < spell_models_.size(); m ++){
            if (spell_models_[m].id == id){
                spell_models_[m].anim = fname;
                found = true;
                break;
            }
        }
        if (found == false){
            SpellModel model;
            model.id = id;
            model.anim = fname;
            spell_models_.push_back(model);
        }
    }
    // TODO: b files? a files? s files?
    next_model_to_process_ ++;
    return next_model_to_process_;
}

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

unsigned int BattleDataInstaller::ConvertBattleModelsInit(){
    next_model_to_convert_ = 0;
    // The animations of the limit breaks are in magic.lgp. Take the ones of the party to the folder of the battle
    // models, so they are converted with them.
    try{
        std::set<std::string> wanted;
        for (const auto& entry : LimitAnimationFiles()){
            for (const std::string& tag : entry.second) wanted.insert(tag + ".a00");
        }
        File magic_lgp_file(*disk_image_.filePath("data/battle/magic.lgp"));
        VGears::LGPArchive magic_lgp(*disk_image_.filePath("data/battle/magic.lgp"), "LGP");
        magic_lgp.open(*disk_image_.filePath("data/battle/magic.lgp"), true);
        magic_lgp.load();
        VGears::LGPArchive::FileList file_list = magic_lgp.GetFiles();
        for (int i = 0; i < file_list.size(); i ++){
            VGears::LGPArchive::FileEntry f = file_list.at(i);
            std::string name = f.file_name;
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            if (wanted.count(name) == 0) continue;
            if (f.data_offset + f.data_size > magic_lgp_file.GetFileSize()) continue;
            File limit_file(&magic_lgp_file, f.data_offset, f.data_size);
            limit_file.WriteFile(output_dir_ + "temp/battle_models/limit_" + name);
        }
    }
    catch (const std::exception& ex){
        std::cerr << "[ERROR] Exception reading the limit animations: " << ex.what() << std::endl;
    }
    return battle_models_.size();
}

unsigned int BattleDataInstaller::ConvertSpellModelsInit(){
    next_model_to_convert_ = 0;
    return spell_models_.size();
}

unsigned int BattleDataInstaller::ConvertBattleModel(){
    if (next_model_to_convert_ >= battle_models_.size()) return battle_models_.size();
    BattleModel model = battle_models_[next_model_to_convert_];
    try{
        std::string path = "";
        FF7Data::BattleModelInfo info = FF7Data::GetBattleModelInfo(model.id);
        if (info.is_player) path += "characters/";
        else if (info.is_scene) path += "scenes/";
        else path += "enemies/";
        if (info.is_scene){
            DecompileSceneModelHrc(
              File(output_dir_ + "temp/battle_models/" + model.hrc + "bin"), model,
              output_dir_ + "temp/battle_models/" + model.id + "_" + info.name_normal + ".hrc",
              model.id + "_" + info.name_normal
            );
        }
        else{
            DecompileBattleModelHrc(
              File(output_dir_ + "temp/battle_models/" + model.hrc + "bin"), model,
              output_dir_ + "temp/battle_models/" + model.id + "_" + info.name_normal + ".hrc",
              model.id + "_" + info.name_normal
            );
        }
        GenerateRsdFiles(model, output_dir_ + "temp/battle_models/");

        if ("" != model.anim){ // Skip for non-animated, ie battle backgrounds
            DaFile da(File(output_dir_ + "temp/battle_models/" + model.anim));
            std::vector<std::string> a_files = da.GenerateAFiles(model.id, output_dir_ + "temp/battle_models/");
            for (std::string file_name : a_files) model.a.push_back(file_name);
        }

        // Limit breaks of the party: their animations are in files of magic.lgp. Each one is added to the model with
        // the names <model>_<file>_00 to _07, which replace the empty animations 44 to 51 for that limit.
        if (info.is_player){
            auto limits = LimitAnimationFiles().find(model.id);
            if (limits != LimitAnimationFiles().end()){
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
        }

        if ("" != model.script){ // Skip non scripted models
            // Only the models of the party have the 74 scripts (the limits are the ones from 60 up). The other
            // ones (enemies and the rest of the models with scripts) have 32.
            AbFile ab(File(output_dir_ + "temp/battle_models/" + model.script), !info.is_player);
            ab.GenerateScripts(model.id + "_" + info.name_normal, data_dir_ + "scripts/battle_models/");
            if (!info.is_scene && "" != model.hrc){
                WriteModelCamera(
                  File(output_dir_ + "temp/battle_models/" + model.script),
                  File(output_dir_ + "temp/battle_models/" + model.hrc + "bin"),
                  model.id + "_" + info.name_normal
                );
            }
        }

        // Playable characters hold their weapons in battle (but Red XIII, who has none). Each one is a
        // polygon file by itself, named <id>c<k..z>, so make a model for each with a single bone.
        std::vector<std::pair<std::string, std::string>> weapons; // Polygon file, mesh name.
        if (info.is_player && model.id != "rw"){
            for (std::string p : model.p){
                if (p.length() < 4 || p[2] != 'c' || p[3] < 'k' || p[3] > 'z') continue;
                char number[8];
                std::snprintf(number, sizeof(number), "%02d", p[3] - 'k');
                const std::string weapon_name = model.id + "_" + info.name_normal + "_weapon_" + number;
                std::ofstream hrc(output_dir_ + "temp/battle_models/" + weapon_name + ".hrc");
                hrc << ":HEADER_BLOCK 2\n:SKELETON " << weapon_name << "\n:BONES 1\n\nbone0\nroot\n0.000000\n1 "
                    << p.substr(0, 4) << "\n\n";
                hrc.close();
                weapons.push_back(std::make_pair(p, weapon_name));
            }
        }

        // Reload the resources for the newly created hrc and rsd.
        // TODO: Could this be done with res_mgr_->declareResource() ? Probably faster
        res_mgr_->removeResourceLocation(output_dir_ + "temp/battle_models/", "FFVII");
        res_mgr_->addResourceLocation(output_dir_ + "temp/battle_models/", "FileSystem", "FFVII", true, true);

        Ogre::ResourcePtr hrc = VGears::HRCFileManager::GetSingleton().load(
          model.id + "_" + info.name_normal + ".hrc", "FFVII"
        );
        Ogre::String base_name;
        VGears::StringUtil::splitBase(model.hrc, base_name);
        auto mesh_name = model.id + "_" + info.name_normal + ".mesh";
        Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(mesh_name, "FFVII"));
        Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
        for (std::string anim : model.a){
            VGears::AFileManager &afl_mgr(VGears::AFileManager::GetSingleton());
            Ogre::String a_base_name;
            VGears::StringUtil::splitBase(anim, a_base_name);
            VGears::AFilePtr a = afl_mgr.load(a_base_name + ".a", "FFVII").staticCast<VGears::AFile>();
            // Convert the FF7 name to a more readable name set in the meta data.
            VGears::StringUtil::splitBase(anim, base_name);
            a->AddTo(skeleton, VGears::NameLookup::Animation(base_name));
        }
        ExportMesh(path, mesh);
        for (const auto& weapon : weapons){
            try{
                VGears::HRCFileManager::GetSingleton().load(weapon.second + ".hrc", "FFVII");
                Ogre::MeshPtr weapon_mesh(Ogre::MeshManager::getSingleton().load(weapon.second + ".mesh", "FFVII"));
                ExportMesh(path, weapon_mesh);
            }
            catch (const Ogre::Exception& ex){
                std::cerr << "[ERROR] Ogre exception converting weapon " << weapon.second << ": " << ex.what() << std::endl;
            }
        }
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

unsigned int BattleDataInstaller::ConvertSpellModel(){
    if (next_model_to_convert_ >= spell_models_.size()) return spell_models_.size();
    SpellModel model = spell_models_[next_model_to_convert_];
    try{
        std::string path = "attacks/";
        FF7Data::SpellModelInfo info = FF7Data::GetSpellModelInfo(model.id);

        // Decompile the .hrc file (if any)
        if (model.hrc != ""){
            DecompileSpellModelHrc(
              File(output_dir_ + "temp/spell_models/" + model.hrc), model,
              output_dir_ + "temp/spell_models/" + info.name_normal + ".hrc", model.id
            );
            model.hrc = info.name_normal + ".hrc";
        }
        GenerateRsdFiles(model, output_dir_ + "temp/spell_models/");

        // Decompile the .a00 file (if any) into a .a file.
        if ("" != model.anim){ // Skip for non-animated.
            DaFile da(File(output_dir_ + "temp/spell_models/" + model.anim));
            std::vector<std::string> a_files = da.GenerateAFiles(model.id, output_dir_ + "temp/spell_models/");
            for (std::string file_name : a_files) model.a.push_back(file_name);
        }

        // Reload the resources for the newly created hrc and rsd.
        // TODO: Could this be done with res_mgr_->declareResource() ? Probably faster
        res_mgr_->removeResourceLocation(output_dir_ + "temp/spell_models/", "FFVII");
        res_mgr_->addResourceLocation(output_dir_ + "temp/spell_models/", "FileSystem", "FFVII", true, true);

        // Export meshes thet have an .hrc skeleton.
        if (model.hrc != ""){
            Ogre::ResourcePtr hrc = VGears::HRCFileManager::GetSingleton().load(model.id + ".hrc", "FFVII");
            Ogre::String base_name;
            VGears::StringUtil::splitBase(model.hrc, base_name);
            auto mesh_name = model.id + ".mesh";
            Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(mesh_name, "FFVII"));
            Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
            for (std::string anim : model.a){
                VGears::AFileManager &afl_mgr(VGears::AFileManager::GetSingleton());
                Ogre::String a_base_name;
                VGears::StringUtil::splitBase(anim, a_base_name);
                VGears::AFilePtr a = afl_mgr.load(a_base_name + ".a", "FFVII").staticCast<VGears::AFile>();
                // Convert the FF7 name to a more readable name set in the meta data.
                VGears::StringUtil::splitBase(anim, base_name);
                a->AddTo(skeleton, VGears::NameLookup::Animation(base_name));
            }
            ExportMesh(path, mesh);
        }
    }
    catch (const Ogre::Exception& ex){
        std::cerr << "[ERROR] Ogre exception converting spell model " << model.hrc <<": " << ex.what() << std::endl;
    }
    catch (const std::exception& ex){
        std::cerr << "[ERROR] Exception converting spell model " << model.hrc << ": " << ex.what() << std::endl;
    }
    next_model_to_convert_ ++;
    return next_model_to_convert_;
}

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

unsigned int BattleDataInstaller::InitializeEffectModels(){
    effect_models_.clear();
    next_effect_to_convert_ = 0;
    const std::string dir = output_dir_ + "temp/effects/";
    // Index the files by folder and name.
    std::map<std::pair<std::string, std::string>, size_t> index;
    for (size_t f = 0; f < magic_lgp_files_.size(); f ++){
        std::string name = magic_lgp_file_names_[f];
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){return std::tolower(c);});
        index[{magic_lgp_file_folders_[f], name}] = f;
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
        if (written.insert(name).second) magic_lgp_files_[f].WriteFile(dir + name);
    };
    for (size_t f = 0; f < magic_lgp_files_.size(); f ++){
        std::string rsd_name = magic_lgp_file_names_[f];
        if (rsd_name.size() < 4 || EffectSafeName(rsd_name.substr(rsd_name.size() - 4)) != "_rsd") continue;
        const std::string& folder = magic_lgp_file_folders_[f];
        // Parse the rsd.
        File& rsd_file = magic_lgp_files_[f];
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
    res_mgr_->removeResourceLocation(dir, "FFVII");
    res_mgr_->addResourceLocation(dir, "FileSystem", "FFVII", true, true);
    return effect_models_.size();
}

unsigned int BattleDataInstaller::ConvertEffectModel(){
    if (next_effect_to_convert_ >= effect_models_.size()) return effect_models_.size();
    EffectModel& model = effect_models_[next_effect_to_convert_];
    try{
        VGears::HRCFileManager::GetSingleton().load(model.name + ".hrc", "FFVII");
        Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(model.name + ".mesh", "FFVII"));
        ExportMesh("effects/", mesh);
        for (const std::string& texture : model.textures){
            const std::string path = data_dir_ + "models/battle/effects/"
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

void BattleDataInstaller::WriteEffectModels(){
    std::ofstream file(data_dir_ + "gamedata/effects.lua");
    file << "-- Meshes of the battle effects, extracted from magic.lgp. Names are the folder of the effect\n"
         << "-- and the original file, for example magic_faira_1. Meshes are in models/battle/effects.\n"
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

void BattleDataInstaller::WriteBattleCameras(){
    // The scripts of the cameras of the battles (see BattleCameraScripts.h).
    std::map<int, BattleCameraScripts::Set> sets;
    for (int i = 0; i < 3; ++ i){
        const std::string name = "data/battle/camdat" + std::to_string(i) + ".bin";
        auto path = const_cast<DiskImage&>(disk_image_).filePath(name);
        BattleCameraScripts::Set set;
        if (path != nullptr && BattleCameraScripts::ReadFile(*path, set)) sets[i] = set;
        else std::cerr << "[WARNING] Can't read " << name << "." << std::endl;
    }
    std::ofstream file(data_dir_ + "gamedata/battle_cameras.lua");
    BattleCameraScripts::WriteLua(file, sets);
    // The ones of the start of the battles are in the executable.
    BattleCameraScripts::Set intro;
    auto exe_path = const_cast<DiskImage&>(disk_image_).filePath("ff7.exe");
    if (exe_path != nullptr && BattleCameraScripts::ReadIntro(*exe_path, intro)){
        BattleCameraScripts::WriteIntroLua(file, intro);
    }
    else std::cerr << "[WARNING] Can't read the cameras of the start of the battles from ff7.exe." << std::endl;
}

void BattleDataInstaller::WriteEffectMeshes(){
    // The meshes of the effects that are in the data of ff7.exe, like the ground of Ifrit and the
    // mountain of Ramuh (see ExeEffectMeshes.h). Names are the address of the block in the executable.
    std::map<uint32_t, ExeEffectMeshes::Mesh> meshes;
    auto exe_path = const_cast<DiskImage&>(disk_image_).filePath("ff7.exe");
    if (exe_path == nullptr || !ExeEffectMeshes::Extract(*exe_path, meshes)){
        std::cerr << "[WARNING] Can't read the meshes of the effects from ff7.exe." << std::endl;
    }
    std::ofstream file(data_dir_ + "gamedata/effect_meshes.lua");
    ExeEffectMeshes::WriteLua(file, meshes);
}

void BattleDataInstaller::WriteEffectCameras(){
    // The scripts of the cameras are in the data of ff7.exe, and are found from the code that
    // uses them. Names are the address of the script in memory.
    std::map<uint32_t, std::vector<int16_t>> scripts;
    auto exe_path = const_cast<DiskImage&>(disk_image_).filePath("ff7.exe");
    if (exe_path == nullptr || !ExeEffectCameras::Extract(*exe_path, scripts)){
        std::cerr << "[WARNING] Can't read the camera scripts of the effects from ff7.exe." << std::endl;
    }
    std::ofstream file(data_dir_ + "gamedata/effect_cameras.lua");
    file << "-- Camera scripts of the battle effects, read from ff7.exe. Game.EffectCameras[name] is a list\n"
         << "-- of 16 bit words, with the address of the script in the executable as the name (like\n"
         << "-- 0x8ae268). See scripts/battle/effect_camera.lua for the format.\n"
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

void BattleDataInstaller::WriteEffectSprites(){
    // The sprite files (.s) are lists of frames, and every frame a list of quads of a sprite sheet (see
    // the layout below). They are written twice: all the quads one after the other (the first format,
    // used by the Cure effect) and the frames with their quads (effect_animations.lua).
    // Textures are converted too, with the name of the effect folder as a prefix (common_ for the
    // archive root), to images/battle_fx, so they can be used as images of sprites.
    const std::string key_prefix = "ff7/data/battle/";
    auto key_of = [&](std::string folder){
        if (folder.empty()) return std::string("common");
        if (folder.compare(0, key_prefix.size(), key_prefix) == 0) folder = folder.substr(key_prefix.size());
        return EffectSafeName(folder);
    };
    std::ofstream file(data_dir_ + "gamedata/effect_sprites.lua");
    file << "-- Frames of the sprite files of magic.lgp. Game.EffectSprites[name] is a list of frames with\n"
         << "-- the position (u, v) and size (w, h) in the texture, in pixels, the offset to draw it\n"
         << "-- (ox, oy) and the flags. The names are the folder and the file, like common_iyashi.\n"
         << "-- Textures are in images/battle_fx, like common_iyashi00.png.\n"
         << "Game.EffectSprites = Game.EffectSprites or {}\n";
    // (The list has the quads of all the frames in order, with the position of the corner as ox, oy.)
    std::ofstream animations(data_dir_ + "gamedata/effect_animations.lua");
    animations << "-- Frames of the sprite files (.SPT) of magic.lgp. Game.EffectAnimations[name] is a list of frames and\n"
               << "-- every frame a list of quads: {x, y, u, v, w, h}, with the position of the corner and the size of the\n"
               << "-- quad in units of 8 of the effect, and the place in the texture in pixels. The names are the folder\n"
               << "-- and the file, like common_houden_2.\n"
               << "Game.EffectAnimations = Game.EffectAnimations or {}\n";
    const std::string dir = output_dir_ + "temp/effects/";
    boost::filesystem::create_directories(data_dir_ + "images/battle_fx/");
    for (size_t f = 0; f < magic_lgp_files_.size(); f ++){
        const std::string& name = magic_lgp_file_names_[f];
        if (name.size() < 3) continue;
        std::string ext = name.substr(name.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){return std::tolower(c);});
        const std::string key = key_of(magic_lgp_file_folders_[f]);
        if (ext == "tex"){
            const std::string stem = EffectSafeName(EffectStem(name));
            const std::string path = data_dir_ + "images/battle_fx/" + key + "_" + stem + ".png";
            if (boost::filesystem::exists(path)) continue;
            try{
                magic_lgp_files_[f].WriteFile(dir + key + "_" + stem + ".tex");
                TexFile tex(dir + key + "_" + stem + ".tex");
                tex.SavePng(path);
            }
            catch (const std::exception& ex){
                // Many of the original textures are dummies.
            }
        }
        else if (ext == "s"){
            File& sprite_file = magic_lgp_files_[f];
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

void BattleDataInstaller::WriteEnemies(){
    for (Enemy enemy : enemies_){
        TiXmlDocument xml;
        std::unique_ptr<TiXmlElement> container(new TiXmlElement("enemy"));
        container->SetAttribute("id", enemy.id);
        container->SetAttribute("model", enemy.model);
        container->SetAttribute("name", enemy.name);
        container->SetAttribute("level", enemy.level);
        container->SetAttribute("exp", enemy.exp);
        container->SetAttribute("ap", enemy.ap);
        container->SetAttribute("money", enemy.money);
        container->SetAttribute("morph", enemy.morph);
        container->SetDoubleAttribute("back_damage", enemy.back_damage);
        std::unique_ptr<TiXmlElement> stats(new TiXmlElement("stats"));
        std::unique_ptr<TiXmlElement> stat_str(new TiXmlElement("stat"));
        stat_str->SetAttribute("id", "str");
        stat_str->SetAttribute("value", enemy.str);
        stats->LinkEndChild(stat_str.release());
        std::unique_ptr<TiXmlElement> stat_mag(new TiXmlElement("stat"));
        stat_mag->SetAttribute("id", "mag");
        stat_mag->SetAttribute("value", enemy.mag);
        stats->LinkEndChild(stat_mag.release());
        std::unique_ptr<TiXmlElement> stat_def(new TiXmlElement("stat"));
        stat_def->SetAttribute("id", "def");
        stat_def->SetAttribute("value", enemy.def);
        stats->LinkEndChild(stat_def.release());
        std::unique_ptr<TiXmlElement> stat_mdef(new TiXmlElement("stat"));
        stat_mdef->SetAttribute("id", "mdef");
        stat_mdef->SetAttribute("value", enemy.mdef);
        stats->LinkEndChild(stat_mdef.release());
        std::unique_ptr<TiXmlElement> stat_spd(new TiXmlElement("stat"));
        stat_spd->SetAttribute("id", "spd");
        stat_spd->SetAttribute("value", enemy.spd);
        stats->LinkEndChild(stat_spd.release());
        std::unique_ptr<TiXmlElement> stat_lck(new TiXmlElement("stat"));
        stat_lck->SetAttribute("id", "lck");
        stat_lck->SetAttribute("value", enemy.lck);
        stats->LinkEndChild(stat_lck.release());
        std::unique_ptr<TiXmlElement> stat_eva(new TiXmlElement("stat"));
        stat_eva->SetAttribute("id", "eva");
        stat_eva->SetAttribute("value", enemy.eva);
        stats->LinkEndChild(stat_eva.release());
        std::unique_ptr<TiXmlElement> stat_hp(new TiXmlElement("stat"));
        stat_hp->SetAttribute("id", "hp");
        stat_hp->SetAttribute("value", enemy.hp);
        stats->LinkEndChild(stat_hp.release());
        std::unique_ptr<TiXmlElement> stat_mp(new TiXmlElement("stat"));
        stat_mp->SetAttribute("id", "mp");
        stat_mp->SetAttribute("value", enemy.mp);
        stats->LinkEndChild(stat_mp.release());
        container->LinkEndChild(stats.release());
        std::unique_ptr<TiXmlElement> elements(new TiXmlElement("elements"));
        for (Enemy::Element element : enemy.elements){
            std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("element"));
            xml_element->SetAttribute("id", element.id);
            xml_element->SetDoubleAttribute("factor", element.factor);
            elements->LinkEndChild(xml_element.release());
        }
        container->LinkEndChild(elements.release());
        std::unique_ptr<TiXmlElement> immunities(new TiXmlElement("immunities"));
        for (Enemy::Immunity immunity : enemy.immunities){
            std::unique_ptr<TiXmlElement> xml_immunity(new TiXmlElement("immunity"));
            xml_immunity->SetAttribute("status", immunity.status);
            xml_immunity->SetDoubleAttribute("rate", immunity.rate);
            immunities->LinkEndChild(xml_immunity.release());
        }
        container->LinkEndChild(immunities.release());
        std::unique_ptr<TiXmlElement> attacks(new TiXmlElement("attacks"));
        for (Enemy::Attack attack : enemy.attacks){
            std::unique_ptr<TiXmlElement> xml_attack(new TiXmlElement("attack"));
            xml_attack->SetAttribute("id", attack.id);
            xml_attack->SetAttribute("animation", attack.animation);
            xml_attack->SetAttribute("camera", attack.camera);
            attacks->LinkEndChild(xml_attack.release());
        }
        container->LinkEndChild(attacks.release());
        std::unique_ptr<TiXmlElement> manip_attacks(new TiXmlElement("manipulate"));
        manip_attacks->SetAttribute("manipulable", enemy.manipulate_attacks.size() > 0 ? 1 : 0);
        for (u16 attack : enemy.manipulate_attacks){
            std::unique_ptr<TiXmlElement> xml_manip_attack(new TiXmlElement("attack"));
            xml_manip_attack->SetAttribute("id", attack);
            manip_attacks->LinkEndChild(xml_manip_attack.release());
        }
        container->LinkEndChild(manip_attacks.release());
        std::unique_ptr<TiXmlElement> steals(new TiXmlElement("steal"));
        for (Enemy::Item item : enemy.steal){
            std::unique_ptr<TiXmlElement> xml_steal(new TiXmlElement("item"));
            xml_steal->SetAttribute("id", item.id);
            xml_steal->SetDoubleAttribute("rate", item.rate);
            steals->LinkEndChild(xml_steal.release());
        }
        container->LinkEndChild(steals.release());
        std::unique_ptr<TiXmlElement> drops(new TiXmlElement("drop"));
        for (Enemy::Item item : enemy.drop){
            std::unique_ptr<TiXmlElement> xml_drop(new TiXmlElement("item"));
            xml_drop->SetAttribute("id", item.id);
            xml_drop->SetDoubleAttribute("rate", item.rate);
            drops->LinkEndChild(xml_drop.release());
        }
        container->LinkEndChild(drops.release());
        std::unique_ptr<TiXmlElement> animations(new TiXmlElement("animations"));
        for (unsigned int animation : enemy.animations){
            std::unique_ptr<TiXmlElement> xml_animation(new TiXmlElement("animation"));
            xml_animation->SetAttribute("id", animation);
            animations->LinkEndChild(xml_animation.release());
        }
        container->LinkEndChild(animations.release());
        // AI scripts, as hexadecimal bytecode.
        {
            static const char* AI_SLOT_NAMES[16] = {
              "init", "main", "counter_general", "counter_death", "counter_physical", "counter_magic",
              "battle_end", "pre_turn", "event_1", "event_2", "event_3", "event_4", "event_5",
              "event_6", "event_7", "event_8"
            };
            std::unique_ptr<TiXmlElement> ai(new TiXmlElement("ai"));
            for (size_t slot = 0; slot < enemy.ai_scripts.size() && slot < 16; slot ++){
                if (enemy.ai_scripts[slot].empty()) continue;
                std::string hex;
                char digits[3];
                for (u8 byte : enemy.ai_scripts[slot]){
                    snprintf(digits, sizeof(digits), "%02x", byte);
                    hex += digits;
                }
                std::unique_ptr<TiXmlElement> xml_script(new TiXmlElement("script"));
                xml_script->SetAttribute("slot", static_cast<int>(slot));
                xml_script->SetAttribute("name", AI_SLOT_NAMES[slot]);
                xml_script->SetAttribute("data", hex);
                ai->LinkEndChild(xml_script.release());
            }
            container->LinkEndChild(ai.release());
        }
        xml.LinkEndChild(container.release());
        std::string file_name = BuildEnemyFileName(enemy);
        xml.SaveFile(data_dir_ + file_name);
        enemy_map_[enemy.id] = file_name;

    }
}

void BattleDataInstaller::WriteAttacks(){
    for (Attack attack : attacks_){
        TiXmlDocument xml;
        std::unique_ptr<TiXmlElement> container(new TiXmlElement("Attack"));
        container->SetAttribute("id", attack.id);
        container->SetAttribute("name", attack.name);
        container->SetAttribute("mp", attack.mp);
        container->SetAttribute("reflectable", attack.reflectable);
        std::unique_ptr<TiXmlElement> damage(new TiXmlElement("Damage"));
        std::unique_ptr<TiXmlElement> damage_power(new TiXmlElement("Parameter"));
        damage_power->SetAttribute("id", "power");
        damage_power->SetAttribute("value", attack.damage.power);
        damage->LinkEndChild(damage_power.release());
        std::unique_ptr<TiXmlElement> damage_formula(new TiXmlElement("Parameter"));
        damage_formula->SetAttribute("id", "formula");
        damage_formula->SetAttribute("value", attack.damage.formula);
        damage->LinkEndChild(damage_formula.release());
        std::unique_ptr<TiXmlElement> damage_modifier(new TiXmlElement("Parameter"));
        damage_modifier->SetAttribute("id", "modifier");
        damage_modifier->SetAttribute("value", attack.damage.modifier);
        damage->LinkEndChild(damage_modifier.release());
        std::unique_ptr<TiXmlElement> damage_acc(new TiXmlElement("Parameter"));
        damage_acc->SetAttribute("id", "accuracy");
        // Accuracy is a fraction between 0 and 1, it can't be written as an integer.
        damage_acc->SetDoubleAttribute("value", attack.damage.accuracy);
        damage->LinkEndChild(damage_acc.release());
        std::unique_ptr<TiXmlElement> damage_darkness(new TiXmlElement("Parameter"));
        damage_darkness->SetAttribute("id", "affected_by_darkness");
        damage_darkness->SetAttribute("value", attack.damage.affected_by_darkness);
        damage->LinkEndChild(damage_darkness.release());
        std::unique_ptr<TiXmlElement> damage_restore(new TiXmlElement("Parameter"));
        damage_restore->SetAttribute("id", "restore");
        damage_restore->SetAttribute("value", static_cast<int>(attack.damage.restore));
        damage->LinkEndChild(damage_restore.release());
        std::unique_ptr<TiXmlElement> damage_mp(new TiXmlElement("Parameter"));
        damage_mp->SetAttribute("id", "damage_mp");
        damage_mp->SetAttribute("value", attack.damage.damage_mp);
        damage->LinkEndChild(damage_mp.release());
        std::unique_ptr<TiXmlElement> damage_drain_hp(new TiXmlElement("Parameter"));
        damage_drain_hp->SetAttribute("id", "drain_hp");
        damage_drain_hp->SetAttribute("value", attack.damage.drain_hp);
        damage->LinkEndChild(damage_drain_hp.release());
        std::unique_ptr<TiXmlElement> damage_drain_mp(new TiXmlElement("Parameter"));
        damage_drain_mp->SetAttribute("id", "drain_mp");
        damage_drain_mp->SetAttribute("value", attack.damage.drain_mp);
        damage->LinkEndChild(damage_drain_mp.release());
        std::unique_ptr<TiXmlElement> damage_ignore(new TiXmlElement("Parameter"));
        damage_ignore->SetAttribute("id", "ignore_status_defense");
        damage_ignore->SetAttribute("value", attack.damage.ignore_status_defense);
        damage->LinkEndChild(damage_ignore.release());
        std::unique_ptr<TiXmlElement> damage_critical(new TiXmlElement("Parameter"));
        damage_critical->SetAttribute("id", "always_critical");
        damage_critical->SetAttribute("value", attack.damage.critical);
        damage->LinkEndChild(damage_critical.release());
        container->LinkEndChild(damage.release());
        std::unique_ptr<TiXmlElement> target_opts(new TiXmlElement("Target"));
        std::unique_ptr<TiXmlElement> target_se(new TiXmlElement("Parameter"));
        target_se->SetAttribute("id", "selection_enabled");
        target_se->SetAttribute("value", attack.target.selection_enabled);
        target_opts->LinkEndChild(target_se.release());
        std::unique_ptr<TiXmlElement> target_de(new TiXmlElement("Parameter"));
        target_de->SetAttribute("id", "default_enemy");
        target_de->SetAttribute("value", attack.target.default_enemy);
        target_opts->LinkEndChild(target_de.release());
        std::unique_ptr<TiXmlElement> target_dm(new TiXmlElement("Parameter"));
        target_dm->SetAttribute("id", "default_multiple");
        target_dm->SetAttribute("value", attack.target.default_multiple);
        target_opts->LinkEndChild(target_dm.release());
        std::unique_ptr<TiXmlElement> target_tm(new TiXmlElement("Parameter"));
        target_tm->SetAttribute("id", "toggle_multiple");
        target_tm->SetAttribute("value", attack.target.toggle_multiple);
        target_opts->LinkEndChild(target_tm.release());
        std::unique_ptr<TiXmlElement> target_fr(new TiXmlElement("Parameter"));
        target_fr->SetAttribute("id", "fixed_row");
        target_fr->SetAttribute("value", attack.target.fixed_row);
        target_opts->LinkEndChild(target_fr.release());
        std::unique_ptr<TiXmlElement> target_sr(new TiXmlElement("Parameter"));
        target_sr->SetAttribute("id", "short_range");
        target_sr->SetAttribute("value", attack.target.short_range);
        target_opts->LinkEndChild(target_sr.release());
        std::unique_ptr<TiXmlElement> target_ar(new TiXmlElement("Parameter"));
        target_ar->SetAttribute("id", "all_rows");
        target_ar->SetAttribute("value", attack.target.all_rows);
        target_opts->LinkEndChild(target_ar.release());
        std::unique_ptr<TiXmlElement> target_rd(new TiXmlElement("Parameter"));
        target_rd->SetAttribute("id", "random");
        target_rd->SetAttribute("value", attack.target.random);
        target_opts->LinkEndChild(target_rd.release());
        std::unique_ptr<TiXmlElement> target_od(new TiXmlElement("Parameter"));
        target_od->SetAttribute("id", "only_for_dead");
        target_od->SetAttribute("value", attack.target.only_for_dead);
        target_opts->LinkEndChild(target_od.release());
        std::unique_ptr<TiXmlElement> target_nr(new TiXmlElement("Parameter"));
        target_nr->SetAttribute("id", "no_retarget");
        target_nr->SetAttribute("value", attack.target.no_re_target);
        target_opts->LinkEndChild(target_nr.release());
        container->LinkEndChild(target_opts.release());
        std::unique_ptr<TiXmlElement> effects(new TiXmlElement("Effects"));
        for (Attack::StatusEffect effect : attack.status_effects){
            std::unique_ptr<TiXmlElement> xml_effect(new TiXmlElement("Effect"));
            xml_effect->SetAttribute("id", effect.id);
            xml_effect->SetDoubleAttribute("chance", effect.chance);
            xml_effect->SetAttribute("mode", GameData::STATUS_EFFECT_MODE::to_string(effect.type));
            effects->LinkEndChild(xml_effect.release());
        }
        container->LinkEndChild(effects.release());
        std::unique_ptr<TiXmlElement> elements(new TiXmlElement("Elements"));
        for (unsigned int element : attack.elements){
            std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("Element"));
            xml_element->SetAttribute("id", element);
            elements->LinkEndChild(xml_element.release());
        }
        container->LinkEndChild(elements.release());
        std::unique_ptr<TiXmlElement> anim(new TiXmlElement("Animation"));
        std::unique_ptr<TiXmlElement> anim_e(new TiXmlElement("Parameter"));
        anim_e->SetAttribute("id", "effect");
        anim_e->SetAttribute("value", attack.animation.effect);
        anim->LinkEndChild(anim_e.release());
        std::unique_ptr<TiXmlElement> anim_ie(new TiXmlElement("Parameter"));
        anim_ie->SetAttribute("id", "impact_effect");
        anim_ie->SetAttribute("value", attack.animation.impact_effect);
        anim->LinkEndChild(anim_ie.release());
        std::unique_ptr<TiXmlElement> anim_s(new TiXmlElement("Parameter"));
        anim_s->SetAttribute("id", "sound");
        anim_s->SetAttribute("value", attack.animation.sound);
        anim->LinkEndChild(anim_s.release());
        std::unique_ptr<TiXmlElement> anim_ha(new TiXmlElement("Parameter"));
        anim_ha->SetAttribute("id", "hurt_anim");
        anim_ha->SetAttribute("value", attack.animation.hurt_anim);
        anim->LinkEndChild(anim_ha.release());
        std::unique_ptr<TiXmlElement> anim_c(new TiXmlElement("Parameter"));
        anim_c->SetAttribute("id", "camera_single");
        anim_c->SetAttribute("value", attack.animation.camera_single);
        anim->LinkEndChild(anim_c.release());
        std::unique_ptr<TiXmlElement> anim_cm(new TiXmlElement("Parameter"));
        anim_cm->SetAttribute("id", "camera_multiple");
        anim_cm->SetAttribute("value", attack.animation.camera_multiple);
        anim->LinkEndChild(anim_cm.release());
        container->LinkEndChild(anim.release());
        xml.LinkEndChild(container.release());
        std::string file_name = BuildAttackFileName(attack);
        xml.SaveFile(data_dir_ + file_name);
        attack_map_[attack.id] = file_name;
    }
    // Index of the attack files by ID, as file names include the attack name.
    TiXmlDocument index;
    std::unique_ptr<TiXmlElement> attacks(new TiXmlElement("attacks"));
    for (const auto& entry : attack_map_){
        std::unique_ptr<TiXmlElement> xml_attack(new TiXmlElement("attack"));
        xml_attack->SetAttribute("id", entry.first);
        // The file name in the map has the gamedata/attack/ path.
        xml_attack->SetAttribute("file", entry.second.substr(entry.second.find_last_of('/') + 1));
        attacks->LinkEndChild(xml_attack.release());
    }
    index.LinkEndChild(attacks.release());
    index.SaveFile(data_dir_ + "gamedata/attack/_index.xml");
}

void BattleDataInstaller::WriteFormations(){
    for (Formation formation : formations_){
        TiXmlDocument xml;
        std::unique_ptr<TiXmlElement> container(new TiXmlElement("formation"));
        container->SetAttribute("id", formation.id);
        container->SetAttribute("next", formation.next_formation);
        container->SetDoubleAttribute("escape_difficulty", formation.escape_counter);
        container->SetAttribute("escapable", formation.escapable);
        container->SetAttribute("skip_victory_pose", formation.skip_victory_pose);
        container->SetAttribute("skip_spoils", formation.skip_spoils);
        container->SetAttribute("skip_victory_pose", formation.skip_victory_pose);
        container->SetAttribute("preemptive_disabled", formation.preemptive_disabled);
        container->SetAttribute("layout", static_cast<int>(formation.layout));
        std::unique_ptr<TiXmlElement> location(new TiXmlElement("location"));
        location->SetAttribute("id", formation.location);
        location->SetAttribute("name", formation.location_name);
        container->LinkEndChild(location.release());
        std::unique_ptr<TiXmlElement> enemies(new TiXmlElement("enemies"));
        for (Formation::Enemy enemy : formation.enemies){
            std::unique_ptr<TiXmlElement> enemy_xml(new TiXmlElement("enemy"));
            enemy_xml->SetAttribute("id", enemy.id);
            enemy_xml->SetAttribute("x", enemy.x);
            enemy_xml->SetAttribute("y", enemy.y);
            enemy_xml->SetAttribute("z", enemy.z);
            enemy_xml->SetAttribute("row", enemy.row);
            std::string cover = "";
            for (int c = 0; c < 5; c ++) cover += enemy.cover[c];
            enemy_xml->SetAttribute("cover", cover);
            enemy_xml->SetAttribute("visible", enemy.visible);
            enemy_xml->SetAttribute("direction", enemy.direction);
            enemy_xml->SetAttribute("targeteable", enemy.targeteable);
            enemy_xml->SetAttribute("main_script_active", enemy.main_script_active);
            enemies->LinkEndChild(enemy_xml.release());
        }
        container->LinkEndChild(enemies.release());
        std::unique_ptr<TiXmlElement> cameras(new TiXmlElement("camera"));
        cameras->SetAttribute("initial", formation.initial_camera_position);
        cameras->SetAttribute("intro", formation.intro_camera);
        int c = 0;
        for (Formation::Camera camera : formation.camera_positions){
            std::unique_ptr<TiXmlElement> camera_xml(new TiXmlElement("position"));
            camera_xml->SetAttribute("id", c);
            camera_xml->SetDoubleAttribute("x", camera.x);
            camera_xml->SetDoubleAttribute("y", camera.y);
            camera_xml->SetDoubleAttribute("z", camera.z);
            camera_xml->SetDoubleAttribute("direction_x", camera.d_x);
            camera_xml->SetDoubleAttribute("direction_y", camera.d_y);
            camera_xml->SetDoubleAttribute("direction_z", camera.d_z);
            cameras->LinkEndChild(camera_xml.release());
            c ++;
        }
        container->LinkEndChild(cameras.release());
        std::unique_ptr<TiXmlElement> arena(new TiXmlElement("arena"));
        arena->SetAttribute("is_arena", formation.is_arena_battle);
        for (int next : formation.next_arena_formation_candidates){
            std::unique_ptr<TiXmlElement> next_xml(new TiXmlElement("next_candidate"));
            next_xml->SetAttribute("id", next);
            arena->LinkEndChild(next_xml.release());
        }
        container->LinkEndChild(arena.release());
        xml.LinkEndChild(container.release());
        std::string file_name = std::to_string(formation.id);
        while (file_name.length() < 4) file_name = "0" + file_name;
        file_name = "gamedata/formation/" + file_name + ".xml";
        xml.SaveFile(data_dir_ + file_name);
        formation_map_[formation.id] = file_name;
    }
}

void BattleDataInstaller::WriteCharacterData(){
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
    xml.SaveFile(data_dir_ + "models/battle/characters.xml");
}

void BattleDataInstaller::WriteSceneData(){
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
    xml.SaveFile(data_dir_ + "models/battle/scenes.xml");
}

std::string BattleDataInstaller::BuildEnemyFileName(Enemy enemy){
    std::string file_name = "gamedata/enemy/";
    std::string id = std::to_string(enemy.id);
    while (id.size() < 4) id = "0" + id;
    file_name += id + ".xml";
    return file_name;
}

std::string BattleDataInstaller::BuildAttackFileName(Attack attack){
    std::string file_name = "gamedata/attack/";
    std::string id = std::to_string(attack.id);
    while (id.size() < 4) id = "0" + id;
    file_name += (id + "_");
    for (int n = 0; n < attack.name.size(); n ++){
        if (
          (attack.name[n] >= '0' && attack.name[n] <= '9')
          || (attack.name[n] >= 'a' && attack.name[n] <= 'z')
          || (attack.name[n] >= 'A' && attack.name[n] <= 'Z')
        ) file_name += attack.name[n];
    }
    file_name += ".xml";
    return file_name;
}

std::unique_ptr<File> BattleDataInstaller::ExtractGZipScene(const File& file){
    std::vector<u8> compressed(file.GetFileSize());
    if (!compressed.empty()) file.GetFileBuffer(compressed.data(), 0, compressed.size());
    std::vector<u8> out;
    std::string error;
    if (compressed.empty() || !BinGZipFile::Inflate(compressed.data(), compressed.size(), out, error) || out.empty()){
        std::cout << "Warning: " << (error.empty() ? "empty data" : error) << " in file " << file.GetFileName() << std::endl;
        return nullptr;
    }
    return std::make_unique<File>(out.data(), 0, static_cast<u32>(out.size()));
}

void BattleDataInstaller::ExportMesh(const std::string outdir, const Ogre::MeshPtr &mesh){
    VGears::String base_mesh_name;
    VGears::StringUtil::splitFull(mesh->getName(), base_mesh_name);
    MakeMaterialsUnique(mesh, outdir + base_mesh_name);
    // A model without geometry (enemy 15, Eye, has only a skeleton and animations) has no bounds, and Ogre
    // doesn't export a mesh without them. It is exported as an empty mesh at the origin.
    // Ogre also wants a radius that is not 0, so the bounds are a tiny box.
    if (mesh->getBounds().isNull() || mesh->getBoundingSphereRadius() <= 0){
        mesh->_setBounds(Ogre::AxisAlignedBox(Ogre::Vector3(-0.01f), Ogre::Vector3(0.01f)), false);
        mesh->_setBoundingSphereRadius(0.02f);
    }
    Ogre::MeshSerializer mesh_serializer;
    Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
    Ogre::SkeletonSerializer skeleton_serializer;
    skeleton_serializer.exportSkeleton(
      skeleton.get(), data_dir_ + "models/battle/" + outdir + base_mesh_name + ".skeleton"
    );
    mesh->setSkeletonName(outdir + base_mesh_name + ".skeleton");
    mesh_serializer.exportMesh(mesh.get(), data_dir_ + "models/battle/" + outdir + mesh->getName());
    Ogre::MaterialSerializer mat_ser;
    bool queued = false;
    std::set<std::string> textures;
    Ogre::Mesh::SubMeshList sub_meshes = mesh->getSubMeshes();
    for (Ogre::SubMesh* sub_mesh : sub_meshes){
        Ogre::MaterialPtr mat(Ogre::MaterialManager::getSingleton().getByName(sub_mesh->getMaterialName()));
        if (mat == nullptr) continue;
        for (size_t techs = 0; techs < mat->getNumTechniques(); techs ++){
            Ogre::Technique* tech = mat->getTechnique(techs);
            if (tech == nullptr) continue;
            for (size_t pass_num = 0; pass_num < tech->getNumPasses(); pass_num ++){
                Ogre::Pass* pass = tech->getPass(pass_num);
                if (pass == nullptr) continue;
                for (
                  size_t texture_unit_num = 0; texture_unit_num < pass->getNumTextureUnitStates(); texture_unit_num ++
                ){
                    Ogre::TextureUnitState* unit = pass->getTextureUnitState(texture_unit_num);
                    if (unit && unit->getTextureName().empty() == false){
                        // Ensure the output material script references png files, no tex files.
                        Ogre::String base_name;
                        VGears::StringUtil::splitBase(unit->getTextureName(), base_name);
                        unit->setTextureName(outdir + base_name + ".png");
                        textures.insert(unit->getTextureName());
                    }
                }
            }
        }
        if (std::count(battle_materials_.begin(), battle_materials_.end(), sub_mesh->getMaterialName()) == 0){
            mat_ser.queueForExport(mat);
            queued = true;
            battle_materials_.push_back(sub_mesh->getMaterialName());
        }
    }
    // Ogre throws if there is nothing to export (a mesh without geometry, or whose materials are already exported).
    if (queued)
        mat_ser.exportQueued(data_dir_ + "models/battle/" + outdir + base_mesh_name + VGears::EXT_MATERIAL);
}

void BattleDataInstaller::GenerateRsdFiles(BattleModel model, std::string path){
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

void BattleDataInstaller::GenerateRsdFiles(SpellModel model, std::string path){
    //std::sort(model.tex.begin(), model.tex.end());
    // For each p file, generate a rsd file.
    for (std::string p : model.p){
        std::string base_name = p.substr(0, p.length() - 2);
        std::string content = "@RSD940102\n";
        content += "PLY=" + base_name + ".PLY\n";
        content += "MAT=" + base_name + ".MAT\n";
        content += "GRP=" + base_name + ".GRP\n";
        content += "NTEX=0\n";
        //for (int t = 0; t < model.tex.size(); t ++)
        //    content += "TEX[" + std::to_string(t) + "]=" + model.tex[t] + "\n";
        content += "\n";
        std::ofstream out(path + base_name + ".rsd");
        out << content;
        out.close();
    }
}

void BattleDataInstaller::WriteModelCamera(
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
    std::ofstream file(data_dir_ + "scripts/battle_models/" + model_name + "_camera.lua");
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

void BattleDataInstaller::DecompileBattleModelHrc(
  File compiled, BattleModel model, std::string path, std::string skeleton_name
){
    compiled.SetOffset(0);
    compiled.readU32LE();
    compiled.readU32LE();
    compiled.readU32LE();
    std::sort(model.p.begin(), model.p.end());
    std::string decompiled(":HEADER_BLOCK 2\n:SKELETON " + skeleton_name + "\n");
    int bones = compiled.readU32LE();
    compiled.SetOffset(compiled.GetFileSize() - (bones * 12));
    decompiled += ":BONES " + std::to_string(bones) + "\n\n";
    int p_file_index = 0;
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
            if (p_file_index < model.p.size()) decompiled += model.p[p_file_index].substr(0, 4);
            p_file_index ++;
            decompiled += "\n\n";
        }
    }
    std::ofstream out(path);
    out << decompiled;
    out.close();
}

void BattleDataInstaller::DecompileSpellModelHrc(
  File compiled, SpellModel model, std::string path, std::string skeleton_name
){
    compiled.SetOffset(0);
    compiled.readU32LE();
    compiled.readU32LE();
    compiled.readU32LE();
    std::sort(model.p.begin(), model.p.end());
    std::string decompiled(":HEADER_BLOCK 2\n:SKELETON " + skeleton_name + "\n");
    int bones = compiled.readU32LE();
    compiled.SetOffset(compiled.GetFileSize() - (bones * 12));
    decompiled += ":BONES " + std::to_string(bones) + "\n\n";
    int p_index = 0;
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
            if (p_index < model.p.size()) decompiled += model.p[p_index].substr(0, model.p[p_index].length() - 2);
            p_index ++;
            decompiled += "\n\n";
        }
    }
    std::ofstream out(path);
    out << decompiled;
    out.close();
}

void BattleDataInstaller::DecompileSceneModelHrc(
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
