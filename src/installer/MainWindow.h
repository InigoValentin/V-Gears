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

#include <QtWidgets/QMainWindow>
#include <QtCore/QSettings>
#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <vector>
#include "DataInstaller.h"
#include "DiskImage.h"
#include "ExternalTools.h"

class QCheckBox;
struct InstallOptions;

namespace Ui {
    class MainWindow;
}

/**
 * The installer main window.
 */
class MainWindow : public QMainWindow{

    public:

        /**
         * Constructor.
         *
         * @param[in] parent Widget to be made parent of the window.
         */
        explicit MainWindow(QWidget *parent = 0);

        /**
         * Destructor.
         */
        ~MainWindow();

    private slots:

        /**
         * Triggered when About is chosen in the menu. Shows the version and the license.
         */
        void on_actionABout_triggered();

        /**
         * Triggered when Help is chosen in the menu. Explains how to use the installer.
         */
        void on_actionHelp_triggered();

        /**
         * Triggered when Exit is chosen in the menu. Disabled while installing, like the close button.
         */
        void on_actionExit_triggered();

        /**
         * Triggered when the input data directory button is clicked.
         *
         * Opens a file manager for directory selection.
         */
        void on_btn_data_src_clicked();

        /**
         * Triggered when the output data directory has done being edited.
         *
         * Sets the selected directory.
         */
        void on_line_data_dst_editingFinished();

        /**
         * Triggered when the output data directory button is clicked.
         *
         * Opens a file manager for directory selection.
         */
        void on_btn_data_dst_clicked();

        /**
         * Triggered when the install button is clicked.
         *
         * Launches the installation.
         */
        void on_btn_data_run_clicked();

        /**
         * Triggered when the verify button is clicked.
         *
         * Checks that the installation has everything the game needs.
         */
        void on_btn_verify_clicked();

        /**
         * Triggered when the launch game button is clicked.
         */
        void on_btn_launch_clicked();

        /**
         * Triggered when the advanced options checkbox is checked or unchecked.
         */
        void on_chk_advanced_options_stateChanged();

    //private slots:

        /**
         * Makes the installation progress advance.
         */
        void DoProgress();

    private:

        /**
         * Enables or disables the UI.
         *
         * @param[in] enable True to enable the UI, false to disable it.
         */
        void EnableUi(bool enable);

        /**
         * Sets where the installer looks for FFmpeg and TiMidity.
         *
         * @param[in] install_dir Installation directory.
         */
        void UpdateToolLocations(const QString& install_dir);

        /**
         * Looks for FFmpeg and TiMidity and shows what was found in the window.
         */
        void RefreshToolStatus();

        /**
         * Asks the user for the path of FFmpeg or TiMidity. It is saved if it works.
         *
         * @param[in] ffmpeg True for FFmpeg, false for TiMidity.
         * @return True if a working program was chosen.
         */
        bool LocateTool(bool ffmpeg);

        /**
         * Checks that the programs the options need can be used and, if not, tells the user what would be lost and
         * offers to locate them, get them, check again, continue without them, or cancel.
         *
         * @param[in] options The installation options.
         * @return True to go on with the installation, false to cancel it.
         */
        bool ResolveMissingTools(const InstallOptions& options);

        /**
         * Starts the log of an installation: the file in the installation directory and the log window.
         *
         * @param[in] dir Installation directory, ending with '/'.
         * @param[in] append True to keep what a previous run wrote to the log file.
         */
        void OpenInstallLog(const QString& dir, bool append);

        /**
         * Extracts the videos of the selected game disk, in the worker thread.
         *
         * @param[in] output Installation directory, ending with '/'.
         */
        void StartVideoExtraction(const QString& output);

        /**
         * The checkboxes of the options that choose what to install.
         */
        std::vector<QCheckBox*> OptionBoxes() const;

        /**
         * Reads the options from the window.
         */
        InstallOptions CurrentOptions() const;

        /**
         * Sets the options in the window.
         */
        void SetOptions(const InstallOptions& options);

        /**
         * Applies a preset.
         *
         * @param[in] index Index of the preset in the preset box.
         */
        void ApplyPreset(int index);

        /**
         * Enforces the rules between options (disabling what can't be chosen) and updates the space estimate.
         */
        void UpdateOptions();

        /**
         * Starts V-Gears from an installation directory.
         */
        void LaunchGame(const QString& dir);

        /**
         * Index of the "Custom" preset.
         */
        static const int PRESET_CUSTOM = 5;

        /**
         * Shows the installation progress.
         *
         * @param[in] progress Installation progress [0-100].
         */
        void ShowProgress(float progress);

        /**
         * Runs the whole installation. It is the body of the worker thread.
         *
         * @param[in] options The installation options.
         * @param[in] output_path The installation directory.
         * @param[in] write_line Receives the installer messages.
         */
        void RunInstallation(
          DataInstaller::AdvancedOptions options, std::string output_path,
          std::function<void(std::string, int, bool)> write_line
        );

        /**
         * Triggered when the installation starts.
         *
         * Disables the UI.
         */
        void OnInstallStarted();

        /**
         * Triggered when the installation stops.
         *
         * Enables the UI.
         */
        void OnInstallStopped();

        /**
         * Initializes the installer settings.
         */
        void InitSettings(void);

        // The Q_OBJECT macro must appear in the private section of a class
        // definition that declares its own signals and slots or that uses other
        // services provided by Qt's meta-object system.
        Q_OBJECT

        /**
         * The installer window.
         */
        Ui::MainWindow* main_window_;

        /**
         * A timer.
         */
        QTimer* timer_;

        /**
         * The installer settings.
         */
        QSettings *settings_;

        /**
         * The installer.
         */
        std::unique_ptr<class DataInstaller> installer_;

        /**
         * The thread that runs the installation.
         */
        std::thread worker_;

        /**
         * Set to ask the worker thread to stop as soon as possible.
         */
        std::atomic<bool> stop_requested_{false};

        /**
         * True if the current installation runs in the worker thread, false if it is advanced by the timer.
         */
        bool background_ = false;

        /**
         * True once an installation has started. The installer can only run once.
         */
        bool install_started_ = false;

        /**
         * True while the option rules are being applied, to avoid applying them recursively.
         */
        bool updating_options_ = false;


        /**
         * True if the game can be launched from the window (after an installation or a verification).
         */
        bool launch_available_ = false;

        /**
         * The conversion programs found by {@see RefreshToolStatus}.
         */
        ExternalTools::Tool ffmpeg_tool_;
        ExternalTools::Tool timidity_tool_;

        /**
         * The selected disk image.
         */
        DiskImage disk_image_;
};
