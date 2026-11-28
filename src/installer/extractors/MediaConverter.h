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

#pragma once

#include <functional>
#include <memory>
#include <string>
#include "installer/steps/ExternalTools.h"
#include "installer/steps/TaskPool.h"

/**
 * Converts the audio with external programs, in the background.
 */
class MediaConverter{

    public:

        /**
         * Finds the conversion programs the first time it is called, and reports what is missing.
         *
         * @param[in] need_timidity True if TiMidity is needed too (the music).
         * @return True if the programs are usable.
         */
        bool ToolsReady(bool need_timidity);

        /**
         * Runs a conversion in the background.
         *
         * A failed conversion is logged as a warning.
         *
         * @param[in] description What is being converted, for the log.
         * @param[in] job Does the conversion. Returns false, and sets the error, if it fails.
         * @param[in] on_success Called, in the background thread, if the conversion succeeds.
         */
        void Run(
          const std::string& description, std::function<bool(std::string*)> job,
          std::function<void()> on_success = nullptr
        );

        /**
         * Waits until every audio conversion started by the install steps has finished.
         *
         * The conversions run in the background, so the other steps don't wait for them. Call it before using or
         * deleting their results.
         */
        void Wait();

        /**
         * Retrieves the path of FFmpeg. {@see ToolsReady} has to be called first.
         *
         * @return The path.
         */
        const std::string& GetFfmpegPath() const{return ffmpeg_tool_.path;}

        /**
         * Retrieves the path of TiMidity. {@see ToolsReady} has to be called first.
         *
         * @return The path.
         */
        const std::string& GetTimidityPath() const{return timidity_tool_.path;}

    private:

        /**
         * FFmpeg and TiMidity, found by {@see ToolsReady}.
         */
        ExternalTools::Tool ffmpeg_tool_;
        ExternalTools::Tool timidity_tool_;
        bool tools_resolved_ = false;

        /**
         * Threads that run the conversions. Created when the first conversion starts.
         */
        std::unique_ptr<TaskPool> conversions_;
};
