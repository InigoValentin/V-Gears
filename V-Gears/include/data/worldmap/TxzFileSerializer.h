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
 * but WITHOUT ANY WARRANTY; without even the implied warrantexture_y of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#pragma once

#include <array>
#include "common/TypeDefine.h"
#include "data/QGearsSerializer.h"

namespace QGears {

    class TxzFile;

    class TxzFileSerializer : public Serializer{

    public:

#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif


        struct WorldMapTexture{
            unsigned int clut_x:6;
            unsigned int clut_y:10;
            unsigned int texture_x:4;
            unsigned int texture_y:1;
            unsigned int abr:2;
            unsigned int tp:2;
            unsigned int reserved:7;
#ifdef _MSC_VER
        };
#pragma pack(pop)
#else
        } __attribute__((packed)); // Assume GCC
#endif

#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif

        /**
         * A play station pixel representation.
         */
        struct PsxPixel{

            /**
             * Red component.
             */
            uint16 r:5;

            /**
             * Green component.
             */
            uint16 g:5;

            /**
             * Blue component.
             */
            uint16 b:5;

            /**
             * Alpha component.
             */
            uint16 a:1;

#ifdef _MSC_VER
        };
#pragma pack(pop)
#else
        } __attribute__((packed)); // Assume GCC
#endif

#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif

        /**
         * Play station pixel representation (4 bits).
         */
        struct PsxPixel4Bit{

            /**
             * First pixel index.
             */
            uint8 index1:4;

            /**
             * Second pixel index.
             */
            uint8 index2:4;

#ifdef _MSC_VER
        };
#pragma pack(pop)
#else
        } __attribute__((packed)); // Assume GCC
#endif

#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif

        /**
         * RGBA colour representation.
         */
        struct Rgba{

            /**
             * Red component.
             */
            unsigned char r = 0;

            /**
             * Green component.
             */
            unsigned char g = 0;

            /**
             * Blue component.
             */
            unsigned char b = 0;

            /**
             * Alpha component.
             */
            unsigned char a = 0;
#ifdef _MSC_VER
        };
#pragma pack(pop)
#else
        } __attribute__((packed)); // Assume GCC
#endif

        /**
         * A video RAM block representation
         */
        struct VRamBlock{

            /**
             * Block size, in bytes.
             */
            uint32 block_size = 0;

            /**
             * X component of the top left cornet of the block, in pixels.
             */
            uint16 x = 0;

            /**
             * Y component of the top left cornet of the block, in pixels.
             */
            uint16 y = 0;

            /**
             * Width of the block, in pixels.
             */
            uint16 width = 0;

            /**
             * Heigh of the block, in pixels.
             */
            uint16 height = 0;

            /**
             * The list of pixels.
             *
             * It's size shoud be width * height * 2
             */
            std::vector<uint8> pixels;
        };

        /**
         * Constructor.
         */
        TxzFileSerializer();

        /**
         * Destructor.
         */
        virtual ~TxzFileSerializer();

        /**
         * Imports a TXZ file.
         */
        void Import(Ogre::DataStreamPtr& stream, TxzFile& dest);

        /**
         * Retrieves a texture color pixels from a texture ID.
         *
         * @param id[in] ID of the texture.
         * @return List of color pixels conforming the texture, or an empty
         * vector if the texture doesn't exist.
         */
        std::vector<std::vector<Rgba>> GetWorldMapTexture(uint32 id);

    private:

        /**
         * The list of video RAM blocks.
         */
        std::vector<VRamBlock> v_ram_blocks_;

        /**
         * Imports a texture from decompressed file contents.
         *
         * @param stream[in] Contents of the decompressed file.
         * @dest[out] The texture data will be loaded here.
         */
        void ImportDecompressed(Ogre::DataStream& stream, TxzFile& dest);

        /**
         * Partially imports a texture from decompressed file contents.
         *
         * @param stream[in] Contents of the decompressed file.
         * @dest[out] The texture data will be loaded here.
         * @param section_end[in] Bytes to import.
         * @todo Sure it is decompressed?
         */
        void ImportTextureSection(
          Ogre::DataStream& stream, TxzFile& dest, size_t section_end
        );

        /**
         * @todo Understand and document. Where is this implemented?
         *
         * @param pallete_table[in] @todo.
         *
         */
        void ExtractTexture(
          const std::array<WorldMapTexture,512>& pallete_table
        );

        /**
         * The list of textures.
         */
        std::map<uint32, std::vector<std::vector<Rgba>>> textures_;
    };

    static_assert(
      sizeof(TxzFileSerializer::WorldMapTexture) == 4,
      "WorldMapTexture is not packed"
    );

    static_assert(
      sizeof(TxzFileSerializer::PsxPixel) == 2, "PsxPixel is not packed"
    );

}
