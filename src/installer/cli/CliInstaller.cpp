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

#include <cmath>
#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <mutex>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include "CliInstaller.h"
#include "installer/steps/DataInstaller.h"
#include "installer/formats/DiskImage.h"
#include "installer/steps/ExternalTools.h"
#include "installer/steps/InstallLayout.h"
#include "installer/steps/InstallLog.h"
#include "installer/steps/InstallSteps.h"
#include "installer/steps/InstallVerifier.h"

#ifndef VGEARS_INSTALLER_VERSION
#define VGEARS_INSTALLER_VERSION "unknown"
#endif

namespace{

    // The messages are written with stdio, not with std::cout or std::cerr: while the installation log is open it
    // captures those streams, and the progress and the summary don't belong in the log.

    /**
     * Writes a line to the standard output.
     */
    void Print(const std::string& text){
        std::fputs((text + "\n").c_str(), stdout);
        std::fflush(stdout);
    }

    /**
     * Writes a line to the standard error.
     */
    void PrintError(const std::string& text){
        std::fflush(stdout);
        std::fputs((text + "\n").c_str(), stderr);
        std::fflush(stderr);
    }

    /**
     * Writes a list of problems to the standard error.
     *
     * @param[in] title Line written before the list.
     * @param[in] problems The problems.
     */
    void PrintList(const std::string& title, const QStringList& problems){
        PrintError(title);
        for (const QString& problem : problems) PrintError("  - " + problem.toStdString());
    }

    /**
     * Tells if the standard output is a terminal, where a line can be redrawn.
     */
    bool IsTerminal(){
#ifdef _WIN32
        return _isatty(_fileno(stdout)) != 0;
#else
        return isatty(fileno(stdout)) != 0;
#endif
    }

    /**
     * Gets the width of the terminal, in columns.
     */
    size_t TerminalWidth(){
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
            return static_cast<size_t>(info.srWindow.Right - info.srWindow.Left + 1);
#else
        winsize size;
        if (ioctl(fileno(stdout), TIOCGWINSZ, &size) == 0 && size.ws_col > 0) return size.ws_col;
#endif
        return 80;
    }

    /**
     * Tells if the messages can be coloured: the output is a terminal that understands colours, and the user didn't
     * turn them off (https://no-color.org).
     */
    bool CanColour(){
        if (!IsTerminal() || std::getenv("NO_COLOR") != nullptr) return false;
#ifdef _WIN32
        // The Windows console only understands the colour codes if it is asked to.
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (!GetConsoleMode(console, &mode)) return false;
        return SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
        const char* term = std::getenv("TERM");
        return term == nullptr || std::string(term) != "dumb";
#endif
    }

    /**
     * Colours a message by its severity: orange for warnings, red for errors.
     *
     * @param[in] level The severity.
     * @param[in] text The message.
     * @param[in] colour False to leave it as it is.
     */
    std::string Coloured(InstallLog::Level level, const std::string& text, bool colour){
        if (!colour || level == InstallLog::INFO) return text;
        return (level == InstallLog::ERROR_LEVEL ? "\x1b[31m" : "\x1b[38;5;208m") + text + "\x1b[0m";
    }

    /**
     * Shows the progress of an installation, and its problems.
     *
     * In a terminal, the progress is a single line at the bottom, redrawn in place, with the percent, a bar and
     * what the installer is doing. The warnings and the errors are written above it as they happen. The rest of
     * the messages are only in the installation log. The warnings are orange and the errors red, if the terminal
     * can show colours.
     *
     * Elsewhere (the output is a file or a pipe) the log writes every message, and the progress is a line for each
     * percent.
     *
     * The problems can come from other threads (the audio conversions), so everything is drawn with a lock.
     */
    class ProgressDisplay{

        public:

            ProgressDisplay(): interactive_(IsTerminal()), colour_(CanColour()){}

            /**
             * Colours a message by its severity, if the terminal can show colours.
             */
            std::string Colour(InstallLog::Level level, const std::string& text) const{
                return Coloured(level, text, colour_);
            }

