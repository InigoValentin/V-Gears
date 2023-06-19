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
#include "Event.h"

/**
 * A base manager.
 *
 * Contain functionalities shared by all managers that contains.
 */
class Manager{

    public:

        /**
         * The modules the entity manager can handle.
         */
        enum Module{

            /**
             * Field module.
             *
             * Used in field maps. It has background, walkmesh, entities...
             */
            FIELD = 0,

            /**
             * Battle module.
             *
             * Used in battles. During battles there is no walkmesh.
             */
            BATTLE = 1,

            /**
             * World map module.
             *
             * Used in the world map. It has background, walkmesh, entities...
             */
            WORLD = 2
        };

        /**
         * Constructor.
         */
        Manager();

        /**
         * Destructor.
         */
        virtual ~Manager();

        /**
         * Retrieves the currently selected module.
         *
         * @return The currently selected module.
         */
        Module GetModule();

        /**
         * Checks the currently selected module.
         *
         * @param[in] module The module to check.
         * @return True if the currently selected module matches the module to check, false
         * otherwise.
         */
        bool IsModule(Module module);

        /**
         * Checks if the currently selected module is the field module.
         *
         * @return True if the currently selected module is the field module, false otherwise.
         */
        bool IsFieldModule();

        /**
         * Checks if the currently selected module is the battle module.
         *
         * @return True if the currently selected module is the battle module, false otherwise.
         */
        bool IsBattleModule();

        /**
         * Checks if the currently selected module is the world module.
         *
         * @return True if the currently selected module is the world module, false otherwise.
         */
        bool IsWorldModule();

        /**
         * Sets the current module for the entity manager.
         *
         * Operation availability and entity visibility will depend on the loaded module. Check
         * {@see SetFieldModule()}, {@see SetBattleModule()} and {@see SetWorldModule()} for
         * information about what changing modules implies.
         */
        void SetModule(Module module);

        /**
         * Sets the current module to the field mode.
         *
         * Setting the field module will clear all the information stored in the battle and world
         * modules.
         */
        void SetFieldModule();

        /**
         * Sets the current module to the battle mode.
         *
         * Setting the field module will not clear the information stored in the field and world
         * modules.
         */
        void SetBattleModule();

        /**
         * Sets the current module to the world map mode.
         *
         * Setting the world module will clear all the information stored in the battle and field
         * modules.
         */
        void SetWorldModule();

        /**
         * Sets the module that was loaded before a battle.
         *
         * Calling this will call either {@see SetFieldModule()} or {@see SetWorldModule},
         * depending on which module was loaded before. Calling this while not in the battle module
         * will do nothing.
         */
        void SetPreviousModule();

        /**
         * Handles an input event.
         *
         * @param[in] event Event to handle.
         */
        virtual void Input(const VGears::Event& event) = 0;

        /**
         * Called every frame, performs an update on the things controlled by the manager.
         *
         * It's functionality may be dependent on the current module.
         */
        void Update();

        /**
         * Called every frame, performs an update on the things controlled by the manager.
         *
         * It must update debug contents. It's functionality may depend on the current module.
         */
        virtual void UpdateDebug() = 0;

        /**
         * Handles resizing events
         */
        virtual void OnResize() = 0;

        /**
         * Clears the manager.
         *
         * Clears the manager for the currently loaded module. Check {@see ClearField()},
         * {@see ClearBattle()}, {@see ClearWorld()} for information about what this does
         * depending on the currently loaded module.
         */
        void Clear();

        /**
         * Clears the manager.
         *
         * Clears the manager for the selected module. Check {@see ClearField()},
         * {@see ClearBattle()}, {@see ClearWorld()} for information about what this does
         * depending on the currently loaded module.
         *
         * @param[in] module The module to clear.
         */
        void Clear(Module module);

        /**
         * Clear all field information in the manager.
         */
        virtual void ClearField() = 0;

        /**
         * Clear all battle information in the manager.
         */
        virtual void ClearBattle() = 0;

        /**
         * Clear all world map information in the manager.
         */
        virtual void ClearWorld() = 0;

        /**
         * Clears the manager.
         *
         * Clears the entity manager for every module. Check {@see ClearField()},
         * {@see ClearBattle()}, {@see ClearWorld()} for information about what this does
         * depending on the currently loaded module.
         */
        void ClearAll();

        /**
         * Handles game pausing.
         *
         * @param[in] paused True to pause, false to resume.
         */
        void ScriptSetPaused(const bool paused);

    protected:

        /**
         * Updates the field entities in the manager.
         */
        virtual void UpdateField() = 0;

        /**
         * Updates the battle entities in the manager.
         */
        virtual void UpdateBattle() = 0;

        /**
         * Updates the world map entities in the manager.
         */
        virtual void UpdateWorld() = 0;

        /**
         * Updates the entities of one module in the manager.
         *
         * @param[in] module The module whose entities to update.
         */
        void Update(Module module);

        /**
         * The currently selected module.
         */
        Module module_;

        /**
         * The previous module.
         */
        Module prev_module_;

        /**
         * Indicates if the game is paused.
         */
        bool paused_;

};
