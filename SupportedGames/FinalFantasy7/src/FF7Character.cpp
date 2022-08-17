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


#include "FFVIICharacter.h"

namespace FFVII{

    Character::Character(){}

    Character::~Character(){}

    Character::CharacterIdLookupMap Character::CreateLookupMap(){
        character_id_lookup_ lookup;
        lookup["AAAA"]      = CLOUD;
        lookup["n_cloud"]   = CLOUD;
        lookup["AAGB"]      = TIFA;
        lookup["n_tifa"]    = TIFA;
        lookup["AUFF"]      = AERIS;
        lookup["CAHC"]      = AERIS;
        lookup["n_erith"]   = AERIS;
        lookup["earithf"]   = AERIS;
        lookup["ACGD"]      = BARRET;
        lookup["ballet"]    = BARRET;
        lookup["ADDA"]      = RED;
        lookup["red"]       = RED;
        lookup["ABDA"]      = CID;
        lookup["cid"]       = CID;
        lookup["AEHD"]      = VINCENT;
        lookup["vincent"]   = VINCENT;
        lookup["ABJB"]      = YUVI;
        lookup["yufi"]      = YUVI;
        lookup["AEBC"]      = KETCY;
        lookup["ketcy"]     = KETCY;
        return lookup;
    }

    const Character::CharacterIdLookupMap Character::character_id_lookup_(
      Character::CreateLookupMap()
    );

    Character::CharacterId Character::GetIdByName(const QGears::String& name){
        character_id_lookup_::const_iterator entry;
        entry = ms_character_id_lookups.find(name);
        if (entry != ms_character_id_lookups.end()) return entry->second;
        return UNKNOWN;
    }
}
