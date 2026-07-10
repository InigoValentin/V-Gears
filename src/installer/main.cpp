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

#include <QtWidgets/QApplication>
#include <QtGui/QIcon>

#include "MainWindow.h"

/**
 * Installer main function.
 *
 * @param[in] argc Number of arguments passed to the application.
 * @param[in] argv List of arguments passed to the application.
 * @return The application return code. 0 is OK.
 */
int main(int argc, char *argv[]){
    QApplication::setApplicationName(QStringLiteral("v-gears-installer"));
    QApplication::setApplicationDisplayName(QStringLiteral("V-Gears Installer"));
    QApplication application(argc, argv);
    QApplication::setDesktopFileName(QStringLiteral("v-gears-installer"));
    const QIcon installerIcon(QStringLiteral(":/icons/v-gears-installer.png"));
    if (!installerIcon.isNull()) {
        application.setWindowIcon(installerIcon);
    }
    MainWindow window;
    if (!installerIcon.isNull()) {
        window.setWindowIcon(installerIcon);
    }
    window.show();
    return application.exec();
}