            ~ProgressDisplay(){Detach();}

            /**
             * Gets the messages of the installation log, which must be open.
             */
            void Attach(){
                if (!interactive_) return;
                InstallLog& log = InstallLog::Get();
                log.SetEcho(false);
                log.SetSink([this](InstallLog::Level level, const std::string& message){
                    if (level == InstallLog::INFO) SetStatus(message);
                    else Message(Colour(level, message));
                });
            }

            /**
             * Stops getting the messages of the installation log, and leaves the progress line as it is.
             */
            void Detach(){
                if (!interactive_) return;
                InstallLog::Get().SetSink(nullptr);
                std::lock_guard<std::mutex> lock(mutex_);
                if (drawn_ > 0) std::fputs("\n", stdout);
                std::fflush(stdout);
                drawn_ = 0;
            }

            /**
             * Shows the progress, if it has gone up since it was last shown.
             *
             * @param[in] progress The progress [0-100].
             */
            void SetProgress(float progress){
                const int percent = std::max(0, std::min(100, static_cast<int>(std::floor(progress))));
                std::lock_guard<std::mutex> lock(mutex_);
                // The progress of an installation doesn't go back.
                if (percent <= percent_) return;
                percent_ = percent;
                if (interactive_) Draw();
                else{
                    char line[32];
                    std::snprintf(line, sizeof(line), "[%3d%%]", percent);
                    Print(line);
                }
            }

            /**
             * Shows what the installer is doing.
             *
             * In a terminal it is the text of the progress line. Elsewhere it is written as a line, if it changed.
             */
            void SetStatus(const std::string& status){
                std::lock_guard<std::mutex> lock(mutex_);
                if (status == status_) return;
                status_ = status;
                if (interactive_) Draw();
                else Print(status);
            }

            /**
             * Ends the progress line, so what is written next starts on a line of its own.
             */
            void EndLine(){
                if (!interactive_) return;
                std::lock_guard<std::mutex> lock(mutex_);
                if (drawn_ > 0) std::fputs("\n", stdout);
                std::fflush(stdout);
                drawn_ = 0;
            }

        private:

            /**
             * Writes a message above the progress line.
             */
            void Message(const std::string& message){
                std::lock_guard<std::mutex> lock(mutex_);
                Clear();
                std::fputs((message + "\n").c_str(), stdout);
                Draw();
            }

            /**
             * Erases the progress line. The lock must be held.
             */
            void Clear(){
                if (drawn_ == 0) return;
                std::fputs(("\r" + std::string(drawn_, ' ') + "\r").c_str(), stdout);
                drawn_ = 0;
            }

            /**
             * Draws the progress line over the previous one. The lock must be held.
             */
            void Draw(){
                // One column less than the terminal, so the line never wraps.
                const size_t width = std::max<size_t>(TerminalWidth(), 20) - 1;
                const int percent = std::max(percent_, 0);
                char head[16];
                std::snprintf(head, sizeof(head), "[%3d%%] ", percent);
                std::string line = head;
                size_t columns = line.size();
                // The bar, if there is room for it and some of the status.
                const size_t bar = width >= 60 ? 30 : (width >= 40 ? 15 : 0);
                if (bar > 0){
                    const size_t full = bar * static_cast<size_t>(percent) / 100;
#ifdef _WIN32
                    line += std::string(full, '#') + std::string(bar - full, '-');
#else
                    for (size_t i = 0; i < bar; ++ i) line += i < full ? "\u2588" : "\u2591";
#endif
                    line += " ";
                    columns += bar + 1;
                }
                std::string status = status_;
                if (columns + status.size() > width){
                    status = status.substr(0, width > columns + 3 ? width - columns - 3 : 0);
                    // Don't cut a UTF-8 character.
                    while (!status.empty() && (static_cast<unsigned char>(status.back()) & 0xC0) == 0x80)
                        status.pop_back();
                    if (!status.empty() && (static_cast<unsigned char>(status.back()) & 0x80)) status.pop_back();
                    status += "...";
                }
                line += status;
                columns += status.size();
                // Written over the previous line, padded to cover what is left of it.
                std::string out = "\r" + line;
                if (columns < drawn_) out += std::string(drawn_ - columns, ' ');
                std::fputs(out.c_str(), stdout);
                std::fflush(stdout);
                drawn_ = columns;
            }

