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
#include <iostream>
#include <cstdio>
#include <OgreDataStream.h>
#include <OgreResourceGroupManager.h>
#include <OgreImage.h>
#include <OgreColourValue.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <tinyxml.h>
#include "MediaDataInstaller.h"
#include "data/VGearsLGPArchive.h"
#include "data/VGearsTexFile.h"
#include "TexFile.h"

int MediaDataInstaller::TOTAL_SOUNDS = 723;

u8 MediaDataInstaller::WAV_HEADER[] = {
  0x52, 0x49, 0x46, 0x46, 0x70, 0x0B, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20,
  0x32, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x44, 0xAC, 0x00, 0x00, 0x00, 0x54, 0x00, 0x00,
  0x00, 0x04, 0x04, 0x00, 0x20, 0x00, 0xF4, 0x07, 0x07, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02,
  0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x40, 0x00, 0xF0, 0x00, 0x00, 0x00, 0xCC, 0x01,
  0x30, 0xFF, 0x88, 0x01, 0x18, 0xFF, 0x64, 0x61, 0x74, 0x61, 0x2A, 0x0B, 0x00, 0x00
};

MediaDataInstaller::MediaDataInstaller(
  const std::string input_dir, const std::string output_dir, const bool keep_originals,
  const bool no_ffmpeg, const bool no_timidity
):
  input_dir_(input_dir), output_dir_(output_dir), keep_originals_(keep_originals),
  no_ffmpeg_(no_ffmpeg), no_timidity_(no_timidity),
  menu_(input_dir + "data/menu/menu_us.lgp", "LGP"), window_(input_dir + "data/kernel/WINDOW.BIN"),
  fmt_(input_dir_ + "data/sound/audio.fmt"), dat_(input_dir_ + "data/sound/audio.dat"),
  midi_(input_dir + "data/midi/midi.lgp", "LGP")
{PopulateMaps();}

void MediaDataInstaller::PopulateMaps(){
    // TODO: Names ending in "_?" are sounds that I think they match their name, but I'm not 100%
    // sure.
    sound_map_[0] = "Cursor";
    sound_map_[1] = "Window";
    sound_map_[2] = "Error";
    sound_map_[3] = "Back";
    sound_map_[4] = "Slash_Miss_?";
    sound_map_[6] = "Chirp";
    // TODO: Fill this list with somewhat descriptive names.
}

MediaDataInstaller::~MediaDataInstaller(){}

