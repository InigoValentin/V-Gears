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
#include <sstream>
#include <iomanip>
#include <QtCore/QProcess>
#include <QtWidgets/QFileDialog>
#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QCoreApplication>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QStorageInfo>
#include <chrono>
#include "DataInstaller.h"
#include "ExternalTools.h"
#include "InstallLayout.h"
#include "InstallLog.h"
#include "InstallOptions.h"
#include "InstallVerifier.h"
#include "MainWindow.h"
#include <ui_MainWindow.h>

/**
 * Indicates if an installer has already been created.
 */
static bool installer_created = false;

/**
 * Finds the V-Gears "data" folder shipped with the game.
 *
 * It contains the configuration files and plugins (copied to the installation directory) and a "data" subfolder with
 * the game data (copied to <installation directory>/data).
 *
 * @return Path of the folder, or an empty string if it can't be found.
 */
static QString FindGameDataDir(){
    const QString app = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
      app + "/data", app + "/../data", app + "/../../data", app + "/../share/v-gears",
      "/usr/share/v-gears", QDir::currentPath() + "/data", QDir::currentPath() + "/../data"
    };
    for (const QString& c : candidates)
        if (QDir(c + "/data").exists() && QFile::exists(c + "/plugins.cfg")) return QDir::cleanPath(c);
    return QString();
}

/**
 * Directories where the system's Ogre plugins are usually installed.
 */
static const QStringList kSystemPluginDirs = {
  "/usr/lib/x86_64-linux-gnu/OGRE", "/usr/lib/OGRE", "/usr/lib64/OGRE", "/usr/local/lib/OGRE",
  "/usr/lib/aarch64-linux-gnu/OGRE"
};

/**
 * Gets the names of the plugins enabled in a plugins.cfg file.
 */
static QStringList ListOgrePlugins(const QString& cfg_path){
    QStringList plugins;
    QFile cfg(cfg_path);
    if (!cfg.open(QIODevice::ReadOnly | QIODevice::Text)) return plugins;
    while (!cfg.atEnd()){
        const QString line = QString::fromUtf8(cfg.readLine()).trimmed();
        if (line.startsWith("Plugin=") && !line.mid(7).trimmed().isEmpty()) plugins << line.mid(7).trimmed();
    }
    return plugins;
}

/**
 * Finds a plugin library in a directory.
 *
 * @return Directory-relative file names of the library (it can have several names), empty if not found.
 */
static QStringList FindPluginFiles(const QString& dir, const QString& name){
    return QDir(dir).entryList({name + ".so*", name + ".dll", name + ".dylib"}, QDir::Files);
}

/**
 * Names of the plugins in the plugins.cfg of a folder that are neither in it nor in the system's Ogre folders.
 */
static QStringList MissingOgrePlugins(const QString& source_dir){
    QStringList missing;
    for (const QString& name : ListOgrePlugins(source_dir + "/plugins.cfg")){
        bool found = !FindPluginFiles(source_dir, name).isEmpty();
        for (const QString& dir : kSystemPluginDirs) found = found || !FindPluginFiles(dir, name).isEmpty();
        if (!found) missing << name;
    }
    return missing;
}

/**
 * Copies the Ogre plugins listed in the installation's plugins.cfg that are not already present there (for example
 * the stock plugins shipped with the system's Ogre) from the system's Ogre plugin directory.
 *
 * @return Names of the plugins that couldn't be found anywhere.
 */
static QStringList CopyMissingOgrePlugins(const QString& install_dir){
    QStringList missing;
    for (const QString& name : ListOgrePlugins(install_dir + "plugins.cfg")){
        if (!FindPluginFiles(install_dir, name).isEmpty()) continue;
        bool found = false;
        for (const QString& dir : kSystemPluginDirs){
            for (const QString& f : FindPluginFiles(dir, name)){
                if (QFile::copy(dir + "/" + f, install_dir + f)) found = true;
            }
            if (found) break;
        }
        if (!found) missing << name;
    }
    return missing;
}

/**
 * Minimum free space needed besides the ISO extraction: converted data, intermediate files and logs.
 */
static const qint64 kRequiredFreeSpace = 3LL * 1024 * 1024 * 1024;

/**
 * Result of the checks done before starting an installation.
 */
struct PreflightResult{
    QStringList errors;
    QStringList warnings;
};

/**
 * Checks that an installation can succeed before anything is extracted or written.
 *
 * @param[in] disk_image The selected disk image.
 * @param[in] output Absolute installation directory, ending with '/'.
 * @param[in] copy_data True if the V-Gears data files will be copied.
 * @param[in] game_data_dir The V-Gears "data" folder to copy (only used if copy_data is true).
 */
