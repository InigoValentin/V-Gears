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
#include <zlib.h>
#include <tinyxml.h>
#include <boost/filesystem.hpp>
#include <OgreMesh.h>
#include "BattleDataInstaller.h"
#include "TexFile.h"
#include "data/VGearsHRCFileManager.h"
#include "data/VGearsAFileManager.h"
#include "data/AbFile.h"
#include "data/DaFile.h"
#include "data/FF7Data.h"
#include "common/VGearsStringUtil.h"
#include "common/FinalFantasy7/FF7NameLookup.h"

BattleDataInstaller::BattleDataInstaller(
  const std::string input_dir, const std::string output_dir, Ogre::ResourceGroupManager* res_mgr
):
  input_dir_(input_dir), output_dir_(output_dir),
  scene_bin_(input_dir + "data/battle/scene.bin"), total_scenes_(0), next_scene_(0),
  res_mgr_(res_mgr)
{}

BattleDataInstaller::~BattleDataInstaller(){}

unsigned int BattleDataInstaller::InitializeScenes(){
    next_scene_ = 0;
    scenes_.clear();
    enemies_.clear();
    attacks_.clear();
    // Get all scenes from scene.bin.
    for (int i = 0; i < 32; i ++){
        File scene_bin_block(&scene_bin_, i * 0x2000, 0x2000);
        for (int o = 0; o < 10; o ++){
            scene_bin_block.SetOffset(4 * o);
            u32 start_offset = scene_bin_block.readU32LE() * 4;
            u32 end_offset = scene_bin_block.readU32LE() * 4;
            if (end_offset > start_offset + 0x2000) end_offset = start_offset + 0x2000;
            if (start_offset >= 0 && start_offset < 0x2000){
                File scene_bin_fragment(
                  &scene_bin_block, start_offset, end_offset - start_offset - 1
                );
                File* extracted = ExtractGZipScene(scene_bin_fragment);
                if (extracted != NULL){
                    scenes_uncompressed_.push_back(extracted);
                    total_scenes_ ++;
                }
            }
            else break;
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
    File battle_lgp_file(input_dir_ + "data/battle/battle.lgp");
    // Also, open it as a LGP archive.
    VGears::LGPArchive battle_lgp(input_dir_ + "data/battle/battle.lgp", "LGP");
    battle_lgp.open(input_dir_ + "data/battle/battle.lgp", true);
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
    // Open battle.lgp
    File magic_lgp_file(input_dir_ + "data/battle/magic.lgp");
    // Also, open it as a LGP archive.
    VGears::LGPArchive magic_lgp(input_dir_ + "data/battle/magic.lgp", "LGP");
    magic_lgp.open(input_dir_ + "data/battle/magic.lgp", true);
    magic_lgp.load();
    VGears::LGPArchive::FileList file_list = magic_lgp.GetFiles();
    for (int i = 0; i < file_list.size(); i ++){
        VGears::LGPArchive::FileEntry f = file_list.at(i);
        if (f.data_offset + f.data_size <= magic_lgp_file.GetFileSize()){
            File m_lgp_file(&magic_lgp_file, f.data_offset, f.data_size);
            magic_lgp_files_.push_back(m_lgp_file);
            magic_lgp_file_names_.push_back(f.file_name);
        }
    }
    return magic_lgp_files_.size();
}

unsigned int BattleDataInstaller::ProcessScene(){
    if (next_scene_ >= scenes_uncompressed_.size()) return total_scenes_;
    BattleSceneFile scene(next_scene_, scenes_uncompressed_[next_scene_]);
    scenes_.push_back(scene);
    // Add new enemies to the list.
    std::vector<Enemy> enemies = scene.GetEnemies();
    for (int e = 0; e < enemies.size(); e ++){
        bool already_added = false;
        for (int l = 0; l < enemies_.size(); l ++){
            if (enemies_[l].id == enemies[e].id){
                already_added = true;
                break;
            }
        }
        if (already_added) continue;
        enemies_.push_back(enemies[e]);
    }
    // Add new attacks to the list.
    std::vector<Attack> attacks = scene.GetAttacks();
    for (int a = 0; a < attacks.size(); a ++){
        bool already_added = false;
        for (int l = 0; l < attacks_.size(); l ++){
            if (attacks_[l].id == attacks[a].id){
                already_added = true;
                break;
            }
        }
        if (already_added) continue;
        attacks_.push_back(attacks[a]);
    }
    // Add new formations to the list.
    std::vector<Formation> formations = scene.GetFormations();
    for (Formation f : formations){
        f.id = formations_.size(); // Assign the next ID.
        formations_.push_back(f);
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
        bool found = false;
        for (int m = 0; m < battle_models_.size(); m ++){
            if (battle_models_[m].id == id){
                battle_models_[m].script
                  = battle_lgp_file_names_[next_model_to_process_] + ".script";
                found = true;
                break;
            }
        }
        if (found == false){
            BattleModel model;
            model.id = id;
            model.script = battle_lgp_file_names_[next_model_to_process_] + ".script";
            battle_models_.push_back(model);
        }
    }
    else if (type == "da"){
        // .da Animation file.
        battle_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/battle_models/"
          + battle_lgp_file_names_[next_model_to_process_] + ".anim"
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
          output_dir_ + "temp/battle_models/"
          + battle_lgp_file_names_[next_model_to_process_] + ".tex"
        );
        TexFile tex(battle_lgp_files_[next_model_to_process_]);
        std::string path = output_dir_ + "models/battle/";
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
            scene_data_.push_back(scene_data);
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
          output_dir_ + "temp/battle_models/"
          + battle_lgp_file_names_[next_model_to_process_] + ".p"
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
        magic_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/spell_models/" + fname
        );
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
        std::cout << "[BONE RENAMED] " << fname << " --> " << p_name << std::endl;
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
        magic_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/spell_models/" + fname
        );
        TexFile tex(output_dir_ + "temp/spell_models/" + fname);
        std::string path = output_dir_ + "models/battle/attacks/" + id + ".png";
        // Many of these tex files are wrong and dummied out in the OG, so try/catch.
        try{
            tex.SavePng(path);
        }
        catch(const std::exception& e){
            std::cerr << "Error converting texture " << (output_dir_ + "temp/spell_models/" + fname)
              << " to png: " << e.what() << std::endl;
        }
    }
    if (ext == "a00"){
        // Animation file.
        magic_lgp_files_[next_model_to_process_].WriteFile(
          output_dir_ + "temp/spell_models/" + fname
        );
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

unsigned int BattleDataInstaller::ConvertBattleModelsInit(){
    next_model_to_convert_ = 0;
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
            std::vector<std::string> a_files = da.GenerateAFiles(
              model.id, output_dir_ + "temp/battle_models/"
            );
            for (std::string file_name : a_files) model.a.push_back(file_name);
        }

        if ("" != model.script){ // Skip non scripted models
            AbFile ab(File(output_dir_ + "temp/battle_models/" + model.script), info.is_enemy);
            ab.GenerateScripts(
                model.id + "_" + info.name_normal, output_dir_ + "scripts/battle_models/"
            );
        }

        // Reload the resources for the newly created hrc and rsd.
        // TODO: Could this be done with res_mgr_->declareResource() ? Probably faster
        res_mgr_->removeResourceLocation(output_dir_ + "temp/battle_models/", "FFVII");
        res_mgr_->addResourceLocation(
          output_dir_ + "temp/battle_models/", "FileSystem", "FFVII", true, true
        );

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
            VGears::AFilePtr a
              = afl_mgr.load(a_base_name + ".a", "FFVII").staticCast<VGears::AFile>();
            // Convert the FF7 name to a more readable name set in the meta data.
            VGears::StringUtil::splitBase(anim, base_name);
            a->AddTo(skeleton, VGears::NameLookup::Animation(base_name));
        }
        ExportMesh(path, mesh);
    }
    catch (const Ogre::Exception& ex){
        std::cerr << "[ERROR] Ogre exception converting battle model "
          << model.hrc <<": " << ex.what() << std::endl;
    }
    catch (const std::exception& ex){
        std::cerr << "[ERROR] Exception converting battle model "
          << model.hrc << ": " << ex.what() << std::endl;
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
            std::cout << "[DECOMPILE] " << output_dir_ + "temp/spell_models/" + model.hrc << std::endl;
            DecompileSpellModelHrc(
              File(output_dir_ + "temp/spell_models/" + model.hrc), model,
              output_dir_ + "temp/spell_models/" + info.name_normal + ".hrc", model.id
            );
            model.hrc = info.name_normal + ".hrc";
            std::cout << "    TO: " << (output_dir_ + "temp/spell_models/" + info.name_normal + ".hrc") << std::endl;
        }
        GenerateRsdFiles(model, output_dir_ + "temp/spell_models/");

        // Decompile the .a00 file (if any) into a .a file.
        if ("" != model.anim){ // Skip for non-animated.
            DaFile da(File(output_dir_ + "temp/spell_models/" + model.anim));
            std::vector<std::string> a_files = da.GenerateAFiles(
              model.id, output_dir_ + "temp/spell_models/"
            );
            for (std::string file_name : a_files) model.a.push_back(file_name);
        }

        // Reload the resources for the newly created hrc and rsd.
        // TODO: Could this be done with res_mgr_->declareResource() ? Probably faster
        res_mgr_->removeResourceLocation(output_dir_ + "temp/spell_models/", "FFVII");
        res_mgr_->addResourceLocation(
          output_dir_ + "temp/spell_models/", "FileSystem", "FFVII", true, true
        );

        // Export meshes thet have an .hrc skeleton.
        if (model.hrc != ""){
            std::cout << "[EXPORT MESH] " << model.hrc << std::endl;
            Ogre::ResourcePtr hrc = VGears::HRCFileManager::GetSingleton().load(
              model.id + ".hrc", "FFVII"
            );
            Ogre::String base_name;
            VGears::StringUtil::splitBase(model.hrc, base_name);
            auto mesh_name = model.id + ".mesh";
            Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(mesh_name, "FFVII"));
            Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
            for (std::string anim : model.a){
                VGears::AFileManager &afl_mgr(VGears::AFileManager::GetSingleton());
                Ogre::String a_base_name;
                VGears::StringUtil::splitBase(anim, a_base_name);
                VGears::AFilePtr a
                  = afl_mgr.load(a_base_name + ".a", "FFVII").staticCast<VGears::AFile>();
                // Convert the FF7 name to a more readable name set in the meta data.
                VGears::StringUtil::splitBase(anim, base_name);
                a->AddTo(skeleton, VGears::NameLookup::Animation(base_name));
            }
            ExportMesh(path, mesh);
        }
    }
    catch (const Ogre::Exception& ex){
        std::cerr << "[ERROR] Ogre exception converting spell model "
          << model.hrc <<": " << ex.what() << std::endl;
    }
    catch (const std::exception& ex){
        std::cerr << "[ERROR] Exception converting spell model "
          << model.hrc << ": " << ex.what() << std::endl;
    }
    next_model_to_convert_ ++;
    return next_model_to_convert_;
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
            xml_attack->SetAttribute("status", attack.id);
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
        xml.LinkEndChild(container.release());
        std::string file_name = BuildEnemyFileName(enemy);
        xml.SaveFile(output_dir_ + file_name);
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
        damage_acc->SetAttribute("value", attack.damage.accuracy);
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
        xml.SaveFile(output_dir_ + file_name);
        attack_map_[attack.id] = file_name;
    }
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
        int c = 0;
        for (Formation::Camera camera : formation.camera_positions){
            std::unique_ptr<TiXmlElement> camera_xml(new TiXmlElement("position"));
            camera_xml->SetAttribute("id", c);
            camera_xml->SetAttribute("x", camera.x);
            camera_xml->SetAttribute("y", camera.y);
            camera_xml->SetAttribute("z", camera.z);
            camera_xml->SetAttribute("direction_x", camera.d_x);
            camera_xml->SetAttribute("direction_y", camera.d_y);
            camera_xml->SetAttribute("direction_z", camera.d_z);
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
        xml.SaveFile(output_dir_ + file_name);
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
    xml.SaveFile(output_dir_ + "models/battle/characters.xml");
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
    xml.SaveFile(output_dir_ + "models/battle/scenes.xml");
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
        ){
            file_name += attack.name[n];
        }
    }
    file_name += ".xml";
    return file_name;
}

