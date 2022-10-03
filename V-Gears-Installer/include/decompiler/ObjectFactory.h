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

#include <map>
#include <string>
#include "common/scummsys.h"

/**
 * Template function for creating an instance of Type.
 */
template<typename BaseType, typename Type> BaseType *CreateObject(){return new Type();}

/**
 * Generic factory for a class and its subclasses.
 */
template<typename KeyType, typename BaseType> class ObjectFactory{

    public:
        /**
         * Register a new entry.
         *
         * @param key[in] The key to register the class under.
         */
        template<typename Type> void AddEntry(const KeyType &key) {
            registry_[key] = &CreateObject<BaseType, Type>;
        }

        /**
         * Creates an instance of some registered class.
         *
         * @param key[in] The key associated with the desired class.
         * @return NULL if the name is not registered, else an instance of the
         * associated class.
         */
        BaseType *Create(const KeyType &key) const{
            typename RegistryMap::const_iterator entry = registry_.find(key);
            if (entry == registry_.end()) return NULL;
            return (entry->second)();
        }

    private:

        /**
         * Function pointer to the object creation function.
         */
        typedef BaseType *(*CreateFunc)();

        /**
         * Type used to store registered entries.
         */
        typedef std::map<KeyType, CreateFunc> RegistryMap;

        /**
         * Map from an identifier to a creation function.
         */
        RegistryMap registry_;
};

