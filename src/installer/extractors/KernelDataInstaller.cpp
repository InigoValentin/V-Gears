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
#include "installer/formats/DiskImage.h"
#include "installer/extractors/KernelDataInstaller.h"

KernelDataInstaller::KernelDataInstaller(DiskImage& disk_image):
  kernel_(*disk_image.filePath("data/kernel/KERNEL.BIN")), exe_path_(*disk_image.filePath("ff7.exe")
){
    for (int i = 0; i < 416; i ++) prices_[i] = 50;
}

KernelDataInstaller::~KernelDataInstaller(){}

void KernelDataInstaller::ReadPrices(){
    File exe_file(exe_path_);
    exe_file.SetOffset(0x00514518); // Items, weapons, armors, accessories start here
    for (int i = 0; i < 320; i ++) prices_[i] = exe_file.readU32LE();
    exe_file.SetOffset(0x00514B18); // Materias start here
    for (int i = 320; i < 416; i ++) prices_[i] = exe_file.readU32LE();
}

void KernelDataInstaller::WriteShops(std::string file_name){
    // Shop inventories are a table of 80 records of 84 bytes in ff7.exe: u16 type, u16 number of
    // entries, and 10 entries of u32 kind (0: item, weapon, armor or accessory; 1: materia) and u32
    // index (the index of the item as in the prices table, or the ID of the materia).
    const int SHOP_TABLE_OFFSET = 0x00512AD8;
    const int SHOP_COUNT = 80;
    std::fstream file;
    file.open(file_name, std::ios::out);
    if (!file) return;
    File exe_file(exe_path_);
    file << "-- Inventory of each shop. Prices are the ones of the items and the materia.\n"
      << "-- type: kind of shop (1: weapons, 2: items, 3: materia...). Entries are {materia, id}.\n"
      << "Game.Shops = {}\n";
    for (int s = 0; s < SHOP_COUNT; s ++){
        exe_file.SetOffset(SHOP_TABLE_OFFSET + s * 84);
        u16 type = exe_file.readU16LE();
        u16 count = exe_file.readU16LE();
        file << "Game.Shops[" << s << "] = {\n    type = " << type << ",\n    items = {";
        for (int i = 0; i < 10; i ++){
            u32 kind = exe_file.readU32LE();
            u32 index = exe_file.readU32LE();
            if (i >= count) continue;
            file << "\n        {" << (kind == 1 ? "true" : "false") << ", " << index << "},";
        }
        file << "\n    }\n}\n";
    }
}
