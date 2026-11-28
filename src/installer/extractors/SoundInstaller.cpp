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

#include <cstdio>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <tinyxml.h>
#include "installer/extractors/SoundInstaller.h"
#include "installer/extractors/SfxDump.h"
#include "installer/tables/SoundNames.h"

const int SoundInstaller::TOTAL_SOUNDS = 750;

SoundInstaller::SoundInstaller(
  DiskImage& disk_image, const std::string& data_dir, const bool keep_originals,
  MediaConverter& converter
):
  disk_image_(disk_image), data_dir_(data_dir), keep_originals_(keep_originals),
  converter_(converter)
{}

int SoundInstaller::InstallSoundsInit(){
    std::unique_ptr<std::string> fmt_path = disk_image_.filePath("data/sound/audio.fmt");
    std::unique_ptr<std::string> data_path = disk_image_.filePath("data/sound/audio.dat");
    std::string sound_dir =
      (boost::filesystem::path(data_dir_) / "audio" / "sounds").string();

    if (!fmt_path){
        std::cerr << "MediaDataInstaller: could not find data/sound/audio.fmt in the selected game data."
                  << std::endl;
    }
    else if (!data_path){
        std::cerr << "MediaDataInstaller: could not find data/sound/audio.dat in the selected game data."
                  << std::endl;
    }
    else if (SfxDumpConvert(fmt_path->c_str(), data_path->c_str(), sound_dir.c_str()) != 0){
        std::cerr << "MediaDataInstaller: failed to convert FFVII sound effects."
                  << std::endl;
    }
    processed_sounds_ = 0;
    return TOTAL_SOUNDS;
}

bool SoundInstaller::InstallSounds(){
    if (converter_.ToolsReady(false)){
        const std::string wav = data_dir_ + "audio/sounds/" + std::to_string(processed_sounds_) + ".wav";
        const std::string ogg = data_dir_ + "audio/sounds/" + std::to_string(processed_sounds_) + ".ogg";
        std::ifstream file(wav);
        if (file.is_open()){
            file.close();
            // Converted in the background. The wav file is removed once it has been converted.
            const std::string ffmpeg = converter_.GetFfmpegPath();
            const bool keep = keep_originals_;
            converter_.Run(
              "sound " + std::to_string(processed_sounds_),
              [ffmpeg, wav, ogg](std::string* error){return ExternalTools::ConvertToOgg(ffmpeg, wav, ogg, error);},
              [wav, keep](){if (!keep) std::remove(wav.c_str());}
            );
            sounds_.push_back("audio/sounds/" + std::to_string(processed_sounds_) + ".ogg");
        }
        else{
            sounds_.push_back("audio/sounds/INVALID.ogg");
        }
    }
    processed_sounds_ ++;
    if (processed_sounds_ >= TOTAL_SOUNDS) return true;
    else return false;
}

void SoundInstaller::WriteSoundIndex(){
    std::unordered_map<int, const SoundName*> names;
    for (const SoundName& name : SOUND_NAMES) names[name.id] = &name;
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("sounds"));
    for (int id = 0; id < sounds_.size(); id ++){
        std::string path = sounds_[id];
        // A sound that could not be converted has no file, and listing it would make the game look for it.
        if (path != "audio/sounds/INVALID.ogg" && !boost::filesystem::exists(data_dir_ + path)) continue;
        std::unique_ptr<TiXmlElement> xml_sound(new TiXmlElement("sound"));
        xml_sound->SetAttribute("file_name", path);
        xml_sound->SetAttribute("name", id);
        container->LinkEndChild(xml_sound.release());
        // If there is a friendly name for this sound, add another entry
        const auto found = names.find(id);
        if (found != names.end() && found->second->name[0] != 0){
            std::unique_ptr<TiXmlElement> xml_sound_name(new TiXmlElement("sound"));
            xml_sound_name->SetAttribute("file_name", path);
            xml_sound_name->SetAttribute("name", found->second->name);
            xml_sound_name->SetAttribute("description", found->second->description);
            container->LinkEndChild(xml_sound_name.release());
        }
    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(data_dir_ + "audio/sounds/_sounds.xml");
}
