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

#include <OgreString.h>

class ConfigVarManager;

/**
 * A configuration variable
 */
class ConfigVar{
    friend class ConfigVarManager;

    public:

        /**
         * Constructor.
         *
         * @param name[in] The variable name.
         * @param description[in] A human-friendly description for the
         * variable.
         * @param default_value[in] The default value for the variable.
         */
        ConfigVar(
          const Ogre::String& name, const Ogre::String& description,
          const Ogre::String& default_value
        );

        /**
         * Retrieves the value of the variable in integer form.
         *
         * @return The variable value.
         */
        int GetI() const;

        /**
         * Retrieves the value of the variable in float form.
         *
         * @return The variable value.
         */
        float GetF() const;

        /**
         * Retrieves the value of the variable in boolean form.
         *
         * @return The variable value.
         */
        bool GetB() const;

        /**
         * Retrieves the value of the variable in string form.
         *
         * @return The variable value.
         */
        Ogre::String GetS() const;

        /**
         * Sets the integer value of the variable.
         *
         * @param value[in] The variable value.
         */
        void SetI(int value);

        /**
         * Sets the float value of the variable.
         *
         * @param value[in] The variable value.
         */
        void SetF(float value);

        /**
         * Sets the boolean value of the variable.
         *
         * @param value[in] The variable value.
         */
        void SetB(bool value);

        /**
         * Sets the string value of the variable.
         *
         * @param value[in] The variable value.
         */
        void SetS(const Ogre::String& value);

        /**
         * Retrieves the variable name.
         *
         * @return The variable name.
         */
        const Ogre::String& GetName() const;

        /**
         * Retrieves the variable description.
         *
         * @return The variable description.
         */
        const Ogre::String& GetDescription() const;

        /**
         * Retrieves the variable default value.
         *
         * @return The variable default value.
         */
        const Ogre::String& GetDefaultValue() const;

        /**
         * Updates the integer and float values from the string value.
         */
        void UpdateVariables();

    private:

        /**
         * Forbidden copy constructor.
         */
        ConfigVar(const ConfigVar&) = delete;

        /**
         * Forbidden copy constructor.
         */
        ConfigVar&operator=(const ConfigVar&) = delete;

        /**
         * The variable name.
         */
        Ogre::String    name_;

        /**
         * The variable description.
         */
        Ogre::String    description_;

        /**
         * The variable default value.
         */
        Ogre::String    default_value_;

        /**
         * Variable value, integer format.
         */
        int             value_i_;

        /**
         * Variable value, float format.
         */
        float           value_f_;

        /**
         * Variable value, integer format.
         */
        bool            value_b_;

        /**
         * Variable value, string format.
         */
        Ogre::String    value_s_;

        /**
         * @todo Understand and document.
         */
        ConfigVar*          previous_;

        /**
         * @todo Understand and document.
         */
        static ConfigVar*   static_config_var_list_;
};

