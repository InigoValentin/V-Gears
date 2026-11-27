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

namespace VGears{

    /**
     * What the engine needs from the battle module.
     *
     * The battle manager implements it, so the engine doesn't depend on the module. There is at
     * most one instance, which registers itself when it is created.
     */
    class BattleInterface{

        public:

            /**
             * Retrieves the battle module.
             *
             * @return The battle module, or nullptr if there isn't any.
             */
            static BattleInterface* Get(){return Instance();}

            /**
             * Sets the battle mode of the following battles.
             *
             * @param[in] mode The battle mode.
             */
            virtual void ScriptSetMode(const int mode) = 0;

        protected:

            /**
             * Constructor. Registers the instance.
             */
            BattleInterface(){Instance() = this;}

            /**
             * Destructor. Unregisters the instance.
             */
            virtual ~BattleInterface(){if (Instance() == this) Instance() = nullptr;}

        private:

            /**
             * Holds the registered instance.
             *
             * @return Reference to the instance pointer.
             */
            static BattleInterface*& Instance(){
                static BattleInterface* instance = nullptr;
                return instance;
            }
    };
}
