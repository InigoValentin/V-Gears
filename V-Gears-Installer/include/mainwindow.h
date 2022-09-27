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

    // TODO ??? What is this.
    Q_OBJECT

    public:

        /**
         * Constructor.
         *
         * @param parent[in] Widget to be made parent of the window.
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
        void on_lineConfigDir_editingFinished();

        /**
         * Triggered when the configuration directory button is clicked.
         *
         * Opens a file manager for directory selection.
         */
        void on_btnConfigDir_clicked();

        /**
         * Triggered when the executable directory has done being edited.
         *
         * Sets the selected directory.
         */
        void on_lineVGearsExe_editingFinished();

        /**
         * Triggered when the executable directory button is clicked.
         *
         * Opens a file manager for file selection.
         */
        void on_btnVGearsExe_clicked();

        /**
         * Triggered when the launch button is clicked.
         *
         * Launches V-Gears.
         */
        void on_btnLaunch_clicked();

        /**
         * Triggered when the input data directory button is clicked.
         *
         * Opens a file manager for directory selection.
         */
        void on_btnInput_clicked();

        /**
         * Triggered when the output data directory has done being edited.
         *
         * Sets the selected directory.
         */
        void on_lineDataDir_editingFinished();

        /**
         * Triggered when the output data directory button is clicked.
         *
         * Opens a file manager for directory selection.
         */
        void on_btnDataDir_clicked();

        /**
         * Triggered when the install button is clicked.
         *
         * Launches the installation.
         */
        void on_btnGO_clicked();

    //private slots:

        /**
         * Makes the installation progress advance.
         */
        void DoProgress();

    private:

        /**
         * Enables or disables the UI.
         *
         * @param enable[in] True to enable the UI, false to disable it.
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

    private:

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
        std::unique_ptr<class FF7DataInstaller> installer_;
};
