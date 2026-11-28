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
#include <fstream>
#include <iostream>
#include <memory>
#include <archive.h>
#include <archive_entry.h>
#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include "DiskExtractor.h"
#include "DiskIdentifier.h"
#include "Iso9660Reader.h"

namespace{

    /**
     * Pointer to a libarchive reader or writer, freed when it goes out of scope.
     */
    using ArchivePointer = std::unique_ptr<struct archive, int(*)(struct archive*)>;

    /**
     * Copies the contents of the current file of an image to the disk.
     *
     * @param[in] image The image.
     * @param[in] disk The disk writer.
     * @param[in] report Called after each block.
     * @param[out] error_message Why the file could not be copied.
     * @return True if the file was copied.
     */
    bool CopyData(
      struct archive* image, struct archive* disk, const std::function<void()>& report,
      std::string& error_message
    ){
        const void* buffer;
        size_t size;
        la_int64_t offset;
        while (true){
            const int read = archive_read_data_block(image, &buffer, &size, &offset);
            if (read == ARCHIVE_EOF) return true;
            if (read < ARCHIVE_WARN){
                error_message = std::string("Error reading data block: ") + archive_error_string(image);
                return false;
            }
            if (archive_write_data_block(disk, buffer, size, offset) < ARCHIVE_WARN){
                error_message = std::string("Error writing data block: ") + archive_error_string(disk);
                return false;
            }
            report();
        }
    }
}

DiskExtractor::DiskExtractor(
  const std::string& image_path, const std::function<void(float)>& progress
):
  image_path_(image_path), progress_(progress),
  image_size_(QFileInfo(QString::fromStdString(image_path)).size()), last_percent_(-1)
{}

bool DiskExtractor::Extract(const std::string& output_dir, std::string& error_message){
    Report(0);
    if (!QDir().mkpath(QString::fromStdString(output_dir))){
        error_message = "Failed to create extraction directory: " + output_dir;
        return false;
    }
    bool has_entries = false;
    if (!ExtractArchive(output_dir, has_entries, error_message)) return false;
    if (!has_entries && !ExtractFolders(output_dir, error_message)) return false;
    if (progress_) progress_(100);
    return true;
}

bool DiskExtractor::ExtractArchive(
  const std::string& output_dir, bool& has_entries, std::string& error_message
){
    ArchivePointer image(archive_read_new(), archive_read_free);
    archive_read_support_format_all(image.get());
    archive_read_support_filter_all(image.get());
    ArchivePointer disk(archive_write_disk_new(), archive_write_free);
    archive_write_disk_set_options(
      disk.get(),
      ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS
      | ARCHIVE_EXTRACT_SECURE_NODOTDOT
    );
    archive_write_disk_set_standard_lookup(disk.get());
    if (archive_read_open_filename(image.get(), image_path_.c_str(), 10240) != ARCHIVE_OK){
        error_message = std::string("Could not open image file: ") + archive_error_string(image.get());
        return false;
    }
    const auto report = [this, &image](){Report(archive_filter_bytes(image.get(), -1));};
    struct archive_entry* entry;
    int status;
    while (
      (status = archive_read_next_header(image.get(), &entry)) == ARCHIVE_OK || status == ARCHIVE_WARN
    ){
        has_entries = true;
        if (status == ARCHIVE_WARN){
            std::cerr << "archive_read_next_header warning during extraction: "
              << archive_error_string(image.get()) << std::endl;
        }
        const char* name = archive_entry_pathname(entry);
        if (name == nullptr){
            std::cerr << "Skipping entry without a pathname" << std::endl;
            continue;
        }
        const std::string path = output_dir + "/" + name;
        archive_entry_set_pathname(entry, path.c_str());
        const int written = archive_write_header(disk.get(), entry);
        if (written < ARCHIVE_OK)
            std::cerr << "Warning (Header): " << archive_error_string(disk.get()) << std::endl;
        if (written < ARCHIVE_WARN){
            error_message = "Could not create " + path;
            return false;
        }
        if (
          archive_entry_size(entry) > 0
          && !CopyData(image.get(), disk.get(), report, error_message)
        ) return false;
        archive_write_finish_entry(disk.get());
    }
    if (status != ARCHIVE_EOF){
        error_message = std::string("Error reading the image: ") + archive_error_string(image.get());
        return false;
    }
    return true;
}

bool DiskExtractor::ExtractFolders(const std::string& output_dir, std::string& error_message){
    Iso9660Reader reader(image_path_);
    if (!reader.IsValid()){
        error_message = "Direct ISO9660 fallback failed: invalid primary volume descriptor.";
        return false;
    }
    bool ok = true;
    const auto extract = [&](const Iso9660Reader::Entry& entry){
        const QString path = QString::fromStdString(output_dir + "/" + entry.path);
        if (entry.directory){
            ok = QDir().mkpath(path);
            return ok;
        }
        if (!QDir().mkpath(QFileInfo(path).absolutePath())){
            ok = false;
            return false;
        }
        std::ofstream file(path.toStdString(), std::ios::binary);
        ok = file && reader.Read(entry, [&](const char* data, size_t size){
            file.write(data, static_cast<std::streamsize>(size));
            Report(reader.Position());
            return file.good();
        });
        file.close();
        ok = ok && !file.fail();
        return ok;
    };
    // Only the version suffix and the trailing dots are removed: the files keep their case.
    const Iso9660Reader::Result result = reader.Walk(
      [](const std::string& name){
          std::string normalized = name;
          std::replace(normalized.begin(), normalized.end(), '\\', '/');
          return DiskIdentifier::RemoveVersion(normalized);
      },
      extract
    );
    if (!ok || result == Iso9660Reader::Result::READ_ERROR){
        error_message = "Direct ISO9660 fallback failed while extracting files.";
        return false;
    }
    return true;
}

void DiskExtractor::Report(const int64_t position){
    if (!progress_ || image_size_ <= 0) return;
    const int percent = static_cast<int>(std::min<int64_t>(100, 100 * position / image_size_));
    if (percent <= last_percent_) return;
    last_percent_ = percent;
    progress_(static_cast<float>(percent));
}
