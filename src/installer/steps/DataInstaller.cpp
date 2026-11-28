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

#include <QtCore/QDir>
#include <boost/filesystem.hpp>
#include "DataInstaller.h"
#include "InstallLayout.h"
#include "InstallLog.h"
#include "installer/formats/DiskImage.h"
#include "formats/model/AFileManager.h"
#include "formats/model/HRCFileManager.h"
#include "formats/model/PFileManager.h"
#include "formats/model/RSDFileManager.h"

DataInstaller::DataInstaller(
  const DiskImage& disk_image, const std::string output_dir, AdvancedOptions options,
  std::function<void(std::string, int, bool)> write_output_line
)
#ifdef _DEBUG
  : application_("plugins_d.cfg", "resources_d.cfg", "logs/ogre_d.log"),
#else
  : application_("plugins.cfg", "resources.cfg", "logs/ogre.log"),
#endif
  disk_image_(disk_image), output_dir_(output_dir), data_dir_(InstallLayout::GameDir(output_dir)),
  options_(options), write_output_line_(write_output_line)
{
    // The installer only converts data, so it needs no render system, no window and no display.
    if (!application_.initOgreHeadless()) throw std::runtime_error("Ogre init failure");
    Ogre::Log* default_log(Ogre::LogManager::getSingleton().getDefaultLog());
    assert( default_log );
    default_log->setLogDetail(Ogre::LL_LOW);
    // Ogre's messages only go to its own log (logs/ogre.log, see Application::initOgreHeadless). Most are expected
    // while converting (meshes without a skeleton yet, materials without a render system), and in the terminal and
    // installer.log they hide the messages of the installer. The problems that V-Gears itself writes to that log
    // (LOG_ERROR and LOG_WARNING) are passed to the installation log.
    default_log->setDebugOutputEnabled(false);
    default_log->addListener(&ogre_problems_);

    // Assign weights.
    for (int i = IDLE; i < STATE_COUNT; i ++) step_weight_[i] = 1;
    step_weight_[IDLE] = 0;
    step_weight_[BATTLE_MODELS_CONVERT] = 2;
    step_weight_[SPELL_MODELS_CONVERT] = 2;
    step_weight_[EFFECT_MODELS_CONVERT] = 2;
    step_weight_[MEDIA_IMAGES] = 3;
    step_weight_[MEDIA_SOUNDS] = 8;
    step_weight_[MEDIA_MUSICS] = 9;
    step_weight_[MEDIA_MUSICS_HQ] = 2;
    step_weight_[FIELD_SPAWN_POINTS_AND_SCALE_FACTORS] = 3;
    step_weight_[FIELD_CONVERT] = 3;
    step_weight_[FIELD_WRITE] = 2;
    step_weight_[FIELD_CONVERT_MODELS] = 3;
}

DataInstaller::~DataInstaller(){
    Ogre::Log* default_log = Ogre::LogManager::getSingletonPtr() ? Ogre::LogManager::getSingleton().getDefaultLog() : nullptr;
    if (default_log) default_log->removeListener(&ogre_problems_);
}

void DataInstaller::OgreProblems::messageLogged(
  const Ogre::String& message, Ogre::LogMessageLevel, bool, const Ogre::String&, bool&
){
    if (message.compare(0, 7, "[ERROR]") == 0 || message.compare(0, 9, "[WARNING]") == 0) InstallLog::Get().Add(message);
}

const char* DataInstaller::StepGroup(int state){
    if (state >= FIELD_CONVERT_MODELS_INIT && state <= FIELD_CONVERT_MODELS)
        return "Field models";
    if (state >= FIELD_SPAWN_POINTS_AND_SCALE_FACTORS_INIT && state <= FIELD_WRITE_END)
        return "Field maps";
    if (state == WM_MODELS) return "World map models";
    if (state >= WM_INIT && state <= WM_SCRIPTS) return "World map";
    if (state >= MEDIA_MUSICS_INIT && state < FIELD_SPAWN_POINTS_AND_SCALE_FACTORS_INIT)
        return "Music";
    if (state >= MEDIA_SOUNDS_INIT && state < MEDIA_MUSICS_INIT) return "Sounds";
    if (state == MEDIA_IMAGES) return "Images";
    if (state >= KERNEL_PRICES && state < MEDIA_IMAGES) return "Kernel data";
    if (state >= SPELL_MODELS_INIT && state < KERNEL_PRICES) return "Spell models";
    if (state >= BATTLE_MODELS_INIT && state < SPELL_MODELS_INIT) return "Battle models";
    if (state >= BATTLE_SCENES_INIT && state < BATTLE_MODELS_INIT) return "Battle data";
    if (state == CLEAN) return "Cleanup";
    return "Preparation";
}