            /**
             * True if the output is a terminal.
             */
            const bool interactive_;

            /**
             * True if the messages are coloured.
             */
            const bool colour_;

            /**
             * The last percent shown, -1 if none.
             */
            int percent_ = -1;

            /**
             * What the installer is doing.
             */
            std::string status_;

            /**
             * Columns of the progress line on the screen, 0 if there is none.
             */
            size_t drawn_ = 0;

            /**
             * Lock for the drawing.
             */
            std::mutex mutex_;
    };

    /**
     * Opens the installation log in the installation directory, and shows its messages in the display.
     */
    void OpenInstallLog(const QString& output, bool append, ProgressDisplay& display){
        InstallLog::Get().Open((output + "logs/installer.log").toStdString(), append);
        display.Attach();
    }

    /**
     * Closes the installation log, and writes a summary of the problems.
     *
     * @return The number of errors in the log.
     */
    size_t CloseInstallLog(ProgressDisplay& display){
        InstallLog& log = InstallLog::Get();
        log.EndStep();
        display.Detach();
        const size_t errors = log.Count(InstallLog::ERROR_LEVEL);
        const size_t warnings = log.Count(InstallLog::WARNING);
        const std::string path = log.Path();
        log.Close();
        Print("");
        Print(std::to_string(errors) + " error(s), " + std::to_string(warnings) + " warning(s).");
        for (const std::string& message : log.First(InstallLog::ERROR_LEVEL, 10))
            Print("  " + display.Colour(InstallLog::ERROR_LEVEL, message));
        if (errors > 10) Print("  ...");
        Print("The full log is in " + path);
        return errors;
    }

    /**
     * Checks that the audio tools the installation needs can be used.
     *
     * @param[in] options What is installed.
     * @param[in] output The installation directory.
     * @param[in] yes True to continue without the tools.
     * @return True if the installation can go on.
     */
    bool CheckTools(const InstallOptions& options, const QString& output, bool yes){
        const ExternalTools::Locations& locations = ExternalTools::CurrentLocations();
        const InstallRules::ToolNeeds needs = InstallRules::ToolsNeeded(options);
        const ExternalTools::Tool ffmpeg = needs.ffmpeg ? ExternalTools::FindFfmpeg(locations) : ExternalTools::Tool();
        const ExternalTools::Tool timidity
          = needs.timidity ? ExternalTools::FindTimidity(locations) : ExternalTools::Tool();
        const bool missing_ffmpeg = needs.ffmpeg && !ffmpeg.usable;
        const bool missing_timidity = needs.timidity && !timidity.usable;
        if (needs.ffmpeg && ffmpeg.usable) Print("FFmpeg: " + ffmpeg.path + " (" + ExternalTools::ShortVersion(ffmpeg) + ")");
        if (needs.timidity && timidity.usable)
            Print("TiMidity: " + timidity.path + " (" + ExternalTools::ShortVersion(timidity) + ")");
        const std::vector<std::string> consequences
          = InstallRules::ToolConsequences(options, !missing_ffmpeg, !missing_timidity);
        if (consequences.empty()) return true;

        QStringList reasons;
        if (missing_ffmpeg) reasons << QString::fromStdString("FFmpeg: " + ffmpeg.problem);
        if (missing_timidity) reasons << QString::fromStdString("TiMidity: " + timidity.problem);
        QStringList lost;
        for (const std::string& line : consequences) lost << QString::fromStdString(line);
        PrintList("The installer needs programs it can't use:", reasons);
        PrintList("Without them:", lost);
        if (yes) return true;
        PrintError("");
        PrintError("Install them, copy them to " + QDir::toNativeSeparators(output + "tools").toStdString()
          + ", or give their paths with --ffmpeg and --timidity.");
#if defined(_WIN32)
        PrintError("Windows: download a build of FFmpeg (ffmpeg.exe) from https://ffmpeg.org/download.html. "
          "TiMidity++ also needs its timidity.cfg and instrument files next to timidity.exe.");
#elif defined(__APPLE__)
        PrintError("macOS: brew install ffmpeg timidity");
#else
        PrintError("Debian, Ubuntu: sudo apt install ffmpeg timidity");
        PrintError("Fedora: sudo dnf install ffmpeg timidity++");
        PrintError("Arch: sudo pacman -S ffmpeg timidity++");
#endif
        PrintError("To install without them, use --yes.");
        return false;
    }