File* BattleDataInstaller::ExtractGZipScene(File file){
    u32 offset = 0;
    u8* file_buffer = static_cast<u8*>(malloc(file.GetFileSize()));
    file.GetFileBuffer(file_buffer, 0, file.GetFileSize());
    u32 extract_size = 256 * 1024;
    u8* extract_buffer = static_cast<u8*>(malloc(extract_size));
    int ret;
    z_stream strm;
    strm.zalloc = Z_NULL; // Used to allocate the internal state.
    strm.zfree = Z_NULL; // Used to free the internal state.
    strm.opaque = Z_NULL; // Private data object passed to zalloc and zfree.
    strm.next_in = file_buffer + offset; //Next input byte.
    strm.avail_in = file.GetFileSize() - offset; // Number of bytes available at next_in.
    strm.next_out = extract_buffer; // Next output byte should be put there.
    strm.avail_out = extract_size; // Remaining free space at next_out.
    ret = inflateInit2(&strm, 15 + 32);
    if (ret != Z_OK){
        switch (ret){
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                std::cout << "Warning: inflateInit2 - Z_MEM_ERROR in file "
                  << file.GetFileName() << std::endl;
                break;
            case Z_STREAM_ERROR:
                inflateEnd(&strm);
                std::cout << "Warning: inflateInit2 - Z_STREAM_ERROR in file "
                  << file.GetFileName() << std::endl;
                break;
            default:
                inflateEnd(&strm);
                std::cout << "Warning: inflateInit2 - unknown error in file "
                  << file.GetFileName() << std::endl;
        }
        return NULL;
    }
    do{
        if (strm.next_out == NULL){
            inflateEnd(&strm);
            std::cout << "Warning: strm.next_out == NULL in file "
              << file.GetFileName() << std::endl;
            return NULL;
        }
        ret = inflate(&strm, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);
        switch (ret){
            case Z_NEED_DICT:
                inflateEnd(&strm);
                std::cout << "Warning: inflate - Z_NEED_DICT in file "
                  << file.GetFileName() << std::endl;
                return NULL;
            case Z_DATA_ERROR:
                inflateEnd(&strm);
                std::cout << "Warning: inflate - Z_DATA_ERROR in file "
                  << file.GetFileName() << std::endl;
                return NULL;
            case Z_MEM_ERROR:
                inflateEnd(&strm);
                std::cout << "Warning: inflate - Z_MEM_ERROR in file "
                  << file.GetFileName() << std::endl;
                return NULL;
        }
        if (ret != Z_STREAM_END){
            extract_buffer = static_cast<u8*>(realloc(extract_buffer, extract_size * 2));
            if (extract_buffer == NULL){
                inflateEnd(&strm);
                //std::cout << "Warning: extract_buffer == NULL in file "
                //  << file.GetFileName() << std::endl;
                return NULL;
            }
            strm.next_out = static_cast<u8*>(extract_buffer + extract_size);
            strm.avail_out = extract_size;
            extract_size *= 2;
        }
    } while (ret != Z_STREAM_END);
    extract_size = extract_size - strm.avail_out;
    (void)inflateEnd(&strm);
    if (ret != Z_STREAM_END){
        std::cout << "Warning: ret != Z_STREAM_END in file " << file.GetFileName() << std::endl;
        return NULL;
    }
    File* out_file = new File(extract_buffer, 0, extract_size);
    return out_file;
}

