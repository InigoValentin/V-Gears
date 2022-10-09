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
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <OgreConfigFile.h>
#include <OgreArchiveManager.h>
#include <OgreMeshSerializer.h>
#include <OgreMaterialSerializer.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreSkeletonSerializer.h>
#include <OgreMeshManager.h>
#include <OgreRenderWindow.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreResourceGroupManager.h>
#include <OgreLog.h>
#include "VGearsGameState.h"
#include "data/VGearsAFileManager.h"
#include "data/VGearsBackgroundFileManager.h"
#include "data/VGearsCameraMatrixFileManager.h"
#include "data/VGearsHRCFileManager.h"
#include "data/VGearsLZSFLevelFileManager.h"
#include "data/VGearsPaletteFileManager.h"
#include "data/VGearsPFileManager.h"
#include "data/VGearsRSDFileManager.h"
#include "data/VGearsTexFile.h"
#include "data/VGearsTexCodec.h"
#include "map/VGearsBackground2DFileManager.h"
#include "map/VGearsWalkmeshFileManager.h"
#include "data/FinalFantasy7/FF7ModelListFileManager.h"
#include "data/VGearsLGPArchiveFactory.h"
#include "common/FinalFantasy7/FF7NameLookup.h"
#include "data/VGearsTexCodec.h"
#include "data/VGearsMapListFile.h"
#include <memory>
#include <QtCore/QDir>

#include "../include/DataInstaller.h"
#include "decompiler/field/FieldScriptFormatter.h"
#include "decompiler/field/FieldDecompiler.h"

float DataInstaller::LINE_SCALE_FACTOR = 0.0078124970964f;

DataInstaller::DataInstaller(
  const std::string input_dir, const std::string output_dir,
  std::function<void(std::string)> write_output_line
)
#ifdef _DEBUG
    : application_("plugins_d.cfg", "resources_d.cfg", "install_d.log"),
#else
  : application_("plugins.cfg", "resources.cfg", "install.log"),
#endif
  input_dir_(input_dir), output_dir_(output_dir), write_output_line(write_output_line),
  iterator_counter_(0), conversion_step_(0), progress_step_num_elements_(0)
{
    if (!application_.initOgre(true)) throw std::runtime_error("Ogre init failure");
    Ogre::Log* default_log( Ogre::LogManager::getSingleton().getDefaultLog());
    assert( default_log );
    default_log->setLogDetail(Ogre::LL_LOW);

}

DataInstaller::~DataInstaller(){}

int DataInstaller::CalcProgress(){
    // TODO: Make more accurate with iterator_counter_ and
    // progress_step_num_elements_.
    float curr_step = installation_state_ / static_cast<float>(STATE_COUNT);
    curr_step = curr_step * 100.0f;
    return static_cast<int>(curr_step);
}

int DataInstaller::Progress(){
    switch (installation_state_){
        case IDLE:
            write_output_line("Loading flevel.lgp");
            progress_step_num_elements_ = 1;
            iterator_counter_ = 0;
            fields_lgp_ = std::make_unique<ScopedLgp>(
              application_.getRoot(), input_dir_ + "field/flevel.lgp", "LGP", "FFVIIFields"
            );
            kernel_installer_ = std::make_unique<KernelDataInstaller>(
              input_dir_ += "kernel/KERNEL.BIN"
            );
            if (kernel_installer_->ReadCommands() > 0){
                CreateDir("game");
                kernel_installer_->WriteCommands(output_dir_ + "game/commands.xml");
            }
            if (kernel_installer_->ReadAttacks() > 0){
                CreateDir("game");
                kernel_installer_->WriteAttacks(output_dir_ + "game/attacks.xml");
            }
            if (kernel_installer_->ReadCharacters() > 0){
                CreateDir("game");
                kernel_installer_->WriteCharacters(output_dir_ + "game/characters.xml");
            }
            if (kernel_installer_->ReadItems() > 0){
                CreateDir("game");
                kernel_installer_->WriteItems(output_dir_ + "game/items.xml");
            }
            kernel_installer_->ReadGrowth();
            kernel_installer_->WriteGrowth(output_dir_ + "game/growth.xml");
            if (kernel_installer_->ReadWeapons() > 0){
                CreateDir("game");
                kernel_installer_->WriteWeapons(output_dir_ + "game/weapons.xml");
            }
            exit(0);

            installation_state_ = SPAWN_POINTS_AND_SCALE_FACTORS_INIT;
            // TODO: DEBUG:
            //installation_state_ = CONVERT_FIELDS;
            return CalcProgress();
        case SPAWN_POINTS_AND_SCALE_FACTORS_INIT:
            write_output_line("Collecting spawn points and scale factors...");
            InitCollectSpawnAndScaleFactors();
            return CalcProgress();
        case SPAWN_POINTS_AND_SCALE_FACTORS:
            CollectionFieldSpawnAndScaleFactors();
            return CalcProgress();
        case CONVERT_FIELDS:
            ConvertFieldsIteration();
            return CalcProgress();
        case WRITE_MAPS_INIT:
            write_output_line("Writing fields...");
            WriteMapsXmlBegin();
            return CalcProgress();
        case WRITE_MAPS:
            WriteMapsXmlIteration();
            return CalcProgress();
        case WRITE_MAPS_CLEAN:
            EndWriteMapsXml();
            return CalcProgress();
        case CONVERT_FIELD_MODELS_INIT:
            write_output_line("Converting field models...");
            ConvertFieldModelsBegin();
            return CalcProgress();
        case CONVERT_FIELD_MODELS:
            ConvertFieldModelsIteration();
            return CalcProgress();
        case DONE:
        default:
            return 100;
    }
}

