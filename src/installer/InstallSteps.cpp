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

#include <iostream>
#include <boost/filesystem.hpp>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QObject>
#include <QtCore/QStorageInfo>
#include <OgrePrerequisites.h>
#include "common/DataDirectory.h"
#include "common/OgrePlugins.h"
#include "ExternalTools.h"
#include "InstallLog.h"
#include "InstallSteps.h"
#include "MovieCamera.h"

#ifndef VGEARS_OGRE_PLUGIN_DIR
#define VGEARS_OGRE_PLUGIN_DIR ""
#endif

namespace{

    /**
     * Minimum free space needed besides the ISO extraction: converted data, intermediate files and logs.
     */
    const qint64 kRequiredFreeSpace = 3LL * 1024 * 1024 * 1024;

    /**
     * Free space needed to extract the videos of a game disk.
     */
    const qint64 kRequiredVideoSpace = 1024LL * 1024 * 1024;

    /**
     * Checks that a directory can be created and written, and that it has enough free space.
     *
     * @param[in] output The directory, ending with '/'.
     * @param[in] needed Free space needed, in bytes.
     * @param[out] errors The problems found are added here.
     */
    void CheckWritable(const QString& output, qint64 needed, QStringList& errors){
        if (!QDir().mkpath(output)){
            errors << QObject::tr("The installation directory can't be created: %1").arg(output);
            return;
        }
        QFile probe(output + ".v-gears-write-test");
        if (!probe.open(QIODevice::WriteOnly))
            errors << QObject::tr("The installation directory is not writable: %1").arg(output);
        else{
            probe.close();
            probe.remove();
        }
        const QStorageInfo storage(output);
        if (storage.isValid() && storage.bytesAvailable() < needed){
            errors << QObject::tr("Not enough free space in %1: %2 MB available, about %3 MB needed.")
              .arg(storage.rootPath()).arg(storage.bytesAvailable() / (1024 * 1024)).arg(needed / (1024 * 1024));
        }
    }

    /**
     * Names of the plugins in the plugins.cfg of a folder that the game won't find.
     *
     * The game looks for them in the same places (see VGears::OgrePlugins::SearchDirectories), so they don't need to
     * be copied to the installation directory.
     */
    QStringList MissingOgrePlugins(const QString& source_dir){
        namespace bfs = boost::filesystem;
        QStringList missing;
        const bfs::path dir(source_dir.toStdString());
        VGears::OgrePlugins::Config config;
        if (!VGears::OgrePlugins::ReadConfig(dir / "plugins.cfg", config)) return missing;
        const std::vector<bfs::path> directories = VGears::OgrePlugins::SearchDirectories(
          dir, config.folder, bfs::path(QCoreApplication::applicationDirPath().toStdString()), VGEARS_OGRE_PLUGIN_DIR
        );
        const std::string version = std::to_string(OGRE_VERSION_MAJOR) + "." + std::to_string(OGRE_VERSION_MINOR)
          + "." + std::to_string(OGRE_VERSION_PATCH);
        for (const std::string& name : config.plugins){
            if (VGears::OgrePlugins::Find(name, directories, version).empty()) missing << QString::fromStdString(name);
        }
        return missing;
    }

}

namespace InstallSteps{

    QString DefaultInstallDir(){
        const boost::filesystem::path dir = VGears::DataDirectory::DefaultDirectory();
        return dir.empty() ? QString() : QDir::fromNativeSeparators(QString::fromStdString(dir.string()));
    }

    QString NormalizeOutputDir(const QString& dir){
        QString output = QDir::fromNativeSeparators(dir);
        if (QDir::isRelativePath(output)) output = QDir(output).absolutePath();
        if (!output.endsWith("/")) output += "/";
        return output;
    }

    QString FindGameDataDir(){
        // The same places the game copies its files from when the data directory is empty, then the current directory.
        std::vector<boost::filesystem::path> candidates = VGears::DataDirectory::TemplateCandidates(
          boost::filesystem::path(QCoreApplication::applicationDirPath().toStdString())
        );
        candidates.emplace_back((QDir::currentPath() + "/data").toStdString());
        candidates.emplace_back((QDir::currentPath() + "/../data").toStdString());
        const boost::filesystem::path found = VGears::DataDirectory::FindTemplate(candidates, boost::filesystem::path());
        return found.empty() ? QString() : QDir::cleanPath(QString::fromStdString(found.string()));
    }

    PreflightResult RunPreflight(
      DiskImage& disk_image, const QString& output, bool copy_data, const QString& game_data_dir
    ){
        PreflightResult result;
        const QString iso_path = QString::fromStdString(disk_image.getPath());

        // Input.
        if (iso_path.isEmpty()) result.errors << QObject::tr("No ISO image selected.");
        else if (!QFileInfo(iso_path).isReadable())
            result.errors << QObject::tr("The ISO image can't be read: %1").arg(iso_path);
        else if (!disk_image.isValid())
            result.errors << QObject::tr("The selected ISO is not a valid Final Fantasy VII image.");
        else if (!disk_image.isSupported())
            result.warnings << QString::fromStdString(disk_image.getWarningMessage());

        // Output: it must be creatable and writable, with room for the extracted ISO plus the converted data.
        CheckWritable(
          output, kRequiredFreeSpace + (iso_path.isEmpty() ? 0 : QFileInfo(iso_path).size()), result.errors
        );

        // Configuration and plugins.
        if (copy_data){
            if (
              game_data_dir.isEmpty() || !QFile::exists(game_data_dir + "/plugins.cfg")
              || !QDir(game_data_dir + "/data").exists()
            ){
                result.errors << QObject::tr(
                  "The V-Gears 'data' folder was not found (it must contain plugins.cfg and a data folder)."
                );
            }
            else{
                const QStringList missing = MissingOgrePlugins(game_data_dir);
                if (!missing.isEmpty()){
                    result.errors << QObject::tr("Ogre plugins not found in the data folder or the system: %1")
                      .arg(missing.join(", "));
                }
            }
        }
        else{
            if (!QFile::exists(output + "plugins.cfg") || !QFile::exists(output + "resources.cfg")){
                result.errors << QObject::tr(
                  "The installation directory has no plugins.cfg and resources.cfg. Enable 'Copy V-Gears data files'."
                );
            }
            else{
                const QStringList missing = MissingOgrePlugins(output);
                if (!missing.isEmpty()){
                    result.errors << QObject::tr(
                      "Ogre plugins not found in the installation directory or the system: %1"
                    ).arg(missing.join(", "));
                }
            }
        }
        return result;
    }

