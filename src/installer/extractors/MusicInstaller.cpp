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

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <tinyxml.h>
#include "common/File.h"
#include "installer/extractors/MusicInstaller.h"
#include "installer/steps/InstallLog.h"

MusicInstaller::MusicInstaller(
  DiskImage& disk_image, const std::string& data_dir, const bool keep_originals,
  MediaConverter& converter
):
  disk_image_(disk_image), data_dir_(data_dir), keep_originals_(keep_originals),
  converter_(converter),
  midi_(VGears::LGPArchive(*disk_image.filePath("data/midi/midi.lgp"), "LGP"))
{}

int MusicInstaller::InstallMusicsInit(){
    // Read the music index file.
    std::ifstream music_idx(*disk_image_.filePath("data/music/music.idx"));
    int i = 0;
    std::string name;
    if (music_idx.is_open()){
        while (std::getline(music_idx, name)){
            // Newlines are very likely WINDOWS/DOS format, but check them all.
            boost::replace_all(name, "\r\n", "");
            boost::replace_all(name, "\n", "");
            boost::replace_all(name, "\r", "");
            musics_map_[i] = name;
            i ++;
        }
        music_idx.close();
    }

    // Load midi_
    midi_.load();

    processed_musics_ = 0;
    return midi_.list(true, true)->size();
}

bool MusicInstaller::InstallMusics(){
    VGears::LGPArchive::FileEntry f = midi_.GetFiles().at(processed_musics_);

    // Find the index in the map.
    int index = 1000 + processed_musics_;
    for (auto const& m : musics_map_){
        if (m.second + ".mid" == f.file_name){
            index = m.first;
            break;
        }
    }

    File midi(*disk_image_.filePath("data/midi/midi.lgp"));

    std::fstream out;
    out.open(data_dir_ + "audio/music/" + std::to_string(index) + ".mid", std::ios::out);
    midi.SetOffset(f.data_offset);
    for (int j = 0; j < f.data_size; j ++) out << midi.readU8();
    out.close();

    // Convert to ogg (TiMidity + FFMpeg).
    const std::string midi_path = data_dir_ + "audio/music/" + std::to_string(index) + ".mid";
    const std::string ogg_path = data_dir_ + "audio/music/" + std::to_string(index) + ".ogg";
    const bool keep = keep_originals_;
    if (converter_.ToolsReady(true)){
        // Converted in the background. The midi file is removed once it has been converted.
        const std::string timidity = converter_.GetTimidityPath();
        const std::string ffmpeg = converter_.GetFfmpegPath();
        converter_.Run(
          "music " + std::to_string(index),
          [timidity, ffmpeg, midi_path, ogg_path](std::string* error){
              return ExternalTools::ConvertMidiToOgg(timidity, ffmpeg, midi_path, ogg_path, error);
          },
          [midi_path, keep](){if (!keep) std::remove(midi_path.c_str());}
        );
    }
    // Without the programs the midi file is the only copy of the track, so it stays.

    musics_.push_back("audio/music/" + std::to_string(processed_musics_) + ".ogg");
    processed_musics_ ++;
    if (processed_musics_ == midi_.list(true, true)->size()) return true;
    else return false;
}

void MusicInstaller::InstallHQMusics(){
    // Four tracks are not MIDI files: they are the WAV files of DATA/MUSIC (HEART.WAV, SATO.WAV,
    // SENSUI.WAV and WIND.WAV). The index of the music (MUSIC.IDX) says which track is each one, and
    // they are installed as the other ones are, with the number of the track as the name. Without
    // them tracks 14, 64, 87 and 96 are missing (the music of junair2, for example).
    std::vector<std::string> hq_musics = {"heart", "sato", "sensui", "wind"};
    for (const std::string& hq_music : hq_musics){
        int index = -1;
        for (auto const& m : musics_map_){
            if (m.second == hq_music){
                index = m.first;
                break;
            }
        }
        if (index < 0){
            InstallLog::Get().Add("[WARNING] The music " + hq_music + " is not in the index of the music.");
            continue;
        }
        if (!converter_.ToolsReady(false)) continue;
        const std::string ffmpeg = converter_.GetFfmpegPath();
        std::string upper = hq_music;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        const std::unique_ptr<std::string> wav_path = disk_image_.filePath("data/music/" + upper + ".wav");
        if (!wav_path || !boost::filesystem::exists(*wav_path)){
            InstallLog::Get().Add("[WARNING] The file of the music " + hq_music + " was not found.");
            continue;
        }
        const std::string wav = *wav_path;
        const std::string ogg = data_dir_ + "audio/music/" + std::to_string(index) + ".ogg";
        converter_.Run(
          "high quality music " + hq_music,
          [ffmpeg, wav, ogg](std::string* error){return ExternalTools::ConvertToOgg(ffmpeg, wav, ogg, error);}
        );
    }
    // The index lists the tracks by their position, and the MIDI files are four less than the tracks:
    // the last ones (94 to 97) were left out of it. A track with no file is not written to the index.
    int last_id = 0;
    for (auto const& m : musics_map_) last_id = std::max(last_id, m.first);
    for (int id = static_cast<int>(musics_.size()); id <= last_id; ++ id)
        musics_.push_back("audio/music/" + std::to_string(id) + ".ogg");
}

void MusicInstaller::WriteMusicsIndex(){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("musics"));
    for (int id = 0; id < musics_.size(); id ++){
        std::string path = musics_[id];
        // A track that could not be converted has no file, and listing it would make the game look for it.
        if (!boost::filesystem::exists(data_dir_ + path)) continue;
        std::unique_ptr<TiXmlElement> xml_music(new TiXmlElement("music"));
        xml_music->SetAttribute("file_name", path);
        xml_music->SetAttribute("name", id);
        xml_music->SetAttribute("loop", "0");
        switch (id){
            case 1:
                xml_music->SetAttribute("default", "initial");
                break;
            case 6:
                xml_music->SetAttribute("default", "battle");
                break;
            case 9:
                xml_music->SetAttribute("default", "boss");
                break;
            case 13:
                xml_music->SetAttribute("default", "world_map");
                break;
            case 43:
                xml_music->SetAttribute("default", "chocobo_battle");
                break;
            case 45:
                xml_music->SetAttribute("default", "victory");
                break;
            case 46:
                xml_music->SetAttribute("default", "airship");
                break;
            case 50:
                xml_music->SetAttribute("default", "chocobo_ride");
                break;
            case 57:
                xml_music->SetAttribute("default", "game_over");
                break;
            case 70:
                xml_music->SetAttribute("default", "world_map_danger");
                break;
            case 84:
                xml_music->SetAttribute("default", "submarine");
                break;

        }
        container->LinkEndChild(xml_music.release());
        // If there is a friendly name for this sound, add another entry
        if (musics_map_.count(id) != 0){
            std::unique_ptr<TiXmlElement> xml_music_name(new TiXmlElement("music"));
            xml_music_name->SetAttribute("file_name", path);
            xml_music_name->SetAttribute("name", musics_map_[id]);
            xml_music_name->SetAttribute("loop", "0");
            container->LinkEndChild(xml_music_name.release());
        }
    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(data_dir_ + "audio/music/_musics.xml");
}
