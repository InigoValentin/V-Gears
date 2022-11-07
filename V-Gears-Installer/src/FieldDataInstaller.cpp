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

#include <string>
#include <boost/filesystem.hpp>
#include <QtCore/QDir>
#include "FieldDataInstaller.h"
#include "TexFile.h"
#include "common/File.h"
#include "data/VGearsMapListFile.h"
#include "data/VGearsLZSFLevelFileManager.h"
#include "data/VGearsHRCFileManager.h"
#include "data/VGearsLGPArchive.h"
#include "data/VGearsAFileManager.h"

float FieldDataInstaller::LINE_SCALE_FACTOR(0.0078124970964f);

std::string FieldDataInstaller::FIELD_MODELS_DIR("models/fields/entities");

std::string FieldDataInstaller::FIELD_MAPS_DIR("fields");

int FieldDataInstaller::INACTIVE_GATEWAY_ID(32767);

std::string FieldDataInstaller::CreateGateWayScript(
  const std::string& gateway_entity_name, const std::string& target_map_name,
  const std::string& source_spawn_point_name
){
    return
      "\nEntityContainer[ \"" + gateway_entity_name + "\" ] = {\n\n"
      "    on_start = function(self)\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_approach = function(self, entity)\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_cross = function(self, entity)\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_near = function(self, entity)\n"
      "        if entity == \"Cloud\" then\n"
      "            if not FFVII.Data.DisableGateways then\n"
      "                load_field_map_request(\""
      + target_map_name + "\", \"" + source_spawn_point_name + "\")\n"
      "            end\n"
      "        end\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_leave = function(self, entity)\n"
      "        return 0\n"
      "    end,\n"
      "}\n\n";
}

size_t FieldDataInstaller::GetFieldId(
  const std::string& name, const std::vector<std::string>& field_id_to_name_lookup
){
    for (size_t i = 0; i < field_id_to_name_lookup.size(); i ++)
        if (field_id_to_name_lookup[i] == name) return i;
    throw std::runtime_error("No Id found for field name");
}

bool FieldDataInstaller::IsAFieldFile(const Ogre::String& resource_name){
    return (
      !VGears::StringUtil::endsWith(resource_name, ".tex")
      && !VGears::StringUtil::endsWith(resource_name, ".tut")
      && !VGears::StringUtil::endsWith(resource_name, ".siz")
      && resource_name != "maplist"
    );
}

bool FieldDataInstaller::WillCrash(const Ogre::String& resource_name){
    if (
      resource_name == "bugin1a" // crashes in back ground image generation
      || resource_name == "frcyo" // Hangs in decompliation
      || resource_name == "fr_e" // Background image crash
      || resource_name == "las4_2" // Background image crash
      || resource_name == "las4_3" // Background image crash
      || resource_name == "lastmap" // Background image crash
      || resource_name == "md_e1" // Background image crash
      || resource_name == "trnad_3" // Background image crash
      || resource_name == "bonevil2" // stof
      || resource_name == "coloin1" // boost::bad_format_string: format-string is ill-formed
      || resource_name == "del3" // Segmentation fault.
      || resource_name == "elmin4_2" // boost::bad_format_string: format-string is ill-formed
    ){return true;}
    return false;
}

bool FieldDataInstaller::IsTestField(const Ogre::String& resource_name){
    if (
      resource_name == "startmap"
      || resource_name == "md1stin"
      || resource_name == "md1_1"
      || resource_name == "md1_2"
      || resource_name == "md8_1"
      || resource_name == "nmkin_1"
      || resource_name == "nmkin_2"
      || resource_name == "nmkin_3"
      || resource_name == "nmkin_4"
      || resource_name == "nmkin_5"
      || resource_name == "nrthmk"
      || resource_name == "elevtr1"
      || resource_name == "tin_1"
      || resource_name == "tin_2"
      || resource_name == "rootmap"
      || resource_name == "md8_4"
    ){return true;}
    return false;
}

void FieldDataInstaller::CreateDir(const std::string& path){
    QString target = QString::fromStdString(output_dir_ + path);
    QDir dir(target);
    if (!dir.mkpath(".")) throw std::runtime_error("Failed to mkpath");
}

void FieldDataInstaller::CollectFieldScaleFactors(
  VGears::FLevelFilePtr& field, FieldScaleFactorMap& scale_factors,
  const std::vector<std::string>& field_id_to_name_lookup
){
    scale_factors[GetFieldId(field->getName(), field_id_to_name_lookup)]
      = FieldDecompiler::ScaleFactor(field->GetRawScript());
}

