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
#include <algorithm>
#include <OgreMesh.h>
#include "installer/formats/TexFile.h"
#include "formats/model/HRCFileManager.h"
#include "installer/formats/DaFile.h"
#include "installer/tables/FF7Data.h"
#include "installer/extractors/SpellModelInstaller.h"

SpellModelInstaller::SpellModelInstaller(
  DiskImage& disk_image, const std::string& output_dir, Ogre::ResourceGroupManager* res_mgr,
  BattleMeshExporter& exporter, std::vector<LgpEntry>& files
):
  disk_image_(disk_image), output_dir_(output_dir), data_dir_(output_dir + "game/"),
  res_mgr_(res_mgr), exporter_(exporter), files_(files)
{}

unsigned int SpellModelInstaller::InitializeSpellModels(){
    next_model_to_process_ = 0;
    next_model_to_convert_ = 0;
    models_.clear();
    files_ = ReadLgp(*disk_image_.filePath("data/battle/magic.lgp"));
    return files_.size();
}

unsigned int SpellModelInstaller::ProcessSpellModel(){
    if (next_model_to_process_ >= files_.size()) return files_.size();
    // TODO: Do something with the files.
    // A little explanation. There are no patterns on file names, and there are multiple extensions.
    //  - d: Skeleton file, similar to aa from battle models.
    //  - b: Unknown.
    //  - p*: Polygon files, for each bone.
    //  - tex: Texture files, TIM format.
    //  - rsd: Resource files
    //  - s: Unknown.
    //  - a00: Animation files, similar to da from battle models.
    const std::string fname = files_[next_model_to_process_].name;
    File& file = files_[next_model_to_process_].file;
    const std::string id = fname.substr(0, fname.find_last_of("."));
    const std::string ext = fname.substr(fname.find_last_of(".") + 1);
    const std::string temp = output_dir_ + "temp/spell_models/";

    if (ext == "d"){
        // Skeleton file.
        file.WriteFile(temp + fname);
        FindOrAddModel(models_, id).hrc = fname;
    }
    else if (ext.at(0) == 'p'){
        // Bone file.
        // If the extension is just ".p", use the file name.
        std::string p_name;
        if (ext == "p") p_name = fname;
        // If the extension is ".pXX", save it with altered name.
        else p_name =
          fname.substr(0, fname.length() - 4) + "_" + fname.substr(fname.length() - 2, 2) + ".p";
        file.WriteFile(temp + p_name);
        FindOrAddModel(models_, id).p.push_back(p_name);
    }
    if (ext == "tex"){
        // Texture files. They can be shared among various spells, so just convert them and don't
        // add them to a particular model.
        file.WriteFile(temp + fname);
        TexFile tex(temp + fname);
        // Many of these tex files are wrong and dummied out in the OG, so try/catch.
        try{
            tex.SavePng(data_dir_ + "battle/models/attacks/" + id + ".png");
        }
        catch(const std::exception& e){
            std::cerr << "[WARNING] Error converting texture " << (temp + fname)
              << " to png: " << e.what() << std::endl;
        }
    }
    if (ext == "a00"){
        // Animation file.
        file.WriteFile(temp + fname);
        FindOrAddModel(models_, id).anim = fname;
    }
    // TODO: b files? a files? s files?
    next_model_to_process_ ++;
    return next_model_to_process_;
}

unsigned int SpellModelInstaller::ConvertSpellModelsInit(){
    next_model_to_convert_ = 0;
    return models_.size();
}

unsigned int SpellModelInstaller::ConvertSpellModel(){
    if (next_model_to_convert_ >= models_.size()) return models_.size();
    SpellModel model = models_[next_model_to_convert_];
    try{
        std::string path = "attacks/";
        FF7Data::SpellModelInfo info = FF7Data::GetSpellModelInfo(model.id);

        // Decompile the .hrc file (if any)
        if (model.hrc != ""){
            std::vector<std::string> meshes;
            std::vector<std::string> sorted = model.p;
            std::sort(sorted.begin(), sorted.end());
            for (const std::string& p : sorted) meshes.push_back(p.substr(0, p.length() - 2));
            DecompileHrc(
              File(output_dir_ + "temp/spell_models/" + model.hrc), meshes,
              output_dir_ + "temp/spell_models/" + info.name_normal + ".hrc", model.id
            );
            model.hrc = info.name_normal + ".hrc";
        }
        WriteRsdFiles(model, output_dir_ + "temp/spell_models/");

        // Decompile the .a00 file (if any) into a .a file.
        if ("" != model.anim){ // Skip for non-animated.
            DaFile da(File(output_dir_ + "temp/spell_models/" + model.anim));
            std::vector<std::string> a_files = da.GenerateAFiles(model.id, output_dir_ + "temp/spell_models/");
            for (std::string file_name : a_files) model.a.push_back(file_name);
        }

        // Reload the resources for the newly created hrc and rsd.
        ReloadModelResources(res_mgr_, output_dir_ + "temp/spell_models/");

        // Export meshes thet have an .hrc skeleton.
        if (model.hrc != ""){
            VGears::HRCFileManager::GetSingleton().load(model.id + ".hrc", "FFVII");
            Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(model.id + ".mesh", "FFVII"));
            AddAnimations(mesh->getSkeleton(), model.a);
            exporter_.Export(path, mesh);
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

void SpellModelInstaller::WriteRsdFiles(const SpellModel& model, const std::string& path){
    // For each p file, generate a rsd file.
    for (std::string p : model.p){
        std::string base_name = p.substr(0, p.length() - 2);
        std::string content = "@RSD940102\n";
        content += "PLY=" + base_name + ".PLY\n";
        content += "MAT=" + base_name + ".MAT\n";
        content += "GRP=" + base_name + ".GRP\n";
        content += "NTEX=0\n";
        content += "\n";
        std::ofstream out(path + base_name + ".rsd");
        out << content;
        out.close();
    }
}
