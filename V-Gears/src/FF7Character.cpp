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


#include "FF7Character.h"

namespace FFVII{

    Character::Character(){}

    Character::~Character(){}

    Character::CharacterIdLookupMap Character::CreateLookupMap(){
        CharacterIdLookupMap lookup;
        lookup["Cloud"] = CLOUD;
        lookup["cloud"] = CLOUD;
        lookup["CLOUD"] = CLOUD;
        lookup["AAAA"] = CLOUD;
        lookup["n_cloud"] = CLOUD;
        lookup["Tifa"] = TIFA;
        lookup["tifa"] = TIFA;
        lookup["TIFA"] = TIFA;
        lookup["AAGB"] = TIFA;
        lookup["n_tifa"] = TIFA;
        lookup["Aeris"] = AERIS;
        lookup["AERIS"] = AERIS;
        lookup["aerit"] = AERIS;
        lookup["AUFF"] = AERIS;
        lookup["CAHC"] = AERIS;
        lookup["n_erith"] = AERIS;
        lookup["earithf"] = AERIS;
        lookup["Barret"] = BARRET;
        lookup["barret"] = BARRET;
        lookup["BARRET"] = BARRET;
        lookup["ACGD"] = BARRET;
        lookup["ballet"] = BARRET;
        lookup["Red"] = RED;
        lookup["red"] = RED;
        lookup["RED"] = RED;
        lookup["Red XIII"] = RED;
        lookup["RedXIII"] = RED;
        lookup["ADDA"] = RED;
        lookup["red"] = RED;
        lookup["Cid"] = CID;
        lookup["CID"] = CID;
        lookup["cid"] = CID;
        lookup["ABDA"] = CID;
        lookup["AEHD"] = VINCENT;
        lookup["Vincent"] = VINCENT;
        lookup["VINCENT"] = VINCENT;
        lookup["vincent"] = VINCENT;
        lookup["Yuffie"] = YUVI;
        lookup["yuffie"] = YUVI;
        lookup["YUFFIE"] = YUVI;
        lookup["ABJB"] = YUVI;
        lookup["yufi"] = YUVI;
        lookup["Cait Sith"] = KETCY;
        lookup["Cait sith"] = KETCY;
        lookup["CAIT SITH"] = KETCY;
        lookup["CaitSith"] = KETCY;
        lookup["Caitsith"] = KETCY;
        lookup["CAITSITH"] = KETCY;
        lookup["AEBC"] = KETCY;
        lookup["ketcy"] = KETCY;
        return lookup;
    }

    const Character::CharacterIdLookupMap Character::character_id_lookup_(
      Character::CreateLookupMap()
    );

    Character::CharacterId Character::GetIdByName(const VGears::String& name){
        CharacterIdLookupMap::const_iterator entry;
        entry = character_id_lookup_.find(name);
        if (entry != character_id_lookup_.end()) return entry->second;
        return UNKNOWN;
    }
}