void BattleDataInstaller::ExportMesh(const std::string outdir, const Ogre::MeshPtr &mesh){
    VGears::String base_mesh_name;
    VGears::StringUtil::splitFull(mesh->getName(), base_mesh_name);
    Ogre::MeshSerializer mesh_serializer;
    Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
    Ogre::SkeletonSerializer skeleton_serializer;
    skeleton_serializer.exportSkeleton(
      skeleton.get(), output_dir_ + "models/battle/" + outdir + base_mesh_name + ".skeleton"
    );
    mesh->setSkeletonName(outdir + base_mesh_name + ".skeleton");
    mesh_serializer.exportMesh(
      mesh.get(), output_dir_ + "models/battle/" + outdir + mesh->getName()
    );
    Ogre::MaterialSerializer mat_ser;
    std::set<std::string> textures;
    Ogre::Mesh::SubMeshList sub_meshes = mesh->getSubMeshes();
    for (Ogre::SubMesh* sub_mesh : sub_meshes){
        // Change material name to avoid conflicts with field model materials.
        sub_mesh->setMaterialName(sub_mesh->getMaterialName());
        Ogre::MaterialPtr mat(
            Ogre::MaterialManager::getSingleton().getByName(sub_mesh->getMaterialName())
        );
        if (mat == nullptr) continue;
        for (size_t techs = 0; techs < mat->getNumTechniques(); techs ++){
            Ogre::Technique* tech = mat->getTechnique(techs);
            if (tech == nullptr) continue;
            for (size_t pass_num = 0; pass_num < tech->getNumPasses(); pass_num ++){
                Ogre::Pass* pass = tech->getPass(pass_num);
                if (pass == nullptr) continue;
                for (
                  size_t texture_unit_num = 0;
                  texture_unit_num < pass->getNumTextureUnitStates();
                  texture_unit_num ++
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
            battle_materials_.push_back(sub_mesh->getMaterialName());
        }
    }
    mat_ser.exportQueued(
      output_dir_ + "models/battle/" + outdir + base_mesh_name + VGears::EXT_MATERIAL
    );
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
        for (int t = 0; t < model.tex.size(); t ++)
            content += "TEX[" + std::to_string(t) + "]=" + model.tex[t] + "\n";
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
            if (p_index < model.p.size())
                decompiled += model.p[p_index].substr(0, model.p[p_index].length() - 2);
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
        decompiled += "100.000000\n";
        decompiled += "1 " + model.p[b].substr(0, 4) + "\n\n";
    }
    std::ofstream out(path);
    out << decompiled;
    out.close();
}