static std::string FieldModelDir(){return "models/ffvii/field/units";}

std::vector<std::string> materials_;

/**
 * Converts a tex file to png.
 *
 * The image is saved in data/models/ffvii/field/units/
 *
 * @param[in] name Path to the tex file to convert.
 */
void TexToPng(const std::string name){
    Ogre::DataStreamPtr stream(
      Ogre::ResourceGroupManager::getSingleton().openResource(name, "FFVIITextures", true, NULL)
    );
    VGears::TexFile* tex = new VGears::TexFile();
    tex->Read(stream);
    Ogre::ImageCodec::ImageData* image_data = tex->GetImageData();
    Ogre::Image* image = new Ogre::Image(
      Ogre::PF_R8G8B8A8_UINT, image_data->width, image_data->height
    );
    for (int y = 0; y < image_data->height; y ++){
        for (int x = 0; x < image_data->width; x ++){
            // This is the position from which to read from the tex file colour data
            // TODO: It doesn't work for non-square images. For example, the lower part of bxje is
            // all wrong.
            int i = (image_data->height * y) + x;
            image->setColourAt(
              Ogre::ColourValue(
                tex->image_data_[i].comp.red / 255.0f,
                tex->image_data_[i].comp.green / 255.0f,
                tex->image_data_[i].comp.blue / 255.0f,
                tex->image_data_[i].comp.alpha),
              x, y, 0
            );
        }
    }
    Ogre::String base_name;
    VGears::StringUtil::splitBase(name, base_name);
    // TODO: Detect path
    image->save("data/models/ffvii/field/units/" + base_name + ".png");

}

/**
 * Exports a mesh to a file.
 *
 * The file will have the mesh name.
 *
 * @param[in] outdir Path to the directory where the file will be saved.
 * @param[in] mesh The mesh to export.
 */
