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

#include <OgreHardwareVertexBuffer.h>
#include <OgreMaterial.h>
#include <OgreRenderOperation.h>
#include <OgreRenderSystem.h>
#include <OgreSceneManager.h>
#include "formats/field/Background2DFile.h"
#include "QuadWriter.h"

/**
 * Where the tiles of each layer are in a vertex buffer.
 *
 * It's only useful if the tiles of a layer are together, one layer after the other, which is how
 * the installer writes them.
 */
class BackgroundLayerRanges{

    public:

        /**
         * The number of layers of a background.
         */
        static const int LAYER_COUNT = 4;

        /**
         * Part of a vertex buffer that has the tiles of a layer.
         */
        struct Range{

            /** First vertex. */
            unsigned int first = 0;

            /** Number of vertices. */
            unsigned int count = 0;
        };

        /**
         * Constructor.
         */
        BackgroundLayerRanges(){Clear();}

        /**
         * Forgets all the tiles.
         */
        void Clear();

        /**
         * Records the vertices of a tile.
         *
         * @param[in] layer Layer of the tile. Out of range layers count as the nearest one.
         * @param[in] first First vertex of the tile.
         * @param[in] count Number of vertices of the tile.
         */
        void Add(const int layer, const unsigned int first, const unsigned int count);

        /**
         * Checks if the tiles of the layers are together.
         */
        bool AreContiguous() const{return contiguous_;}

        /**
         * Retrieves the vertices of a layer.
         *
         * @param[in] layer The layer, from 0 to LAYER_COUNT - 1.
         */
        const Range& Of(const int layer) const{return ranges_[layer];}

    private:

        /** The vertices of each layer. */
        Range ranges_[LAYER_COUNT];

        /** The last layer a tile has been added to. */
        int last_layer_;

        /** If the tiles of the layers are together. */
        bool contiguous_;
};

/**
 * The tiles of a field background that are blended in the same way: their vertex buffer and the
 * material they are drawn with.
 */
class BackgroundTileBuffer{

    public:

        /**
         * Constructor. Creates the material and the vertex buffer.
         *
         * @param[in] blending How the tiles are blended.
         * @param[in] material_name Name of the material.
         * @param[in] max_tiles Number of tiles that fit in the buffer.
         */
        BackgroundTileBuffer(
          const VGears::Blending blending, const Ogre::String& material_name,
          const unsigned int max_tiles
        );

        /**
         * Destructor.
         */
        ~BackgroundTileBuffer();

        /**
         * Removes all the tiles.
         */
        void Clear();

        /**
         * Sets the image of the tiles.
         *
         * @param[in] image Name of the image.
         */
        void SetImage(const Ogre::String& image);

        /**
         * Checks if no more tiles fit.
         */
        bool IsFull() const;

        /**
         * Retrieves the number of tiles that fit in the buffer.
         */
        unsigned int MaxTiles() const{return max_tiles_;}

        /**
         * Adds a tile. The buffer must not be full.
         *
         * @param[in] corners Corners of the tile.
         * @param[in] texture Part of the image of the tile.
         * @param[in] depth Depth of the tile.
         * @param[in] intensity Part of the colour of the image that is used.
         * @param[in] layer Layer of the tile.
         * @return The first vertex of the tile.
         */
        unsigned int Add(
          const QuadWriter::Corners& corners, const QuadWriter::Rectangle& texture,
          const float depth, const float intensity, const int layer
        );

        /**
         * Moves a tile.
         *
         * @param[in] first_vertex First vertex of the tile.
         * @param[in] corners The new corners.
         */
        void SetPositions(const unsigned int first_vertex, const QuadWriter::Corners& corners);

        /**
         * Changes the part of the image of a tile.
         *
         * @param[in] first_vertex First vertex of the tile.
         * @param[in] texture The new part of the image.
         */
        void SetTexture(const unsigned int first_vertex, const QuadWriter::Rectangle& texture);

        /**
         * Checks if there are no tiles.
         */
        bool IsEmpty() const;

        /**
         * Retrieves where the tiles of each layer are.
         */
        const BackgroundLayerRanges& Layers() const{return layers_;}

        /**
         * Draws all the tiles.
         */
        void Draw(Ogre::SceneManager& scene_manager, Ogre::RenderSystem& render_system);

        /**
         * Draws some of the tiles.
         *
         * @param[in] range The vertices to draw.
         */
        void Draw(
          Ogre::SceneManager& scene_manager, Ogre::RenderSystem& render_system,
          const BackgroundLayerRanges::Range& range
        );

    private:

        /**
         * Creates the vertex buffer.
         */
        void CreateBuffer();

        /**
         * Frees the vertex buffer.
         */
        void DestroyBuffer();

        /**
         * Retrieves the vertices of the buffer, which has to be unlocked afterwards.
         *
         * @param[in] first_vertex First vertex wanted.
         */
        float* Lock(const unsigned int first_vertex);

        /** Number of tiles that fit in the buffer. */
        unsigned int max_tiles_;

        /** What is drawn. */
        Ogre::RenderOperation operation_;

        /** The vertex buffer. */
        Ogre::HardwareVertexBufferSharedPtr buffer_;

        /** The material. */
        Ogre::MaterialPtr material_;

        /** Where the tiles of each layer are. */
        BackgroundLayerRanges layers_;
};