float DataInstaller::Progress(){
    // Report each part of the installation separately in the log.
    const char* group = StepGroup(installation_state_);
    if (current_group_ != group){
        current_group_ = group;
        InstallLog::Get().BeginStep(group);
    }
    if (installation_state_ == DONE) InstallLog::Get().EndStep();
    if (installation_state_ == DONE){
        Say("Installation complete");
        return 100;
    }
    // The steps of each part are together, in the order they are done.
    if (installation_state_ < BATTLE_SCENES_INIT) PreparationStep();
    else if (installation_state_ < BATTLE_MODELS_INIT) BattleDataStep();
    else if (installation_state_ < SPELL_MODELS_INIT) BattleModelStep();
    else if (installation_state_ < KERNEL_PRICES) SpellModelStep();
    else if (installation_state_ < MEDIA_IMAGES) KernelStep();
    else if (installation_state_ < MEDIA_SOUNDS_INIT) ImageStep();
    else if (installation_state_ < MEDIA_MUSICS_INIT) SoundStep();
    else if (installation_state_ < FIELD_SPAWN_POINTS_AND_SCALE_FACTORS_INIT) MusicStep();
    else if (installation_state_ < FIELD_CONVERT_MODELS_INIT) FieldStep();
    else if (installation_state_ < WM_INIT) FieldModelStep();
    else if (installation_state_ < CLEAN) WorldStep();
    else CleanStep();
    return CalcProgress();
}

void DataInstaller::Say(const std::string& message){write_output_line_(message, 2, true);}

void DataInstaller::Skip(const std::string& part, const InstallationSteps next){
    Say("Skipping " + part + " installation...");
    installation_state_ = next;
}

void DataInstaller::Repeat(const int substeps, const InstallationSteps next){
    substeps_ = substeps;
    cur_substep_ = 0;
    installation_state_ = next;
}

bool DataInstaller::NextSubstep(){
    ++ cur_substep_;
    return cur_substep_ >= substeps_;
}

void DataInstaller::PreparationStep(){
    switch (installation_state_){
        case IDLE:
            installation_state_ = CREATE_DIRECTORIES;
            break;
        case CREATE_DIRECTORIES:
            Say("Creating directories...");
            CreateDirectories();
            installation_state_ = INITIALIZE;
            break;
        default:
            Say("Initializing installers...");
            CreateInstallers();
            installation_state_ = BATTLE_SCENES_INIT;
    }
}

void DataInstaller::CreateInstallers(){
    if (disk_image_.getContentPath().empty())
        throw std::runtime_error("Input data directory is empty");
    kernel_installer_ = std::make_unique<KernelDataInstaller>(disk_image_);
    media_installer_ = std::make_unique<MediaDataInstaller>(
      disk_image_, output_dir_, options_.keep_originals
    );
    field_installer_ = std::make_unique<FieldDataInstaller>(
      disk_image_, output_dir_, [this](std::string message){Say(message);}
    );
    battle_installer_ = std::make_unique<BattleDataInstaller>(
      disk_image_, output_dir_, application_.ResMgr()
    );
    world_installer_ = std::make_unique<WorldInstaller>(
      disk_image_, output_dir_, application_.ResMgr()
    );
}

void DataInstaller::BattleDataStep(){
    switch (installation_state_){
        case BATTLE_SCENES_INIT:
            if (!options_.battle_data){
                Skip("battle data", BATTLE_MODELS_INIT);
                break;
            }
            Say("Parsing battle scenes...");
            Repeat(battle_installer_->InitializeScenes(), BATTLE_SCENES_PROCESS);
            break;
        case BATTLE_SCENES_PROCESS:
            cur_substep_ = battle_installer_->ProcessScene();
            if (cur_substep_ >= substeps_) installation_state_ = BATTLE_SCENES_WRITE_ATTACKS;
            break;
        case BATTLE_SCENES_WRITE_ATTACKS:
            Say("Saving attacks...");
            battle_installer_->WriteAttacks();
            Repeat(0, BATTLE_SCENES_WRITE_ENEMIES);
            break;
        case BATTLE_SCENES_WRITE_ENEMIES:
            Say("Saving enemies...");
            battle_installer_->WriteEnemies();
            Repeat(0, BATTLE_SCENES_WRITE_FORMATIONS);
            break;
        default:
            Say("Saving enemy formations...");
            battle_installer_->WriteFormations();
            Repeat(0, BATTLE_MODELS_INIT);
    }
}

