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

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

/**
 * The installer log.
 *
 * Collects every message of an installation in one place: it is written to a log file in the installation
 * directory, passed to an optional sink (the log window), and counted by severity to show a summary at the end.
 *
 * While open, anything the installer prints to std::cout or std::cerr is captured too, so messages printed by
 * code that knows nothing about the log are not lost. The severity of a line is taken from its prefix:
 * "[ERROR]" and "Exception" are errors, "[WARNING]" is a warning, anything else is information.
 *
 * It is thread-safe.
 */
class InstallLog{

    public:

        /**
         * Message severity.
         */
        enum Level{INFO, WARNING, ERROR_LEVEL};

        /**
         * Receives the messages that must be shown to the user.
         */
        typedef std::function<void(Level, const std::string&)> Sink;

        /**
         * Gets the log.
         */
        static InstallLog& Get();

        /**
         * Starts a new log: truncates the file, resets the counters and starts capturing std::cout and std::cerr.
         *
         * @param[in] path Path of the log file.
         * @param[in] append True to add to the file instead of replacing it.
         */
        void Open(const std::string& path, bool append = false);

        /**
         * Stops capturing the standard streams and closes the file.
         */
        void Close();

        /**
         * Sets the sink that receives the messages that must be shown to the user.
         */
        void SetSink(Sink sink);

        /**
         * Adds a message.
         *
         * @param[in] message The message. Its prefix sets the severity.
         * @param[in] show True if the message must also be passed to the sink.
         */
        void Add(const std::string& message, bool show = true);

        /**
         * Starts a step of the installation. The previous step, if any, is ended.
         *
         * @param[in] name Name of the step, shown in its summary.
         */
        void BeginStep(const std::string& name);

        /**
         * Ends the current step. If it had errors or warnings, a line with their number is added to the log.
         */
        void EndStep();

        /**
         * Gets the number of messages of a severity.
         */
        size_t Count(Level level) const;

        /**
         * Gets the first messages of a severity.
         *
         * @param[in] level The severity.
         * @param[in] max Maximum number of messages.
         */
        std::vector<std::string> First(Level level, size_t max) const;

        /**
         * Gets the path of the log file.
         */
        std::string Path() const;

        /**
         * Gets the severity of a message according to its prefix.
         */
        static Level Classify(const std::string& message);

    private:

        class Tee;

        InstallLog();

        /**
         * Handles a line captured from a standard stream.
         */
        void AddCaptured(const std::string& line);

        mutable std::mutex mutex_;
        std::ofstream file_;
        std::string path_;
        Sink sink_;
        size_t counts_[3];
        std::vector<std::string> first_[3];
        std::string last_message_;
        std::string step_name_;
        size_t step_start_[3];
        bool in_step_ = false;
        std::unique_ptr<Tee> tee_out_;
        std::unique_ptr<Tee> tee_err_;
};
