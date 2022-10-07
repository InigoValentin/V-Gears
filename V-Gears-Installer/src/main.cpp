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

#include "MainWindow.h"

/**
 * Installer main function.
 *
 * @param[in] argc Number of arguments passed to the application.
 * @param[in] argv List of arguments passed to the application.
 * @return The application return code. 0 is OK.
 */
int main(int argc, char *argv[]){
    QApplication application(argc, argv);
    MainWindow window;
    window.show();
    return application.exec();
}
