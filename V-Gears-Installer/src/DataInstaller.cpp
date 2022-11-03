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
#include "DataInstaller.h"

float DataInstaller::LINE_SCALE_FACTOR = 0.0078124970964f;

DataInstaller::DataInstaller(
  const std::string input_dir, const std::string output_dir,
  std::function<void(std::string, int, bool)> write_output_line
)
#ifdef _DEBUG
    : application_("plugins_d.cfg", "resources_d.cfg", "install_d.log"),
#else
  : application_("plugins.cfg", "resources.cfg", "install.log"),
#endif
  input_dir_(input_dir), output_dir_(output_dir),
  write_output_line_(write_output_line)
{
    if (!application_.initOgre(true)) throw std::runtime_error("Ogre init failure");
    Ogre::Log* default_log( Ogre::LogManager::getSingleton().getDefaultLog());
    assert( default_log );
    default_log->setLogDetail(Ogre::LL_LOW);

}

DataInstaller::~DataInstaller(){}

int DataInstaller::Progress(){
    switch (installation_state_){
        case IDLE:
            installation_state_ = CREATE_DIRECTORIES;
            return CalcProgress();
        case CREATE_DIRECTORIES:
            write_output_line_("Creating directories...", 2, true);
            CreateDirectories();
            installation_state_ = INITIALIZE;
            return CalcProgress();
        case INITIALIZE:
            write_output_line_("Initializing installers...", 2, true);
            kernel_installer_ = std::make_unique<KernelDataInstaller>(input_dir_);
            media_installer_ = std::make_unique<MediaDataInstaller>(input_dir_, output_dir_);
            field_installer_ = std::make_unique<FieldDataInstaller>(input_dir_, output_dir_);
            installation_state_ = KERNEL_PRICES;
            return CalcProgress();
        case KERNEL_PRICES:
            write_output_line_("Parsing item and materia prices...", 2, true);
            kernel_installer_->ReadPrices();
            installation_state_ = KERNEL_COMMANDS;
            return CalcProgress();
        case KERNEL_COMMANDS:
            write_output_line_("Extracting command data...", 2, true);
            kernel_installer_->ReadCommands();
            kernel_installer_->WriteCommands(output_dir_ + "game/commands.lua");
            installation_state_ = KERNEL_ATTACKS;
            return CalcProgress();
        case KERNEL_ATTACKS:
            write_output_line_("Extracting attack data...", 2, true);
            kernel_installer_->ReadAttacks();
            kernel_installer_->WriteAttacks(output_dir_ + "game/attacks.lua");
            installation_state_ = KERNEL_CHARACTERS;
            return CalcProgress();
        case KERNEL_CHARACTERS:
            write_output_line_("Extracting character data...", 2, true);
            kernel_installer_->ReadCharacters();
            kernel_installer_->WriteCharacters(output_dir_ + "game/characters.lua");
            installation_state_ = KERNEL_ITEMS;
            return CalcProgress();
        case KERNEL_ITEMS:
            write_output_line_("Extracting item data...", 2, true);
            kernel_installer_->ReadItems();
            kernel_installer_->WriteItems(output_dir_ + "game/items.lua");
            installation_state_ = KERNEL_GROWTH;
            return CalcProgress();
        case KERNEL_GROWTH:
            write_output_line_("Extracting growth data...", 2, true);
            kernel_installer_->ReadGrowth();
            kernel_installer_->WriteGrowth(output_dir_ + "game/growth.lua");
            installation_state_ = KERNEL_WEAPONS;
            return CalcProgress();
        case KERNEL_WEAPONS:
            write_output_line_("Extracting weapon data...", 2, true);
            kernel_installer_->ReadWeapons();
            kernel_installer_->WriteWeapons(output_dir_ + "game/weapons.lua");
            installation_state_ = KERNEL_ARMORS;
            return CalcProgress();
        case KERNEL_ARMORS:
            write_output_line_("Extracting armor data...", 2, true);
            kernel_installer_->ReadArmors();
            kernel_installer_->WriteArmors(output_dir_ + "game/armors.lua");
            installation_state_ = KERNEL_ACCESSORIES;
            return CalcProgress();
        case KERNEL_ACCESSORIES:
            write_output_line_("Extracting accessory data...", 2, true);
            kernel_installer_->ReadAccessories();
            kernel_installer_->WriteAccessories(output_dir_ + "game/accessories.lua");
            installation_state_ = KERNEL_MATERIA;
            return CalcProgress();
        case KERNEL_MATERIA:
            write_output_line_("Extracting materia data...", 2, true);
            kernel_installer_->ReadMateria();
            kernel_installer_->WriteMateria(output_dir_ + "game/materia.lua");
            installation_state_ = KERNEL_KEY_ITEMS;
            return CalcProgress();
        case KERNEL_KEY_ITEMS:
            write_output_line_("Extracting key item data...", 2, true);
            kernel_installer_->ReadKeyItems();
            kernel_installer_->WriteKeyItems(output_dir_ + "game/key_items.lua");
            installation_state_ = KERNEL_SUMMON_NAMES;
            return CalcProgress();
        case KERNEL_SUMMON_NAMES:
            write_output_line_("Extracting summon attack names data...", 2, true);
            kernel_installer_->ReadSummonNames();
            kernel_installer_->WriteSummonNames(output_dir_ + "game/summons.lua");
            installation_state_ = KERNEL_SAVEMAP;
            return CalcProgress();
        case KERNEL_SAVEMAP:
            write_output_line_("Extracting the initial savemap...", 2, true);
            kernel_installer_->ReadInitialSaveMap();
            kernel_installer_->WriteInitialSaveMap(output_dir_ + "game/initial_savemap.lua");
            installation_state_ = MEDIA_IMAGES;
            return CalcProgress();
        case MEDIA_IMAGES:
            write_output_line_("Extracting game images...", 2, true);
            media_installer_->InstallSprites();
            installation_state_ = MEDIA_SOUNDS;
            return CalcProgress();
        case MEDIA_SOUNDS:
            write_output_line_("Extracting sounds...", 2, true);
            media_installer_->InstallSounds();
            installation_state_ = MEDIA_SOUNDS_INDEX;
            return CalcProgress();
        case MEDIA_SOUNDS_INDEX:
            write_output_line_("Building sound index...", 2, true);
            media_installer_->WriteSoundIndex();
            installation_state_ = FIELD_SPAWN_POINTS_AND_SCALE_FACTORS_INIT;
            return CalcProgress();
        case FIELD_SPAWN_POINTS_AND_SCALE_FACTORS_INIT:
            write_output_line_("Collecting spawn points and scale factors...", 2, true);
            substeps_ = field_installer_->CollectSpawnAndScaleFactorsInit(application_.ResMgr());
            cur_substep_ = 0;
            installation_state_ = FIELD_SPAWN_POINTS_AND_SCALE_FACTORS;
            return CalcProgress();
        case FIELD_SPAWN_POINTS_AND_SCALE_FACTORS:
            field_installer_->CollectSpawnAndScaleFactors(cur_substep_);
            cur_substep_ ++;
            if (cur_substep_ == substeps_){
                installation_state_ = FIELD_CONVERT;
                cur_substep_ = 0;
            }
            return CalcProgress();
        case FIELD_CONVERT:
            field_installer_->Convert(cur_substep_);
            cur_substep_ ++;
            if (cur_substep_ == substeps_){
                installation_state_ = FIELD_WRITE_INIT;
                cur_substep_ = 0;
            }
            return CalcProgress();
        case FIELD_WRITE_INIT:
            write_output_line_("Writing fields...", 2, true);
            substeps_ = field_installer_->WriteInit();
            cur_substep_ = 0;
            installation_state_ = FIELD_WRITE;
            return CalcProgress();
        case FIELD_WRITE:
            field_installer_->Write(cur_substep_);
            cur_substep_ ++;
            if (cur_substep_ == substeps_){
                installation_state_ = FIELD_WRITE_END;
                cur_substep_ = 0;
            }
            return CalcProgress();
        case FIELD_WRITE_END:
            field_installer_->WriteEnd();
            installation_state_ = FIELD_CONVERT_MODELS_INIT;
            return CalcProgress();
        case FIELD_CONVERT_MODELS_INIT:
            write_output_line_("Converting field models...", 2, true);
            field_model_names_ = field_installer_->ConvertModelsInit();
            substeps_ = field_model_names_.size();
            cur_substep_ = 0;
            installation_state_ = FIELD_CONVERT_MODELS;
            return CalcProgress();
        case FIELD_CONVERT_MODELS:
            field_installer_->ConvertModels(field_model_names_[cur_substep_]);
            cur_substep_ ++;
            if (cur_substep_ == substeps_){
                installation_state_ = CLEAN;
                cur_substep_ = 0;
            }
            return CalcProgress();
        case CLEAN:
            CleanInstall();
            installation_state_ = DONE;
            return CalcProgress();
        case DONE:
            write_output_line_("Installation complete", 2, true);
            return 100;
        default:
            return 100;
    }
}