    /**
     * Extracts the videos of a game disk.
     *
     * @param[in] disk_image The game disk.
     * @param[in] output The installation directory.
     * @param[in] strict True to end with an error status if the log has errors.
     * @return The exit status.
     */
    int ExtractVideos(DiskImage& disk_image, const QString& output, bool strict){
        const QStringList errors = InstallSteps::CheckVideoOutput(output);
        if (!errors.isEmpty()){
            PrintList("The extraction can't start:", errors);
            return CliInstaller::EXIT_FAILED;
        }
        // The installation log of a previous run is kept.
        ProgressDisplay progress;
        const QString problem = InstallSteps::PrepareInstallation(output);
        if (!problem.isEmpty()){
            PrintError("Could not prepare the installation directory: " + problem.toStdString());
            return CliInstaller::EXIT_FAILED;
        }
        OpenInstallLog(output, true, progress);
        InstallLog::Get().Add("Extracting the videos of " + disk_image.getId() + "...");
        int extracted = 0;
        int skipped = 0;
        bool ok = false;
        try{
            ok = disk_image.extractVideos(
              output.toStdString() + "game/", [&progress](float value){progress.SetProgress(value);},
              [](const std::string& message){InstallLog::Get().Add(message);}, &extracted, &skipped
            );
            if (ok) InstallSteps::ConvertMovies(output.toStdString() + "game/video/");
        }
        catch (const std::exception& ex){
            InstallLog::Get().Add(std::string("[ERROR] Video extraction failed: ") + ex.what());
            ok = false;
        }
        if (ok){
            progress.SetProgress(100);
            InstallSteps::MarkVideosInstalled(output);
            InstallLog::Get().Add(
              "Videos extracted: " + std::to_string(extracted) + " new, " + std::to_string(skipped)
              + " already installed."
            );
        }
        else InstallLog::Get().Add("[ERROR] Some videos could not be extracted.");
        const size_t logged_errors = CloseInstallLog(progress);
        if (!ok) return CliInstaller::EXIT_FAILED;
        return strict && logged_errors > 0 ? CliInstaller::EXIT_ERRORS : CliInstaller::EXIT_OK;
    }