void FieldDataInstaller::CollectSpawnPoints(
  VGears::FLevelFilePtr& field, const std::vector<std::string>& field_id_to_name_lookup,
  FieldSpawnPointsMap& spawn_points
){
    const size_t this_field_id = GetFieldId(field->getName(), field_id_to_name_lookup);
    // Decompile the script just so the MAPJUMPs can be collected. This is needed to construct the
    // full list of entries to each field.
    try{
        // Get the raw script bytes.
        FieldDecompiler::DecompiledScript decompiled;
        const std::vector<u8> raw_field_data = field->GetRawScript();
        // Decompile to LUA.
        FF7FieldScriptGatewayCollector formatter(
          field->getName(), field_id_to_name_lookup, spawn_points, this_field_id
        );
        decompiled = FieldDecompiler::Decompile(
          field->getName(), raw_field_data, formatter, "", "EntityContainer = {}\n\n"
        );
    }
    catch (const ::DecompilerException& ex){
        std::cerr << "CollectSpawnPoints: DecompilerException: " << ex.what() << std::endl;
    }
    const VGears::TriggersFilePtr& triggers = field->GetTriggers();
    const auto& gateways = triggers->GetGateways();
    for (size_t i = 0; i < gateways.size(); i ++){
        const VGears::TriggersFile::Gateway& gateway = gateways[i];
        if (gateway.destination_field_id != INACTIVE_GATEWAY_ID){
            auto it = spawn_points.find(gateway.destination_field_id);
            if (it != std::end(spawn_points)){
                // Add to the list of gateways that link to
                // destination_field_id.
                SpawnPointDb::Record rec;
                rec.field_id = this_field_id;
                rec.gateway = gateway;
                rec.gateway_index_or_map_jump_address = i;
                it->second.gateways_to_this_field.push_back(rec);
            }
            else{
                // Create a new record for destination_field_id.
                SpawnPointDb db;
                db.target_field_id = gateway.destination_field_id;
                SpawnPointDb::Record rec;
                rec.field_id = this_field_id;
                rec.gateway = gateway;
                rec.gateway_index_or_map_jump_address = i;
                db.gateways_to_this_field.push_back(rec);
                spawn_points.insert(std::make_pair(db.target_field_id, db));
            }
        }
    }
}

FieldDataInstaller::FieldDataInstaller(const std::string input_dir, const std::string output_dir):
  input_dir_(input_dir), output_dir_(output_dir)
{}

FieldDataInstaller::~FieldDataInstaller(){}

int FieldDataInstaller::CollectSpawnAndScaleFactorsInit(Ogre::ResourceGroupManager* res_mgr){
    // List what's in the LGP archive.
    //flevel_file_list_ = application_.ResMgr()->listResourceNames("FFVIIFields", "*");
    flevel_file_list_ = res_mgr->listResourceNames("FFVIIFields", "*");
    // Load the map list field.
    VGears::MapListFilePtr map_list = VGears::MapListFileManager::GetSingleton().load(
        "maplist", "FFVIIFields"
      ).staticCast<VGears::MapListFile>();
    map_list_ = map_list->GetMapList();
    return flevel_file_list_->size();
}

void FieldDataInstaller::CollectSpawnAndScaleFactors(int field_index){
    // On the first pass collate required field information.
    progress_step_num_elements_ = flevel_file_list_->size();
    if (field_index >= flevel_file_list_->size()){
        // TODO: Show invalid index.
        return;
    }
    auto resource_name = (*flevel_file_list_)[field_index];
    // Exclude things that are not fields.
    //if (IsAFieldFile(resource_name) && !WillCrash(resource_name)){
    // TODO: DEBUG: Only testing fields.
    if (IsAFieldFile(resource_name) && IsTestField(resource_name) && !WillCrash(resource_name)){
        VGears::FLevelFilePtr field = VGears::LZSFLevelFileManager::GetSingleton().load(
          resource_name, "FFVIIFields"
        ).staticCast<VGears::FLevelFile>();
        CollectSpawnPoints(field, map_list_, spawn_points_);
        CollectFieldScaleFactors(field, scale_factors_, map_list_);
    }
}

void FieldDataInstaller::Convert(int field_index){
    // Do the full conversion with the collated data.
    if (field_index >= flevel_file_list_->size()){
        // TODO: Show invalid index.
        return;
    }
    auto resource_name = (*flevel_file_list_)[field_index];
    // Exclude things that are not fields.
    if (IsAFieldFile(resource_name)){
        //if (/*IsTestField(resource_name) &&*/ !WillCrash(resource_name)){
        // TODO: DEBUG: Only test fields
        if (IsTestField(resource_name) && !WillCrash(resource_name)){
            //write_output_line_("Converting field " + resource_name);
            std::cout << " - Converting field: " << resource_name << std::endl;
            CreateDir(FIELD_MAPS_DIR + "/" + resource_name);
            VGears::FLevelFilePtr field = VGears::LZSFLevelFileManager::GetSingleton().load(
                resource_name, "FFVIIFields"
              ).staticCast<VGears::FLevelFile>();
            PcFieldToVGearsField(field);
        }
        else{
            /*write_output_line_(
              "[ERROR] Skip field " + resource_name + " due to crash or hang issue."
            );*/
            std::cerr << "[ERROR] Skip field: " << resource_name
              << " due to crash or hang issue." << std::endl;
        }
    }
}

