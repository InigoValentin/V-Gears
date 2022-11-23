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

#include <OIS/OIS.h>
#include "Event.h"

/**
 * Possible states of a module.
 */
enum MODULE_STATE{

    /**
     * The module is starting.
     */
    MODULE_START,

    /**
     * The module is running.
     */
    MODULE_RUN,

    /**
     * The module has finished running.
     */
    MODULE_FINISH,

    /**
     * The module is stopped.
     */
    MODULE_STOP,

    /**
     * The module is paused.
     */
    MODULE_PAUSE
};

/**
 * A game module
 */
class Module {

    public:

        /**
         * Destructor.
         */
        virtual ~Module(){}

        /**
         * Handles an input event.
         */
        virtual void Input(const VGears::Event& event) = 0;

        /**
         * Updates the module state.
         */
        virtual void Update() = 0;

        /**
         * Sets the module state.
         *
         * @param[in] state Module's new state.
         */
        void SetState(const MODULE_STATE state){state_ = state;}

        /**
         * Retrieves the module state.
         *
         * @return The module's current state.
         */
        const MODULE_STATE GetState() const {return state_;}

    private:

        /**
         * The current module state.
         */
        MODULE_STATE state_;
};