    /**
     * Installs the game from the install disk.
     *
     * @param[in] arguments The command line.
     * @param[in] disk_image The install disk.
     * @param[in] output The installation directory.
     * @return The exit status.
     */
    int Install(const InstallCommandLine::Arguments& arguments, DiskImage& disk_image, const QString& output){
        const InstallOptions& options = arguments.options;
        bool anything = false;
        for (const InstallCommandLine::Part& part : InstallCommandLine::Parts()) anything |= options.*(part.option);
        if (!anything){
            PrintError("Nothing to install. Choose a preset, or parts with --with.");
            return CliInstaller::EXIT_USAGE;
        }
        Print("Installing:");
        for (const InstallCommandLine::Part& part : InstallCommandLine::Parts())
            if (options.*(part.option)) Print("  - " + std::string(part.description));

        // The V-Gears data folder to copy.
        QString game_data;
        if (arguments.copy_data){
            game_data = arguments.data_dir.empty()
              ? InstallSteps::FindGameDataDir() : QDir::cleanPath(QString::fromStdString(arguments.data_dir));
            if (game_data.isEmpty()){
                PrintError("The V-Gears 'data' folder was not found. Give it with --data-dir.");
                return CliInstaller::EXIT_FAILED;
            }
            Print("V-Gears data files: " + QDir::toNativeSeparators(game_data).toStdString());
        }

        // Check everything that can be checked before touching the disk.
        InstallSteps::PreflightResult preflight
          = InstallSteps::RunPreflight(
          disk_image, output, arguments.copy_data, game_data, arguments.clean
        );
        for (
          const std::string& problem
          : InstallRules::Problems(
            options, !arguments.clean && QFile::exists(output + "game/field/maps/_fields.xml")
          )
        ) preflight.errors << QString::fromStdString(problem);
        if (!preflight.errors.isEmpty()){
            PrintList("The installation can't start:", preflight.errors);
            return CliInstaller::EXIT_FAILED;
        }
        if (!preflight.warnings.isEmpty()){
            PrintList("Warnings:", preflight.warnings);
            if (!arguments.yes){
                PrintError("To install anyway, use --yes.");
                return CliInstaller::EXIT_FAILED;
            }
        }
        // The audio is converted with external programs: say now, before anything is extracted, if they are missing.
        if (!CheckTools(options, output, arguments.yes)) return CliInstaller::EXIT_FAILED;

        // Everything is checked: the previous installation can be deleted.
        if (arguments.clean){
            const std::string root = output.toStdString();
            if (!InstallLayout::IsInstallation(root)) Print("Nothing to clean: there is no installation in the directory.");
            else{
                Print("Deleting the previous installation (the saves, the controls, the game configuration and the "
                  "videos are kept)...");
                const InstallLayout::CleanResult cleaned = InstallLayout::CleanInstallation(root);
                if (!cleaned.errors.empty()){
                    QStringList errors;
                    for (const std::string& error : cleaned.errors) errors << QString::fromStdString(error);
                    PrintList("The previous installation could not be deleted:", errors);
                    return CliInstaller::EXIT_FAILED;
                }
                Print("Deleted " + std::to_string(cleaned.removed) + " files and folders.");
            }
        }

        // From here on, the installation directory is being changed: until it finishes, the record says so.
        InstallSteps::MarkInstallStarted(output, options, disk_image.getId());

        // The progress of the whole installation: the extraction, the copy and the conversion (see
        // InstallSteps::OverallProgress).
        ProgressDisplay progress;
        // Ends the progress line before writing why the installation stops.
        const auto fail = [&progress](const std::string& message){
            progress.EndLine();
            PrintError(message);
            return CliInstaller::EXIT_FAILED;
        };
        progress.SetStatus("Extracting the install disk...");
        if (!disk_image.extractImage(output.toStdString(), [&progress](float value){
              progress.SetProgress(InstallSteps::OverallProgress(InstallSteps::Phase::EXTRACT, value));
            })) return fail("Failed to extract the ISO image.");
        if (disk_image.getContentPath().empty()) return fail("The extracted data path is invalid.");
        const std::vector<std::string> missing_files = InstallSteps::MissingRequiredFiles(disk_image);
        if (!missing_files.empty()){
            progress.EndLine();
            QStringList names;
            for (const std::string& file : missing_files) names << QString::fromStdString(file);
            PrintList("Some of the required files were not found among the extracted data:", names);
            return CliInstaller::EXIT_FAILED;
        }
        // Copy the V-Gears data files (scripts, screens, fonts...) to the installation directory.
        if (arguments.copy_data){
            progress.SetStatus("Copying the V-Gears data files...");
            if (!InstallSteps::CopyGameFiles(game_data, output, [&progress](float value){
                  progress.SetProgress(InstallSteps::OverallProgress(InstallSteps::Phase::COPY, value));
                })) return fail("Failed to copy the V-Gears data files to the installation directory.");
        }
        // The installer's Ogre setup loads plugins.cfg and resources.cfg from the working directory (and writes
        // logs/ogre.log there), so run from the installation directory.
        if (!QFile::exists(output + "plugins.cfg") || !QFile::exists(output + "resources.cfg"))
            return fail("The installation directory must contain plugins.cfg and resources.cfg. Don't use --no-copy-data.");
        if (!QDir::setCurrent(output)) return fail("Could not enter the installation directory.");
        const QString problem = InstallSteps::PrepareInstallation(output);
        if (!problem.isEmpty()) return fail("Could not prepare the installation directory: " + problem.toStdString());

        OpenInstallLog(output, false, progress);
        if (options.videos){
            InstallLog::Get().Add(
              "[WARNING] The install disk has no videos. To extract them, run the installer again with game disk 1, "
              "2 or 3 as the ISO."
            );
        }
        const DataInstaller::AdvancedOptions advanced
          = InstallSteps::ToAdvancedOptions(options, arguments.keep_originals);
        const auto write_line = [](const std::string line, int, bool){InstallLog::Get().Add(line);};
        std::string error;
        try{
            DataInstaller installer(disk_image, QDir::toNativeSeparators(output).toStdString(), advanced, write_line);
            float value = 0;
            while (value < 100){
                value = installer.Progress();
                progress.SetProgress(InstallSteps::OverallProgress(InstallSteps::Phase::CONVERT, value));
            }
        }
        catch (const std::exception& ex){
            error = ex.what();
        }
        catch (...){
            error = "Unknown error";
        }
        if (!error.empty()) InstallLog::Get().Add("[ERROR] Installation failed: " + error);
        else InstallSteps::MarkInstallFinished(output);
        const size_t errors = CloseInstallLog(progress);
        if (!error.empty()) return CliInstaller::EXIT_FAILED;
        Print("Installation finished in " + QDir::toNativeSeparators(output).toStdString());
        if (arguments.strict && errors > 0){
            Print("It finished with errors, and --strict was given: the exit status is 3.");
            return CliInstaller::EXIT_ERRORS;
        }
        return CliInstaller::EXIT_OK;
    }

