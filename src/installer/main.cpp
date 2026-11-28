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

#include <string>
#include <vector>
#include <QtCore/QCoreApplication>
#include <QtWidgets/QApplication>
#include <QtGui/QIcon>
#ifdef _WIN32
#include <cstdio>
#include <windows.h>
#endif

#include "installer/cli/CliInstaller.h"
#include "installer/cli/InstallCommandLine.h"
#include "installer/gui/MainWindow.h"

/**
 * Connects the standard streams to the console the installer was started from.
 *
 * In Windows the installer is a window application, so it has no console of its own. Without this, nothing written
 * from the command line would be seen.
 */
static void AttachToConsole(){
#ifdef _WIN32
    if (GetConsoleWindow() != nullptr) return;
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) return;
    FILE* stream = nullptr;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
    freopen_s(&stream, "CONIN$", "r", stdin);
    std::ios::sync_with_stdio(true);
#endif
}

/**
 * Gets the name of the program, for the usage text.
 */
static std::string ProgramName(const char* argv0){
    std::string name = argv0 == nullptr ? std::string() : std::string(argv0);
    const size_t slash = name.find_last_of("/\\");
    if (slash != std::string::npos) name = name.substr(slash + 1);
    return name.empty() ? std::string("v-gears-installer") : name;
}

/**
 * Installer main function.
 *
 * Without arguments it opens the installer window. With command line options it installs from the command line
 * instead (see InstallCommandLine).
 *
 * @param[in] argc Number of arguments passed to the application.
 * @param[in] argv List of arguments passed to the application.
 * @return The application return code. 0 is OK.
 */
int main(int argc, char *argv[]){
    const InstallCommandLine::Arguments arguments
      = InstallCommandLine::Parse(std::vector<std::string>(argv + (argc > 0 ? 1 : 0), argv + argc));
    if (arguments.mode != InstallCommandLine::Mode::GUI){
        AttachToConsole();
        // No window, so no display is needed for Qt.
        QCoreApplication::setApplicationName(QStringLiteral("v-gears-installer"));
        QCoreApplication application(argc, argv);
        return CliInstaller::Run(arguments, ProgramName(argc > 0 ? argv[0] : nullptr));
    }
    QApplication::setApplicationName(QStringLiteral("v-gears-installer"));
    QApplication::setApplicationDisplayName(QStringLiteral("V-Gears Installer"));
    QApplication application(argc, argv);
    QApplication::setDesktopFileName(QStringLiteral("v-gears-installer"));
    const QIcon installerIcon(QStringLiteral(":/icons/v-gears-installer.png"));
    if (!installerIcon.isNull()) application.setWindowIcon(installerIcon);
    MainWindow window;
    if (!installerIcon.isNull()) window.setWindowIcon(installerIcon);
    window.show();
    return application.exec();
}
