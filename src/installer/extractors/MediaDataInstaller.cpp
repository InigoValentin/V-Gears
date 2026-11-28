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

#include "installer/extractors/MediaDataInstaller.h"

MediaDataInstaller::MediaDataInstaller(
  const DiskImage& disk_image, const std::string output_dir, const bool keep_originals
):
  disk_image_(disk_image),
  images_(disk_image_, output_dir + "game/", keep_originals),
  sounds_(disk_image_, output_dir + "game/", keep_originals, converter_),
  music_(disk_image_, output_dir + "game/", keep_originals, converter_)
{}

MediaDataInstaller::~MediaDataInstaller(){}

void MediaDataInstaller::InstallSprites(){images_.InstallSprites();}

bool MediaDataInstaller::InstallFonts(){return images_.InstallFonts();}

int MediaDataInstaller::InstallSoundsInit(){return sounds_.InstallSoundsInit();}

bool MediaDataInstaller::InstallSounds(){return sounds_.InstallSounds();}

void MediaDataInstaller::WriteSoundIndex(){sounds_.WriteSoundIndex();}

int MediaDataInstaller::InstallMusicsInit(){return music_.InstallMusicsInit();}

bool MediaDataInstaller::InstallMusics(){return music_.InstallMusics();}

void MediaDataInstaller::InstallHQMusics(){music_.InstallHQMusics();}

void MediaDataInstaller::WaitForConversions(){converter_.Wait();}

void MediaDataInstaller::WriteMusicsIndex(){music_.WriteMusicsIndex();}