int FieldDataInstaller::WriteInit(){
    doc_ = std::make_unique<TiXmlDocument>();
    element_ = std::make_unique<TiXmlElement>("maps");
    return converted_map_list_.size();
}

void FieldDataInstaller::Write(int field_index){
    // TODO: Probably need to inject "empty" and "test" fields.
    if (field_index >= flevel_file_list_->size()){
        // TODO: Show invalid index.
        return;
    }
    auto map = converted_map_list_[field_index];
    //write_output_line_("Writing field " + FieldName(map));
    std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("map"));
    xml_element->SetAttribute("name", map);
    xml_element->SetAttribute("file_name", FIELD_MAPS_DIR + "/" + map + "/map.xml");
    element_->LinkEndChild(xml_element.release());
}

void FieldDataInstaller::WriteEnd(){
    doc_->LinkEndChild(element_.release());
    doc_->SaveFile(output_dir_ + "/maps.xml");
}

std::vector<std::string> FieldDataInstaller::ConvertModelsInit(){

    std::vector<std::string> models;

    // Open char_lgp as a lgp archive
    VGears::LGPArchive char_lgp(input_dir_ + "data/field/char.lgp", "LGP");
    char_lgp.open(input_dir_ + "data/field/char.lgp", true);
    char_lgp.load();
    // Also, open it as a file for reading
    File char_file(input_dir_ + "data/field/char.lgp");

    //Ogre::StringVectorPtr file_list = char_lgp.list(true, true);
    field_model_file_list_ = char_lgp.list(true, true);
    VGears::LGPArchive::FileList files = char_lgp.GetFiles();
    for (int i = 0; i < field_model_file_list_->size(); i ++){
        VGears::LGPArchive::FileEntry f = files.at(i);
        // Save the TEX File
        std::fstream out;
        out.open(output_dir_ + "temp/char/" + f.file_name, std::ios::out);
        char_file.SetOffset(f.data_offset);
        for (int j = 0; j < f.data_size; j ++) out << char_file.readU8();
        out.close();
        //field_model_file_list_->push_back(f.file_name);
    }
    for (auto it = used_models_and_anims_.map.begin(); it != used_models_and_anims_.map.end(); it ++){
        models.push_back(it->first);
    }
    return models;
}

void FieldDataInstaller::ConvertModels(std::string model_name){
    auto model = used_models_and_anims_.map.find(model_name);
    try{
        Ogre::ResourcePtr hrc = VGears::HRCFileManager::GetSingleton().load(model->first, "FFVII");
        //Ogre::ResourcePtr hrc = VGears::HRCFile();
        Ogre::String base_name;
        VGears::StringUtil::splitBase(model->first, base_name);
        auto mesh_name = VGears::NameLookup::model(base_name) + ".mesh";
        Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(mesh_name, "FFVII"));
        Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
        for (auto& anim : model->second){
            VGears::AFileManager &afl_mgr(VGears::AFileManager::GetSingleton());
            VGears::AFilePtr a = afl_mgr.load(anim, "FFVII").staticCast<VGears::AFile>();
            // Convert the FF7 name to a more readable name set in the meta data.
            VGears::StringUtil::splitBase(anim, base_name);
            a->AddTo(skeleton, VGears::NameLookup::Animation(base_name));
        }
        ExportMesh(output_dir_ + FIELD_MODELS_DIR + "/", mesh);
    }
    catch (const Ogre::Exception& ex){
        write_output_line_(
          "[ERROR] Ogre exception converting model "
          + model->first + ": " + ex.what()
        );
        std::cerr << "[ERROR] Ogre exception converting model "
          << model->first <<": " << ex.what() << std::endl;
    }
    catch (const std::exception& ex){
        write_output_line_(
          "[ERROR] Exception converting model " + model->first + ": " + ex.what()
        );
        std::cerr << "[ERROR] Exception converting model "
          << model->first << ": " << ex.what() << std::endl;
    }
}


