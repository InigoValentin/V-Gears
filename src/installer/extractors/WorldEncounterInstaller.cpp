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

#include <iostream>
#include <boost/filesystem.hpp>
#include "WorldEncounterInstaller.h"

namespace{

    /**
     * Size of the data before the records of the encounters.
     */
    const unsigned int HEADER_SIZE = 0xA0;

    /**
     * Number of records of encounters.
     */
    const unsigned int RECORD_COUNT = 64;

    /**
     * Size of a record of encounters.
     */
    const unsigned int RECORD_SIZE = 32;

    /**
     * Walkability types that select each of the 4 terrain groups of every region. This table is
     * in the executable of the game, not in the data files. The region of a triangle is
     * (location & 0x1F), and a region over 15 is 15.
     */
    const unsigned int TERRAIN_TYPES[16][4] = {
      {0, 9, 0, 17}, {0, 0, 0, 17}, {0, 9, 1, 17}, {0, 20, 0, 17},
      {0, 9, 8, 17}, {0, 0, 25, 17}, {0, 9, 19, 17}, {0, 0, 1, 17},
      {0, 0, 1, 17}, {0, 9, 14, 17}, {0, 9, 25, 17}, {0, 10, 9, 17},
      {0, 9, 25, 17}, {0, 0, 8, 11}, {0, 0, 8, 0}, {0, 0, 1, 0}
    };
}

WorldEncounterInstaller::WorldEncounterInstaller(const std::string& data_dir): data_dir_(data_dir){}

void WorldEncounterInstaller::Install(const std::vector<u8>& data){
    std::unique_ptr<TiXmlElement> encounters = CreateEncounters(data);
    if (!encounters){
        std::cerr << "ENC_W.BIN has an unexpected size." << std::endl;
        return;
    }
    boost::filesystem::create_directories(data_dir_ + "world");
    TiXmlDocument doc;
    doc.LinkEndChild(new TiXmlDeclaration("1.0", "", ""));
    doc.LinkEndChild(encounters.release());
    doc.SaveFile(data_dir_ + "world/world_encounters.xml");
}

std::unique_ptr<TiXmlElement> WorldEncounterInstaller::CreateEncounters(const std::vector<u8>& data){
    if (data.size() < HEADER_SIZE + RECORD_COUNT * RECORD_SIZE) return nullptr;
    std::unique_ptr<TiXmlElement> root(new TiXmlElement("world_encounters"));
    for (unsigned int region = 0; region < 16; ++ region){
        for (unsigned int group = 0; group < 4; ++ group){
            const unsigned int offset = HEADER_SIZE + (region * 4 + group) * RECORD_SIZE;
            std::unique_ptr<TiXmlElement> record(new TiXmlElement("record"));
            record->SetAttribute("region", region);
            record->SetAttribute("terrain", TERRAIN_TYPES[region][group]);
            record->SetAttribute("enabled", data[offset] & 0x01);
            record->SetAttribute("rate", data[offset + 1]);
            std::string words;
            for (unsigned int w = 1; w < 16; ++ w){
                const unsigned int word = data[offset + w * 2] | (data[offset + w * 2 + 1] << 8);
                words += (w > 1 ? " " : "") + std::to_string(word);
            }
            record->SetAttribute("words", words);
            root->LinkEndChild(record.release());
        }
    }
    return root;
}