static void ExportMesh(const std::string outdir, const Ogre::MeshPtr &mesh){

    // TODO: Share function with pc model exporter
    VGears::String base_mesh_name;
    VGears::StringUtil::splitFull(mesh->getName(), base_mesh_name);
    Ogre::MeshSerializer mesh_serializer;
    Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
    Ogre::SkeletonSerializer skeleton_serializer;
    skeleton_serializer.exportSkeleton(
      skeleton.getPointer(), outdir + base_mesh_name + ".skeleton"
    );
    mesh->setSkeletonName(FieldModelDir() + "/" + base_mesh_name + ".skeleton");
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
                                    TexToPng(unit->getTextureName());
                                    // Ensure the output material script references png files
                                    // rather than tex
                                    // files.
                                    Ogre::String base_name;
                                    VGears::StringUtil::splitBase(
                                      unit->getTextureName(), base_name
                                    );
                                    unit->setTextureName(
                                      FieldModelDir() + "/" + base_mesh_name
                                      + "_" + base_name + ".png"
                                    );
                                    // Copy subtexture (xxxx.png) to
                                    // model_xxxx.png
                                    // TODO: obtain the "data" folder
                                    // programatically.
                                    boost::filesystem::copy_file(
                                      "data/" + FieldModelDir() + "/" + base_name + ".png",
                                      "data/" + FieldModelDir() + "/"
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

/**
 * Normalizes a field name.
 *
 * It prefixes a name witg "ffvii_".
 *
 * @param[in] name The field name.
 * @return The normalized field name.
 */
static std::string FieldName(const std::string& name){return "ffvii_" + name;}

void DataInstaller::CreateDir(const std::string& path){
    QString target = QString::fromStdString(output_dir_ + path);
    QDir dir(target);
    if (!dir.mkpath(".")) throw std::runtime_error("Failed to mkpath");
}

/**
 * Retrieves the path to the field map directory.
 *
 * @return The relative path to the field map directory.
 */
static std::string FieldMapDir(){return "maps/ffvii/field";}

/**
 * Utilities for formatting field scripts.
 */
class BaseFF7FieldScriptFormatter : public FieldScriptFormatter{

    public:

        /**
         * Constructor.
         *
         * @param[in] field_name The field name.
         * @param[in] field_id_to_name_lookup Field name lookup table to use.
         * @param[in] spawn_points The list of spawn points.
         */
        BaseFF7FieldScriptFormatter(
          const std::string& field_name, const std::vector<std::string>& field_id_to_name_lookup,
          FieldSpawnPointsMap& spawn_points
        ) :
          field_name_(field_name), field_id_to_name_lookup_(field_id_to_name_lookup),
          spawn_points_(spawn_points)
        {}

        /**
         * Composes a spawn point name
         *
         * The name will have the format
         * {FIELD_NAME}_{ENTITY}_{FUNCTION_NAME}_addr_{ADDRESS}
         *
         * @param[in] map_id Unused.
         * @param[in] entity Entity name.
         * @param[in] function_name Name of the function.
         * @param[in] address @todo Understand and document.
         * @return The composed spawn point name.
         */
        virtual std::string GetSpawnPointName(
          unsigned int map_id, const std::string& entity,
          const std::string& function_name, unsigned int address
        ) override{
            return
              field_name_ + "_" + entity + "_" + function_name + "_addr_" + std::to_string(address);
        }

        /**
         * Retrieves a map name from it's ID.
         *
         * @param[in] map_id The map ID.
         * @return The map name.
         */
        virtual std::string GetMapName(unsigned int map_id) override{
            return FieldName(field_id_to_name_lookup_[map_id]);
        }

        /**
         * Retrieves a user friendly variable name.
         *
         * @param[in] bank Variable bank.
         * @param[in] addr Variable address.
         * @return Friendly name for the variable, or an empty string if it
         * doesn't have one assigned.
         */
        virtual std::string GetFriendlyVarName(
          unsigned int bank, unsigned int addr
        ) override{
            return VGears::NameLookup::FieldScriptVarName(bank, addr);
        }

        /**
         * Retrieves a user friendly entity name.
         *
         * @param[in] entity Entity name.
         * @return Friendly name for the entity, or the current name if it
         * doesn't have one assigned.
         */
        virtual std::string GetFriendlyEntityName(const std::string& entity) override{
            return VGears::NameLookup::FieldScriptEntityName(entity);
        }

        /**
         * Retrieves a user friendly character name.
         *
         * @param[in] char_id Character ID.
         * @return Friendly name for the entity, or an empty string if it
         * doesn't have one assigned.
         */
        virtual std::string GetFriendlyCharName(int char_id) override{
            return VGears::NameLookup::CharName(char_id);
        }

        /**
         * Retrieves a user friendly function name.
         *
         * @param[in] entity The name of the entity the function belongs to.
         * @param[in] function_name Function name.
         * @return Friendly name for the function, or the current name if it
         * doesn't have one assigned.
         */
        virtual std::string GetFriendlyFunctionName(
          const std::string& entity, const std::string& function_name
        ) override{
            return VGears::NameLookup::FieldScriptFunctionName(
              field_name_, entity, function_name
            );
        }

        /**
         * Retrieves the header comment for a function in an entity.
         *
         * @param[in] entity The name of the entity the function belongs to.
         * @param[in] function_name Function name.
         * @return Friendly comment for the function, or an empty string if it
         * doesn't have one assigned.
         */
        virtual std::string GetFunctionComment(
          const std::string& entity, const std::string& function_name
        ) override{
            return VGears::NameLookup::FieldScriptFunctionComment(
              field_name_, entity, function_name
            );
        }

    protected:

        /**
         * The field name.
         */
        std::string field_name_;

        /**
         * The list of spawn points.
         */
        FieldSpawnPointsMap& spawn_points_;

        /**
         * Look up table relatingfield IDs and names.
         */
        const std::vector<std::string>& field_id_to_name_lookup_;
};

/**
 * Handles the formatting of field scripts.
 */
class FF7FieldScriptFormatter : public BaseFF7FieldScriptFormatter{

    public:

        /**
         * Constructor.
         *
         * @param[in] field_name The field name.
         * @param[in] models The list of field models.
         * @param[in] field_id_to_name_lookup Field name lookup table to use.
         * @param[in] spawn_points The list of spawn points.
         */
        FF7FieldScriptFormatter(
          const std::string& field_name, const VGears::ModelListFilePtr& models,
          const std::vector<std::string>& field_id_to_name_lookup,
          FieldSpawnPointsMap& spawn_points
        ) :
          BaseFF7FieldScriptFormatter(field_name, field_id_to_name_lookup, spawn_points),
          model_loader_(models)
        {}

        // Names an animation, can't return empty
        /**
         * Retreieves an animation name from iths ID.
         *
         * @param[in] char_id ID of the character the animation belogs to.
         * @param[in] id Animation ID.
         * @return The animation name. If there is no name assigned, or it
         * can't be found, a string with the animation ID.
         */
        virtual std::string GetFriendlyAnimationName(int char_id, int id) override{
            // Get the animation file name, then look up the friendly name of
            // the "raw" animation.
            if (static_cast<unsigned int>(char_id) >= model_loader_->GetModels().size()){
                std::cerr << "FF7FieldScriptFormatter::AnimationName ERROR:"
                  << "Char ID " << char_id << " out of bounds" << std::endl;
                return std::to_string(id);
            }
            const auto& model_info = model_loader_->GetModels().at(char_id);
            if (static_cast<unsigned int>(id) >= model_info.animations.size()){
                std::cerr << "FF7FieldScriptFormatter::AnimationName ERROR:"
                  << "In field " << field_name_ << " the model " << model_info.name
                  << " animation with ID " << id << " is out of bounds ("
                  << model_info.animations.size() << ")" << std::endl;
                return std::to_string(id);
            }
            const auto raw_name = model_info.animations.at(id).name;
            // Trim off ".yos" or whatever extension the model loader adds in.
            Ogre::String base_name;
            VGears::StringUtil::splitBase(raw_name, base_name);
            VGears::StringUtil::toLowerCase(base_name);
            return VGears::NameLookup::Animation(base_name);
        }

    private:

        /**
         * The field model loader.
         */
        const VGears::ModelListFilePtr& model_loader_;
};

/**
 * Creates a gateway script.
 *
 * @param[in] gateway_entity_name Name of the gateway entity.
 * @param[in] target_map_name Name of the target map.
 * @param[in] source_spawn_point_name The spawn point name.
 * @return Gateway LUA script.
 */
static std::string CreateGateWayScript(
  const std::string& gateway_entity_name, const std::string& target_map_name,
  const std::string& source_spawn_point_name
){
    return
      "\nEntityContainer[ \"" + gateway_entity_name + "\" ] = {\n"
      "on_start = function(self)\n"
      "    return 0\n"
      "end,\n"
      "on_enter_line = function(self, entity)\n"
      "    return 0\n"
      "end,\n\n"
      "on_move_to_line = function(self, entity)\n"
      "   return 0\n"
      "end,\n\n"
      "on_cross_line = function(self, entity)\n"
      "    if entity == \"Cloud\" then\n"
      "        if not FFVII.Data.DisableGateways then\n"
      "            load_field_map_request(\""
      + target_map_name + "\", \"" + source_spawn_point_name + "\")\n"
      "        end\n"
      "    end\n\n"
      "    return 0\n"
      "end,\n\n"
      "on_leave_line = function(self, entity)\n"
      "   return 0\n"
      "end,\n"
      "}\n\n";

}

/**
 * Gateways whith map ID are considered to be inactive.
 */
const int INACTIVE_GATEWAY_ID = 32767;

/**
 * Retrieves a field ID from a name.
 *
 * @param[in] name The name of the field.
 * @param[in] field_id_to_name_lookup Lookup table to use.
 * @return The field ID for the specified name.
 * @throws std::runtime_error If there is no ID for the field name.
 */
static size_t FieldId(
  const std::string& name,
  const std::vector<std::string>& field_id_to_name_lookup
){
    for (size_t i = 0; i < field_id_to_name_lookup.size(); i ++)
        if (field_id_to_name_lookup[i] == name) return i;
    throw std::runtime_error("No Id found for field name");
}

/**
 * Retrieves a field scale factor.
 *
 * @param[in] scale_factor_map Lookup table of map scale factors.
 * @param[in] field_id The id (name) of the field.
 * @return The scale factor for the map.
 * @throws std::runtime_error If there is no ID for the field name.
 */
static float FieldScaleFactor(
  const FieldScaleFactorMap& scale_factor_map, size_t field_id
){
    auto it = scale_factor_map.find(field_id);
    if (it == std::end(scale_factor_map))
        throw std::runtime_error("Scale factor not found for field id");
    return it->second;
}

/**
 * Converts a FFVII PC field to a V-Gears field.
 *
 * @param[in] field_text_writter The field text writer.
 * @param[in] field The field map.
 * @param[in] out_dir The path to the directory where to save the map file.
 * @param[in] field_id_to_name_lookup Lookup table used to relate map IDs and
 * names
 * @param[in] spawn_map List of spawn points.
 * @param[in] scale_factor_map List of scale factors.
 * @param[in] model_animation_db Database of model animations.
 * @param[out] maps The list of maps. The converted one will be added at the
 * end.
 * @param[in] write_output_line Functon to print output to console.
 */
static void FF7PcFieldToVGearsField(
  FieldTextWriter& field_text_writter,
  VGears::FLevelFilePtr& field,
  const std::string& out_dir,
  const std::vector<std::string>& field_id_to_name_lookup,
  FieldSpawnPointsMap& spawn_map,
  const FieldScaleFactorMap& scale_factor_map,
  ModelsAndAnimationsDb& model_animation_db,
  MapCollection& maps,
  std::function<void(std::string)> write_output_line
){
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
              FieldName(field_id_to_name_lookup.at(gateway.destination_field_id)),
              "Spawn_" + field->getName() + "_" + std::to_string(i)
            );
        }
    }
    const VGears::ModelListFilePtr& models = field->GetModelList();
    FieldDecompiler::DecompiledScript decompiled;
    FF7FieldScriptFormatter formatter(field->getName(), models, field_id_to_name_lookup, spawn_map);
    try{
        // Get the raw script bytes.
        const std::vector<u8> raw_field_data = field->GetRawScript();
        // Decompile to LUA.
        decompiled = FieldDecompiler::Decompile(
          field->getName(), raw_field_data, formatter, gateway_script_data,
          "EntityContainer = {}\n\n"
        );
        std::ofstream script_file(
          out_dir + "/" + FieldMapDir() + "/" + field->getName() + "/script.lua"
        );
        if (script_file.is_open()){
            script_file << decompiled.luaScript;
            try{field_text_writter.Write(raw_field_data, field->getName());}
            catch (const std::out_of_range& ex){
                write_output_line(
                  "[ERROR] Failed to read texts from field " + field->getName() + ": " + ex.what()
                );
                std::cerr << "[ERROR] Failed to read texts from field "
                  << field->getName() << ": " << ex.what() << std::endl;
            }
        }
        else{
            write_output_line(
              "[ERROR] Failed to open script file from field " + field->getName() + " for writing."
            );
            std::cerr << "[ERROR] Failed to open script file from field "
              << field->getName() << "for writing." << std::endl;
        }
    }
    catch (const ::DecompilerException& ex){
        write_output_line(
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
          "file_name", FieldMapDir() + "/" + field->getName() + "/script.lua"
        );
        element->LinkEndChild(xml_script_element.release());
        // Background.
        std::unique_ptr<TiXmlElement> xml_background_2d(new TiXmlElement("background2d"));
        xml_background_2d->SetAttribute(
          "file_name", FieldMapDir() + "/" + field->getName() + "/bg.xml"
        );
        element->LinkEndChild(xml_background_2d.release());
        // Walkmesh.
        std::unique_ptr<TiXmlElement> xml_walkmesh(new TiXmlElement("walkmesh"));
        xml_walkmesh->SetAttribute(
          "file_name", FieldMapDir() + "/" + field->getName() + "/wm.xml"
        );
        element->LinkEndChild(xml_walkmesh.release());
        // Forward direction (angle player moves when "up" is pressed)
        std::unique_ptr<TiXmlElement> xml_forward_direction(new TiXmlElement("movement_rotation"));
        xml_forward_direction->SetAttribute(
          "degree", std::to_string(triggers->MovementRotation())
        );
        element->LinkEndChild(xml_forward_direction.release());
        // Get this fields Id.
        const size_t this_field_id = FieldId(field->getName(), field_id_to_name_lookup);
        // Use the ID to find the pre-computed list of gateways in all other fields that link to
        // this field.
        auto spawn_iterator = spawn_map.find(this_field_id);
        Ogre::Vector3 first_entity_point = Ogre::Vector3::ZERO;

        // If none found it probably just means no other fields have doors to this one.
        if (spawn_iterator != std::end(spawn_map)){
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
                      field_id_to_name_lookup.at(
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
                      "Spawn_"
                        + field_id_to_name_lookup.at(spawn_point_records[i].field_id)
                        + "_"
                        + std::to_string(spawn_point_records[i].gateway_index_or_map_jump_address)
                    );
                }
                const float downscaler_next = 128.0f * FieldScaleFactor(
                  scale_factor_map, gateway.destination_field_id
                );

                // Position Z is actually the target walkmesh triangle ID, so this is tiny bit more
                // complex. Now "get the Z value of the triangle with that ID". Note that ID
                // actually just means index.
                unsigned int triangle_index = static_cast<unsigned int>(gateway.destination.z);
                if (
                  triangle_index >= field->GetWalkmesh()->GetTriangles().size()
                ){
                    write_output_line(
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
              std::to_string(line.point_a[0] * DataInstaller::LINE_SCALE_FACTOR)
                + " " + std::to_string(line.point_a[1] * DataInstaller::LINE_SCALE_FACTOR)
                + " " + std::to_string(line.point_a[2] * DataInstaller::LINE_SCALE_FACTOR)
            );
            xml_entity_trigger->SetAttribute(
              "point2",
              std::to_string(line.point_b[0] * DataInstaller::LINE_SCALE_FACTOR)
                + " " + std::to_string(line.point_b[1] * DataInstaller::LINE_SCALE_FACTOR)
                + " " + std::to_string(line.point_b[2] * DataInstaller::LINE_SCALE_FACTOR)
            );
            xml_entity_trigger->SetAttribute("enabled", "true");
            element->LinkEndChild(xml_entity_trigger.release());
        }

        // Get entities.
        for (FieldDecompiler::FieldEntity entity : decompiled.entities){
            // If the entity has been added as a line, skip.
            if (line_entities.size() > 0){
                if (*std::find(line_entities.begin(), line_entities.end(), entity.name) == entity.name) {
                    continue;
                }
            }
            const int char_id = entity.char_id;
            if (char_id != -1){
                const VGears::ModelListFile::ModelDescription& desc
                  = models->GetModels().at(char_id);
                auto& animations = model_animation_db.ModelAnimations(desc.hrc_name);
                for (const auto& anim : desc.animations)
                    animations.insert(model_animation_db.NormalizeAnimationName(anim.name));
                std::unique_ptr<TiXmlElement> xml_entity_script(new TiXmlElement("entity_model"));
                xml_entity_script->SetAttribute("name", entity.name);
                // TODO: Add to list of HRC's to convert, obtain name of converted .mesh file.
                auto lower_case_hrc_name = desc.hrc_name;
                VGears::StringUtil::toLowerCase(lower_case_hrc_name);
                xml_entity_script->SetAttribute(
                  "file_name",
                  FieldModelDir() + "/" + model_animation_db.ModelMetaDataName(lower_case_hrc_name)
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
        const float downscaler_this = 128.0f * FieldScaleFactor(scale_factor_map, this_field_id);
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
        doc.LinkEndChild(element.release());
        doc.SaveFile(out_dir + "/" + FieldMapDir() + "/" + field->getName() + "/map.xml");
        const VGears::PaletteFilePtr& pal = field->GetPalette();
        const VGears::BackgroundFilePtr& bg = field->GetBackground();
        std::unique_ptr<Ogre::Image> bg_image(bg->CreateImage(pal));
        bg_image->encode("png");
        bg_image->save(out_dir + "/" + FieldMapDir() + "/" + field->getName() + "/tiles.png");
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
              = camera_matrix->GetPosition() / FieldScaleFactor(scale_factor_map, this_field_id);
            const Ogre::Quaternion orientation = camera_matrix->GetOrientation();
            const Ogre::Degree fov
              = camera_matrix->GetFov(static_cast<float>(BG_PSX_SCREEN_HEIGHT));
            const int min_x = triggers->GetCameraRange().left * BG_SCALE_UP_FACTOR;
            const int min_y = triggers->GetCameraRange().top * BG_SCALE_UP_FACTOR;
            const int max_x = triggers->GetCameraRange().right * BG_SCALE_UP_FACTOR;
            const int max_y = triggers->GetCameraRange().bottom * BG_SCALE_UP_FACTOR;
            bg_element->SetAttribute("image", FieldMapDir() + "/" + field->getName() + "/tiles.png");
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
            bg_doc.SaveFile(out_dir + "/" + FieldMapDir() + "/" + field->getName() + "/bg.xml");
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
        doc.SaveFile(out_dir + "/" + FieldMapDir() + "/" + field->getName() + "/wm.xml");
    }
    maps.insert(field->getName());
}

/**
 * Handles the script gateway colection.
 */
class FF7FieldScriptGatewayCollector : public BaseFF7FieldScriptFormatter{

    // TODO: Move declaration to a .h file.

    public:

        /**
         * Cosntructor.
         */
        FF7FieldScriptGatewayCollector(
          const std::string& field_name, const std::vector<std::string>& field_id_to_name_lookup,
          FieldSpawnPointsMap& spawn_points, size_t this_field_id
        ) :
          BaseFF7FieldScriptFormatter(field_name,field_id_to_name_lookup, spawn_points),
          field_id_(this_field_id)
        {}

        /**
         * Adds an spawn point.
         *
         * If the spawn is new, a new record will be added to the spawn point
         * database. If it was already there, update the record to add the
         * origin.
         *
         * @param[in] map_id The target map ID.
         * @param[in] entity The entity that acts as the spawn point.
         * @param[in] function_name The spawn function name.
         * @param[in] address @todo Understand and document.
         * @param[in] x X coordinate of the field target at which to spawn.
         * @param[in] y Y coordinate of the field target at which to spawn.
         * @param[in] z Z coordinate of the field target at which to spawn.
         * @param[in] angle Orientation at which to spawn.
         */
        virtual void AddSpawnPoint(
          unsigned int map_id, const std::string& entity, const std::string& function_name,
          unsigned int address, int x, int y, int z, int angle
        ) override {
            auto it = spawn_points_.find(map_id);
            VGears::TriggersFile::Gateway gw = {};
            gw.destination_field_id = map_id;
            gw.destination.x = x;
            gw.destination.y = y;
            gw.destination.z = z;
            gw.dir = angle;
            if (it != std::end(spawn_points_)){
                // Add to the list of gateways that link to destination_field_id.
                SpawnPointDb::Record rec;
                rec.from_script = true;
                rec.field_id = field_id_;
                rec.gateway = gw;
                rec.entity_name = entity;
                rec.script_function_name = function_name;
                rec.gateway_index_or_map_jump_address = address;
                it->second.gateways_to_this_field.push_back(rec);
            }
            else{
                // Create a new record for destination_field_id
                SpawnPointDb db;
                db.target_field_id = map_id;
                SpawnPointDb::Record rec;
                rec.from_script = true;
                rec.field_id = field_id_;
                rec.gateway = gw;
                rec.entity_name = entity;
                rec.script_function_name = function_name;
                rec.gateway_index_or_map_jump_address = address;
                db.gateways_to_this_field.push_back(rec);
                spawn_points_.insert(std::make_pair(db.target_field_id, db));
            }
        }

    private:

        /**
         * The field map ID.
         */
        size_t field_id_;

};

/**
 * Collects the spawn point from a field.
 *
 * If a spawn is new, a new record will be added to the spawn point database.
 * If it was already there, update the record to add the origin.
 *
 * @param[in] field The field to collect from.
 * @param[in] field_id_to_name_lookup Lookup table to relate field IDs and
 * names.
 * @param[out] spawn_points Spawn points will be added sequentially to the end
 * of this map.
 */
static void CollectSpawnPoints(
  VGears::FLevelFilePtr& field, const std::vector<std::string>& field_id_to_name_lookup,
  FieldSpawnPointsMap& spawn_points
){
    const size_t this_field_id = FieldId(field->getName(), field_id_to_name_lookup);
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

/**
 * Checks if a file is a field file.
 *
 * To verify it, it just checks that the extension is not '.tex', '.tut' or
 * '.siz' and that the name is not 'maplist'.
 *
 * @param[in] resource_name The name of the file to test.
 * @return False if the file is surely not a field file, true otherwise.
 */
static bool IsAFieldFile(const Ogre::String& resource_name){
    return (
      !VGears::StringUtil::endsWith(resource_name, ".tex")
      && !VGears::StringUtil::endsWith(resource_name, ".tut")
      && !VGears::StringUtil::endsWith(resource_name, ".siz")
      && resource_name != "maplist"
    );
}


/**
 * Checks if a map cause fatal crash bugs.
 *
 * It uses a hardcoded list of maps that are known to cause errors. This can be
 * removed when whatever is causing the crash(s) is fixed.
 *
 * NOTE: Even so, conversion of all models will fail with a bone index out of
 * bounds.
 *
 * @param[in] resource_name The name of the map file to test.
 * @return True if the map will crash, false otherwise.
 */
static bool WillCrash(const Ogre::String& resource_name){
    if (
      resource_name == "bugin1a" // crashes in back ground image generation
      || resource_name == "frcyo" // Hangs in decompliation
      || resource_name == "fr_e" // Background image crash
      || resource_name == "las4_2" // Background image crash
      || resource_name == "las4_3" // Background image crash
      || resource_name == "lastmap" // Background image crash
      || resource_name == "md_e1" // Background image crash
      || resource_name == "trnad_3" // Background image crash
    ){return true;}
    return false;
}

/**
 * Checks if a map is a test field.
 *
 * It uses a hardcoded list of maps that are known to be test maps. Unused maps
 * are considered to be test maps.
 *
 * @param[in] resource_name The name of the map file to test.
 * @return True if the map is a test map, false otherwise.
 */
static bool IsTestField(const Ogre::String& resource_name){
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

/**
 * Collects the scale factor from a map.
 *
 * @param[in] field The field to collect from.
 * @param[out] scale_factors The scale factor for the map will be set here, in
 * the appropiate map entry.
 * @param[in] field_id_to_name_lookup Lookup table to relate field IDs and
 * names.
 */
static void CollectFieldScaleFactors(
  VGears::FLevelFilePtr& field, FieldScaleFactorMap& scale_factors,
  const std::vector<std::string>& field_id_to_name_lookup
){
    scale_factors[FieldId(field->getName(), field_id_to_name_lookup)]
      = FieldDecompiler::ScaleFactor(field->GetRawScript());
}

void DataInstaller::InitCollectSpawnAndScaleFactors(){
    progress_step_num_elements_ = 1;
    CreateDir(FieldMapDir());
    CreateDir(FieldModelDir());
    // List what's in the LGP archive.
    flevel_file_list_ = application_.ResMgr()->listResourceNames("FFVIIFields", "*");
    // Load the map list field.
    VGears::MapListFilePtr map_list
      = VGears::MapListFileManager::GetSingleton().load(
        "maplist", "FFVIIFields"
      ).staticCast<VGears::MapListFile>();
    map_list_ = map_list->GetMapList();
    iterator_counter_ = 0;
    conversion_step_ = 0;
    installation_state_ = SPAWN_POINTS_AND_SCALE_FACTORS;
}

void DataInstaller::CollectionFieldSpawnAndScaleFactors(){
    // On the first pass collate required field information.
    progress_step_num_elements_ = flevel_file_list_->size();
    if (iterator_counter_ < flevel_file_list_->size()){
        auto resource_name = (*flevel_file_list_)[iterator_counter_];
        // Exclude things that are not fields.
        //if (IsAFieldFile(resource_name) /*&& IsTestField(resource_name)*/){
        // TODO: DEBUG: Only testing fields.
        if (IsAFieldFile(resource_name) && IsTestField(resource_name)){
            conversion_step_ ++;
            if (conversion_step_ == 1){
                field_ = VGears::LZSFLevelFileManager::GetSingleton().load(
                  resource_name, "FFVIIFields"
                ).staticCast<VGears::FLevelFile>();
                //write_output_line("Load field " + resource_name);
                return;
            }
            if (conversion_step_ == 2){
                //write_output_line("Read spawn points from scripts");
                CollectSpawnPoints(field_, map_list_, spawn_points_);
                return;
            }
            if (conversion_step_ == 3){
                //write_output_line("Read scale factor");
                CollectFieldScaleFactors(field_, scale_factors_, map_list_);
                conversion_step_ = 0;
                iterator_counter_ ++;
                return;
            }
        }
        else iterator_counter_ ++;
    }
    else{
        field_.reset();
        iterator_counter_ = 0;
        installation_state_ = CONVERT_FIELDS;
        field_text_writer_.Begin(output_dir_ + "/texts/english/dialogs_mission1.xml");
    }
}

void DataInstaller::ConvertFieldsIteration(){
    // Do the full conversion with the collated data.
    progress_step_num_elements_ = flevel_file_list_->size();
    if (iterator_counter_ < flevel_file_list_->size()){
        auto resource_name = (*flevel_file_list_)[iterator_counter_];
        // Exclude things that are not fields.
        if (IsAFieldFile(resource_name)){
            //if (/*IsTestField(resource_name) &&*/ !WillCrash(resource_name)){
            // TODO: DEBUG: Only test fields
            if (IsTestField(resource_name) && !WillCrash(resource_name)){
                //write_output_line("Converting field " + resource_name);
                std::cout << " - Converting field: " << resource_name << std::endl;
                CreateDir(FieldMapDir() + "/" + resource_name);
                VGears::FLevelFilePtr field
                  = VGears::LZSFLevelFileManager::GetSingleton().load(
                    resource_name, "FFVIIFields"
                  ).staticCast<VGears::FLevelFile>();
                FF7PcFieldToVGearsField(
                  field_text_writer_, field, output_dir_, map_list_, spawn_points_, scale_factors_,
                  used_models_and_anims_, converted_map_list_, write_output_line
                );
            }
            else{
                write_output_line(
                  "[ERROR] Skip field " + resource_name + " due to crash or hang issue."
                );
                std::cerr << "[ERROR] Skip field: " << resource_name
                  << " due to crash or hang issue." << std::endl;
            }
        }
        iterator_counter_ ++;
    }
    else{
        iterator_counter_ = 0;
        installation_state_ = WRITE_MAPS_INIT;
        field_text_writer_.End();
    }
}

void DataInstaller::WriteMapsXmlBegin(){
    // Write out maps.xml.
    progress_step_num_elements_ = 1;
    doc_ = std::make_unique<TiXmlDocument>();
    element_ = std::make_unique<TiXmlElement>("maps");
    iterator_counter_ = 0;
    installation_state_ = WRITE_MAPS;
    iterator_counter_ = 0;
    converted_map_list_iterator_ = converted_map_list_.begin();
}

void DataInstaller::WriteMapsXmlIteration(){
    // TODO: Probably need to inject "empty" and "test" fields.
    progress_step_num_elements_ = converted_map_list_.size();
    if (converted_map_list_iterator_ != converted_map_list_.end()){
        auto map = *converted_map_list_iterator_;
        write_output_line("Writing field " + FieldName(map));
        std::unique_ptr<TiXmlElement> xml_element(new TiXmlElement("map"));
        xml_element->SetAttribute("name", FieldName(map));
        xml_element->SetAttribute("file_name", FieldMapDir() + "/" + map + "/map.xml");
        element_->LinkEndChild(xml_element.release());
        iterator_counter_ ++;
        ++ converted_map_list_iterator_;
    }
    else installation_state_ = WRITE_MAPS_CLEAN;
}

void DataInstaller::EndWriteMapsXml(){
    progress_step_num_elements_ = 1;
    doc_->LinkEndChild(element_.release());
    doc_->SaveFile(output_dir_ + "/maps.xml");
    installation_state_ = CONVERT_FIELD_MODELS_INIT;
}

void DataInstaller::ConvertFieldModelsBegin(){
    // TODO: Convert models and animations in model_animation_db.
    progress_step_num_elements_ = 1;
    field_models_lgp_ = std::make_unique<ScopedLgp>(
      application_.getRoot(), input_dir_ + "field/char.lgp", "LGP", "FFVII"
    );
    field_model_file_list_ = application_.ResMgr()->listResourceNames("FFVII", "*");
    iterator_counter_ = 0;
    installation_state_ = CONVERT_FIELD_MODELS;
    iterator_counter_ = 0;
    conversion_step_ = 0;
    model_animation_map_iterator_ = used_models_and_anims_.map.begin();
}

void DataInstaller::ConvertFieldModelsIteration(){
    progress_step_num_elements_ = used_models_and_anims_.map.size();
    if (model_animation_map_iterator_ != used_models_and_anims_.map.end()){
        if (conversion_step_ == 0){
            write_output_line("Converting model " + model_animation_map_iterator_->first);
            conversion_step_ ++;
        }
        else{
            try{
                Ogre::ResourcePtr hrc = VGears::HRCFileManager::GetSingleton().load(
                    model_animation_map_iterator_->first, "FFVII"
                  );
                Ogre::String base_name;
                VGears::StringUtil::splitBase(model_animation_map_iterator_->first, base_name);
                auto mesh_name = VGears::NameLookup::model(base_name) + ".mesh";
                Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(mesh_name, "FFVII"));
                Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
                for (auto& anim : model_animation_map_iterator_->second){
                    VGears::AFileManager &afl_mgr(VGears::AFileManager::GetSingleton());
                    VGears::AFilePtr a = afl_mgr.load(anim, "FFVII").staticCast<VGears::AFile>();
                    // Convert the FF7 name to a more readable name set in the meta data.
                    VGears::StringUtil::splitBase(anim, base_name);
                    a->AddTo(skeleton, VGears::NameLookup::Animation(base_name));
                }
                ExportMesh(output_dir_ + "/" + FieldModelDir() + "/", mesh);
            }
            catch (const Ogre::Exception& ex){
                write_output_line(
                  "[ERROR] Ogre exception converting model "
                  + model_animation_map_iterator_->first + ": " + ex.what()
                );
                std::cerr << "[ERROR] Ogre exception converting model "
                  << model_animation_map_iterator_->first <<": " << ex.what() << std::endl;
            }
            catch (const std::exception& ex){
                write_output_line(
                  "[ERROR] Exception converting model "
                  + model_animation_map_iterator_->first + ": " + ex.what()
                );
                std::cerr << "[ERROR] Exception converting model "
                  << model_animation_map_iterator_->first << ": " << ex.what() << std::endl;
            }
            iterator_counter_ ++;
            ++ model_animation_map_iterator_;
            conversion_step_ = 0;
        }
    }
    else{
        std::cout << "Installation finished." << std::endl;
        write_output_line("Installation finished.");
        installation_state_ = DONE;
    }
}