    QStringList CheckVideoOutput(const QString& output){
        QStringList errors;
        CheckWritable(output, kRequiredVideoSpace, errors);
        return errors;
    }

    bool CopyDirectory(const QString& src, const QString& dst){
        const QDir src_dir(src);
        if (!QDir().mkpath(dst)) return false;
        bool ok = true;
        QDirIterator it(src, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (it.hasNext()){
            const QString file = it.next();
            const QString target = dst + "/" + src_dir.relativeFilePath(file);
            QDir().mkpath(QFileInfo(target).absolutePath());
            if (QFile::exists(target)) QFile::remove(target);
            if (!QFile::copy(file, target)){
                std::cout << "Could not copy " << file.toStdString() << " to " << target.toStdString() << std::endl;
                ok = false;
            }
        }
        return ok;
    }

    std::vector<std::string> MissingRequiredFiles(DiskImage& disk_image){
        // TODO: Enumerate files or find some better way to do this.
        static const std::vector<std::string> required_files = {
          "data/field/char.lgp",
          "data/field/flevel.lgp",
          "data/kernel/KERNEL.BIN",
          "data/menu/menu_us.lgp",
          "data/sound/audio.fmt",
          "data/sound/audio.dat",
          "data/music/music.idx",
          "data/midi/midi.lgp",
          "data/battle/scene.bin",
          "data/battle/battle.lgp",
          "data/battle/magic.lgp",
          "data/wm/WM0.MAP",
          "data/wm/WM2.MAP",
          "data/wm/WM3.MAP",
          "data/wm/world_us.lgp",
          "ff7.exe"
        };
        std::vector<std::string> missing;
        for (const std::string& file : required_files){
            std::cout << "Checking for required file: " << file << std::endl;
            if (!disk_image.filePath(file)) missing.push_back(file);
        }
        return missing;
    }

    void ConvertMovies(const std::string& video_dir){
        const ExternalTools::Tool ffmpeg = ExternalTools::FindFfmpeg(ExternalTools::CurrentLocations());
        if (!ffmpeg.usable){
            InstallLog::Get().Add(
              "[WARNING] FFmpeg can't be used, so the videos are not converted and the game can't play them: "
              + ffmpeg.problem
            );
            return;
        }
        QDir directory(QString::fromStdString(video_dir));
        for (const QString& file : directory.entryList({"*.avi"}, QDir::Files)){
            const std::string name = QFileInfo(file).completeBaseName().toStdString();
            const std::string destination = video_dir + name;
            if (QFileInfo::exists(QString::fromStdString(destination + "/movie.xml"))) continue;
            InstallLog::Get().Add("Converting " + name + ".avi...");
            std::string error;
            if (ExternalTools::ConvertMovie(
                  ffmpeg.path, video_dir + file.toStdString(), destination, name,
                  MovieCamera::LoadCamera(video_dir + "moviecam.lgp", name), &error
                ))
                InstallLog::Get().Add("Converted " + name);
            else InstallLog::Get().Add("[WARNING] Could not convert " + name + ": " + error);
        }
    }

    void SetToolLocations(const QString& install_dir, const std::string& ffmpeg, const std::string& timidity){
        QString dir = QDir::fromNativeSeparators(install_dir);
        if (!dir.isEmpty() && !dir.endsWith("/")) dir += "/";
        ExternalTools::Locations& tools = ExternalTools::CurrentLocations();
        tools.ffmpeg = ffmpeg;
        tools.timidity = timidity;
        const QString app_dir = QCoreApplication::applicationDirPath();
        tools.directories.clear();
        if (!dir.isEmpty()) tools.directories.push_back((dir + "tools").toStdString());
        tools.directories.push_back((app_dir + "/tools").toStdString());
        tools.directories.push_back(app_dir.toStdString());
    }

    DataInstaller::AdvancedOptions ToAdvancedOptions(const InstallOptions& options, bool keep_originals){
        DataInstaller::AdvancedOptions advanced;
        advanced.battle_data = options.battle_data;
        advanced.battle_models = options.battle_models;
        advanced.kernel = options.kernel;
        advanced.images = options.images;
        advanced.sounds = options.sounds;
        advanced.music = options.music;
        advanced.videos = false;
        advanced.fields = options.fields;
        advanced.field_models = options.fields && options.field_models;
        advanced.wm = options.wm;
        advanced.wm_models = options.wm && options.wm_models;
        advanced.keep_originals = keep_originals;
        advanced.clean_install = false;
        return advanced;
    }

}