static PreflightResult RunPreflight(
  DiskImage& disk_image, const QString& output, bool copy_data, const QString& game_data_dir
){
    PreflightResult result;
    const QString iso_path = QString::fromStdString(disk_image.getPath());

    // Input.
    if (iso_path.isEmpty()) result.errors << QObject::tr("No ISO image selected.");
    else if (!QFileInfo(iso_path).isReadable()) result.errors << QObject::tr("The ISO image can't be read: %1").arg(iso_path);
    else if (!disk_image.isValid()) result.errors << QObject::tr("The selected ISO is not a valid Final Fantasy VII image.");
    else if (!disk_image.isSupported()) result.warnings << QString::fromStdString(disk_image.getWarningMessage());

    // Output: it must be creatable and writable.
    if (!QDir().mkpath(output)) result.errors << QObject::tr("The installation directory can't be created: %1").arg(output);
    else{
        QFile probe(output + ".v-gears-write-test");
        if (!probe.open(QIODevice::WriteOnly)) result.errors << QObject::tr("The installation directory is not writable: %1").arg(output);
        else{
            probe.close();
            probe.remove();
        }
        // Free space: the extracted ISO plus the converted data.
        QStorageInfo storage(output);
        const qint64 needed = kRequiredFreeSpace + (iso_path.isEmpty() ? 0 : QFileInfo(iso_path).size());
        if (storage.isValid() && storage.bytesAvailable() < needed){
            result.errors << QObject::tr("Not enough free space in %1: %2 MB available, about %3 MB needed.")
              .arg(storage.rootPath()).arg(storage.bytesAvailable() / (1024 * 1024)).arg(needed / (1024 * 1024));
        }
    }

    // Configuration and plugins.
    if (copy_data){
        if (game_data_dir.isEmpty() || !QFile::exists(game_data_dir + "/plugins.cfg") || !QDir(game_data_dir + "/data").exists())
            result.errors << QObject::tr("The V-Gears 'data' folder was not found (it must contain plugins.cfg and a data folder).");
        else{
            const QStringList missing = MissingOgrePlugins(game_data_dir);
            if (!missing.isEmpty())
                result.errors << QObject::tr("Ogre plugins not found in the data folder or the system: %1").arg(missing.join(", "));
        }
    }
    else{
        if (!QFile::exists(output + "plugins.cfg") || !QFile::exists(output + "resources.cfg"))
            result.errors << QObject::tr("The installation directory has no plugins.cfg and resources.cfg. Enable 'Copy V-Gears data files'.");
        else{
            const QStringList missing = MissingOgrePlugins(output);
            if (!missing.isEmpty())
                result.errors << QObject::tr("Ogre plugins not found in the installation directory or the system: %1").arg(missing.join(", "));
        }
    }
    return result;
}

/**
 * Recursively copies a directory, overwriting existing files.
 *
 * @return True if every file was copied.
 */
static bool CopyDirectory(const QString& src, const QString& dst){
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

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), main_window_(new Ui::MainWindow){
    main_window_->setupUi(this);
    main_window_->advancedOptions->setVisible(main_window_->chk_advanced_options->isChecked());
    // Presets and the rules between options. Changing an option by hand switches the preset to "Custom".
    for (QCheckBox* box : OptionBoxes()){
        connect(box, &QCheckBox::toggled, this, [this](){UpdateOptions();});
        connect(box, &QCheckBox::clicked, this, [this](){
            if (main_window_->combo_preset->currentIndex() != PRESET_CUSTOM){
                const QSignalBlocker blocker(main_window_->combo_preset);
                main_window_->combo_preset->setCurrentIndex(PRESET_CUSTOM);
            }
        });
    }
    connect(
      main_window_->combo_preset, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
      [this](int index){ApplyPreset(index);}
    );
    ApplyPreset(main_window_->combo_preset->currentIndex());
    InitSettings();
    main_window_->line_data_dst->setText(settings_->value("InstallDir").toString());
    // The audio tools are looked for once the window is shown.
    connect(main_window_->label_tools, &QLabel::linkActivated, this, [this](const QString& link){
        if (link == "locate-ffmpeg") LocateTool(true);
        else if (link == "locate-timidity") LocateTool(false);
    });
    QTimer::singleShot(0, this, [this](){RefreshToolStatus();});
    connect(main_window_->btn_close, &QPushButton::clicked, this, &QWidget::close);
    timer_ = new QTimer(this);
    connect(timer_, SIGNAL(timeout()), this, SLOT(DoProgress()));
}

MainWindow::~MainWindow(){
    // Stop the installation, if there is one running, before the window goes away.
    stop_requested_ = true;
    if (worker_.joinable()) worker_.join();
    delete main_window_;
}

void MainWindow::InitSettings(void){
    bool win = false;
#ifdef Q_OS_WIN
    settings_ = new QSettings(
      QCoreApplication::applicationDirPath() + "/" + "launcherSettings.ini",
      QSettings::IniFormat
    );
    win = true;
#else
    settings_ = new QSettings(QSettings::NativeFormat,QSettings::UserScope, "v-gears", "launcher", 0);
#endif
    // There is a single installation directory: it holds the configuration (plugins.cfg, resources.cfg...) and the
    // game data (<dir>/data), and it is where the game runs. Older versions stored two settings, ConfigDir and
    // DataDir (the latter pointing to <dir>/data), so migrate them.
    if (settings_->value("InstallDir").isNull()){
        const QString default_dir = win ? QString("%1/v-gears").arg(QDir::homePath()) : QString("%1/.v-gears").arg(QDir::homePath());
        const QString dir = QString::fromStdString(InstallLayout::MigrateInstallDir(
          settings_->value("DataDir").toString().toStdString(), settings_->value("ConfigDir").toString().toStdString(),
          default_dir.toStdString()
        ));
        settings_->setValue("InstallDir", dir);
    }
    settings_->remove("DataDir");
    settings_->remove("ConfigDir");

    if (settings_->value("VGearsEXE").isNull()){
        if (win) settings_->setValue("VGearsEXE",QString("%1/v-gears.exe").arg(QCoreApplication::applicationDirPath()));
        else settings_->setValue("VGearsEXE", QString("/usr/games/v-gears"));
    }
}

void MainWindow::OpenInstallLog(const QString& dir, bool append){
    InstallLog& install_log = InstallLog::Get();
    install_log.Open((dir + "installer.log").toStdString(), append);
    install_log.SetSink([this](InstallLog::Level level, const std::string& message){
        const QString text = QString::fromStdString(message).toHtmlEscaped();
        const QString color = level == InstallLog::ERROR_LEVEL ? "#c62828" : (level == InstallLog::WARNING ? "#e08600" : "");
        const QString html = color.isEmpty() ? text : QString("<span style=\"color:%1\">%2</span>").arg(color, text);
        // The log can be written from any thread.
        QMetaObject::invokeMethod(this, [this, html](){main_window_->data_log->append(html);}, Qt::QueuedConnection);
    });
}

std::vector<QCheckBox*> MainWindow::OptionBoxes() const{
    return {
      main_window_->chk_battle_data, main_window_->chk_battle_models, main_window_->chk_kernel,
      main_window_->chk_images, main_window_->chk_sounds, main_window_->chk_music, main_window_->chk_videos,
      main_window_->chk_fields, main_window_->chk_field_models, main_window_->chk_wm, main_window_->chk_wm_models
    };
}

