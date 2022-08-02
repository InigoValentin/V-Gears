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

namespace QGears{

    /**
     * Indicators for the game state.
     */
    enum GAME_STATE{

        /**
         * The game is ready to be quit and can exit in the next loop pass.
         */
        G_EXIT,

        /**
         * The game is running normally.
         */
        G_GAME
    };

    /**
     * Global application status indicator.
     */
    extern GAME_STATE g_ApplicationState;
}

