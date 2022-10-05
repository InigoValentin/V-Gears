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

#include "common/TypeDefine.h"

/**
 * A playable character.
 */
class Character{

    public:

        /**
         * Constructor.
         */
        Character();

        /**
         * Destructor.
         */
        virtual ~Character();

        /**
         * List of character IDs.
         */
        enum CharacterId{

            /**
             * Cloud's ID.
             */
            CLOUD   = 0,

            /**
             * Tifa's ID.
             */
            TIFA = 1,

            /**
             * Aerith's ID.
             */
            AERIS = 2,

            /**
             * Barret's ID.
             */
            BARRET = 3,

            /**
             * Red XIII's ID.
             */
            RED = 4,

            /**
             * Cid's ID.
             */
            CID = 5,

            /**
             * Vincent's ID.
             */
            VINCENT = 6,

            /**
             * Yuffie's ID.
             */
            YUVI = 7,

            /**
             * Cait Sith's ID.
             */
            KETCY = 8,

            /**
             * Unknown character ID.
             */
            UNKNOWN = 9
        };

        /**
         * Retrieves a character ID from it's name.
         *
         * @param name[in] Chaarcter name.
         * @return ID of the character with the specified name, or {@see
         * UNKNOWN} if the character doesn't exist.
         */
        static CharacterId GetIdByName( const VGears::String& name);

    private:

        typedef std::map<VGears::String, CharacterId> CharacterIdLookupMap;

        /**
         * Creates a lookup map for character names.
         *
         * Names frequently used in the field scripts for the playable
         * characters are included in the lookup map.
         *
         * @return The character name lookup map.
         */
        static CharacterIdLookupMap CreateLookupMap();


        static const CharacterIdLookupMap character_id_lookup_;
};

