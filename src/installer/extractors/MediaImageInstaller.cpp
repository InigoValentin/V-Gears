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
#include <iterator>
#include "installer/extractors/MediaImageInstaller.h"
#include "installer/extractors/WindowFonts.h"
#include "installer/formats/TexFile.h"
#include "installer/tables/MenuImages.h"

MediaImageInstaller::MediaImageInstaller(
  DiskImage& disk_image, const std::string& data_dir, const bool keep_originals
):
  disk_image_(disk_image), data_dir_(data_dir), keep_originals_(keep_originals),
  menu_(VGears::LGPArchive(*disk_image.filePath("data/menu/menu_us.lgp"), "LGP")),
  window_(*disk_image.filePath("data/kernel/WINDOW.BIN"))
{}

void MediaImageInstaller::InstallSprites(){
    // Actually open the lgp as a file for reading
    File menu(*disk_image_.filePath("data/menu/menu_us.lgp"));
    // Also, open it as a LGP archive.
    menu_.open(*disk_image_.filePath("data/menu/menu_us.lgp"), true);
    menu_.load();

    const std::string img_dir = data_dir_ + "ui/images/";
    for (const VGears::LGPArchive::FileEntry& entry : menu_.GetFiles()){
        const std::string& name = entry.file_name;
        const bool used = std::any_of(
          std::begin(MENU_IMAGES), std::end(MENU_IMAGES),
          [&name](const MenuImage& image){return name == image.texture;}
        );
        if (!used) continue;

        // Save the TEX File
        std::fstream out;
        out.open(img_dir + name, std::ios::out);
        menu.SetOffset(entry.data_offset);
        for (int j = 0; j < entry.data_size; j ++) out << menu.readU8();
        out.close();

        // Open the TEX file and save PNGs.
        TexFile tex(img_dir + name);
        SaveImages(tex, name);
        // The fonts of numbers are in this texture.
        if (name == "btl_win_b_h.tex") SaveNumberFonts(tex);

        // Delete Tex file.
        if (!keep_originals_) std::remove((img_dir + name).c_str());
    }
}

void MediaImageInstaller::SaveImages(TexFile& tex, const std::string& texture) const{
    const std::string img_dir = data_dir_ + "ui/images/";
    for (const MenuImage& image : MENU_IMAGES){
        if (texture != image.texture) continue;
        tex.SavePng(
          img_dir + image.image, image.x, image.y, image.width, image.height, image.palette
        );
    }
    for (const MenuImagePair& image : MENU_IMAGE_PAIRS){
        if (texture != image.texture) continue;
        tex.SavePng(
          img_dir + image.image, image.x1, image.x2, image.y1, image.y2, image.width1,
          image.width2, image.height, image.palette
        );
    }
}

void MediaImageInstaller::SaveNumberFonts(TexFile& tex) const{
    const std::string font_dir = data_dir_ + "ui/images/fonts/";
    // Save all the palettes, and make one the default.
    for (int p = 0; p < 16; p ++){
        // Digits font. Default palette 7.
        tex.SavePng(font_dir + "digits_" + std::to_string(p) + ".png", 16, 0, 156, 16, p);
        if (p == 7) tex.SavePng(font_dir + "digits.png", 16, 0, 156, 16, p);
    }
    for (int p = 0; p < 16; p ++){
        // Time font. Default palette 12.
        tex.SavePng(font_dir + "timer_" + std::to_string(p) + ".png", 32, 160, 224, 90, p);
        if (p == 12) tex.SavePng(font_dir + "timer.png", 32, 160, 224, 90, p);
    }
}

bool MediaImageInstaller::InstallFonts(){
    // WINDOW.BIN: the first file is the window graphics TIM, the second one the dialog font TIM.
    WindowFonts::IndexImage window;
    WindowFonts::IndexImage text;
    for (u32 i = 0; i < 2; i ++){
        const File tim = window_.ExtractGZip(i);
        if (tim.GetFileSize() == 0) return false;
        std::vector<u8> buffer(tim.GetFileSize());
        tim.GetFileBuffer(buffer.data(), 0, buffer.size());
        if (!WindowFonts::ReadTim4(buffer.data(), buffer.size(), i == 0 ? window : text)) return false;
    }
    const std::string font_dir = data_dir_ + "ui/fonts/";
    WindowFonts::SavePng(WindowFonts::BuildTextFont(text), font_dir + "ffvii_en.png");
    WindowFonts::SavePng(WindowFonts::BuildMenuFont(window), font_dir + "ffvii_menu.png");
    WindowFonts::SavePng(WindowFonts::BuildDigitsFont(window), font_dir + "ffvii_digits.png");
    WindowFonts::SavePng(WindowFonts::BuildTimerFont(window), font_dir + "ffvii_timer.png");
    return true;
}