InstallOptions MainWindow::CurrentOptions() const{
    InstallOptions o;
    o.battle_data = main_window_->chk_battle_data->isChecked();
    o.battle_models = main_window_->chk_battle_models->isChecked();
    o.kernel = main_window_->chk_kernel->isChecked();
    o.images = main_window_->chk_images->isChecked();
    o.sounds = main_window_->chk_sounds->isChecked();
    o.music = main_window_->chk_music->isChecked();
    o.videos = main_window_->chk_videos->isChecked();
    o.fields = main_window_->chk_fields->isChecked();
    o.field_models = main_window_->chk_field_models->isChecked();
    o.wm = main_window_->chk_wm->isChecked();
    o.wm_models = main_window_->chk_wm_models->isChecked();
    return o;
}

void MainWindow::SetOptions(const InstallOptions& o){
    main_window_->chk_battle_data->setChecked(o.battle_data);
    main_window_->chk_battle_models->setChecked(o.battle_models);
    main_window_->chk_kernel->setChecked(o.kernel);
    main_window_->chk_images->setChecked(o.images);
    main_window_->chk_sounds->setChecked(o.sounds);
    main_window_->chk_music->setChecked(o.music);
    // Videos can only be chosen when the disk has them.
    main_window_->chk_videos->setChecked(o.videos);
    main_window_->chk_fields->setChecked(o.fields);
    main_window_->chk_field_models->setChecked(o.field_models);
    main_window_->chk_wm->setChecked(o.wm);
    main_window_->chk_wm_models->setChecked(o.wm_models);
}

void MainWindow::ApplyPreset(int index){
    if (index == PRESET_CUSTOM){
        UpdateOptions();
        return;
    }
    static const InstallPreset presets[] = {
      InstallPreset::EVERYTHING, InstallPreset::FIELDS, InstallPreset::WORLD_MAP, InstallPreset::BATTLE,
      InstallPreset::MEDIA
    };
    if (index < 0 || index >= static_cast<int>(sizeof(presets) / sizeof(presets[0]))) return;
    // Videos are not on the install disk: they are extracted from the game disks, which have their own flow.
    SetOptions(InstallRules::Preset(presets[index], false));
    UpdateOptions();
}

void MainWindow::UpdateOptions(){
    if (updating_options_) return;
    updating_options_ = true;
    // Options whose requirements are not met are disabled, and so unchecked.
    InstallOptions options = CurrentOptions();
    InstallRules::ApplyDependencies(options);
    const InstallOptions selectable = InstallRules::Selectable(options);
    const auto apply = [](QCheckBox* box, bool checked, bool enabled){
        if (box->isChecked() != checked) box->setChecked(checked);
        box->setEnabled(enabled);
    };
    // The options can only be changed by hand with the "Custom" preset, and then only if their requirements are met.
    // "Clean install" doesn't depend on what is installed, so it is always available.
    const bool custom = main_window_->combo_preset->currentIndex() == PRESET_CUSTOM;
    apply(main_window_->chk_field_models, options.field_models, custom && selectable.field_models);
    apply(main_window_->chk_wm_models, options.wm_models, custom && selectable.wm_models);
    apply(main_window_->chk_videos, main_window_->chk_videos->isChecked(), custom);
    for (QCheckBox* box : {
      main_window_->chk_battle_data, main_window_->chk_battle_models, main_window_->chk_kernel,
      main_window_->chk_images, main_window_->chk_sounds, main_window_->chk_music, main_window_->chk_fields,
      main_window_->chk_wm, main_window_->chk_copy_data, main_window_->chk_keep_original
    }) box->setEnabled(custom);
    main_window_->chk_clean_install->setEnabled(true);

    // Disk space.
    uint64_t temporary = 0;
    const uint64_t installed = InstallRules::EstimateMegabytes(options, &temporary);
    const auto size = [](uint64_t mb){
        return mb >= 1024 ? QString("%1 GB").arg(mb / 1024.0, 0, 'f', 1) : QString("%1 MB").arg(mb);
    };
    QString text = tr("Estimated disk space: about %1 for the converted data").arg(size(installed));
    if (temporary > 0) text += tr(", plus %1 of temporary files while installing").arg(size(temporary));
    text += tr(", plus the extracted ISO (up to about 1 GB).");
    if (options.videos) text += tr(" Videos are not included in the estimate.");
    main_window_->label_estimate->setText(text);
    updating_options_ = false;
}

void MainWindow::on_btn_verify_clicked(){
    QString dir = QDir::fromNativeSeparators(main_window_->line_data_dst->text());
    if (dir.isEmpty() || !QDir(dir).exists()){
        QMessageBox::warning(this, tr("Verify install"), tr("The installation directory does not exist: %1").arg(dir));
        return;
    }
    const VerifyResult result = VerifyInstallation(dir.toStdString(), FindGameDataDir().toStdString());
    // The game can start if its configuration and the V-Gears data files are there.
    bool can_launch = true;
    QString text;
    std::string group;
    for (const VerifyProblem& problem : result.problems){
        if (problem.group == "Configuration" || problem.group == "Game files" || problem.group == "Copied data files")
            can_launch = false;
        if (problem.group != group){
            group = problem.group;
            text += QString("\n%1:\n").arg(QString::fromStdString(group));
        }
        text += QString("  - %1\n").arg(QString::fromStdString(problem.message));
    }
    if (result.Ok()){
        QMessageBox::information(
          this, tr("Verify install"), tr("Everything the game needs is installed (%1 checks).").arg(result.checked)
        );
    }
    else{
        QMessageBox::warning(
          this, tr("Verify install"),
          tr("Some files are missing:\n%1\nIf you skipped a part of the installation on purpose (for example the "
             "music), the files of that part are expected to be missing.").arg(text)
        );
    }
    launch_available_ = can_launch;
    main_window_->btn_launch->setEnabled(launch_available_);
}

