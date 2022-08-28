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

#include <OgreArchiveFactory.h>
#include "QGearsLGPArchive.h"

namespace QGears{

    /**
     * A factory for LGP archives.
     */
    class LGPArchiveFactory : public Ogre::ArchiveFactory{

        // TODO: Move implemented methods to cpp file.

        public:

            /**
             * Constructor.
             */
            LGPArchiveFactory();

            /**
             * Destructor.
             */
            virtual ~LGPArchiveFactory();

            /**
             * Retrieves the factory type.
             *
             * @return The factory type.
             */
            const String& getType(void) const{return ARCHIVE_TYPE;}

            /**
             * Creates a new LGP archive.
             *
             * @param name[in] Name of the archive to create.
             * @param readOnly[in] True to make the archive read-only, false to
             * enable writting.
             * @return A new LGP archive.
             */
            Ogre::Archive* createInstance(
              const String& name, bool readOnly
            ) override final{
                return OGRE_NEW LGPArchive(name, ARCHIVE_TYPE);
            }

            /**
             * Destroys a LGP archive created by this factory.
             *
             * @param arch[in] The archive to destroy.
             */
            void destroyInstance( Ogre::Archive* arch ) override final{
                OGRE_DELETE arch;
            }

            /**
             * The archive type.
             */
            static const String ARCHIVE_TYPE;

    };
}