void FieldDataInstaller::ExportMesh(const std::string outdir, const Ogre::MeshPtr &mesh){

    // TODO: Share function with pc model exporter
    VGears::String base_mesh_name;
    VGears::StringUtil::splitFull(mesh->getName(), base_mesh_name);
    Ogre::MeshSerializer mesh_serializer;
    Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
    Ogre::SkeletonSerializer skeleton_serializer;
    skeleton_serializer.exportSkeleton(
      skeleton.getPointer(), outdir + base_mesh_name + ".skeleton"
    );
    mesh->setSkeletonName(FIELD_MODELS_DIR + "/" + base_mesh_name + ".skeleton");
    mesh_serializer.exportMesh(mesh.getPointer(), outdir + mesh->getName());
    Ogre::Mesh::SubMeshIterator it(mesh->getSubMeshIterator());
    Ogre::MaterialSerializer    mat_ser;
    size_t i(0);
    std::set<std::string> textures;
    while (it.hasMoreElements()){
        Ogre::SubMesh *sub_mesh(it.getNext());
        Ogre::MaterialPtr mat(Ogre::MaterialManager::getSingleton().getByName(
          sub_mesh->getMaterialName())
        );
        if (mat != nullptr){
            for (size_t techs = 0; techs < mat->getNumTechniques(); techs ++){
                Ogre::Technique* tech = mat->getTechnique(techs);
                if (tech){
                    for (size_t pass_num = 0; pass_num < tech->getNumPasses(); pass_num ++){
                        Ogre::Pass* pass = tech->getPass(pass_num);
                        if (pass){
                            for (
                              size_t texture_unit_num = 0;
                              texture_unit_num < pass->getNumTextureUnitStates();
                              texture_unit_num ++
                            ){
                                Ogre::TextureUnitState* unit
                                  = pass->getTextureUnitState(texture_unit_num);
                                if (unit && unit->getTextureName().empty() == false){
                                    // Convert the texture from .tex to .png.
                                    TexFile tex(
                                      output_dir_ + "temp/char/" + unit->getTextureName()
                                    );

                                    // Ensure the output material script references png files
                                    // rather than tex files.
                                    Ogre::String base_name;
                                    VGears::StringUtil::splitBase(
                                      unit->getTextureName(), base_name
                                    );
                                    unit->setTextureName(
                                      FIELD_MODELS_DIR + "/" + base_mesh_name
                                      + "_" + base_name + ".png"
                                    );

                                    tex.SavePng(
                                      output_dir_ + FIELD_MODELS_DIR + "/" + base_name + ".png", 0
                                    );
                                    // Copy subtexture (xxxx.png) to model_xxxx.png
                                    // TODO: obtain the "data" folder
                                    // programatically.
                                    boost::filesystem::copy_file(
                                        output_dir_ + FIELD_MODELS_DIR + "/" + base_name + ".png",
                                        output_dir_ + FIELD_MODELS_DIR + "/"
                                        + base_mesh_name + "_" + base_name + ".png",
                                      boost::filesystem::copy_option::overwrite_if_exists
                                    );
                                    textures.insert(unit->getTextureName());
                                }
                            }
                        }
                    }
                }
            }
            if (std::count(materials_.begin(), materials_.end(), sub_mesh->getMaterialName()) == 0){
                //std::cout << "[MATERIAL] Writting material "
                // << sub_mesh->getMaterialName() << std::endl;
                mat_ser.queueForExport(mat);
                materials_.push_back(sub_mesh->getMaterialName());
            }

        }
        ++ i;
    }
    mat_ser.exportQueued(outdir + base_mesh_name + VGears::EXT_MATERIAL);
    for (auto& texture_name : textures){
        std::string tex_name = texture_name.c_str();
        try{
            Ogre::TexturePtr texture_ptr
              = Ogre::TextureManager::getSingleton().load(tex_name, "FFVIITextures" /*"FFVII"*/);
            Ogre::Image image;
            texture_ptr->convertToImage(image);
            Ogre::String base_name;
            VGears::StringUtil::splitBase(texture_name, base_name);
            image.save(outdir + base_mesh_name + "_" + base_name + ".png");
        }
        catch (std::exception const& ex){
            std::cerr << "[ERROR] Exception: " << ex.what() << std::endl;
        }
    }
}

float FieldDataInstaller::GetFieldScaleFactor(size_t field_id){
    auto it = scale_factors_.find(field_id);
    if (it == std::end(scale_factors_))
        throw std::runtime_error("Scale factor not found for field id");
    return it->second;
}

std::vector<int> FieldDataInstaller::ExtractMusicTrackIds(VGears::FLevelFilePtr& field){
    // Search for the "AKAO" string in the raw field data.
    // The next byte is a track id.
    const std::vector<u8> raw = field->GetRawScript();
    std::vector<int> tracks;
    for (int i = 0; i < raw.size() - 5; i ++){
        if (raw[i] == 0x41){                   // A
            if (raw[i + 1] == 0x4B){           // K
                if (raw[i + 2] == 0x41){       // A
                    if (raw[i + 3] == 0x4f){   // O
                        tracks.push_back(static_cast<int>(raw[i + 4]) - 2);
                    }
                }
            }
        }
    }
    return tracks;
}