void MainWindow::on_btn_launch_clicked(){
    LaunchGame(QDir::fromNativeSeparators(main_window_->line_data_dst->text()));
}

void MainWindow::LaunchGame(const QString& dir){
    // The executable of the settings, or the one built next to the installer.
    QString exe = settings_->value("VGearsEXE").toString();
    if (exe.isEmpty() || !QFileInfo(exe).isExecutable()){
        exe.clear();
        for (const QString& candidate : {QCoreApplication::applicationDirPath() + "/v-gears", QString("/usr/games/v-gears")}){
            if (QFileInfo(candidate).isExecutable()){
                exe = candidate;
                break;
            }
        }
    }
    if (exe.isEmpty()){
        exe = QFileDialog::getOpenFileName(this, tr("Location of the V-Gears executable"), QDir::homePath());
        if (exe.isEmpty()) return;
    }
    settings_->setValue("VGearsEXE", exe);
    // The game runs from the installation directory, where it finds its data.
    QStringList args;
    args << QString("--resources-file=%1/resources.cfg").arg(dir) << QString("--config-file=%1/v-gears.cfg").arg(dir)
         << QString("--plugins-file=%1/plugins.cfg").arg(dir);
    if (!QProcess::startDetached(exe, args, dir)){
        QMessageBox::critical(this, tr("Launch game"), tr("Could not start %1.").arg(exe));
    }
}

/**
 * Version of the installer, set by the build.
 */
#ifndef VGEARS_INSTALLER_VERSION
#define VGEARS_INSTALLER_VERSION "unknown"
#endif

/**
 * Web pages of the project.
 */
static const char* const kHomepage = "https://v-gears.inigovalentin.com";
static const char* const kRepository = "https://github.com/InigoValentin/V-Gears";

void MainWindow::on_actionABout_triggered(){
    QMessageBox box(this);
    box.setWindowTitle(tr("About V-Gears Installer"));
    box.setIconPixmap(windowIcon().pixmap(64, 64));
    box.setTextFormat(Qt::RichText);
    box.setTextInteractionFlags(Qt::TextBrowserInteraction);
    box.setText(tr(
      "<h3>V-Gears Installer %1</h3>"
      "<p>Prepares the data that <b>V-Gears</b>, a free engine for Final Fantasy VII, needs: it converts the data of "
      "your own copy of the PC game into formats the engine can use.</p>"
      "<p>It doesn't include any data from the original game, and you need a legal copy of it.</p>"
      "<p><a href=\"%2\">Website</a> &middot; <a href=\"%3\">Source code</a></p>"
      "<p>Copyright &copy; The V-Gears Team.<br>"
      "Free software, licensed under the GNU General Public License, version 3.</p>"
    ).arg(VGEARS_INSTALLER_VERSION, kHomepage, kRepository));
    box.exec();
}

void MainWindow::on_actionHelp_triggered(){
    QDialog dialog(this);
    dialog.setWindowTitle(tr("V-Gears Installer Help"));
    dialog.resize(640, 560);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTextBrowser* browser = new QTextBrowser(&dialog);
    browser->setOpenExternalLinks(true);
    browser->setHtml(tr(
      "<h2>Installing the game data</h2>"
      "<ol>"
      "<li><b>Select the ISO</b> of the <i>install disk</i> of the PC version of Final Fantasy VII. The installer "
      "tells you if the image is valid.</li>"
      "<li><b>Choose the installation directory</b>, for example <code>~/.v-gears</code>. It holds the "
      "configuration and the converted data, and it is where the game runs.</li>"
      "<li>Click <b>Install data</b>. The log shows the progress. Problems are shown in color and also written to "
      "<code>installer.log</code> in the installation directory.</li>"
      "</ol>"
      "<p>The installation can only be done once per window. To install again, close the installer and open it "
      "again.</p>"
      "<h3>Options</h3>"
      "<p>Select <b>Show advanced options</b> to choose what to install. <b>What to install</b> offers ready-made "
      "sets; choose <b>Custom</b> to pick the options by hand. Some options need others (for example the field "
      "models need the field maps). <b>Clean install</b> deletes the data of a previous installation first. The "
      "estimate under the options shows about how much disk space it needs.</p>"
      "<p>With <b>Copy V-Gears data files</b> the installer also copies the files that come with V-Gears (scripts, "
      "screens, fonts and configuration) to the installation directory.</p>"
      "<h3>Videos</h3>"
      "<p>The videos are on the PC <i>game disks</i> 1, 2 and 3, not on the install disk. Select one of those "
      "disks as the ISO, with the same installation directory, and click <b>Install data</b>: only its videos are "
      "extracted, to <code>data/video</code>. Repeat it for each disk. Videos that are already installed are "
      "skipped.</p>"
      "<h3>FFmpeg and TiMidity</h3>"
      "<p>The sounds and the music are converted with <b>FFmpeg</b>, and the music also with <b>TiMidity</b>. "
      "The installer looks for them in the folder you chose with <i>Locate…</i>, in a <code>tools</code> folder "
      "inside the installation directory or next to the installer (handy on Windows: put <code>ffmpeg.exe</code> "
      "there), and in the system <code>PATH</code>. The window shows what it found. If something is missing when "
      "you start, it tells you what you would lose and lets you locate the program, open its download page, check "
      "again, continue without it, or cancel.</p>"
      "<h3>When it finishes</h3>"
      "<p><b>Verify install</b> checks that the installation directory has everything the game needs. If you "
      "skipped a part on purpose, the files of that part are reported as missing. <b>Launch game</b> starts "
      "V-Gears from the installation directory.</p>"
      "<h3>More help</h3>"
      "<p><a href=\"%1\">Project website</a> &middot; <a href=\"%2\">Source code and documentation</a></p>"
    ).arg(kHomepage, kRepository));
    layout->addWidget(browser);
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    dialog.exec();
}

