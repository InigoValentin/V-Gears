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

#include <algorithm>
#include <iterator>
#include "DiskReleases.h"

namespace{

    /**
     * The PC disks. The game disks only have the videos; the rest of the game is installed from
     * the install disk.
     */
    const DiskReleases::Release PC_RELEASES[] = {
      {"FF7CONFIG.EXE", "Final Fantasy VII (PC, 1998, USA) Install Disk",
        DiskImage::PLATFORM_PC, DiskImage::REGION_NORTH_AMERICA, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_INSTALL, true, ""},
      {"FF7CONFI.EXE", "Final Fantasy VII (PC, 1998, USA) Install Disk",
        DiskImage::PLATFORM_PC, DiskImage::REGION_NORTH_AMERICA, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_INSTALL, true, ""},
      {"FF7CON~1.EXE", "Final Fantasy VII (PC, 1998, USA) Install Disk",
        DiskImage::PLATFORM_PC, DiskImage::REGION_NORTH_AMERICA, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_INSTALL, true, ""},
      {"FF7INST.EXE", "Final Fantasy VII (PC, 1998, USA) Install Disk",
        DiskImage::PLATFORM_PC, DiskImage::REGION_NORTH_AMERICA, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_INSTALL, true, ""},
      {"FF7/MOVIES/BIKE.AVI", "Final Fantasy VII (PC, 1998, USA) Disk 1",
        DiskImage::PLATFORM_PC, DiskImage::REGION_NORTH_AMERICA, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_1, true, ""},
      {"FF7/MOVIES/BIGLIGHT.AVI", "Final Fantasy VII (PC, 1998, USA) Disk 2",
        DiskImage::PLATFORM_PC, DiskImage::REGION_NORTH_AMERICA, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_2, true, ""},
      {"FF7/MOVIES/ENDING1.AVI", "Final Fantasy VII (PC, 1998, USA) Disk 3",
        DiskImage::PLATFORM_PC, DiskImage::REGION_NORTH_AMERICA, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_3, true, ""}
    };

    /**
     * The PlayStation disks.
     */
    const DiskReleases::Release PLAYSTATION_RELEASES[] = {
      {"SCUS-94163", "Final Fantasy VII (USA) Disc 1",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_NORTH_AMERICA, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_1, true,
        ""},
      {"SCUS-94164", "Final Fantasy VII (USA) Disc 2",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_NORTH_AMERICA, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_2, true,
        ""},
      {"SCUS-94165", "Final Fantasy VII (USA) Disc 3",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_NORTH_AMERICA, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_3, true,
        ""},
      {"SCES-00867", "Final Fantasy VII (Europe, English) Disc 1",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_1, true,
        ""},
      {"SCES-10867", "Final Fantasy VII (Europe, English) Disc 2",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_2, true,
        ""},
      {"SCES-20867", "Final Fantasy VII (Europe, English) Disc 3",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_ENGLISH,
        DiskImage::DISK_3, true,
        ""},
      {"SCES-00868", "Final Fantasy VII (Europe, French) Disc 1",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_FRENCH,
        DiskImage::DISK_1, false,
        "French language is not fully supported in V-Gears."},
      {"SCES-10868", "Final Fantasy VII (Europe, French) Disc 2",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_FRENCH,
        DiskImage::DISK_2, false,
        "French language is not fully supported in V-Gears."},
      {"SCES-20868", "Final Fantasy VII (Europe, French) Disc 3",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_FRENCH,
        DiskImage::DISK_3, false,
        "French language is not fully supported in V-Gears."},
      {"SCES-00900", "Final Fantasy VII (Europe, Spanish) Disc 1",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_SPANISH,
        DiskImage::DISK_1, true,
        "Spanish language is not fully supported in V-Gears."},
      {"SCES-10900", "Final Fantasy VII (Europe, Spanish) Disc 2",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_SPANISH,
        DiskImage::DISK_2, false,
        "Spanish language is not fully supported in V-Gears."},
      {"SCES-20900", "Final Fantasy VII (Europe, Spanish) Disc 3",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_SPANISH,
        DiskImage::DISK_3, false,
        "Spanish language is not fully supported in V-Gears."},
      {"SCES-00869", "Final Fantasy VII (Europe, German) Disc 1",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_GERMAN,
        DiskImage::DISK_1, true,
        "German language is not fully supported in V-Gears."},
      {"SCES-10869", "Final Fantasy VII (Europe, German) Disc 2",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_GERMAN,
        DiskImage::DISK_2, false,
        "German language is not fully supported in V-Gears."},
      {"SCES-20869", "Final Fantasy VII (Europe, German) Disc 3",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_GERMAN,
        DiskImage::DISK_3, false,
        "German language is not fully supported in V-Gears."},
      {"SCES-00901", "Final Fantasy VII (Europe, Italian) Disc 1",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_ITALIAN,
        DiskImage::DISK_1, true,
        "Italian language is not fully supported in V-Gears."},
      {"SCES-10901", "Final Fantasy VII (Europe, Italian) Disc 2",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_ITALIAN,
        DiskImage::DISK_2, false,
        "Italian language is not fully supported in V-Gears."},
      {"SCES-20901", "Final Fantasy VII (Europe, Italian) Disc 3",
        DiskImage::PLATFORM_PS1, DiskImage::REGION_EUROPE, DiskImage::LANGUAGE_ITALIAN,
        DiskImage::DISK_3, false,
        "Italian language is not fully supported in V-Gears."}
    };

    /**
     * Finds the release with a marker.
     */
    template<typename Releases>
    const DiskReleases::Release* Find(const Releases& releases, const std::string& marker){
        const auto found = std::find_if(
          std::begin(releases), std::end(releases),
          [&marker](const DiskReleases::Release& release){return marker == release.marker;}
        );
        return found == std::end(releases) ? nullptr : &*found;
    }
}

namespace DiskReleases{

    const Release* FindPcRelease(const std::string& path){return Find(PC_RELEASES, path);}

    const Release* FindPlayStationRelease(const std::string& game_id){
        return Find(PLAYSTATION_RELEASES, game_id);
    }
}