void DataInstaller::BattleModelStep(){
    switch (installation_state_){
        case BATTLE_MODELS_INIT:
            // The spell and effect models are part of the battle models.
            if (!options_.battle_models){
                Skip("battle models", KERNEL_PRICES);
                break;
            }
            Say("Extracting battle models...");
            Repeat(battle_installer_->InitializeBattleModels(), BATTLE_MODELS_PROCESS);
            break;
        case BATTLE_MODELS_PROCESS:
            cur_substep_ = battle_installer_->ProcessBattleModel();
            if (cur_substep_ >= substeps_) installation_state_ = BATTLE_MODELS_CONVERT_INIT;
            break;
        case BATTLE_MODELS_CONVERT_INIT:
            UseModelParts("temp/battle_models/");
            Repeat(battle_installer_->ConvertBattleModelsInit(), BATTLE_MODELS_CONVERT);
            break;
        case BATTLE_MODELS_CONVERT:
            cur_substep_ = battle_installer_->ConvertBattleModel();
            if (cur_substep_ >= substeps_) installation_state_ = BATTLE_MODELS_WRITE_CHARACTERS;
            break;
        case BATTLE_MODELS_WRITE_CHARACTERS:
            Say("Writing character data...");
            battle_installer_->WriteCharacterData();
            installation_state_ = BATTLE_MODELS_WRITE_SCENES;
            break;
        default:
            Say("Writing battle scene data...");
            battle_installer_->WriteSceneData();
            installation_state_ = SPELL_MODELS_INIT;
    }
}

void DataInstaller::SpellModelStep(){
    switch (installation_state_){
        case SPELL_MODELS_INIT:
            Say("Extracting attack models...");
            Repeat(battle_installer_->InitializeSpellModels(), SPELL_MODELS_PROCESS);
            break;
        case SPELL_MODELS_PROCESS:
            cur_substep_ = battle_installer_->ProcessSpellModel();
            if (cur_substep_ >= substeps_) installation_state_ = SPELL_MODELS_CONVERT_INIT;
            break;
        case SPELL_MODELS_CONVERT_INIT:
            UseModelParts("temp/spell_models/");
            Repeat(battle_installer_->ConvertSpellModelsInit(), SPELL_MODELS_CONVERT);
            break;
        case SPELL_MODELS_CONVERT:
            cur_substep_ = battle_installer_->ConvertSpellModel();
            if (cur_substep_ >= substeps_) installation_state_ = EFFECT_MODELS_INIT;
            break;
        case EFFECT_MODELS_INIT:
            Say("Extracting battle effects...");
            UseModelParts("temp/effects/");
            Repeat(battle_installer_->InitializeEffectModels(), EFFECT_MODELS_CONVERT);
            break;
        default:
            cur_substep_ = battle_installer_->ConvertEffectModel();
            if (cur_substep_ >= substeps_){
                battle_installer_->WriteEffectModels();
                installation_state_ = KERNEL_PRICES;
            }
    }
}

