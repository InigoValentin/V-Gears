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

#include <OgreImageCodec.h>
#include <OgreCodec.h>
#include "QGearsPrerequisites.h"

namespace QGears{

    /**
     * Handles text encoding from font images.
     */
    class _QGearsExport TexCodec : public Ogre::ImageCodec{

        public:

            /**
             * Constructor.
             */
            TexCodec();

            /**
             * Destructor.
             */
            virtual ~TexCodec();

            /**
             * Retrieves the codec type.
             *
             * @return The codec type.
             */
            virtual Ogre::String getType() const;

            /**
             * Encodes data.
             *
             * Unimplemented, not required.
             *
             * @param input[in] Data to encode.
             * @param output[out] Encoded data.
             */
            virtual Ogre::DataStreamPtr encode(
              Ogre::MemoryDataStreamPtr& input, CodecDataPtr& output
            ) const final;

            /**
             * Encodes data to a file.
             *
             * Unimplemented, not required.
             *
             * @param input[in] Data to encode.
             * @param output[in] Nome for the file with the encoded data.
             * @param extra[in] Extra information for the encoder.
             */
            virtual void encodeToFile(
              Ogre::MemoryDataStreamPtr& input, const Ogre::String& output,
              CodecDataPtr& extra
            ) const final;

            /**
             * Decodes data.
             *
             * @param input[in] Data to encode.
             * @return Decoded data.
             */
            Ogre::Codec::DecodeResult decode(Ogre::DataStreamPtr& input) const;

            /**
             * Maps a magic number header to a file extension.
             *
             * @param magic_number[in] Pointer to a stream of bytes which
             * should identify the file. Note that this may be more than
             * needed - each codec may be looking for a different size magic
             * number.
             * @param max_bytes[in] The number of bytes passed.
             * @return A blank string if the magic number was unknown, or a
             * file extension.
             */
            virtual Ogre::String magicNumberToFileExt(
              const char *magic_number, size_t max_bytes
            ) const;

            /**
             * The codec type name.
             */
            static Ogre::String TYPE_NAME;

            /**
             * Perform the plugin initial installation sequence.
             *
             * An implementation must be supplied for this method. It must
             * perform the startup tasks necessary to install any rende rsystem
             * customizations or anything else that is not dependent on system
             * initialization, ie only dependent on the core of Ogre. It must
             * not perform any operations that would create
             * rendersystem-specific objects at this stage, that should be done
             * in {@see initialise()}.
             */
            static void install();

            /**
             * Perform any tasks the plugin needs to perform initialization.
             *
             * An implementation must be supplied for this method. It is called
             * just after the system is fully initialised (either after
             * Root::initialise if a window is created then, or after the first
             * window is created) and therefore all rendersystem functionality
             * is available at this time. You can use this hook to create any
             * resources which are dependent on a rendersystem or have
             * rendersystem-specific implementations.
             */
            static void initialise();

            /**
             * Perform any tasks the needed when the system is shut down.
             *
             * An implementation must be supplied for this method. This method
             * is called just before key parts of the system are unloaded, such
             * as rendersystems being shut down. This hook should be used to
             * free up resources and decouple custom objects from the OGRE
             * system, whilst all the instances of other plugins (e.g.
             * rendersystems) still exist.
             */
            static void shutdown();

            /**
             * Perform the final plugin uninstallation sequence.
             *
             * An implementation must be supplied for this method. It must
             * perform the cleanup tasks which haven't already been performed
             * in shutdown() (e.g. final deletion of custom instances,if they
             * are kept around in case the system was reinitialised). At this
             * stage it can't be guaranteed what other plugins are still loaded
             * or active. It must therefore not perform any operations that
             * would reference any rendersystem-specific objects - those should
             * have been sorted out in the 'shutdown' method.
             */
            static void uninstall();

          private:

            /**
             * The text codec.
             */
            static TexCodec *tex_codec_;
    };
}
