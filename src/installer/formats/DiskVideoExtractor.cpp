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
#include <cstdio>
#include <fstream>
#include <memory>
#include <sys/stat.h>
#include <archive.h>
#include <archive_entry.h>
#include <QtCore/qdir.h>
#include "DiskVideoExtractor.h"
#include "installer/steps/VideoFiles.h"

namespace{

    /**
     * Pointer to a libarchive reader, freed when it goes out of scope.
     */
    using ArchivePointer = std::unique_ptr<struct archive, int(*)(struct archive*)>;

    /**
     * Opens a disk image.
     *
     * @param[in] path Path of the image.
     * @param[in] log Function to write the error to.
     * @return The image, or null if it can't be opened.
     */
    ArchivePointer Open(const std::string& path, const std::function<void(const std::string&)>& log){
        ArchivePointer image(archive_read_new(), archive_read_free);
        archive_read_support_format_all(image.get());
        archive_read_support_filter_all(image.get());
        if (archive_read_open_filename(image.get(), path.c_str(), 10240) != ARCHIVE_OK){
            log("[ERROR] Could not open the disk image: " + std::string(archive_error_string(image.get())));
            image.reset();
        }
        return image;
    }

    /**
     * Moves to the next video of a disk.
     *
     * @param[in] image The disk image.
     * @param[out] name Name of the video.
     * @return The entry of the video, or null if there are no more.
     */
    struct archive_entry* NextVideo(struct archive* image, std::string& name){
        struct archive_entry* entry;
        while (archive_read_next_header(image, &entry) == ARCHIVE_OK){
            const char* path = archive_entry_pathname(entry);
            if (path && archive_entry_filetype(entry) == AE_IFREG && VideoFiles::IsMovie(path, &name))
                return entry;
        }
        return nullptr;
    }
}

DiskVideoExtractor::DiskVideoExtractor(
  const std::string& image_path, const std::function<void(float)>& progress,
  const std::function<void(const std::string&)>& log
):
  image_path_(image_path), progress_(progress), log_(log), total_bytes_(0), done_bytes_(0),
  extracted_(0), skipped_(0)
{}

bool DiskVideoExtractor::Extract(const std::string& video_dir){
    extracted_ = 0;
    skipped_ = 0;
    done_bytes_ = 0;
    if (!QDir().mkpath(QString::fromStdString(video_dir))){
        log_("[ERROR] Could not create " + video_dir);
        return false;
    }
    // First pass: which videos the disk has, and their total size, to report the progress.
    total_bytes_ = TotalSize();
    if (total_bytes_ == 0) return false;
    // Second pass: copy them.
    ArchivePointer image = Open(image_path_, log_);
    if (!image) return false;
    bool ok = true;
    std::string name;
    while (struct archive_entry* entry = NextVideo(image.get(), name)){
        const uint64_t size = archive_entry_size(entry);
        const std::string destination = video_dir + name;
        // Several disks have the same videos: skip the ones that are already there.
        struct stat existing;
        if (stat(destination.c_str(), &existing) == 0 && static_cast<uint64_t>(existing.st_size) == size){
            ++ skipped_;
            Advance(size);
        }
        else if (Copy(image.get(), name, destination)){
            log_("Extracted " + name);
            ++ extracted_;
        }
        else ok = false;
    }
    progress_(100);
    return ok;
}

uint64_t DiskVideoExtractor::TotalSize(){
    ArchivePointer image = Open(image_path_, log_);
    if (!image) return 0;
    uint64_t total = 0;
    std::string name;
    while (struct archive_entry* entry = NextVideo(image.get(), name))
        total += archive_entry_size(entry);
    if (total == 0) log_("[ERROR] The disk image has no videos.");
    return total;
}

bool DiskVideoExtractor::Copy(
  struct archive* image, const std::string& name, const std::string& destination
){
    const std::string partial = destination + ".part";
    std::ofstream out(partial, std::ios::binary | std::ios::trunc);
    if (!out){
        log_("[ERROR] Could not write " + destination);
        return false;
    }
    const void* buffer;
    size_t length;
    la_int64_t offset;
    bool read_ok = true;
    while (true){
        const int status = archive_read_data_block(image, &buffer, &length, &offset);
        if (status == ARCHIVE_EOF) break;
        if (status < ARCHIVE_OK){
            log_("[ERROR] Could not read " + name + ": " + archive_error_string(image));
            read_ok = false;
            break;
        }
        out.seekp(offset);
        out.write(static_cast<const char*>(buffer), length);
        Advance(length);
    }
    out.close();
    if (!read_ok || !out){
        if (read_ok) log_("[ERROR] Could not write " + destination);
        std::remove(partial.c_str());
        return false;
    }
    std::remove(destination.c_str());
    if (std::rename(partial.c_str(), destination.c_str()) != 0){
        log_("[ERROR] Could not create " + destination);
        return false;
    }
    return true;
}

void DiskVideoExtractor::Advance(const uint64_t bytes){
    done_bytes_ += bytes;
    progress_(100.0f * std::min<uint64_t>(done_bytes_, total_bytes_) / total_bytes_);
}