void FieldDataInstaller::PcFieldToVGearsField(VGears::FLevelFilePtr& field){
    // Generate triggers script to insert into main
    // decompiled FF7 field -> LUA script.
    std::string gateway_script_data;
    const VGears::TriggersFilePtr& triggers = field->GetTriggers();
    const auto& gateways = triggers->GetGateways();
    for (size_t i = 0; i < gateways.size(); i ++){
        const VGears::TriggersFile::Gateway& gateway = gateways[i];
        if (gateway.destination_field_id != INACTIVE_GATEWAY_ID){
            const std::string gateway_entity_name = "Gateway" + std::to_string(i);
            gateway_script_data += CreateGateWayScript(
              gateway_entity_name,
              map_list_.at(gateway.destination_field_id),
              "Spawn_" + field->getName() + "_" + std::to_string(i)
            );
        }
    }
    const VGears::ModelListFilePtr& models = field->GetModelList();
    FieldDecompiler::DecompiledScript decompiled;
    FF7FieldScriptFormatter formatter(field->getName(), models, map_list_, spawn_points_);
    try{
        // Get the raw script bytes.
        const std::vector<u8> raw_field_data = field->GetRawScript();

        // Decompile to LUA.
        decompiled = FieldDecompiler::Decompile(
          field->getName(), raw_field_data, formatter, gateway_script_data,
          "EntityContainer = {}\n\n"
        );
        std::ofstream script_file(
          output_dir_ + "/" + FIELD_MAPS_DIR + "/" + field->getName() + "/script.lua"
        );
        if (script_file.is_open()){
            script_file << decompiled.luaScript;
            field_text_writer_.Begin(
              output_dir_ + "/" + FIELD_MAPS_DIR + "/" + field->getName() + "/text.xml"
            );
            try{field_text_writer_.Write(raw_field_data, field->getName());}
            catch (const std::out_of_range& ex){
                write_output_line_(
                  "[ERROR] Failed to read texts from field " + field->getName() + ": " + ex.what()
                );
                std::cerr << "[ERROR] Failed to read texts from field "
                  << field->getName() << ": " << ex.what() << std::endl;
            }
            field_text_writer_.End();
        }
        else{
            write_output_line_(
              "[ERROR] Failed to open script file from field " + field->getName() + " for writing."
            );
            std::cerr << "[ERROR] Failed to open script file from field "
              << field->getName() << "for writing." << std::endl;
        }
    }
    catch (const ::DecompilerException& ex){
        write_output_line_(
          "[ERROR] Internal decompiler error in field " + field->getName() + ": " + ex.what()
        );
        std::cerr << "[ERROR] Internal decompiler error in field "
          << field->getName() << ": " << ex.what() << std::endl;
    }
    // Write out the map file which links all the other files together for a given field.
    {
        TiXmlDocument doc;
        std::unique_ptr<TiXmlElement> element(new TiXmlElement("map"));
        // Script.
        std::unique_ptr<TiXmlElement> xml_script_element(new TiXmlElement("script"));
        xml_script_element->SetAttribute(
          "file_name", FIELD_MAPS_DIR + "/" + field->getName() + "/script.lua"
        );
        element->LinkEndChild(xml_script_element.release());
        // Background.
        std::unique_ptr<TiXmlElement> xml_background_2d(new TiXmlElement("background2d"));
        xml_background_2d->SetAttribute(
          "file_name", FIELD_MAPS_DIR + "/" + field->getName() + "/bg.xml"
        );
        element->LinkEndChild(xml_background_2d.release());
        // Texts.
        std::unique_ptr<TiXmlElement> xml_texts(new TiXmlElement("texts"));
        xml_texts->SetAttribute(
          "file_name", FIELD_MAPS_DIR + "/" + field->getName() + "/text.xml"
        );
        element->LinkEndChild(xml_texts.release());
        // Walkmesh.
        std::unique_ptr<TiXmlElement> xml_walkmesh(new TiXmlElement("walkmesh"));
        xml_walkmesh->SetAttribute(
          "file_name", FIELD_MAPS_DIR + "/" + field->getName() + "/wm.xml"
        );
        element->LinkEndChild(xml_walkmesh.release());
        // Forward direction (angle player moves when "up" is pressed)
        std::unique_ptr<TiXmlElement> xml_forward_direction(new TiXmlElement("movement_rotation"));
        xml_forward_direction->SetAttribute(
          "degree", std::to_string(triggers->MovementRotation())
        );
        element->LinkEndChild(xml_forward_direction.release());
        // Get this fields Id.
        const size_t this_field_id = GetFieldId(field->getName(), map_list_);
        // Use the ID to find the pre-computed list of gateways in all other fields that link to
        // this field.
        auto spawn_iterator = spawn_points_.find(this_field_id);
        Ogre::Vector3 first_entity_point = Ogre::Vector3::ZERO;

        // If none found it probably just means no other fields have doors to this one.
        if (spawn_iterator != std::end(spawn_points_)){
            const std::vector<SpawnPointDb::Record>& spawn_point_records
              = spawn_iterator->second.gateways_to_this_field;
            for (size_t i = 0; i < spawn_point_records.size(); i ++){
                const VGears::TriggersFile::Gateway& gateway = spawn_point_records[i].gateway;
                // Entity_point:
                std::unique_ptr<TiXmlElement> xml_entity_point(new TiXmlElement("entity_point"));
                if (spawn_point_records[i].from_script){
                    // Spawn points from map jumps have a another algorithm.
                    xml_entity_point->SetAttribute(
                      "name",
                      map_list_.at(
                        spawn_point_records[i].field_id) + "_" + spawn_point_records[i].entity_name
                        + "_" + spawn_point_records[i].script_function_name + "_addr_"
                        + std::to_string(spawn_point_records[i].gateway_index_or_map_jump_address)
                      );
                }
                else{
                    // Must also include the gateway index for the case where 2
                    // fields have more than one door linking to each other.
                    xml_entity_point->SetAttribute(
                      "name",
                      "Spawn_" + map_list_.at(spawn_point_records[i].field_id) + "_"
                        + std::to_string(spawn_point_records[i].gateway_index_or_map_jump_address)
                    );
                }
                const float downscaler_next = 128.0f * GetFieldScaleFactor(
                  gateway.destination_field_id
                );

                // Position Z is actually the target walkmesh triangle ID, so this is tiny bit more
                // complex. Now "get the Z value of the triangle with that ID". Note that ID
                // actually just means index.
                unsigned int triangle_index = static_cast<unsigned int>(gateway.destination.z);
                if (
                  triangle_index >= field->GetWalkmesh()->GetTriangles().size()
                ){
                    write_output_line_(
                      "[WARNING] In field " + field->getName() + ": Map jump triangle ("
                      + std::to_string(triangle_index) + ") out of bounds ("
                      + std::to_string(field->GetWalkmesh()->GetTriangles().size()) + ")"
                    );
                    std::cerr << "[WARNING] In field " << field->getName()
                      << ": Map jump triangle (" << triangle_index << ") out of bounds ("
                      << field->GetWalkmesh()->GetTriangles().size() << ")" << std::endl;
                    triangle_index = 0;
                }
                const float z_of_triangle_with_id
                  = field->GetWalkmesh()->GetTriangles().at(triangle_index).a.z;
                const Ogre::Vector3 position(
                  gateway.destination.x / downscaler_next, gateway.destination.y / downscaler_next,
                  z_of_triangle_with_id
                );
                if (position != Ogre::Vector3::ZERO && first_entity_point == Ogre::Vector3::ZERO)
                    first_entity_point = position;
                xml_entity_point->SetAttribute(
                  "position", Ogre::StringConverter::toString(position)
                );
                const float rotation = (360.0f * static_cast<float>(gateway.dir)) / 255.0f;
                xml_entity_point->SetAttribute("rotation", std::to_string(rotation));
                element->LinkEndChild(xml_entity_point.release());
            }
        }

        // Get lines. Add them to a list, so they aren't processed later as regular entities.
        std::vector<std::string> line_entities;
        for (FieldDecompiler::Line line : decompiled.lines){

            std::unique_ptr<TiXmlElement> xml_entity_trigger(new TiXmlElement("entity_trigger"));
            line_entities.push_back(line.name);
            xml_entity_trigger->SetAttribute("name", line.name);
            xml_entity_trigger->SetAttribute(
              "point1",
              std::to_string(line.point_a[0] * LINE_SCALE_FACTOR)
                + " " + std::to_string(line.point_a[1] * LINE_SCALE_FACTOR)
                + " " + std::to_string(line.point_a[2] * LINE_SCALE_FACTOR)
            );
            xml_entity_trigger->SetAttribute(
              "point2",
              std::to_string(line.point_b[0] * LINE_SCALE_FACTOR)
                + " " + std::to_string(line.point_b[1] * LINE_SCALE_FACTOR)
                + " " + std::to_string(line.point_b[2] * LINE_SCALE_FACTOR)
            );
            xml_entity_trigger->SetAttribute("enabled", "true");
            element->LinkEndChild(xml_entity_trigger.release());
        }

        // Get entities.
        for (FieldDecompiler::FieldEntity entity : decompiled.entities){
            // If the entity has been added as a line, skip.
            if (line_entities.size() > 0){
                if (
                  *std::find(line_entities.begin(), line_entities.end(), entity.name) == entity.name
                ){
                    continue;
                }
            }
            const int char_id = entity.char_id;
            if (char_id != -1){
                const VGears::ModelListFile::ModelDescription& desc = models->GetModels().at(char_id);
                auto& animations = used_models_and_anims_.ModelAnimations(desc.hrc_name);
                for (const auto& anim : desc.animations)
                    animations.insert(used_models_and_anims_.NormalizeAnimationName(anim.name));
                std::unique_ptr<TiXmlElement> xml_entity_script(new TiXmlElement("entity_model"));
                xml_entity_script->SetAttribute("name", entity.name);
                // TODO: Add to list of HRC's to convert, obtain name of converted .mesh file.
                auto lower_case_hrc_name = desc.hrc_name;
                VGears::StringUtil::toLowerCase(lower_case_hrc_name);
                xml_entity_script->SetAttribute(
                  "file_name",
                  FIELD_MODELS_DIR + "/" + used_models_and_anims_.ModelMetaDataName(lower_case_hrc_name)
                );
                xml_entity_script->SetAttribute("index", entity.index);
                if (desc.type == VGears::ModelListFile::PLAYER){
                    // For player models the position is set manually in the xml because if a map
                    // is loaded manually this is where the player will end up. Hence we set the
                    // position to the first entity_point that we have.
                    xml_entity_script->SetAttribute(
                      "position", Ogre::StringConverter::toString(first_entity_point)
                    );
                }
                else{
                    xml_entity_script->SetAttribute(
                      "position", Ogre::StringConverter::toString(Ogre::Vector3::ZERO)
                    );
                }
                xml_entity_script->SetAttribute("direction", "0");
                // TODO: This isn't quite right, the models animation
                // translation seems to be inverted?
                xml_entity_script->SetAttribute("scale", "0.03125 0.03125 0.03125");
                xml_entity_script->SetAttribute(
                  "root_orientation", "0.7071067811865476 0.7071067811865476 0 0"
                );
                element->LinkEndChild(xml_entity_script.release());
            }
            else{
                std::unique_ptr<TiXmlElement> xml_entity_script(new TiXmlElement("entity_script"));
                xml_entity_script->SetAttribute("name", entity.name);//it.first);
                element->LinkEndChild(xml_entity_script.release());
            }
        }
        const float downscaler_this = 128.0f * GetFieldScaleFactor(this_field_id);
        for (size_t i = 0; i < gateways.size(); i ++){
            const VGears::TriggersFile::Gateway& gateway = gateways[i];
            // If non-inactive gateway:
            if (gateway.destination_field_id != INACTIVE_GATEWAY_ID){
                std::unique_ptr<TiXmlElement> xml_entity_trigger(
                  new TiXmlElement("entity_trigger")
                );
                xml_entity_trigger->SetAttribute("name", "Gateway" + std::to_string(i));
                xml_entity_trigger->SetAttribute(
                  "point1",
                  Ogre::StringConverter::toString(
                    Ogre::Vector3(
                      gateway.exit_line[0].x,
                      gateway.exit_line[0].y,
                      gateway.exit_line[0].z) / downscaler_this
                  )
                );
                xml_entity_trigger->SetAttribute(
                  "point2",
                  Ogre::StringConverter::toString(
                    Ogre::Vector3(
                      gateway.exit_line[1].x,
                      gateway.exit_line[1].y,
                      gateway.exit_line[1].z) / downscaler_this
                    )
                  );
                // Enabled is hard coded to true.
                xml_entity_trigger->SetAttribute("enabled", "true");
                element->LinkEndChild(xml_entity_trigger.release());
            }
        }

        // Get music tracks
        std::vector<int> tracks = ExtractMusicTrackIds(field);
        std::unique_ptr<TiXmlElement> xml_tracks(new TiXmlElement("tracks"));
        for (int t = 0; t < tracks.size(); t ++){
            std::unique_ptr<TiXmlElement> xml_track(new TiXmlElement("track"));
            xml_track->SetAttribute("id", t);
            xml_track->SetAttribute("track_id", tracks[t]);
            xml_tracks->LinkEndChild(xml_track.release());
        }
        element->LinkEndChild(xml_tracks.release());

        doc.LinkEndChild(element.release());

        doc.SaveFile(output_dir_ + "/" + FIELD_MAPS_DIR + "/" + field->getName() + "/map.xml");
        const VGears::PaletteFilePtr& pal = field->GetPalette();
        const VGears::BackgroundFilePtr& bg = field->GetBackground();
        std::unique_ptr<Ogre::Image> bg_image(bg->CreateImage(pal));
        bg_image->encode("png");
        bg_image->save(output_dir_ + "/" + FIELD_MAPS_DIR + "/" + field->getName() + "/tiles.png");
        {
            TiXmlDocument bg_doc;
            std::unique_ptr<TiXmlElement> bg_element(new TiXmlElement("background2d"));
            // Magic constants.
            const int BG_SCALE_UP_FACTOR = 3;
            const int BG_PSX_SCREEN_WIDTH = 320;
            const int BG_PSX_SCREEN_HEIGHT = 240;
            // Get texture atlas size.
            const int width = bg_image->getWidth();
            const int height = bg_image->getHeight();
            const VGears::CameraMatrixFilePtr& camera_matrix = field->GetCameraMatrix();
            const Ogre::Vector3 position
              = camera_matrix->GetPosition() / GetFieldScaleFactor(this_field_id);
            const Ogre::Quaternion orientation = camera_matrix->GetOrientation();
            const Ogre::Degree fov
              = camera_matrix->GetFov(static_cast<float>(BG_PSX_SCREEN_HEIGHT));
            const int min_x = triggers->GetCameraRange().left * BG_SCALE_UP_FACTOR;
            const int min_y = triggers->GetCameraRange().top * BG_SCALE_UP_FACTOR;
            const int max_x = triggers->GetCameraRange().right * BG_SCALE_UP_FACTOR;
            const int max_y = triggers->GetCameraRange().bottom * BG_SCALE_UP_FACTOR;
            bg_element->SetAttribute("image", FIELD_MAPS_DIR + "/" + field->getName() + "/tiles.png");
            bg_element->SetAttribute("position", Ogre::StringConverter::toString(position));
            bg_element->SetAttribute("orientation", Ogre::StringConverter::toString(orientation));
            bg_element->SetAttribute("fov", Ogre::StringConverter::toString(fov));
            bg_element->SetAttribute(
              "range",
              std::to_string(min_x) + " " + std::to_string(min_y) + " "
              + std::to_string(max_x) + " " + std::to_string(max_y)
            );
            bg_element->SetAttribute(
              "clip",
              std::to_string(BG_PSX_SCREEN_WIDTH * BG_SCALE_UP_FACTOR) + " "
              + std::to_string(BG_PSX_SCREEN_HEIGHT * BG_SCALE_UP_FACTOR)
            );
            // Write out the *_BG.XML data.
            auto& layers = bg->GetLayers();
            for (const auto& layer : layers){
                // TODO: Should only the tiles to construct enabled layers be outputted?
                //  if (layer.enabled){

                for (const VGears::BackgroundFile::SpriteData& sprite : layer.sprites){
                    std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("tile"));
                    xml_element->SetAttribute(
                      "destination",
                      Ogre::StringConverter::toString(sprite.dst.x * BG_SCALE_UP_FACTOR) + " "
                      + Ogre::StringConverter::toString(sprite.dst.y * BG_SCALE_UP_FACTOR)
                    );
                    xml_element->SetAttribute(
                      "width", Ogre::StringConverter::toString(sprite.width * BG_SCALE_UP_FACTOR)
                    );
                    xml_element->SetAttribute(
                      "height", Ogre::StringConverter::toString(sprite.height * BG_SCALE_UP_FACTOR)
                    );
                    // Each tile is added to a big texture atlas with hard coded size of 1024x1024,
                    // convert UV's to the 0.0f to 1.0f range.
                    const float u0 = static_cast<float>(sprite.src.x) / width;
                    const float v0 = static_cast<float>(sprite.src.y) / height;
                    const float u1 = static_cast<float>(sprite.src.x + sprite.width) / width;
                    const float v1 = static_cast<float>(sprite.src.y + sprite.height) / height;
                    xml_element->SetAttribute(
                      "uv",
                      Ogre::StringConverter::toString(u0) + " "
                      + Ogre::StringConverter::toString(v0) + " "
                      + Ogre::StringConverter::toString(u1) + " "
                      + Ogre::StringConverter::toString(v1)
                    );
                    // TODO: It works (on FFVII PC), but why this number?
                    xml_element->SetAttribute(
                      "depth",
                      Ogre::StringConverter::toString(static_cast<float>(sprite.depth) * (0.03125f))
                    );
                    // TODO: Copied from DatFile::AddTile.
                    // Add to common method.
                    Ogre::String blending_str = "";
                    if (sprite.blending == 0) blending_str = "alpha";
                    // 3 is source + 0.25 * destination:
                    else if (sprite.blending == 1 || sprite.blending == 3) blending_str = "add";
                    else if (sprite.blending == 2) blending_str = "subtract";
                    // TODO: Should probably throw to fail conversion:
                    else blending_str = "unknown";
                    xml_element->SetAttribute("blending", blending_str);
                    bg_element->LinkEndChild(xml_element.release());
                }
                //}
            }
            bg_doc.LinkEndChild(bg_element.release());
            bg_doc.SaveFile(output_dir_ + "/" + FIELD_MAPS_DIR + "/" + field->getName() + "/bg.xml");
        }
    }
    {
        // Save out the walk mesh as XML.
        const VGears::WalkmeshFilePtr& walkmesh = field->GetWalkmesh();
        TiXmlDocument doc;
        std::unique_ptr<TiXmlElement> wmesh_element(new TiXmlElement("walkmesh"));
        for (VGears::WalkmeshFile::Triangle& tri : walkmesh->GetTriangles()){
            std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("triangle"));
            xml_element->SetAttribute("a", Ogre::StringConverter::toString(tri.a));
            xml_element->SetAttribute("b", Ogre::StringConverter::toString(tri.b));
            xml_element->SetAttribute("c", Ogre::StringConverter::toString(tri.c));
            xml_element->SetAttribute("a_b", std::to_string(tri.access_side[0]));
            xml_element->SetAttribute("b_c", std::to_string(tri.access_side[1]));
            xml_element->SetAttribute("c_a", std::to_string(tri.access_side[2]));
            wmesh_element->LinkEndChild(xml_element.release());
        }
        doc.LinkEndChild(wmesh_element.release());
        doc.SaveFile(output_dir_ + "/" + FIELD_MAPS_DIR + "/" + field->getName() + "/wm.xml");
    }
    converted_map_list_.push_back(field->getName());
}
