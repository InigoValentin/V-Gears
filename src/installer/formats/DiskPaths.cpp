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
#include <QtCore/qdir.h>
#include <QtCore/qdiriterator.h>
#include <QtCore/qfileinfo.h>
#include "DiskPaths.h"

namespace{

    /**
     * Normalizes a part of a path: upper case, without version suffix, trailing dots or spaces.
     */
    QString NormalizeSegment(QString segment){
        segment = segment.trimmed();
        while (!segment.isEmpty() && (segment.endsWith('.') || segment.endsWith(' '))) segment.chop(1);
        const int separator = segment.indexOf(';');
        if (separator >= 0){
            const bool version = std::all_of(
              segment.begin() + separator + 1, segment.end(), [](QChar c){return c.isDigit();}
            );
            if (version) segment = segment.left(separator);
        }
        return segment.toUpper();
    }

    /**
     * Splits a path into its normalized parts.
     */
    QStringList SplitSegments(QString path){
        path.replace('\\', '/');
        while (path.startsWith('/')) path.remove(0, 1);
        QStringList segments;
        for (const QString& part : QDir::cleanPath(path).split('/', Qt::SkipEmptyParts)){
            if (part != ".") segments.push_back(NormalizeSegment(part));
        }
        return segments;
    }

    /**
     * Splits a name into its base name and its extension.
     */
    QPair<QString, QString> SplitExtension(const QString& name){
        const int dot = name.lastIndexOf('.');
        if (dot <= 0 || dot == name.size() - 1) return qMakePair(name, QString());
        return qMakePair(name.left(dot), name.mid(dot + 1));
    }

    /**
     * Checks if two names can be of the same file: the same extension, and base names that begin
     * the same way, without the number of the short names.
     */
    bool SegmentMatches(const QString& requested, const QString& actual){
        if (requested == actual) return true;
        const auto wanted = SplitExtension(requested);
        const auto found = SplitExtension(actual);
        if (wanted.second != found.second) return false;
        const QString wanted_base = wanted.first.section('~', 0, 0);
        const QString found_base = found.first.section('~', 0, 0);
        if (wanted_base.isEmpty() || found_base.isEmpty()) return false;
        return wanted_base.startsWith(found_base) || found_base.startsWith(wanted_base);
    }
}

namespace DiskPaths{

    std::unique_ptr<std::string> Find(const std::string& base_dir, const std::string& path){
        const QString base = QDir::cleanPath(QString::fromStdString(base_dir).replace('\\', '/'));
        const QString requested_path = QString::fromStdString(path);
        const QFileInfo direct(QDir(base).filePath(requested_path));
        if (direct.exists() && direct.isFile())
            return std::make_unique<std::string>(direct.absoluteFilePath().toStdString());
        const QStringList requested = SplitSegments(requested_path);
        if (requested.isEmpty()) return nullptr;
        std::unique_ptr<std::string> similar;
        QDirIterator it(base, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()){
            const QString file = it.next();
            const QStringList actual = SplitSegments(QDir(base).relativeFilePath(file));
            if (actual.size() < requested.size()) continue;
            // The last parts of the path of the file have to match.
            const int offset = actual.size() - requested.size();
            bool matches = true;
            bool exact = true;
            for (int i = 0; i < requested.size() && matches; ++ i){
                matches = SegmentMatches(requested[i], actual[offset + i]);
                if (requested[i] != actual[offset + i]) exact = false;
            }
            if (!matches) continue;
            const std::string found = QFileInfo(file).absoluteFilePath().toStdString();
            // A file with the same name wins. The names that only begin the same way (like
            // FF7INST.EXE for ff7.exe) are used only if there is no other one.
            if (exact) return std::make_unique<std::string>(found);
            if (!similar) similar = std::make_unique<std::string>(found);
        }
        return similar;
    }
}
