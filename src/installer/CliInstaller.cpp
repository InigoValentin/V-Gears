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
#include <memory>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include "CliInstaller.h"
#include "DataInstaller.h"
#include "DiskImage.h"
#include "ExternalTools.h"
#include "InstallLog.h"
#include "InstallSteps.h"
#include "InstallVerifier.h"

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
     * Shows the progress of a task, one line for each percent.
     */
    class ProgressPrinter{

        public:

            /**
             * Shows the progress, if it has changed since it was last shown.
             *
             * @param[in] progress The progress [0-100].
             */
            void Show(float progress){
                const int percent = static_cast<int>(std::floor(progress));
                if (percent <= last_) return;
                last_ = percent;
                char line[32];
                std::snprintf(line, sizeof(line), "[%3d%%]", percent);
                Print(line);
            }

        private:

            int last_ = -1;
    };

    /**
     * Opens the installation log in the installation directory.
     *
     * Its messages are already echoed to the terminal by the log itself, so no sink is needed.
     */
    void OpenInstallLog(const QString& output, bool append){
        InstallLog::Get().Open((output + "installer.log").toStdString(), append);
    }

    /**
     * Closes the installation log, and writes a summary of the problems.
     */
    void CloseInstallLog(){
        InstallLog& log = InstallLog::Get();
        log.EndStep();
        const size_t errors = log.Count(InstallLog::ERROR_LEVEL);
        const size_t warnings = log.Count(InstallLog::WARNING);
        const std::string path = log.Path();
        log.Close();
        Print("");
        Print(std::to_string(errors) + " error(s), " + std::to_string(warnings) + " warning(s).");
        for (const std::string& message : log.First(InstallLog::ERROR_LEVEL, 10)) Print("  " + message);
        if (errors > 10) Print("  ...");
        Print("The full log is in " + path);
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
     * @return The exit status.
     */
    int ExtractVideos(DiskImage& disk_image, const QString& output){
        const QStringList errors = InstallSteps::CheckVideoOutput(output);
        if (!errors.isEmpty()){
            PrintList("The extraction can't start:", errors);
            return CliInstaller::EXIT_FAILED;
        }
        // The installation log of a previous run is kept.
        OpenInstallLog(output, true);
        InstallLog::Get().Add("Extracting the videos of " + disk_image.getId() + "...");
        ProgressPrinter progress;
        int extracted = 0;
        int skipped = 0;
        bool ok = false;
        try{
            ok = disk_image.extractVideos(
              output.toStdString() + "data/", [&progress](float value){progress.Show(value);},
              [](const std::string& message){InstallLog::Get().Add(message);}, &extracted, &skipped
            );
            if (ok) InstallSteps::ConvertMovies(output.toStdString() + "data/video/");
        }
        catch (const std::exception& ex){
            InstallLog::Get().Add(std::string("[ERROR] Video extraction failed: ") + ex.what());
            ok = false;
        }
        if (ok){
            progress.Show(100);
            InstallLog::Get().Add(
              "Videos extracted: " + std::to_string(extracted) + " new, " + std::to_string(skipped)
              + " already installed."
            );
        }
        else InstallLog::Get().Add("[ERROR] Some videos could not be extracted.");
        CloseInstallLog();
        return ok ? CliInstaller::EXIT_OK : CliInstaller::EXIT_FAILED;
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
          = InstallSteps::RunPreflight(disk_image, output, arguments.copy_data, game_data);
        for (
          const std::string& problem
          : InstallRules::Problems(options, QFile::exists(output + "data/fields/_fields.xml"))
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

        Print("Extracting " + disk_image.getPath() + "...");
        if (!disk_image.extractImage(output.toStdString())){
            PrintError("Failed to extract the ISO image.");
            return CliInstaller::EXIT_FAILED;
        }
        if (disk_image.getContentPath().empty()){
            PrintError("The extracted data path is invalid.");
            return CliInstaller::EXIT_FAILED;
        }
        const std::vector<std::string> missing_files = InstallSteps::MissingRequiredFiles(disk_image);
        if (!missing_files.empty()){
            QStringList names;
            for (const std::string& file : missing_files) names << QString::fromStdString(file);
            PrintList("Some of the required files were not found among the extracted data:", names);
            return CliInstaller::EXIT_FAILED;
        }
        // Copy the V-Gears data files (scripts, screens, fonts...) to the installation directory.
        if (arguments.copy_data){
            Print("Copying the V-Gears data files...");
            if (!InstallSteps::CopyDirectory(game_data, output)){
                PrintError("Failed to copy the V-Gears data files to the installation directory.");
                return CliInstaller::EXIT_FAILED;
            }
        }
        // The installer's Ogre setup loads plugins.cfg and resources.cfg from the working directory (and writes
        // install.log there), so run from the installation directory.
        if (!QFile::exists(output + "plugins.cfg") || !QFile::exists(output + "resources.cfg")){
            PrintError("The installation directory must contain plugins.cfg and resources.cfg. Don't use --no-copy-data.");
            return CliInstaller::EXIT_FAILED;
        }
        if (!QDir::setCurrent(output)){
            PrintError("Could not enter the installation directory.");
            return CliInstaller::EXIT_FAILED;
        }

        OpenInstallLog(output, false);
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
        ProgressPrinter progress;
        try{
            DataInstaller installer(disk_image, QDir::toNativeSeparators(output).toStdString(), advanced, write_line);
            float value = 0;
            while (value < 100){
                value = installer.Progress();
                progress.Show(value);
            }
        }
        catch (const std::exception& ex){
            error = ex.what();
        }
        catch (...){
            error = "Unknown error";
        }
        if (!error.empty()) InstallLog::Get().Add("[ERROR] Installation failed: " + error);
        CloseInstallLog();
        if (!error.empty()) return CliInstaller::EXIT_FAILED;
        Print("Installation finished in " + QDir::toNativeSeparators(output).toStdString());
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
        const VerifyResult result = VerifyInstallation(output.toStdString(), game_data.toStdString());
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
            Print("Game disk: only its videos will be extracted.");
            return ExtractVideos(disk_image, output);
        }
        return Install(arguments, disk_image, output);
    }

}