    /**
     * Gets the installation directory of the command line, or the default one.
     *
     * @return The directory, normalized. Empty if there is none.
     */
    QString OutputDir(const InstallCommandLine::Arguments& arguments){
        const QString dir = arguments.output.empty()
          ? InstallSteps::DefaultInstallDir() : QString::fromStdString(arguments.output);
        return dir.isEmpty() ? QString() : InstallSteps::NormalizeOutputDir(dir);
    }

    /**
     * Verifies an installation.
     *
     * @return The exit status.
     */
    int Verify(const InstallCommandLine::Arguments& arguments){
        const QString output = OutputDir(arguments);
        if (output.isEmpty() || !QDir(output).exists()){
            PrintError("The installation directory does not exist: " + output.toStdString());
            return CliInstaller::EXIT_FAILED;
        }
        const QString game_data = arguments.data_dir.empty()
          ? InstallSteps::FindGameDataDir() : QString::fromStdString(arguments.data_dir);
        Print("Verifying " + QDir::toNativeSeparators(output).toStdString() + "...");
        const VerifyResult result = VerifyInstallation(
          output.toStdString(), game_data.toStdString(), InstallSteps::PluginSearch()
        );
        if (result.Ok()){
            Print("Everything the game needs is installed (" + std::to_string(result.checked) + " checks).");
            return CliInstaller::EXIT_OK;
        }
        std::string group;
        for (const VerifyProblem& problem : result.problems){
            if (problem.group != group){
                group = problem.group;
                Print(group + ":");
            }
            Print("  - " + problem.message);
        }
        Print(
          std::to_string(result.problems.size()) + " problem(s) in " + std::to_string(result.checked) + " checks. If "
          "you skipped a part of the installation on purpose (for example the music), its files are expected to be "
          "missing."
        );
        return CliInstaller::EXIT_FAILED;
    }

}

namespace{

