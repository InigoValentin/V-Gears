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

#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include "installer/formats/DiskIdentifier.h"
#include "installer/formats/DiskPaths.h"
#include "installer/formats/DiskReleases.h"
#include "installer/formats/Iso9660Reader.h"

namespace{

    /**
     * A folder that is removed when the test ends.
     */
    struct TempFolder{
        const boost::filesystem::path path
          = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
        TempFolder(){boost::filesystem::create_directories(path);}
        ~TempFolder(){boost::filesystem::remove_all(path);}
    };

    /**
     * Adds a folder record to a sector of an image.
     */
    void AddRecord(
      std::vector<char>& image, size_t& offset, const std::string& name, const uint32_t sector,
      const uint32_t size, const bool directory
    ){
        const size_t length = 33 + name.size() + (name.size() % 2 == 0 ? 1 : 0);
        image[offset] = static_cast<char>(length);
        for (int byte = 0; byte < 4; ++ byte){
            image[offset + 2 + byte] = static_cast<char>(sector >> (8 * byte));
            image[offset + 10 + byte] = static_cast<char>(size >> (8 * byte));
        }
        image[offset + 25] = directory ? 2 : 0;
        image[offset + 32] = static_cast<char>(name.size());
        std::copy(name.begin(), name.end(), image.begin() + offset + 33);
        offset += length;
    }
}

BOOST_AUTO_TEST_CASE(TestDiskReleases){
    const DiskReleases::Release* install = DiskReleases::FindPcRelease("FF7INST.EXE");
    BOOST_REQUIRE(install);
    BOOST_CHECK_EQUAL(install->disk, DiskImage::DISK_INSTALL);
    const DiskReleases::Release* disk_2 = DiskReleases::FindPcRelease("FF7/MOVIES/BIGLIGHT.AVI");
    BOOST_REQUIRE(disk_2);
    BOOST_CHECK_EQUAL(disk_2->disk, DiskImage::DISK_2);
    BOOST_CHECK(!DiskReleases::FindPcRelease("FF7/MOVIES/OTHER.AVI"));
    const DiskReleases::Release* french = DiskReleases::FindPlayStationRelease("SCES-10868");
    BOOST_REQUIRE(french);
    BOOST_CHECK_EQUAL(french->platform, DiskImage::PLATFORM_PS1);
    BOOST_CHECK_EQUAL(french->language, DiskImage::LANGUAGE_FRENCH);
    BOOST_CHECK_EQUAL(french->disk, DiskImage::DISK_2);
    BOOST_CHECK_EQUAL(std::string(french->warning), "French language is not fully supported in V-Gears.");
    BOOST_CHECK(!DiskReleases::FindPlayStationRelease("SCUS-00000"));
}

BOOST_AUTO_TEST_CASE(TestDiskPathsAreNormalized){
    BOOST_CHECK_EQUAL(DiskIdentifier::NormalizePath("/ff7\\movies/bike.avi;1 \n"), "FF7/MOVIES/BIKE.AVI");
    BOOST_CHECK_EQUAL(DiskIdentifier::RemoveVersion("Readme.;1"), "Readme");
    BOOST_CHECK_EQUAL(DiskIdentifier::RemoveVersion("a;b"), "a;b");
    BOOST_CHECK_EQUAL(DiskIdentifier::FindGameId("BOOT = cdrom:\\SCUS_941.63;1"), "SCUS-94163");
    BOOST_CHECK_EQUAL(DiskIdentifier::FindGameId("BOOT = cdrom:\\PSX.EXE;1"), "");
}

BOOST_AUTO_TEST_CASE(TestDiskFilesAreFound){
    TempFolder folder;
    boost::filesystem::create_directories(folder.path / "DATA" / "WM");
    std::ofstream((folder.path / "DATA" / "WM" / "WORLD_US.LGP").string()) << "x";
    std::ofstream((folder.path / "FF7INST.EXE").string()) << "x";
    const std::string base = folder.path.string();
    const auto world = DiskPaths::Find(base, "data/wm/world_us.lgp");
    BOOST_REQUIRE(world);
    BOOST_CHECK(world->find("DATA/WM/WORLD_US.LGP") != std::string::npos);
    // A name that begins the same way is used if there is no other.
    BOOST_CHECK(DiskPaths::Find(base, "ff7.exe"));
    std::ofstream((folder.path / "FF7.EXE").string()) << "x";
    BOOST_CHECK(DiskPaths::Find(base, "ff7.exe")->find("FF7.EXE") != std::string::npos);
    BOOST_CHECK(!DiskPaths::Find(base, "data/wm/world_us.bin"));
}

BOOST_AUTO_TEST_CASE(TestIso9660FilesAreRead){
    // The descriptor in the sector 16, the root folder in the 17, a folder in the 18 and a file
    // in the 19.
    const size_t sector = Iso9660Reader::SECTOR_SIZE;
    std::vector<char> image(20 * sector, 0);
    image[16 * sector] = 1;
    const std::string standard = "CD001";
    std::copy(standard.begin(), standard.end(), image.begin() + 16 * sector + 1);
    size_t offset = 16 * sector + 156;
    AddRecord(image, offset, std::string(1, '\0'), 17, sector, true);
    offset = 17 * sector;
    AddRecord(image, offset, std::string(1, '\0'), 17, sector, true);
    AddRecord(image, offset, "..", 17, sector, true);
    AddRecord(image, offset, "DATA", 18, sector, true);
    offset = 18 * sector;
    AddRecord(image, offset, "FILE.TXT;1", 19, 5, false);
    const std::string text = "hello";
    std::copy(text.begin(), text.end(), image.begin() + 19 * sector);
    TempFolder folder;
    const std::string path = (folder.path / "image.iso").string();
    std::ofstream(path, std::ios::binary).write(image.data(), image.size());

    Iso9660Reader reader(path);
    BOOST_REQUIRE(reader.IsValid());
    std::vector<std::string> paths;
    std::string contents;
    const Iso9660Reader::Result result = reader.Walk(
      DiskIdentifier::RemoveVersion,
      [&](const Iso9660Reader::Entry& entry){
          paths.push_back(entry.path);
          if (!entry.directory){
              reader.Read(entry, [&contents](const char* data, size_t size){
                  contents.append(data, size);
                  return true;
              });
          }
          return true;
      }
    );
    // The ".." folder, which would lead outside, is skipped.
    BOOST_CHECK(result == Iso9660Reader::Result::COMPLETE);
    BOOST_REQUIRE_EQUAL(paths.size(), 2u);
    BOOST_CHECK_EQUAL(paths[0], "DATA");
    BOOST_CHECK_EQUAL(paths[1], "DATA/FILE.TXT");
    BOOST_CHECK_EQUAL(contents, "hello");
    // A file past the end of the image can't be read.
    Iso9660Reader::Entry missing{"x", false, 100, 5};
    BOOST_CHECK(!reader.Read(missing, [](const char*, size_t){return true;}));
}
