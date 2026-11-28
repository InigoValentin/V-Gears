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

#include "installer/extractors/MediaConverter.h"
#include "installer/steps/InstallLog.h"

bool MediaConverter::ToolsReady(bool need_timidity){
    if (!tools_resolved_){
        tools_resolved_ = true;
        const ExternalTools::Locations& locations = ExternalTools::CurrentLocations();
        ffmpeg_tool_ = ExternalTools::FindFfmpeg(locations);
        timidity_tool_ = ExternalTools::FindTimidity(locations);
        if (ffmpeg_tool_.usable) InstallLog::Get().Add("Using " + ffmpeg_tool_.version + " (" + ffmpeg_tool_.path + ")");
        else{
            InstallLog::Get().Add(
              "[WARNING] FFmpeg can't be used, so the sound effects and the music will not be converted (the "
              "extracted .wav and .mid files are left in place): " + ffmpeg_tool_.problem
            );
        }
        if (timidity_tool_.usable) InstallLog::Get().Add("Using " + timidity_tool_.version + " (" + timidity_tool_.path + ")");
        else{
            InstallLog::Get().Add(
              "[WARNING] TiMidity can't be used, so the music will not be converted (the extracted .mid files are "
              "left in place): " + timidity_tool_.problem
            );
        }
    }
    return ffmpeg_tool_.usable && (!need_timidity || timidity_tool_.usable);
}

void MediaConverter::Run(
  const std::string& description, std::function<bool(std::string*)> job, std::function<void()> on_success
){
    if (!conversions_) conversions_ = std::make_unique<TaskPool>();
    conversions_->Run([description, job, on_success](){
        std::string error;
        if (job(&error)){
            if (on_success) on_success();
        }
        else{
            // The source file is kept, since it hasn't been converted.
            InstallLog::Get().Add("[WARNING] Could not convert " + description + ": " + error);
        }
    });
}

void MediaConverter::Wait(){
    if (conversions_) conversions_->Wait();
}
