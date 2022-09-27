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

#pragma once

#include <OgreArchive.h>
#include "common/TypeDefine.h"

namespace VGears{

    /**
     * Handles LZS compressed archives.
     */
    class LGPArchive : public Ogre::Archive{

        public:

            /**
             * Constructor.
             *
             * @param name[in] Name for the archive.
             * @param arch_type[in] Archive type code.
             */
            LGPArchive(const String &name, const String &arch_type);

            /**
             * Destructor.
             */
            virtual ~LGPArchive();

            /**
             * Checks whether this archive is case sensitive.
             *
             * @return Always true.
             */
            bool isCaseSensitive() const{return true;}

            /**
             * Loads the archive.
             */
            void load();

            /**
             * Unloads the archive.
             */
            void unload();

            /**
             * Opens a stream on a LGP compressed file.
             *
             * @param filename[in] Path to open the stream on.
             * @param readOnly[in] TRue to open the stream in read-only mode,
             * false to allow writting.
             */
            Ogre::DataStreamPtr open(
              const String& filename, bool readOnly = true
            ) const;

            /**
             * Creates a new file (or overwrite one already there).
             *
             * If the archive is read-only then this method will fail.
             *
             * @param filename[in] Path to the file.
             */
            Ogre::DataStreamPtr create(const String& filename) const;

            /**
             * Deletes a named file.
             *
             * Not possible on read-only archives
             *
             * @param filename[in] Path to the file.
             */
            void remove(const String& filename) const;

            /**
             * Lists all file names in the archive.
             *
             * @param rercursive[in] Whether all paths of the archive are
             * to be searched (if the archive has a concept of that).
             * @param dirs[in] True to list only directories, false to list
             * only files.
             * @return A list of filenames matching the criteria, all fully
             * qualified.
             */
            Ogre::StringVectorPtr list(
              bool recursive = true, bool dirs = false
            ) const;

            /**
             * Lists all files in the archive with accompanying information.
             *
             * @param rercursive[in] Whether all paths of the archive are
             * to be searched (if the archive has a concept of that).
             * @param dirs[in] True to list only directories, false to list
             * only files.
             * @return A list of structures detailing quite a lot of
             * information about all the files in the archive.
             */
            Ogre::FileInfoListPtr listFileInfo(
              bool recursive = true, bool dirs = false
            ) const;

            /**
             * Finds all file or directory names matching a given pattern in the
             * archive.
             *
             * @param pattern[in] The pattern to search for; wildcards (*) are
             * allowed
             * @param recursive[in ]Whether all paths of the archive are
             * searched (if the archive has a concept of that).
             * @param dirs[in] True to list only directories, false to list
             * only files.
             * @return A list of filenames matching the criteria, all fully
             * qualified.
             */
            Ogre::StringVectorPtr find(
              const String& pattern, bool recursive = true, bool dirs = false
            ) const;

            /**
             * Finds all files or directories matching a given pattern in this
             * archive and get some detailed information about them.
             *
             * @param pattern[in] The pattern to search for; wildcards (*) are
             * allowed
             * @param recursive[in ]Whether all paths of the archive are
             * searched (if the archive has a concept of that).
             * @param dirs[in] True to list only directories, false to list
             * only files.
             * @return A list of file information structures for all files
             * matching the criteria.
             */
            Ogre::FileInfoListPtr findFileInfo(
              const String& pattern, bool recursive = true, bool dirs = false
            ) const;

            /**
             * checks if the named file exists.
             *
             * @param filename[in] Fully qualified filename.
             * @return True if FILENAME exists in the archive, false otherwise.
             */
            bool exists(const String& filename) const;

            /**
             * Retrieve the modification time of a given file.
             *
             * @param filename[in] Fully qualified filename.
             * @return Last-modified timestamp.
             */
            time_t getModifiedTime(const String& filename) const;

            /**
             * A file in a LGP archive.
             */
            struct FileEntry{

                /**
                 * The file name.
                 */
                String file_name;

                /**
                 * The file offset from the beginning of the archive.
                 */
                uint32 file_offset;

                /**
                 * Unknown data.
                 */
                uint8  unknown1;

                /**
                 * Unknown data.
                 */
                uint16 unknown2;

                /**
                 * NAme of the data file
                 */
                String datafile_name;

                /**
                 * Size of the data, in bytes.
                 */
                uint32 data_size;

                /**
                 * Data offset.
                 */
                uint32 data_offset;
            };

            typedef std::vector<FileEntry> FileList;

            /**
             * Retrieves the files in the archive.
             *
             * @return The list of files.
             */
            virtual FileList& GetFiles();

        protected:

            /**
             * Loads a LGP archive.
             *
             * @param lgp[in] The contents of the archive.
             */
            void Load(Ogre::DataStream* lgp);

        private:

            /**
             * List of file sin the archive.
             */
            FileList files_;

            /**
             * The LGP archive file.
             */
            Ogre::DataStreamPtr lgp_file_;

            /**
             * List of information blocks about about the files in the LGP.
             */
            Ogre::FileInfoList file_infos_;
    };
}
