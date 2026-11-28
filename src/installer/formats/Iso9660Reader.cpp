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
#include "Iso9660Reader.h"

const size_t Iso9660Reader::SECTOR_SIZE = 2048;

namespace{

    /**
     * Sector of the primary volume descriptor.
     */
    const uint64_t DESCRIPTOR_SECTOR = 16;

    /**
     * Size of a folder record without its name.
     */
    const size_t RECORD_HEADER_SIZE = 33;

    /**
     * Deepest folder that is read. ISO9660 allows 8 levels; a damaged image could have a folder
     * inside itself.
     */
    const int MAX_DEPTH = 32;

    /**
     * Reads a little endian 32 bit value.
     */
    uint32_t ReadLe32(const uint8_t* bytes){
        return static_cast<uint32_t>(bytes[0]) | (static_cast<uint32_t>(bytes[1]) << 8)
          | (static_cast<uint32_t>(bytes[2]) << 16) | (static_cast<uint32_t>(bytes[3]) << 24);
    }
}

Iso9660Reader::Iso9660Reader(const std::string& path):
  file_(path, std::ios::binary), file_size_(0), root_sector_(0), root_size_(0), valid_(false)
{
    if (!file_) return;
    file_.seekg(0, std::ios::end);
    file_size_ = static_cast<uint64_t>(file_.tellg());
    const std::vector<uint8_t> descriptor = ReadBytes(DESCRIPTOR_SECTOR * SECTOR_SIZE, SECTOR_SIZE);
    if (
      descriptor.size() != SECTOR_SIZE || descriptor[0] != 1
      || std::string(descriptor.begin() + 1, descriptor.begin() + 6) != "CD001"
    ) return;
    // The record of the root folder starts at the byte 156.
    root_sector_ = ReadLe32(&descriptor[156 + 2]);
    root_size_ = ReadLe32(&descriptor[156 + 10]);
    valid_ = true;
}

bool Iso9660Reader::IsValid() const{return valid_;}

Iso9660Reader::Result Iso9660Reader::Walk(
  const std::function<std::string(const std::string&)>& normalize,
  const std::function<bool(const Entry&)>& visit
){
    if (!valid_) return Result::READ_ERROR;
    return WalkFolder(root_sector_, root_size_, "", 0, normalize, visit);
}

bool Iso9660Reader::Read(
  const Entry& entry, const std::function<bool(const char*, size_t)>& write
){
    file_.clear();
    file_.seekg(static_cast<std::streamoff>(entry.sector) * SECTOR_SIZE, std::ios::beg);
    std::vector<char> chunk(64 * 1024);
    uint32_t remaining = entry.size;
    while (remaining > 0){
        file_.read(chunk.data(), std::min<size_t>(chunk.size(), remaining));
        const std::streamsize got = file_.gcount();
        if (got <= 0 || !write(chunk.data(), static_cast<size_t>(got))) return false;
        remaining -= static_cast<uint32_t>(got);
    }
    return true;
}

int64_t Iso9660Reader::Position(){
    file_.clear();
    return static_cast<int64_t>(file_.tellg());
}

std::vector<uint8_t> Iso9660Reader::ReadBytes(const uint64_t offset, const size_t size){
    // A damaged image could give any size: not more than what is left is allocated.
    if (offset >= file_size_) return {};
    std::vector<uint8_t> bytes(std::min<uint64_t>(size, file_size_ - offset));
    file_.clear();
    file_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    file_.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    bytes.resize(static_cast<size_t>(file_.gcount()));
    return bytes;
}

Iso9660Reader::Result Iso9660Reader::WalkFolder(
  const uint32_t sector, const uint32_t size, const std::string& prefix, const int depth,
  const std::function<std::string(const std::string&)>& normalize,
  const std::function<bool(const Entry&)>& visit
){
    if (size == 0) return Result::COMPLETE;
    if (depth > MAX_DEPTH) return Result::READ_ERROR;
    const std::vector<uint8_t> records
      = ReadBytes(static_cast<uint64_t>(sector) * SECTOR_SIZE, size);
    if (records.empty()) return Result::READ_ERROR;
    size_t offset = 0;
    while (offset < records.size()){
        const uint8_t record_size = records[offset];
        // Records don't cross sectors: the rest of the sector is empty.
        if (record_size == 0){
            offset = ((offset / SECTOR_SIZE) + 1) * SECTOR_SIZE;
            continue;
        }
        if (offset + record_size > records.size() || offset + RECORD_HEADER_SIZE > records.size()) break;
        const uint8_t* record = &records[offset];
        const size_t name_size = record[32];
        offset += record_size;
        const size_t name_offset = (record - records.data()) + RECORD_HEADER_SIZE;
        if (name_size == 0 || name_offset + name_size > records.size()) continue;
        // The entries of the folder itself and of its parent.
        if (name_size == 1 && (records[name_offset] == 0x00 || records[name_offset] == 0x01)) continue;
        const std::string name = normalize(std::string(
          reinterpret_cast<const char*>(records.data()) + name_offset, name_size
        ));
        if (name.empty() || name == "." || name == ".." || name.find('/') != std::string::npos) continue;
        Entry entry;
        entry.path = prefix.empty() ? name : prefix + "/" + name;
        entry.directory = (record[25] & 0x02) != 0;
        entry.sector = ReadLe32(record + 2);
        entry.size = ReadLe32(record + 10);
        if (!visit(entry)) return Result::STOPPED;
        if (entry.directory){
            const Result result = WalkFolder(
              entry.sector, entry.size, entry.path, depth + 1, normalize, visit
            );
            if (result != Result::COMPLETE) return result;
        }
    }
    return Result::COMPLETE;
}