void DataInstaller::KernelStep(){
    const std::string dir = data_dir_ + "kernel/";
    KernelDataInstaller* kernel = kernel_installer_.get();
    switch (installation_state_){
        case KERNEL_PRICES:
            if (!options_.kernel){
                Skip("kernel data", MEDIA_IMAGES);
                return;
            }
            Say("Parsing item and materia prices...");
            kernel->ReadPrices();
            kernel->WriteShops(dir + "shops.lua");
            break;
        case KERNEL_COMMANDS:
            Say("Extracting command data...");
            kernel->ReadCommands();
            kernel->WriteCommands(dir + "commands.lua");
            break;
        case KERNEL_ATTACKS:
            Say("Extracting attack data...");
            kernel->ReadAttacks();
            kernel->WriteAttacks(dir + "attacks.lua");
            break;
        case KERNEL_CHARACTERS:
            Say("Extracting character data...");
            kernel->ReadCharacters();
            kernel->WriteCharacters(dir + "characters.lua");
            break;
        case KERNEL_ITEMS:
            Say("Extracting item data...");
            kernel->ReadItems();
            kernel->WriteItems(dir + "items.lua");
            break;
        case KERNEL_GROWTH:
            Say("Extracting growth data...");
            kernel->ReadGrowth();
            kernel->WriteGrowth(dir + "growth.lua");
            break;
        case KERNEL_WEAPONS:
            Say("Extracting weapon data...");
            kernel->ReadWeapons();
            kernel->WriteWeapons(dir + "weapons.lua");
            break;
        case KERNEL_ARMORS:
            Say("Extracting armor data...");
            kernel->ReadArmors();
            kernel->WriteArmors(dir + "armors.lua");
            break;
        case KERNEL_ACCESSORIES:
            Say("Extracting accessory data...");
            kernel->ReadAccessories();
            kernel->WriteAccessories(dir + "accessories.lua");
            break;
        case KERNEL_MATERIA:
            Say("Extracting materia data...");
            kernel->ReadMateria();
            kernel->WriteMateria(dir + "materia.lua");
            break;
        case KERNEL_KEY_ITEMS:
            Say("Extracting key item data...");
            kernel->ReadKeyItems();
            kernel->WriteKeyItems(dir + "key_items.lua");
            break;
        case KERNEL_SUMMON_NAMES:
            Say("Extracting summon attack names data...");
            kernel->ReadSummonNames();
            kernel->WriteSummonNames(dir + "summons.lua");
            break;
        default:
            WriteInitialSaveMap();
    }
    // The kernel steps are done one after the other.
    installation_state_ = static_cast<InstallationSteps>(installation_state_ + 1);
}

void DataInstaller::WriteInitialSaveMap(){
    Say("Extracting the initial savemap...");
    kernel_installer_->ReadInitialSaveMap();
    const auto exe_path = disk_image_.filePath("ff7.exe");
    if (exe_path) kernel_installer_->ReadRecruitRecords(*exe_path);
    else Say("ff7.exe not found, Cait Sith and Vincent won't have their data.");
    kernel_installer_->WriteInitialSaveMap(data_dir_ + "kernel/initial_savemap.lua");
}

void DataInstaller::ImageStep(){
    // The fonts are always needed, even if the rest of the images are skipped.
    Say("Extracting fonts...");
    if (!media_installer_->InstallFonts())
        Say("WARNING: Couldn't read WINDOW.BIN, the game won't be able to show text.");
    if (!options_.images){
        Skip("images", MEDIA_SOUNDS_INIT);
        return;
    }
    Say("Extracting game images...");
    media_installer_->InstallSprites();
    installation_state_ = MEDIA_SOUNDS_INIT;
}

void DataInstaller::SoundStep(){
    switch (installation_state_){
        case MEDIA_SOUNDS_INIT:
            if (!options_.sounds){
                Skip("sound effects", MEDIA_MUSICS_INIT);
                break;
            }
            Say("Extracting sounds...");
            Repeat(media_installer_->InstallSoundsInit(), MEDIA_SOUNDS);
            break;
        case MEDIA_SOUNDS:
            if (media_installer_->InstallSounds()) installation_state_ = MEDIA_SOUNDS_INDEX;
            else ++ cur_substep_;
            break;
        default:
            // The index is written when the conversions, which run in the background, have
            // finished.
            installation_state_ = MEDIA_MUSICS_INIT;
    }
}

void DataInstaller::MusicStep(){
    switch (installation_state_){
        case MEDIA_MUSICS_INIT:
            if (!options_.music){
                Skip("music tracks", FIELD_SPAWN_POINTS_AND_SCALE_FACTORS_INIT);
                break;
            }
            Say("Extracting music...");
            Repeat(media_installer_->InstallMusicsInit(), MEDIA_MUSICS);
            break;
        case MEDIA_MUSICS:
            if (media_installer_->InstallMusics()) installation_state_ = MEDIA_MUSICS_HQ;
            else ++ cur_substep_;
            break;
        case MEDIA_MUSICS_HQ:
            media_installer_->InstallHQMusics();
            Repeat(0, MEDIA_MUSICS_INDEX);
            break;
        default:
            // The index is written when the conversions, which run in the background, have
            // finished.
            installation_state_ = FIELD_SPAWN_POINTS_AND_SCALE_FACTORS_INIT;
    }
}