void MainWindow::on_actionExit_triggered(){close();}

void MainWindow::on_btn_data_src_clicked(){
    const QString filter = tr("ISO images (*.iso)");
    QString temp = QFileDialog::getOpenFileName(
      this, tr("Select ISO image"), settings_->value("IsoDir", QDir::homePath()).toString(), filter
    );
    std::cout << "Selected ISO: " << temp.toStdString() << std::endl;
    if (!temp.isNull()){
        main_window_->line_data_src->setText(temp);
        settings_->setValue("IsoDir", QFileInfo(temp).absolutePath());
        disk_image_ = DiskImage(temp.toStdString());
        main_window_->isoData->setText(QString::fromStdString(disk_image_.getId()));
        if (!disk_image_.isValid()){
            main_window_->isoError->setStyleSheet("QLabel { color : red; }");
            main_window_->isoError->setText(QString::fromStdString(disk_image_.getErrorMessage()));
        }
        else if (!disk_image_.isSupported()){
            main_window_->isoError->setStyleSheet("QLabel { color : orange; }");
            main_window_->isoError->setText(QString::fromStdString(disk_image_.getWarningMessage()));
        }
        else if (disk_image_.isVideoDisk()){
            main_window_->isoError->setStyleSheet("QLabel { color : green; }");
            main_window_->isoError->setText(
              tr("Game disk: only its videos will be extracted. Install the game from the install disk.")
            );
        }
        else{
            main_window_->isoError->setStyleSheet("QLabel { color : green; }");
            main_window_->isoError->setText(tr("ISO is valid and supported."));
        }
        // A game disk only has videos, so there is nothing else to choose.
        main_window_->advancedOptions->setEnabled(!disk_image_.isVideoDisk());
        main_window_->combo_preset->setEnabled(!disk_image_.isVideoDisk());
    }
}

void MainWindow::on_line_data_dst_editingFinished(){
    settings_->setValue("InstallDir", main_window_->line_data_dst->text());
    // The installation directory can have a "tools" folder.
    RefreshToolStatus();
}

void MainWindow::on_btn_data_dst_clicked(){
    QString temp = QFileDialog::getExistingDirectory(
      this, tr("V-Gears installation directory"), settings_->value("InstallDir").toString()
    );
    if (!temp.isNull()){
        settings_->setValue("InstallDir", temp);
        main_window_->line_data_dst->setText(temp);
        RefreshToolStatus();
    }
}