    /**
     * Ends the program when it is interrupted (Ctrl+C) or asked to end.
     *
     * The progress line is ended, and the user is told that the installation didn't finish: the record of the
     * installation directory already says so. Only what is safe in a signal handler is used.
     */
    extern "C" void OnInterrupt(int){
        static const char message[] = "\n\nInterrupted: the installation didn't finish. Run the installer again.\n";
#ifdef _WIN32
        _write(2, message, sizeof(message) - 1);
#else
        const ssize_t written = write(2, message, sizeof(message) - 1);
        (void)written;
#endif
        std::_Exit(CliInstaller::EXIT_INTERRUPTED);
    }

}

namespace CliInstaller{

    int Run(const InstallCommandLine::Arguments& arguments, const std::string& program){
        switch (arguments.mode){
            case InstallCommandLine::Mode::HELP:
                std::fputs(InstallCommandLine::Usage(program).c_str(), stdout);
                return EXIT_OK;
            case InstallCommandLine::Mode::VERSION:
                Print("V-Gears Installer " + std::string(VGEARS_INSTALLER_VERSION));
                return EXIT_OK;
            case InstallCommandLine::Mode::LIST_PARTS:
                for (const InstallCommandLine::Part& part : InstallCommandLine::Parts()){
                    char line[128];
                    std::snprintf(line, sizeof(line), "%-18s %s", part.name, part.description);
                    Print(line);
                }
                return EXIT_OK;
            case InstallCommandLine::Mode::INVALID:
                PrintError(arguments.error);
                PrintError("Run '" + program + " --help' for the usage.");
                return EXIT_USAGE;
            case InstallCommandLine::Mode::VERIFY:
                return Verify(arguments);
            case InstallCommandLine::Mode::INSTALL:
                std::signal(SIGINT, OnInterrupt);
                std::signal(SIGTERM, OnInterrupt);
                break;
            case InstallCommandLine::Mode::GUI:
                PrintError("The installer window can't be opened from the command line installer.");
                return EXIT_USAGE;
        }

        const QString output = OutputDir(arguments);
        if (output.isEmpty()){
            PrintError("No installation directory: give one with --output.");
            return EXIT_USAGE;
        }
        Print("Installation directory: " + QDir::toNativeSeparators(output).toStdString());

        // The paths given must be the programs, and work.
        if (!arguments.ffmpeg.empty()){
            const ExternalTools::Tool tool = ExternalTools::ProbeFfmpeg(arguments.ffmpeg);
            if (!tool.usable){
                PrintError("FFmpeg can't be used: " + tool.problem);
                return EXIT_FAILED;
            }
        }
        if (!arguments.timidity.empty()){
            const ExternalTools::Tool tool = ExternalTools::ProbeTimidity(arguments.timidity);
            if (!tool.usable){
                PrintError("TiMidity can't be used: " + tool.problem);
                return EXIT_FAILED;
            }
        }
        InstallSteps::SetToolLocations(output, arguments.ffmpeg, arguments.timidity);

        if (!QFileInfo(QString::fromStdString(arguments.iso)).isReadable()){
            PrintError("The ISO image can't be read: " + arguments.iso);
            return EXIT_FAILED;
        }
        // The working directory changes during installation, so the path must be absolute.
        DiskImage disk_image(QFileInfo(QString::fromStdString(arguments.iso)).absoluteFilePath().toStdString());
        Print("Disk: " + disk_image.getId());
        if (!disk_image.isValid()){
            PrintError("The ISO is not a valid Final Fantasy VII image: " + disk_image.getErrorMessage());
            return EXIT_FAILED;
        }
        // A game disk only has videos: extract them, without the rest of the installation.
        if (disk_image.isVideoDisk()){
            if (arguments.clean){
                PrintError("--clean is for the install disk: a game disk only adds its videos to an installation.");
                return EXIT_USAGE;
            }
            Print("Game disk: only its videos will be extracted.");
            return ExtractVideos(disk_image, output, arguments.strict);
        }
        return Install(arguments, disk_image, output);
    }

}
