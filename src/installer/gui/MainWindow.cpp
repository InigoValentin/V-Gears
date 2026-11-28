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
#include <OgrePrerequisites.h>
#include "common/DataDirectory.h"
#include "common/OgrePlugins.h"
#include "installer/steps/DataInstaller.h"
#include "installer/steps/ExternalTools.h"
#include "installer/steps/InstallLayout.h"
#include "installer/steps/InstallLog.h"
#include "installer/steps/InstallOptions.h"
#include "installer/steps/InstallSteps.h"
#include "installer/steps/InstallVerifier.h"
#include "MainWindow.h"
#include "installer/extractors/MovieCamera.h"
#include <ui_MainWindow.h>

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
    // A new installation starts with an empty log window.
    if (!append) main_window_->data_log->clear();
    InstallLog& install_log = InstallLog::Get();
    install_log.Open((dir + "logs/installer.log").toStdString(), append);
    install_log.SetSink([this](InstallLog::Level level, const std::string& message){
        const QString text = QString::fromStdString(message).toHtmlEscaped();
        const QString color = level == InstallLog::ERROR_LEVEL ? "#c62828" : (level == InstallLog::WARNING ? "#e08600" : "");
        // Every line is in a span: without tags, the window takes it as plain text and shows the
        // escaped characters as they are written.
        const QString style = color.isEmpty() ? "" : "; color:" + color;
        const QString html = QString("<span style=\"white-space:pre-wrap%1\">%2</span>").arg(style, text);
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
    const VerifyResult result = VerifyInstallation(
      dir.toStdString(), InstallSteps::FindGameDataDir().toStdString(), InstallSteps::PluginSearch()
    );
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
    args << QString("--data-dir=%1").arg(dir)
         << QString("--resources-file=%1/resources.cfg").arg(dir) << QString("--config-file=%1/v-gears.cfg").arg(dir)
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
      "<code>logs/installer.log</code> in the installation directory.</li>"
      "</ol>"
      "<h3>Options</h3>"
      "<p>Select <b>Show advanced options</b> to choose what to install. <b>What to install</b> offers ready-made "
      "sets; choose <b>Custom</b> to pick the options by hand. Some options need others (for example the field "
      "models need the field maps). <b>Clean install</b> deletes the data of a previous installation first; the "
      "saves, the controls, the game configuration and the videos are kept. <b>Preserve original data</b> keeps "
      "what the installer extracts and no longer needs (the install disk, temporary files and the original "
      "sounds, music and images); without it, that is deleted at the end. The "
      "estimate under the options shows about how much disk space it needs.</p>"
      "<p>With <b>Copy V-Gears data files</b> the installer also copies the files that come with V-Gears (scripts, "
      "screens, fonts and configuration) to the installation directory.</p>"
      "<h3>Videos</h3>"
      "<p>The videos are on the PC <i>game disks</i> 1, 2 and 3, not on the install disk. Select one of those "
      "disks as the ISO, with the same installation directory, and click <b>Install data</b>: only its videos are "
      "extracted, to <code>game/video</code>. Repeat it for each disk. Videos that are already installed are "
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
    const QString output = InstallSteps::NormalizeOutputDir(main_window_->line_data_dst->text());
    // A game disk only has videos: extract them, without the rest of the installation.
    if (disk_image_.isValid() && disk_image_.isVideoDisk()){
        StartVideoExtraction(output);
        return;
    }
    const bool copy_data = main_window_->chk_copy_data->checkState() == Qt::Checked;
    QString game_data;
    if (copy_data && !FindDataToCopy(game_data)) return;
    // Check everything that can be checked before touching the disk.
    const bool clean = main_window_->chk_clean_install->checkState() == Qt::Checked;
    if (!CheckInstallation(output, copy_data, game_data, clean)) return;
    // The audio is converted with external programs: say now, before anything is extracted, if
    // they are missing.
    if (!ResolveMissingTools(CurrentOptions())) return;
    // Everything is checked: the previous installation can be deleted.
    if (clean && !DeletePreviousInstallation(output)) return;
    // From here on, the installation directory is being changed: until it finishes, the record
    // says so.
    install_dir_ = output;
    InstallSteps::MarkInstallStarted(output, CurrentOptions(), disk_image_.getId());
    main_window_->data_progress_bar->setValue(0);
    if (!ExtractDisk(output)) return;
    if (!PrepareInstallationDir(output, copy_data, game_data)) return;
    // Collect every message of the installation in one place: the log file, the log window and
    // the summary.
    UpdateToolLocations(output);
    OpenInstallLog(output, false);
    StartConversion(output);
}

bool MainWindow::FindDataToCopy(QString& game_data){
    game_data = InstallSteps::FindGameDataDir();
    if (game_data.isEmpty()){
        game_data = QFileDialog::getExistingDirectory(
          this, tr("Select the V-Gears 'data' folder to copy"), QCoreApplication::applicationDirPath()
        );
    }
    return !game_data.isEmpty();
}

bool MainWindow::CheckInstallation(
  const QString& output, const bool copy_data, const QString& game_data, const bool clean
){
    InstallSteps::PreflightResult preflight
      = InstallSteps::RunPreflight(disk_image_, output, copy_data, game_data, clean);
    // A clean installation deletes the field maps that are already installed.
    const bool fields_installed = !clean && QFile::exists(output + "game/field/maps/_fields.xml");
    for (const std::string& problem : InstallRules::Problems(CurrentOptions(), fields_installed))
        preflight.errors << QString::fromStdString(problem);
    if (!preflight.errors.isEmpty()){
        QMessageBox::critical(
          this, tr("The installation can't start"),
          tr("Fix these problems and try again:\n\n- %1").arg(preflight.errors.join("\n- "))
        );
        return false;
    }
    if (preflight.warnings.isEmpty()) return true;
    const auto answer = QMessageBox::warning(
      this, tr("Warnings"), tr("%1\n\nContinue anyway?").arg(preflight.warnings.join("\n")),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No
    );
    return answer == QMessageBox::Yes;
}

bool MainWindow::DeletePreviousInstallation(const QString& output){
    if (!InstallLayout::IsInstallation(output.toStdString())) return true;
    const auto answer = QMessageBox::warning(
      this, tr("Clean install"),
      tr("The data of the installation in %1 will be deleted before installing. The saves, the controls, the "
         "game configuration and the videos are kept.\n\nContinue?").arg(QDir::toNativeSeparators(output)),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No
    );
    if (answer != QMessageBox::Yes) return false;
    const InstallLayout::CleanResult cleaned = InstallLayout::CleanInstallation(output.toStdString());
    if (!cleaned.errors.empty()){
        QStringList errors;
        for (const std::string& error : cleaned.errors) errors << QString::fromStdString(error);
        QMessageBox::critical(
          this, tr("Clean install"),
          tr("The previous installation could not be deleted:\n\n- %1").arg(errors.join("\n- "))
        );
        return false;
    }
    main_window_->data_log->append(
      tr("Deleted the previous installation (%1 files and folders).").arg(cleaned.removed)
    );
    return true;
}

std::function<void(float)> MainWindow::ShowPhase(
  const InstallSteps::Phase phase, const QString& text
){
    // The extraction and the copy run in the window's thread: the progress is shown as they go,
    // and the window is repainted, without taking the clicks (nothing can be started twice).
    return [this, phase, text](float value){
        ShowProgress(InstallSteps::OverallProgress(phase, value));
        main_window_->label_progress->setText(text);
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    };
}

bool MainWindow::ExtractDisk(const QString& output){
    if (!disk_image_.extractImage(
      output.toStdString(),
      ShowPhase(InstallSteps::Phase::EXTRACT, tr("Extracting the install disk..."))
    )){
        QMessageBox::critical(this, tr("Extraction error"), tr("Failed to extract ISO file."));
        return false;
    }
    if (disk_image_.getContentPath().empty()){
        QMessageBox::critical(this, tr("Extraction error"), tr("Extracted data path is invalid."));
        return false;
    }
    const std::vector<std::string> missing_files = InstallSteps::MissingRequiredFiles(disk_image_);
    if (!missing_files.empty()){
        QStringList names;
        for (const std::string& file : missing_files) names << QString::fromStdString(file);
        QMessageBox::critical(
          this, tr("Missing input file"),
          tr("Some of the required files were not found among the extracted data.\n\nMake sure that these "
             "files are in the extracted data:\n\n%1").arg(names.join("\n"))
        );
        return false;
    }
    return true;
}

bool MainWindow::PrepareInstallationDir(
  const QString& output, const bool copy_data, const QString& game_data
){
    // Copy the V-Gears data files (scripts, screens, fonts...) to the installation directory.
    if (copy_data){
        main_window_->data_log->append(tr("Copying data files from %1...").arg(game_data));
        // Everything in the folder goes to the installation directory.
        if (!InstallSteps::CopyGameFiles(
          game_data, output, ShowPhase(InstallSteps::Phase::COPY, tr("Copying the V-Gears data files..."))
        )){
            QMessageBox::critical(
              this, tr("Copy error"), tr("Failed to copy the V-Gears data files to the installation directory.")
            );
            return false;
        }
        main_window_->data_log->append(tr("Data files copied."));
    }
    // The installer's Ogre setup loads plugins.cfg and resources.cfg from the working directory
    // (and writes logs/ogre.log there), so run from the installation directory. The plugins
    // themselves are looked for in the same places as the game does (see
    // VGears::OgrePlugins::SearchDirectories).
    if (!QFile::exists(output + "plugins.cfg") || !QFile::exists(output + "resources.cfg")){
        QMessageBox::critical(
          this, tr("Missing configuration files"),
          tr("The installation directory must contain plugins.cfg and resources.cfg.\n\n"
             "Enable 'Copy V-Gears data files' to put them there.")
        );
        return false;
    }
    if (!QDir::setCurrent(output)){
        QMessageBox::critical(this, tr("Output error"), tr("Could not enter the installation directory."));
        return false;
    }
    const QString problem = InstallSteps::PrepareInstallation(output);
    if (!problem.isEmpty()){
        QMessageBox::critical(
          this, tr("Output error"), tr("Could not prepare the installation directory:\n\n%1").arg(problem)
        );
        return false;
    }
    return true;
}

void MainWindow::StartConversion(const QString& output){
    try{
        // Parse advanced options.
        DataInstaller::AdvancedOptions options = InstallSteps::ToAdvancedOptions(
          CurrentOptions(), Qt::Checked == main_window_->chk_keep_original->checkState()
        );
        if (Qt::Checked == main_window_->chk_videos->checkState()){
            InstallLog::Get().Add(
              "[WARNING] The install disk has no videos. To extract them, select game disk 1, 2 or 3 as the "
              "ISO and install again."
            );
        }
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
        // The installation runs in a worker thread so the window stays responsive. Ogre belongs to the thread
        // that creates it, so the installer is created, run and destroyed there. Setting "BackgroundInstall" to
        // false runs it in the UI thread, step by step from a timer.
        background_ = settings_->value("BackgroundInstall", true).toBool();
        if (background_){
            stop_requested_ = false;
            OnInstallStarted();
            worker_ = std::thread([this, options, output_path, write_line](){
                RunInstallation(options, output_path, write_line);
            });
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
        OnInstallStopped();
        QMessageBox::critical(this, tr("Installation error"), ex.what());
    }
}

void MainWindow::on_chk_advanced_options_stateChanged(){
    main_window_->advancedOptions->setVisible(main_window_->chk_advanced_options->checkState() == Qt::Checked);
}

void MainWindow::EnableUi(bool enable){
    main_window_->btn_data_run->setEnabled(enable);
    main_window_->btn_close->setEnabled(enable);
    main_window_->actionExit->setEnabled(enable);
    main_window_->label_tools->setEnabled(enable);
    main_window_->btn_verify->setEnabled(enable);
    main_window_->btn_launch->setEnabled(enable && launch_available_);
    main_window_->line_data_src->setEnabled(enable);
    main_window_->btn_data_src->setEnabled(enable);
    main_window_->line_data_dst->setEnabled(enable);
    main_window_->btn_data_dst->setEnabled(enable);
    // In background mode the worker thread does the work, so there is nothing to poll.
    if (!enable && !background_) timer_->start(0);
    else timer_->stop();
}

void MainWindow::OnInstallStarted(){EnableUi(false);}

void MainWindow::OnInstallStopped(){EnableUi(true);}

void MainWindow::StartVideoExtraction(const QString& output){
    main_window_->data_progress_bar->setValue(0);
    // Only the output directory is needed: no plugins, no configuration, no Ogre.
    const QStringList errors = InstallSteps::CheckVideoOutput(output);
    if (!errors.isEmpty()){
        QMessageBox::critical(
          this, tr("The extraction can't start"), tr("Fix these problems and try again:\n\n- %1").arg(errors.join("\n- "))
        );
        return;
    }
    const QString problem = InstallSteps::PrepareInstallation(output);
    if (!problem.isEmpty()){
        QMessageBox::critical(
          this, tr("Output error"), tr("Could not prepare the installation directory:\n\n%1").arg(problem)
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
              output.toStdString() + "game/",
              [this](float progress){
                  QMetaObject::invokeMethod(this, [this, progress](){ShowProgress(progress);}, Qt::QueuedConnection);
              },
              [](const std::string& message){InstallLog::Get().Add(message);}, &extracted, &skipped
            );
            if (ok) InstallSteps::ConvertMovies(output.toStdString() + "game/video/");
        }
        catch (const std::exception& ex){
            error = ex.what();
        }
        if (stop_requested_) return;
        QMetaObject::invokeMethod(this, [this, ok, error, extracted, skipped, output](){
            if (worker_.joinable()) worker_.join();
            if (ok && error.empty()){
                ShowProgress(100);
                InstallSteps::MarkVideosInstalled(output);
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
    InstallSteps::SetToolLocations(
      install_dir, settings_->value("FFmpegPath").toString().toStdString(),
      settings_->value("TimidityPath").toString().toStdString()
    );
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
                const float overall = InstallSteps::OverallProgress(InstallSteps::Phase::CONVERT, progress);
                QMetaObject::invokeMethod(this, [this, overall](){ShowProgress(overall);}, Qt::QueuedConnection);
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
            InstallSteps::MarkInstallFinished(install_dir_);
            OnInstallStopped();
            InstallLog::Get().Close();
        }
        else{
            InstallLog::Get().Add("[ERROR] Installation failed: " + error);
            InstallLog::Get().Close();
            OnInstallStopped();
            QMessageBox::critical(this, created ? tr("Data conversion exception") : tr("Installation error"), QString::fromStdString(error));
        }
    }, Qt::QueuedConnection);
}

void MainWindow::DoProgress(){
    try{
        const float progress = installer_->Progress();
        ShowProgress(InstallSteps::OverallProgress(InstallSteps::Phase::CONVERT, progress));
        if (progress >= 100){
            launch_available_ = true;
            InstallSteps::MarkInstallFinished(install_dir_);
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