void MainWindow::on_btn_data_run_clicked(){
    if (main_window_->line_data_dst->text().isEmpty()){
        QMessageBox::critical(this, tr("Output error"), tr("No output path provided"));
        return;
    }
    // Normalize the output path.
    QString output = QDir::fromNativeSeparators(main_window_->line_data_dst->text());
    if (!output.endsWith("/")) output += "/";
    // The working directory changes during installation, so the path must be absolute.
    if (QDir::isRelativePath(output)) output = QDir(output).absolutePath() + "/";

    // A game disk only has videos: extract them, without the rest of the installation.
    if (disk_image_.isValid() && disk_image_.isVideoDisk()){
        StartVideoExtraction(output);
        return;
    }

    // Locate the V-Gears data folder to copy, asking if it can't be found.
    const bool copy_data = main_window_->chk_copy_data->checkState() == Qt::Checked;
    QString game_data;
    if (copy_data){
        game_data = FindGameDataDir();
        if (game_data.isEmpty()){
            game_data = QFileDialog::getExistingDirectory(
              this, tr("Select the V-Gears 'data' folder to copy"), QCoreApplication::applicationDirPath()
            );
            if (game_data.isEmpty()) return;
        }
    }

    // Check everything that can be checked before touching the disk.
    PreflightResult preflight = RunPreflight(disk_image_, output, copy_data, game_data);
    for (const std::string& problem : InstallRules::Problems(CurrentOptions(), QFile::exists(output + "data/fields/_fields.xml")))
        preflight.errors << QString::fromStdString(problem);
    if (!preflight.errors.isEmpty()){
        QMessageBox::critical(
          this, tr("The installation can't start"),
          tr("Fix these problems and try again:\n\n- %1").arg(preflight.errors.join("\n- "))
        );
        return;
    }
    if (!preflight.warnings.isEmpty()){
        const auto answer = QMessageBox::warning(
          this, tr("Warnings"), tr("%1\n\nContinue anyway?").arg(preflight.warnings.join("\n")),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No
        );
        if (answer != QMessageBox::Yes) return;
    }
    // The audio is converted with external programs: say now, before anything is extracted, if they are missing.
    if (!ResolveMissingTools(CurrentOptions())) return;
    {
        bool extraction_success = disk_image_.extractImage(output.toStdString());
        if (!extraction_success) {
            QMessageBox::critical(this, tr("Extraction error"), tr("Failed to extract ISO file."));
            return;
        }

      QString input = QDir::fromNativeSeparators(QString::fromStdString(disk_image_.getContentPath()));
      if (input.isEmpty()) {
        QMessageBox::critical(this, tr("Extraction error"), tr("Extracted data path is invalid."));
        return;
      }
      if (!input.endsWith("/")) input += "/";

        // TODO: Enumerate files or find some better way to do this.
        const std::vector<std::string> required_files = {
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

        // Ensure required files are in the input dir
        for (auto& file : required_files){
            std::cout << "Checking for required file: " << file << std::endl;
            const auto matched_file = disk_image_.filePath(file);
            if (!matched_file){
                QMessageBox::critical(
                  this, tr("Missing input file"),
                  tr("Some of the required files were not found among the extracted data.\n\nMake sure that the file '")
                    + file.c_str() + tr("' is in the extracted data.")
                );
                return;
            }
        }
        // Copy the V-Gears data files (scripts, screens, fonts...) to the installation directory.
        if (copy_data){
            main_window_->data_log->append(tr("Copying data files from %1...").arg(game_data));
            // Everything in the folder goes to the installation directory, and its "data" subfolder to <dir>/data.
            if (!CopyDirectory(game_data, output)){
                QMessageBox::critical(
                  this, tr("Copy error"), tr("Failed to copy the V-Gears data files to the installation directory.")
                );
                return;
            }
            const QStringList missing_plugins = CopyMissingOgrePlugins(output);
            if (!missing_plugins.isEmpty()){
                QMessageBox::critical(
                  this, tr("Missing Ogre plugins"),
                  tr("These Ogre plugins are not in the installation directory nor in the system:\n\n%1")
                    .arg(missing_plugins.join(", "))
                );
                return;
            }
            main_window_->data_log->append(tr("Data files copied."));
        }
        // The installer's Ogre setup loads plugins.cfg, resources.cfg and the render plugin from the working
        // directory (and writes install.log there), so run from the installation directory.
        if (!QFile::exists(output + "plugins.cfg") || !QFile::exists(output + "resources.cfg")){
            QMessageBox::critical(
              this, tr("Missing configuration files"),
              tr("The installation directory must contain plugins.cfg and resources.cfg.\n\n"
                 "Enable 'Copy V-Gears data files' to put them there.")
            );
            return;
        }
        if (!QDir::setCurrent(output)){
            QMessageBox::critical(this, tr("Output error"), tr("Could not enter the installation directory."));
            return;
        }
        if (installer_created){
            // Due to the use of singletons, the installer can't be re-run.
            // TODO: Verify that this is true, and if so, try to fix it.
            QMessageBox::critical(
              this, tr("Error"), tr("Please, fix the errors, and then close the installer before trying again.")
            );
            return;
        }
        // Collect every message of the installation in one place: the log file, the log window and the summary.
        UpdateToolLocations(output);
        OpenInstallLog(output, false);
        // Start data conversion
        try{
            // Parse advanced options.
            DataInstaller::AdvancedOptions options;
            options.battle_data = (Qt::Checked == main_window_->chk_battle_data->checkState());
            options.battle_models = (Qt::Checked == main_window_->chk_battle_models->checkState());
            options.kernel = (Qt::Checked == main_window_->chk_kernel->checkState());
            options.images = (Qt::Checked == main_window_->chk_images->checkState());
            options.sounds = (Qt::Checked == main_window_->chk_sounds->checkState());
            options.music = (Qt::Checked == main_window_->chk_music->checkState());
            options.videos = false;
            if (Qt::Checked == main_window_->chk_videos->checkState()){
                InstallLog::Get().Add(
                  "[WARNING] The install disk has no videos. To extract them, select game disk 1, 2 or 3 as the "
                  "ISO and install again (with a new installer window)."
                );
            }
            options.fields = (Qt::Checked == main_window_->chk_fields->checkState());
            options.field_models = options.fields && (Qt::Checked == main_window_->chk_field_models->checkState());
            options.wm = (Qt::Checked == main_window_->chk_wm->checkState());
            options.wm_models = options.wm && (Qt::Checked == main_window_->chk_wm_models->checkState());
            options.keep_originals = (Qt::Checked == main_window_->chk_keep_original->checkState());
            installer_created = true;
            install_started_ = true;
            const std::string output_path = QDir::toNativeSeparators(output).toStdString();
            const auto write_line = [this](const std::string log_line, int level, bool as_progress = false){
                InstallLog::Get().Add(log_line);
                if (as_progress){
                    const QString text = QString::fromStdString(log_line);
                    QMetaObject::invokeMethod(
                      this, [this, text](){main_window_->label_progress->setText(text);}, Qt::QueuedConnection
                    );
                }
            };
            // The installation runs in a worker thread so the window stays responsive. Ogre, and the OpenGL context
            // it creates, belong to the thread that creates them, so the installer is created, run and destroyed
            // there. Setting "BackgroundInstall" to false runs it in the UI thread, step by step from a timer.
            background_ = settings_->value("BackgroundInstall", true).toBool();
            if (background_){
                stop_requested_ = false;
                OnInstallStarted();
                worker_ = std::thread([this, options, output_path, write_line](){RunInstallation(options, output_path, write_line);});
            }
            else{
                installer_ = std::make_unique<DataInstaller>(disk_image_, output_path, options, write_line);
                OnInstallStarted();
            }
        }
        catch (const std::exception& ex){
            InstallLog::Get().Add(std::string("[ERROR] Installation failed: ") + ex.what());
            InstallLog::Get().Close();
            installer_.reset();
            installer_created = false;
            OnInstallStopped();
            QMessageBox::critical(this, tr("Installation error"), ex.what());
        }
    }
}

void MainWindow::on_chk_advanced_options_stateChanged(){
    main_window_->advancedOptions->setVisible(main_window_->chk_advanced_options->checkState() == Qt::Checked);
}

void MainWindow::EnableUi(bool enable){
    // The installer can't run twice, so once an installation has started the button never comes back.
    main_window_->btn_data_run->setEnabled(enable && !install_started_);
    main_window_->btn_close->setEnabled(enable);
    main_window_->actionExit->setEnabled(enable);
    main_window_->label_tools->setEnabled(enable);
    main_window_->btn_verify->setEnabled(enable);
    main_window_->btn_launch->setEnabled(enable && launch_available_);
    main_window_->line_data_src->setEnabled(enable);
    main_window_->btn_data_src->setEnabled(enable);
    main_window_->line_data_dst->setEnabled(enable);
    main_window_->btn_data_dst->setEnabled(enable);
    main_window_->data_progress_bar->setValue(0);
    // In background mode the worker thread does the work, so there is nothing to poll.
    if (!enable && !background_) timer_->start(0);
    else timer_->stop();
}

void MainWindow::OnInstallStarted(){EnableUi(false);}

void MainWindow::OnInstallStopped(){EnableUi(true);}

void MainWindow::StartVideoExtraction(const QString& output){
    // Only the output directory is needed: no plugins, no configuration, no Ogre.
    QStringList errors;
    if (!QDir().mkpath(output)) errors << tr("The installation directory can't be created: %1").arg(output);
    else{
        QFile probe(output + ".v-gears-write-test");
        if (!probe.open(QIODevice::WriteOnly)) errors << tr("The installation directory is not writable: %1").arg(output);
        else{
            probe.close();
            probe.remove();
        }
        const QStorageInfo storage(output);
        const qint64 needed = 1024LL * 1024 * 1024;
        if (storage.isValid() && storage.bytesAvailable() < needed){
            errors << tr("Not enough free space in %1: %2 MB available, about %3 MB needed.")
              .arg(storage.rootPath()).arg(storage.bytesAvailable() / (1024 * 1024)).arg(needed / (1024 * 1024));
        }
    }
    if (!errors.isEmpty()){
        QMessageBox::critical(
          this, tr("The extraction can't start"), tr("Fix these problems and try again:\n\n- %1").arg(errors.join("\n- "))
        );
        return;
    }
    // The installation log of a previous run is kept.
    OpenInstallLog(output, true);
    // It doesn't use the installer (and its singletons), so it can be done again with another disk.
    background_ = true;
    stop_requested_ = false;
    OnInstallStarted();
    InstallLog::Get().Add("Extracting the videos of " + disk_image_.getId() + "...");
    worker_ = std::thread([this, output](){
        int extracted = 0;
        int skipped = 0;
        bool ok = false;
        std::string error;
        try{
            ok = disk_image_.extractVideos(
              output.toStdString() + "data/",
              [this](float progress){
                  QMetaObject::invokeMethod(this, [this, progress](){ShowProgress(progress);}, Qt::QueuedConnection);
              },
              [](const std::string& message){InstallLog::Get().Add(message);}, &extracted, &skipped
            );
        }
        catch (const std::exception& ex){
            error = ex.what();
        }
        if (stop_requested_) return;
        QMetaObject::invokeMethod(this, [this, ok, error, extracted, skipped](){
            if (worker_.joinable()) worker_.join();
            if (ok && error.empty()){
                ShowProgress(100);
                InstallLog::Get().Add(
                  "Videos extracted: " + std::to_string(extracted) + " new, " + std::to_string(skipped)
                  + " already installed."
                );
            }
            else InstallLog::Get().Add("[ERROR] Video extraction failed" + (error.empty() ? std::string() : ": " + error));
            InstallLog::Get().Close();
            OnInstallStopped();
            if (!ok || !error.empty())
                QMessageBox::critical(this, tr("Video extraction"), tr("Some videos could not be extracted. See the log."));
        }, Qt::QueuedConnection);
    });
}

void MainWindow::UpdateToolLocations(const QString& install_dir){
    // Where to look for FFmpeg and TiMidity: what the user chose, then a "tools" folder in the installation
    // directory or next to the installer, then the system PATH.
    QString dir = QDir::fromNativeSeparators(install_dir);
    if (!dir.isEmpty() && !dir.endsWith("/")) dir += "/";
    ExternalTools::Locations& tools = ExternalTools::CurrentLocations();
    tools.ffmpeg = settings_->value("FFmpegPath").toString().toStdString();
    tools.timidity = settings_->value("TimidityPath").toString().toStdString();
    const QString app_dir = QCoreApplication::applicationDirPath();
    tools.directories.clear();
    if (!dir.isEmpty()) tools.directories.push_back((dir + "tools").toStdString());
    tools.directories.push_back((app_dir + "/tools").toStdString());
    tools.directories.push_back(app_dir.toStdString());
}

void MainWindow::RefreshToolStatus(){
    UpdateToolLocations(main_window_->line_data_dst->text());
    const ExternalTools::Locations& locations = ExternalTools::CurrentLocations();
    ffmpeg_tool_ = ExternalTools::FindFfmpeg(locations);
    timidity_tool_ = ExternalTools::FindTimidity(locations);
    const auto describe = [](const QString& name, const ExternalTools::Tool& tool, const QString& link){
        if (tool.usable)
            return tr("%1 \u2713 %2").arg(name, QString::fromStdString(ExternalTools::ShortVersion(tool)));
        return tr("%1 <span style=\"color:#c62828\">\u2717 not found</span> (<a href=\"%2\">Locate\u2026</a>)").arg(name, link);
    };
    main_window_->label_tools->setText(
      tr("Audio tools: %1 \u00b7 %2").arg(
        describe("FFmpeg", ffmpeg_tool_, "locate-ffmpeg"), describe("TiMidity", timidity_tool_, "locate-timidity")
      )
    );
    const auto tip = [](const QString& name, const ExternalTools::Tool& tool){
        return name + ": " + QString::fromStdString(tool.usable ? tool.path : tool.problem);
    };
    main_window_->label_tools->setToolTip(tip("FFmpeg", ffmpeg_tool_) + "\n" + tip("TiMidity", timidity_tool_));
}

bool MainWindow::LocateTool(bool ffmpeg){
    const QString name = ffmpeg ? "FFmpeg" : "TiMidity";
    const QString key = ffmpeg ? "FFmpegPath" : "TimidityPath";
    const QString saved = settings_->value(key).toString();
#ifdef Q_OS_WIN
    const QString filter = tr("Programs (*.exe);;All files (*)");
#else
    const QString filter = tr("All files (*)");
#endif
    const QString file = QFileDialog::getOpenFileName(
      this, tr("Locate %1").arg(name), saved.isEmpty() ? QDir::homePath() : QFileInfo(saved).absolutePath(), filter
    );
    if (file.isEmpty()) return false;
    // It must really be the program, and work.
    const ExternalTools::Tool tool = ffmpeg ? ExternalTools::ProbeFfmpeg(file.toStdString()) : ExternalTools::ProbeTimidity(file.toStdString());
    if (!tool.usable){
        QMessageBox::warning(this, tr("This program can't be used"), QString::fromStdString(tool.problem));
        return false;
    }
    settings_->setValue(key, file);
    RefreshToolStatus();
    return true;
}

bool MainWindow::ResolveMissingTools(const InstallOptions& options){
    while (true){
        RefreshToolStatus();
        // What is going to be used.
        const InstallRules::ToolNeeds needs = InstallRules::ToolsNeeded(options);
        const bool want_ffmpeg = needs.ffmpeg;
        const bool want_timidity = needs.timidity;
        const bool missing_ffmpeg = want_ffmpeg && !ffmpeg_tool_.usable;
        const bool missing_timidity = want_timidity && !timidity_tool_.usable;
        const std::vector<std::string> consequences
          = InstallRules::ToolConsequences(options, !missing_ffmpeg, !missing_timidity);
        if (consequences.empty()) return true;

        QStringList lost;
        for (const std::string& line : consequences) lost << QString::fromStdString(line);
        QStringList reasons;
        if (missing_ffmpeg) reasons << "FFmpeg: " + QString::fromStdString(ffmpeg_tool_.problem);
        if (missing_timidity) reasons << "TiMidity: " + QString::fromStdString(timidity_tool_.problem);

        QMessageBox box(this);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle(tr("Audio tools missing"));
        box.setText(tr("The installer needs programs it can't use:\n\n%1\n\nIf you continue without them:\n\n%2")
          .arg("- " + reasons.join("\n- "), "- " + lost.join("\n- ")));
        const QString tools_dir
          = QDir::toNativeSeparators(QDir::fromNativeSeparators(main_window_->line_data_dst->text()) + "/tools");
        QString help = tr("You can also copy the programs to %1 and choose 'Check again'.\n\n").arg(tools_dir);
#if defined(Q_OS_WIN)
        help += tr("Windows: download a build of FFmpeg (ffmpeg.exe) from https://ffmpeg.org/download.html. TiMidity++ "
                   "also needs its timidity.cfg and instrument files next to timidity.exe.");
#elif defined(Q_OS_MAC)
        help += tr("macOS: brew install ffmpeg timidity");
#else
        help += tr("Debian, Ubuntu: sudo apt install ffmpeg timidity\nFedora: sudo dnf install ffmpeg timidity++\n"
                   "Arch: sudo pacman -S ffmpeg timidity++");
#endif
        box.setDetailedText(help);
        QPushButton* locate = box.addButton(tr("Locate\u2026"), QMessageBox::ActionRole);
        QPushButton* get = box.addButton(tr("Get it\u2026"), QMessageBox::ActionRole);
        QPushButton* again = box.addButton(tr("Check again"), QMessageBox::ActionRole);
        QPushButton* skip = box.addButton(tr("Continue without"), QMessageBox::AcceptRole);
        box.addButton(QMessageBox::Cancel);
        box.setDefaultButton(locate);
        box.exec();

        QAbstractButton* clicked = box.clickedButton();
        if (clicked == skip) return true;
        if (clicked == locate){
            if (missing_ffmpeg) LocateTool(true);
            if (missing_timidity) LocateTool(false);
        }
        else if (clicked == get){
            if (missing_ffmpeg) QDesktopServices::openUrl(QUrl("https://ffmpeg.org/download.html"));
            if (missing_timidity) QDesktopServices::openUrl(QUrl("https://timidity.sourceforge.net/#download"));
        }
        else if (clicked != again) return false;   // Cancel, or the window was closed.
    }
}

void MainWindow::ShowProgress(float progress){
    main_window_->data_progress_bar->setValue(std::floor(progress));
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << progress << "%";
    main_window_->label_percent->setText(QString::fromUtf8(stream.str().c_str()));
}

void MainWindow::RunInstallation(
  DataInstaller::AdvancedOptions options, std::string output_path,
  std::function<void(std::string, int, bool)> write_line
){
    std::string error;
    bool created = false;
    try{
        installer_ = std::make_unique<DataInstaller>(disk_image_, output_path, options, write_line);
        created = true;
        float progress = 0;
        auto last_shown = std::chrono::steady_clock::now();
        while (progress < 100 && !stop_requested_){
            progress = installer_->Progress();
            // Don't flood the UI thread with an update per step.
            const auto now = std::chrono::steady_clock::now();
            if (progress >= 100 || now - last_shown > std::chrono::milliseconds(100)){
                last_shown = now;
                QMetaObject::invokeMethod(this, [this, progress](){ShowProgress(progress);}, Qt::QueuedConnection);
            }
        }
    }
    catch (const std::exception& ex){
        error = ex.what();
    }
    catch (...){
        error = "Unknown error";
    }
    const bool finished = error.empty() && !stop_requested_;
    // Destroy the installer in the thread that created it.
    try{installer_.reset();}
    catch (...){}
    if (stop_requested_) return;
    QMetaObject::invokeMethod(this, [this, finished, created, error](){
        if (worker_.joinable()) worker_.join();
        if (finished){
            launch_available_ = true;
            OnInstallStopped();
            InstallLog::Get().Close();
        }
        else{
            InstallLog::Get().Add("[ERROR] Installation failed: " + error);
            InstallLog::Get().Close();
            // If the installer could not even be created it can be tried again.
            if (!created) installer_created = false;
            OnInstallStopped();
            QMessageBox::critical(this, created ? tr("Data conversion exception") : tr("Installation error"), QString::fromStdString(error));
        }
    }, Qt::QueuedConnection);
}

void MainWindow::DoProgress(){
    try{
        const float progress = installer_->Progress();
        ShowProgress(progress);
        if (progress >= 100){
            launch_available_ = true;
            OnInstallStopped();
            InstallLog::Get().Close();
        }
    }
    catch (const std::exception& ex){
        InstallLog::Get().Add(std::string("[ERROR] Installation failed: ") + ex.what());
        InstallLog::Get().Close();
        OnInstallStopped();
        QMessageBox::critical(this, tr("Data conversion exception"), ex.what());
    }
}