void MediaDataInstaller::InstallSprites(){
    // Actually open the lgp as a file for reading
    File menu(input_dir_ + "data/menu/menu_us.lgp");

    // Also, open it as a LGP archive.
    menu_.open(input_dir_ + "data/menu/menu_us.lgp", true);
    menu_.load();


    Ogre::StringVectorPtr file_list = menu_.list(true, true);
    //std::cout << " " << file_list << " files \n";
    VGears::LGPArchive::FileList files = menu_.GetFiles();

    for (int i = 0; i < file_list->size(); i ++){
        VGears::LGPArchive::FileEntry f = files.at(i);
        if (
          f.file_name != "barre.tex"
          && f.file_name != "bins.tex"
          && f.file_name != "cido.tex"
          && f.file_name != "cloud.tex"
          && f.file_name != "earith.tex"
          && f.file_name != "ketc.tex"
          && f.file_name != "pcefi.tex"
          && f.file_name != "pcloud.tex"
          && f.file_name != "red.tex"
          && f.file_name != "tifa.tex"
          && f.file_name != "yufi.tex"
          && f.file_name != "buster.tex"
          && f.file_name != "choco.tex"
          && f.file_name != "btl_win_c_l.tex"
          && f.file_name != "btl_win_a_h.tex"
          && f.file_name != "btl_win_b_h.tex"
          && f.file_name != "btl_win_b_l.tex"
        ){
            continue;
        }

        // Save the TEX File
        std::fstream out;
        out.open(output_dir_ + "images/" + f.file_name, std::ios::out);
        menu.SetOffset(f.data_offset);
        for (int j = 0; j < f.data_size; j ++) out << menu.readU8();
        out.close();

        // Open the TEX file and save PNGs.
        std::string img_dir = output_dir_ + "images/";
        TexFile tex(output_dir_ + "images/" + f.file_name);
        if (f.file_name == "barre.tex")
            tex.SavePng(img_dir + "characters/1.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "bins.tex")
            tex.SavePng(img_dir + "characters/7.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "cido.tex")
            tex.SavePng(img_dir + "characters/8.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "cloud.tex")
            tex.SavePng(img_dir + "characters/0.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "earith.tex")
            tex.SavePng(img_dir + "characters/3.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "ketc.tex")
            tex.SavePng(img_dir + "characters/6.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "pcefi.tex")
            tex.SavePng(img_dir + "characters/10.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "pcloud.tex")
            tex.SavePng(img_dir + "characters/9.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "red.tex")
            tex.SavePng(img_dir + "characters/4.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "tifa.tex")
            tex.SavePng(img_dir + "characters/2.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "yufi.tex")
            tex.SavePng(img_dir + "characters/5.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "buster.tex")
            tex.SavePng(img_dir + "other/begin_menu.png", 0, 0, 150, 150, 0);
        else if (f.file_name == "choco.tex")
            tex.SavePng(img_dir + "other/choco.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "btl_win_c_l.tex"){
            // Weapon (no index 0), armor, accessory icons, palette 1.
            tex.SavePng(img_dir + "icons/item_weapon_1.png", 192, 0, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_2.png", 224, 0, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_3.png", 192, 32, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_4.png", 224, 32, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_5.png", 192, 64, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_6.png", 224, 64, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_7.png", 192, 96, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_8.png", 224, 96, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_armor.png", 192, 128, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_accessory.png", 224, 128, 32, 32, 1);
            // Status effects battle animations, palette 1.
            tex.SavePng(img_dir + "icons/status_2_0.png", 128, 188, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_2_1.png", 160, 188, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_2_2.png", 192, 188, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_2_3.png", 224, 188, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_7_0.png", 128, 219, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_7_1.png", 160, 219, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_7_2.png", 192, 219, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_7_3.png", 224, 219, 32, 32, 1);
            // Door arrows, palette 9.
            tex.SavePng(img_dir + "icons/pointer_gateway_0.png", 96, 160, 18, 12, 9);
            tex.SavePng(img_dir + "icons/pointer_gateway_1.png", 128, 160, 18, 12, 9);
            tex.SavePng(img_dir + "icons/pointer_gateway_1.png", 160, 160, 18, 12, 9);
            tex.SavePng(img_dir + "icons/pointer_gateway_3.png", 192, 160, 18, 12, 9);
            tex.SavePng(img_dir + "icons/pointer_gateway_4.png", 224, 160, 18, 12, 9);
            // Ladder arrows, palette 4.
            tex.SavePng(img_dir + "icons/pointer_ladder_0.png", 96, 160, 18, 12, 4);
            tex.SavePng(img_dir + "icons/pointer_ladder_1.png", 128, 160, 18, 12, 4);
            tex.SavePng(img_dir + "icons/pointer_ladder_1.png", 160, 160, 18, 12, 4);
            tex.SavePng(img_dir + "icons/pointer_ladder_3.png", 192, 160, 18, 12, 4);
            tex.SavePng(img_dir + "icons/pointer_ladder_4.png", 224, 160, 18, 12, 4);
            // Recovery word, palette 4.
            tex.SavePng(img_dir + "icons/battle_recovery.png", 64, 64, 212, 232, 64, 36, 20, 4);
            // Death word, palette 2.
            tex.SavePng(img_dir + "icons/battle_death.png", 64, 192, 64, 20, 2);
            // Window borders and corners, palette 0
            tex.SavePng(img_dir + "window/b_t.png", 0, 192, 32, 16, 0);
            tex.SavePng(img_dir + "window/b_b.png", 32, 208, 32, 16, 0);
            tex.SavePng(img_dir + "window/b_l.png", 0, 224, 16, 32, 0);
            tex.SavePng(img_dir + "window/b_r.png", 48, 224, 16, 32, 0);
            tex.SavePng(img_dir + "window/c_tl.png", 0, 208, 16, 16, 0);
            tex.SavePng(img_dir + "window/c_tr.png", 16, 208, 16, 16, 0);
            tex.SavePng(img_dir + "window/c_bl.png", 32, 192, 16, 16, 0);
            tex.SavePng(img_dir + "window/c_br.png", 48, 192, 16, 16, 0);
        }
        else if (f.file_name == "btl_win_a_h.tex"){
            // Items and weapon 0, palette 1.
            tex.SavePng(img_dir + "icons/item_item.png", 192, 223, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_0.png", 220, 223, 32, 32, 1);
            // Battle bar and texts, palette 1.
            tex.SavePng(img_dir + "icons/battle_bar.png", 32, 0, 76, 18, 1);
            tex.SavePng(img_dir + "icons/battle_limit.png", 112, 0, 48, 10, 1);
            tex.SavePng(img_dir + "icons/battle_hp.png", 160, 0, 28, 10, 1);
            tex.SavePng(img_dir + "icons/battle_mp.png", 192, 0, 30, 10, 1);
            tex.SavePng(img_dir + "icons/battle_time.png", 224, 162, 0, 16, 32, 14, 10, 1);
            tex.SavePng(img_dir + "icons/battle_barrier.png", 112, 16, 76, 10, 1);
            tex.SavePng(img_dir + "icons/battle_wait.png", 192, 16, 44, 10, 1);
            tex.SavePng(img_dir + "icons/battle_1.png", 130, 34, 10, 12, 1);
            tex.SavePng(img_dir + "icons/battle_2.png", 146, 34, 10, 12, 1);
            tex.SavePng(img_dir + "icons/battle_3.png", 162, 34, 10, 12, 1);
            tex.SavePng(img_dir + "icons/battle_4.png", 178, 34, 10, 12, 1);
            tex.SavePng(img_dir + "icons/battle_5.png", 194, 34, 10, 12, 1);
            tex.SavePng(img_dir + "icons/battle_mp_needed.png", 0, 40, 80, 24, 1);
            // Some other battle text, palette 7.
            tex.SavePng(img_dir + "icons/battle_all.png", 240, 32, 16, 16, 7);
            tex.SavePng(img_dir + "icons/battle_x.png", 240, 16, 16, 16, 7);
            // No growth materia slot and linker, palette 1.
            tex.SavePng(img_dir + "icons/materia_slot_no_growth.png", 224, 63, 24, 24, 1);
            tex.SavePng(img_dir + "icons/materia_slot_link.png", 227, 95, 14, 24, 1);
            // Buttons, palette 7.
            tex.SavePng(img_dir + "icons/bt_up.png", 160, 70, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_down.png", 192, 70, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_right.png", 160, 102, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_left.png", 192, 102, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_3.png", 192, 134, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_2.png", 224, 134, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_6.png", 192, 166, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_8.png", 224, 166, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_9.png", 192, 198, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_10.png", 224, 198, 26, 20, 7);
            // Crossed out cursor, palette 7.
            tex.SavePng(img_dir + "icons/cursor_crossed.png", 80, 32, 48, 32, 7);
            // Sound settings, palette 1.
            tex.SavePng(img_dir + "icons/settings_music.png", 101, 131, 30, 30, 1);
            tex.SavePng(img_dir + "icons/settings_fx.png", 133, 131, 30, 30, 1);
        }
        else if (f.file_name == "btl_win_b_h.tex"){
            // Don't take cursors fom here, they don't have shadows and are too narrow.
            // Take them from btl_win_b_l.tex

            // Menu and battle text, palette 1.
            tex.SavePng(img_dir + "icons/slash.png", 176, 0, 8, 16, 1);
            tex.SavePng(img_dir + "icons/battle_name.png", 0, 32, 52, 10, 1);
            tex.SavePng(img_dir + "icons/battle_status.png", 0, 48, 64, 10, 1);
            tex.SavePng(img_dir + "icons/battle_status.png", 144, 48, 52, 10, 1);
            // Up/Down pointers, palette 1.
            tex.SavePng(img_dir + "icons/pointer_up.png", 198, 0, 22, 16, 1);
            tex.SavePng(img_dir + "icons/pointer_down.png", 230, 0, 22, 16, 1);
            // Materia slot, palette 1.
            tex.SavePng(img_dir + "icons/materia_slot.png", 0, 95, 24, 24, 1);
            // Materia stuff, several plettes.
            tex.SavePng(img_dir + "icons/materia_1.png", 3, 67, 18, 18, 4);
            tex.SavePng(img_dir + "icons/materia_2.png", 3, 67, 18, 18, 5);
            tex.SavePng(img_dir + "icons/materia_3.png", 3, 67, 18, 18, 8);
            tex.SavePng(img_dir + "icons/materia_4.png", 3, 67, 18, 18, 9);
            tex.SavePng(img_dir + "icons/materia_5.png", 3, 67, 18, 18, 10);
            tex.SavePng(img_dir + "icons/materia_star_1.png", 32, 64, 23, 26, 4);
            tex.SavePng(img_dir + "icons/materia_star_2.png", 32, 64, 23, 26, 5);
            tex.SavePng(img_dir + "icons/materia_star_3.png", 32, 64, 23, 26, 8);
            tex.SavePng(img_dir + "icons/materia_star_4.png", 32, 64, 23, 26, 9);
            tex.SavePng(img_dir + "icons/materia_star_5.png", 32, 64, 23, 26, 10);
            tex.SavePng(img_dir + "icons/materia_star_empty_1.png", 32, 96, 23, 26, 4);
            tex.SavePng(img_dir + "icons/materia_star_empty_2.png", 32, 96, 23, 26, 5);
            tex.SavePng(img_dir + "icons/materia_star_empty_3.png", 32, 96, 23, 26, 8);
            tex.SavePng(img_dir + "icons/materia_star_empty_4.png", 32, 96, 23, 26, 9);
            tex.SavePng(img_dir + "icons/materia_star_empty_5.png", 32, 96, 23, 26, 10);
            // Buttons, palette 7.
            tex.SavePng(img_dir + "icons/bt_1.png", 0, 134, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_4.png", 32, 134, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_5.png", 0, 166, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_7.png", 32, 16, 26, 20, 7);
            // Some other battle text, palette 7.
            tex.SavePng(img_dir + "icons/battle_inf.png", 36, 194, 26, 14, 7);
            // Right pointer, palette 7.
            tex.SavePng(img_dir + "icons/pointer_right.png", 4, 16, 10, 16, 7);
            // Battle and menu bars, palette 1.
            tex.SavePng(img_dir + "icons/battle_bar_barriers.png", 64, 32, 76, 24, 1);
            tex.SavePng(img_dir + "icons/menu_bar.png", 16, 16, 126, 16, 1);
            // Fonts, save all palettes, make one default.
            for (int p = 0; p < 16; p ++){
                // Digits font. Default palette 7.
                tex.SavePng(
                  img_dir + "fonts/digits_" + std::to_string(p) + ".png", 16, 0, 156, 16, p
                );
                if (p == 7) tex.SavePng(img_dir + "fonts/digits.png", 16, 0, 156, 16, p);
            }
            for (int p = 0; p < 16; p ++){
                // Time font. Default palette 12.
                tex.SavePng(
                  img_dir + "fonts/timer_" + std::to_string(p) + ".png", 32, 160, 224, 90, p
                );
                if (p == 12) tex.SavePng(img_dir + "fonts/timer.png", 32, 160, 224, 90, p);
            }
            // Reels, palete 11.
            tex.SavePng(img_dir + "reels/yeah.png", 64, 64, 64, 30, 11);
            tex.SavePng(img_dir + "reels/luck.png", 64, 98, 64, 30, 11);
            tex.SavePng(img_dir + "reels/hit.png", 128, 64, 64, 30, 11);
            tex.SavePng(img_dir + "reels/bar.png", 128, 97, 64, 30, 11);
            tex.SavePng(img_dir + "reels/miss.png", 192, 64, 64, 30, 11);
            tex.SavePng(img_dir + "reels/choso.png", 192, 98, 64, 30, 11);

        }
        else if (f.file_name == "btl_win_b_l.tex"){
            // Cursors, palette 1.
            tex.SavePng(img_dir + "icons/cursor.png", 192, 16, 48, 32, 1);
            tex.SavePng(img_dir + "icons/cursor_left.png", 144, 16, 48, 32, 1);
            tex.SavePng(img_dir + "icons/pointer_position.png", 0, 192, 32, 26, 1);
        }

        // TODO: Extract arena reels from coloa.tex, colob.tex and coloc.tex

        // Delete Tex file.
        if (!keep_originals_) std::remove((output_dir_ + "images/" + f.file_name).c_str());
    }
}

int MediaDataInstaller::InstallSoundsInit(){
    fmt_.SetOffset(0);
    dat_.SetOffset(0);
    processed_sounds_ = 0;
    return TOTAL_SOUNDS;
}

bool MediaDataInstaller::InstallSounds(){

    FmtFile header;
    header.size = fmt_.readU32LE();
    // If size is 0, this is a bad header. There are 112 bytes of bad data, and after that,
    // the next header.
    if (header.size == 0){
        for (int b = 0; b < 112; b += 4) fmt_.readU32LE();
        processed_sounds_ ++;
        if (processed_sounds_ >= TOTAL_SOUNDS) return true;
        else return false;
    }

    header.offset = fmt_.readU32LE();
    // If the offset is less than the previous one, also bad header. 34 bytes of bad data, and
    // after that, the next header.
    if (header.offset < dat_.GetCurrentOffset()){
        for (int b = 0; b < 34; b += 2) fmt_.readU16LE();
        processed_sounds_ ++;
        if (processed_sounds_ >= TOTAL_SOUNDS) return true;
        else return false;
    }

    // This should never happen, but just in case, never read outside the file
    if (header.offset + header.size > dat_.GetFileSize()){
        processed_sounds_ ++;
        if (processed_sounds_ >= TOTAL_SOUNDS) return true;
        else return false;
    }

    for (int l = 0; l < 16; l ++) header.loop_metadata[l] = fmt_.readU8();
    for (int l = 0; l < 18; l ++) header.wav_header[l] = fmt_.readU8();
    header.samples_per_block = fmt_.readU16LE();
    header.adpcm = fmt_.readU16LE();
    for (int l = 0; l < 28; l ++) header.adpcm_sets[l] = fmt_.readU8();

    dat_.SetOffset(header.offset);
    std::ofstream out(
      output_dir_ + "audio/sound/" + std::to_string(processed_sounds_) + ".wav",
      std::ios::out | std::ios::binary
    );


    // Write the standard wav header.
    for (int b = 0; b < 78; b ++) out.put(WAV_HEADER[b]);
    // Write the data from the dat file.
    for (int b = 0; b < header.size; b ++) out.put(dat_.readU8());
    out.close();

    // Convert to OGG.
    // TODO: Don't use system calls! Integrate libav or something that can do the conversion
    // natively
    std::string command = (boost::format(
      "ffmpeg -hide_banner -loglevel panic -y -i %1%audio/sound/%2%.wav %1%audio/sound/%2%.ogg"
    ) % output_dir_ % processed_sounds_).str();
    if (!no_ffmpeg_) std::system(command.c_str());

    // Remove the wav file.
    if (!keep_originals_){
        std::remove(
          (output_dir_ + "audio/sound/" + std::to_string(processed_sounds_) + ".wav").c_str()
        );
    }

    sounds_.push_back("audio/sound/" + std::to_string(processed_sounds_) + ".ogg");
    processed_sounds_ ++;
    if (processed_sounds_ >= TOTAL_SOUNDS) return true;
    else return false;
}

void MediaDataInstaller::WriteSoundIndex(){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("sounds"));
    //for (string item : items_){
    for (int id = 0; id < sounds_.size(); id ++){
        std::string path = sounds_[id];
        std::unique_ptr<TiXmlElement> xml_sound(new TiXmlElement("sound"));
        xml_sound->SetAttribute("file_name", path);
        xml_sound->SetAttribute("name", id);
        container->LinkEndChild(xml_sound.release());
        // If there is a friendly name for this sound, add another entry
        if (sound_map_.count(id) != 0){
            std::unique_ptr<TiXmlElement> xml_sound_name(new TiXmlElement("sound"));
            xml_sound_name->SetAttribute("file_name", path);
            xml_sound_name->SetAttribute("name", sound_map_[id]);
            container->LinkEndChild(xml_sound_name.release());
        }
    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(output_dir_ + "sounds.xml");
}

int MediaDataInstaller::InstallMusicsInit(){
    // Read the music index file.
    std::ifstream music_idx(input_dir_ + "data/music/music.idx");
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

bool MediaDataInstaller::InstallMusics(){
    VGears::LGPArchive::FileEntry f = midi_.GetFiles().at(processed_musics_);

    // Find the index in the map.
    int index = 1000 + processed_musics_;
    for (auto const& m : musics_map_){
        if (m.second + ".mid" == f.file_name){
            index = m.first;
            break;
        }
    }

    File midi(input_dir_ + "data/midi/midi.lgp");

    std::fstream out;
    out.open(output_dir_ + "audio/music/" + std::to_string(index) + ".mid", std::ios::out);
    midi.SetOffset(f.data_offset);
    for (int j = 0; j < f.data_size; j ++) out << midi.readU8();
    out.close();

    // Convert to ogg (TiMidity + FFMpeg)
    std::string command = (boost::format(
      "timidity --quiet=3 %1%audio/music/%2%.mid -Ow -o - "
      "| ffmpeg -hide_banner -loglevel panic -y -i - %1%audio/music/%2%.ogg"
    ) % output_dir_ % index).str();
    if (!no_ffmpeg_ && !no_timidity_) std::system(command.c_str());

    // Remove the midi file.
    if (!keep_originals_)
        std::remove((output_dir_ + "audio/music/" + std::to_string(index) + ".mid").c_str());

    musics_.push_back("audio/music/" + std::to_string(processed_musics_) + ".ogg");
    processed_musics_ ++;
    if (processed_musics_ == midi_.list(true, true)->size()) return true;
    else return false;
}

void MediaDataInstaller::InstallHQMusics(){
    std::vector<std::string> hq_musics = {"hearth", "sato", "sensui", "wind"};
    for (std::string hq_music : hq_musics){

        int index = 2000;
        for (auto const& m : musics_map_){
            if (m.second == hq_music){
                index = m.first;
                break;
            }
        }

        std::string command = (boost::format(
          "ffmpeg -hide_banner -loglevel panic -y -i %1%music/%2%.wav %3%audio/sound/%4%.ogg"
        ) % input_dir_ % hq_music % output_dir_ % index).str();
        if (!no_ffmpeg_)  std::system(command.c_str());
    }
}

void MediaDataInstaller::WriteMusicsIndex(){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("musics"));
    //for (string item : items_){
    for (int id = 0; id < musics_.size(); id ++){
        std::string path = musics_[id];
        std::unique_ptr<TiXmlElement> xml_music(new TiXmlElement("music"));
        xml_music->SetAttribute("file_name", path);
        xml_music->SetAttribute("name", id);
        xml_music->SetAttribute("loop", "0");
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
    xml.SaveFile(output_dir_ + "musics.xml");
}