const int DataInstaller::CalcProgress(){
    // TODO: Make more accurate with iterator_counter_ and
    // progress_step_num_elements_.
    float curr_step = installation_state_ / static_cast<float>(STATE_COUNT);
    curr_step = curr_step * 100.0f;
    int progress = static_cast<int>(curr_step);
    if (progress >= 100) progress = 99;
    return progress;
}

void DataInstaller::CreateDirectories(){
    CreateDir("temp");
    CreateDir("game");
    CreateDir("images/icons");
    CreateDir("images/other");
    CreateDir("images/characters");
    CreateDir("images/fonts");
    CreateDir("images/reels");
    CreateDir("images/window");
    CreateDir("models/fields/entities");
    CreateDir("audio/fx");
    CreateDir("audio/music");
    application_.ResMgr()->addResourceLocation("data/temp/char/", "FileSystem", "FFVII", true, true);
    application_.ResMgr()->addResourceLocation("data/models/", "FileSystem", "FFVII", true, true);
    fields_lgp_ = std::make_unique<ScopedLgp>(
      application_.getRoot(), input_dir_ + "data/field/flevel.lgp", "LGP", "FFVIIFields"
    );
}

void DataInstaller::CleanInstall(){
    // TODO: Remove 'data/temp' directory.
    // Either implement an option to keep it, o do it when the installer is mature enough.
}

void DataInstaller::CreateDir(const std::string& path){
    QString target = QString::fromStdString(output_dir_ + path);
    QDir dir(target);
    if (!dir.mkpath(".")) throw std::runtime_error("Failed to mkpath");
}
