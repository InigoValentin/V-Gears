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

#include <QtWidgets/QMainWindow> // IVV fix path #include <QMainWindow>
#include <QtCore/QSettings>
#include <memory>

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
         * Triggered when the configuration directory has done being edited.
         *
         * Sets the selected directory.
         */
        void on_line_vgears_config_editingFinished();

        /**
         * Triggered when the configuration directory button is clicked.
         *
         * Opens a file manager for directory selection.
         */
        void on_btn_vgears_config_clicked();

        /**
         * Triggered when the executable directory has done being edited.
         *
         * Sets the selected directory.
         */
        void on_line_vgears_exe_editingFinished();

        /**
         * Triggered when the executable directory button is clicked.
         *
         * Opens a file manager for file selection.
         */
        void on_btn_vgears_exe_clicked();

        /**
         * Triggered when the launch button is clicked.
         *
         * Launches V-Gears.
         */
        void on_btn_vgears_run_clicked();

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

        // The Q_OBJECT macro must appear in the private section of a class definition
        //that declares its own signals and slots or that uses other services provided
        // by Qt's meta-object system.
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
};