void DataInstaller::FieldStep(){
    switch (installation_state_){
        case FIELD_SPAWN_POINTS_AND_SCALE_FACTORS_INIT:
            if (!options_.fields && !options_.wm){
                Skip("field maps", WM_INIT);
                break;
            }
            // The world map scripts need the field list and scale factors even if the fields
            // aren't converted.
            if (!options_.fields) Say("Collecting field data for the world map...");
            else Say("Collecting spawn points and scale factors...");
            Repeat(
              field_installer_->CollectSpawnAndScaleFactorsInit(application_.ResMgr()),
              FIELD_SPAWN_POINTS_AND_SCALE_FACTORS
            );
            break;
        case FIELD_SPAWN_POINTS_AND_SCALE_FACTORS:
            field_installer_->CollectSpawnAndScaleFactors(cur_substep_);
            if (!NextSubstep()) break;
            cur_substep_ = 0;
            if (options_.fields) installation_state_ = FIELD_CONVERT;
            else{
                // Fields are not converted: use the ones already installed.
                field_installer_->LoadInstalledFields();
                Skip("field maps", WM_INIT);
            }
            break;
        case FIELD_CONVERT:
            field_installer_->Convert(cur_substep_);
            if (NextSubstep()){
                cur_substep_ = 0;
                installation_state_ = FIELD_WRITE_INIT;
            }
            break;
        case FIELD_WRITE_INIT:
            Say("Writing fields...");
            Repeat(field_installer_->WriteInit(), FIELD_WRITE);
            break;
        case FIELD_WRITE:
            field_installer_->Write(cur_substep_);
            if (NextSubstep()){
                cur_substep_ = 0;
                installation_state_ = FIELD_WRITE_END;
            }
            break;
        default:
            field_installer_->WriteEnd();
            installation_state_ = FIELD_CONVERT_MODELS_INIT;
    }
}

void DataInstaller::FieldModelStep(){
    if (installation_state_ == FIELD_CONVERT_MODELS_INIT){
        if (!options_.field_models){
            Skip("field models", WM_INIT);
            return;
        }
        Say("Converting field models...");
        UseModelParts("temp/char/");
        field_model_names_ = field_installer_->ConvertModelsInit();
        Repeat(static_cast<int>(field_model_names_.size()), FIELD_CONVERT_MODELS);
        return;
    }
    // There are no models if no field was converted.
    if (cur_substep_ < substeps_)
        field_installer_->ConvertModels(field_model_names_[cur_substep_]);
    if (NextSubstep()){
        cur_substep_ = 0;
        installation_state_ = WM_INIT;
    }
}

void DataInstaller::WorldStep(){
    switch (installation_state_){
        case WM_INIT:
            if (!options_.wm){
                Skip("world map data", CLEAN);
                break;
            }
            Say("Extracting world map data...");
            UseModelParts("temp/world_models/");
            world_installer_->InstallMaps();
            Repeat(0, WM_SCRIPTS);
            break;
        case WM_SCRIPTS:{
            std::string error;
            if (world_installer_->GenerateScripts(
              field_installer_->GetMapList(), field_installer_->GetFieldScaleFactors(),
              field_installer_->GetConvertedMapList(), error
            )) Say("Generated world-map scripts.");
            else Say("Could not generate world-map scripts: " + error);
            installation_state_ = WM_MODELS;
            break;
        }
        default:
            if (!options_.wm_models) Skip("world map model", CLEAN);
            else{
                world_installer_->ProcessModels();
                installation_state_ = CLEAN;
            }
    }
}

void DataInstaller::CleanStep(){
    // The audio conversions run in the background while the other steps are done.
    if (media_installer_){
        Say("Waiting for the audio conversions to finish...");
        media_installer_->WaitForConversions();
        // Now the files that exist are known, so the indexes list only those.
        if (options_.sounds){
            Say("Building sound index...");
            media_installer_->WriteSoundIndex();
        }
        if (options_.music){
            Say("Building music track index...");
            media_installer_->WriteMusicsIndex();
        }
    }
    Say("Cleaning up...");
    if (!options_.keep_originals) RemoveOriginals();
    installation_state_ = DONE;
}

const float DataInstaller::CalcProgress(){
    // The end is reported when the next call finds the installation done.
    if (installation_state_ >= STATE_COUNT) return 99.9f;
    float weight_total = 0; // Sum of every step weight.
    float weight_done = 0; // Sum of every previous step weight.
    float weight = static_cast<float>(step_weight_[installation_state_]); // Weight of current step.
    float weight_substeps = 0; // Weight of the already done substeps in current step.

    for (int i = IDLE; i < STATE_COUNT; i ++){
        weight_total += step_weight_[i];
        if (installation_state_ > i) weight_done += step_weight_[i];
    }

    if (substeps_ > 0 && cur_substep_ < substeps_) weight_substeps = (cur_substep_ * weight) / substeps_;

    float progress = 100 * (weight_done + weight_substeps) / weight_total;
    if (progress >= 100.0f) progress = 99.9f;
    return progress;
}

void DataInstaller::UseModelParts(const std::string& temp_dir){
    Ogre::ResourceGroupManager& res_mgr = Ogre::ResourceGroupManager::getSingleton();
    // Only the files of the parts are released: each of them removes what it made (an HRC file its mesh and its
    // skeleton, an RSD file its materials). Ogre's clearResourceGroup can't be used: it removes those too, and then
    // their owners fail to remove them.
    std::vector<Ogre::ResourceManager*> managers = {
      &VGears::HRCFileManager::GetSingleton(), &VGears::RSDFileManager::GetSingleton(),
      &VGears::PFileManager::GetSingleton(), &VGears::AFileManager::GetSingleton()
    };
    for (Ogre::ResourceManager* manager : managers){
        std::vector<Ogre::ResourceHandle> handles;
        auto it = manager->getResourceIterator();
        while (it.hasMoreElements()){
            const Ogre::ResourcePtr resource = it.getNext();
            if (resource->getGroup() == "FFVII") handles.push_back(resource->getHandle());
        }
        for (const Ogre::ResourceHandle handle : handles){
            if (manager->getByHandle(handle)) manager->remove(handle);
        }
    }
    for (const std::string& dir : {"temp/char/", "temp/battle_models/", "temp/spell_models/", "temp/effects/", "temp/world_models/"}){
        if (dir != temp_dir) res_mgr.removeResourceLocation(output_dir_ + dir, "FFVII");
    }
}

void DataInstaller::CreateDirectories(){
    for (const std::string& dir : InstallLayout::GameSubdirs()) CreateDir(dir);
    CreateDir("temp", true);
    for (const std::string& dir : InstallLayout::TempSubdirs()) CreateDir("temp/" + dir, true);

    application_.ResMgr()->addResourceLocation(output_dir_ + "temp/char/", "FileSystem", "FFVII", true, true);
    application_.ResMgr()->addResourceLocation(output_dir_ + "temp/battle_models/", "FileSystem", "FFVII", true, true);
    application_.ResMgr()->addResourceLocation(output_dir_ + "temp/world_models/", "FileSystem", "FFVII", true, true);
    for (const char* dir : {"field/models/", "battle/models/", "world/models/"})
        application_.ResMgr()->addResourceLocation(data_dir_ + dir, "FileSystem", "FFVII", true, true);
    application_.ResMgr()->addResourceLocation(
      data_dir_ + "field/models/", "FileSystem", "FFVIITextures", true, true
    );
    application_.ResMgr()->addResourceLocation(
      data_dir_ + "battle/models/characters/", "FileSystem", "FFVIITextures", true, true
    );
    application_.ResMgr()->addResourceLocation(
      data_dir_ + "battle/models/scenes/", "FileSystem", "FFVIITextures", true, true
    );
    application_.ResMgr()->addResourceLocation(
      data_dir_ + "battle/models/enemies/", "FileSystem", "FFVIITextures", true, true
    );
    application_.ResMgr()->addResourceLocation(
      data_dir_ + "battle/models/attacks/", "FileSystem", "FFVIITextures", true, true
    );
    application_.ResMgr()->addResourceLocation(
      data_dir_ + "world/models/", "FileSystem", "FFVIITextures", true, true
    );
    const auto fields_lgp_path = disk_image_.filePath("data/field/flevel.lgp");
    if (!fields_lgp_path) throw std::runtime_error("Missing required file: data/field/flevel.lgp");
    fields_lgp_ = std::make_unique<ScopedLgp>(application_.getRoot(), *fields_lgp_path, "LGP", "FFVIIFields");
}

void DataInstaller::RemoveOriginals(){
    boost::system::error_code ec;
    boost::filesystem::remove_all(InstallLayout::TempDir(output_dir_), ec);
    boost::filesystem::remove_all(InstallLayout::OriginalDataDir(output_dir_), ec);
}

void DataInstaller::CreateDir(const std::string& path, bool in_root){
    QString target = QString::fromStdString((in_root ? output_dir_ : data_dir_) + path);
    QDir dir(target);
    if (!dir.mkpath(".")) throw std::runtime_error("Failed to mkpath");
}
