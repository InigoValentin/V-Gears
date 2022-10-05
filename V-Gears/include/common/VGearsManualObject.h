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

#include <OgreColourValue.h>
#include <OgreMesh.h>
#include <OgreSubMesh.h>
#include <Ogre.h>
#include "common/TypeDefine.h"

namespace VGears{

    /**
     * A manually handled object.
     *
     * When parsing files this is used to create the rendering engine object.
     * E.g. when a bone is found in the file being parsed, then bone() is
     * called to add it.
     */
    class ManualObject{

        public:

            /**
             * Constructor.
             *
             * @param[in] mesh The object mesh
             */
            explicit ManualObject(Ogre::Mesh *mesh);

            /**
             * Destructor.
             */
            virtual ~ManualObject();

            /**
             * Buffer indexes for object properties.
             */
            enum BufferBinding{

                /**
                 * Object position.
                 */
                BB_POSITION,

                /**
                 * Object normal.
                 */
                BB_NORMAL,

                /**
                 * Object colour.
                 */
                BB_COLOUR,

                /**
                 * Object texture.
                 */
                BB_TEXTURE,

                /**
                 * Number of binds for the object.
                 */
                BINDING_COUNT
            };

            /**
             * Begins creating a object section.
             *
             * It doesn't set position, normal or texture.
             *
             * @param[in] name Name for the object.
             * @param[in] material_name Name of the object's material.
             * @param[in] vertex_count Number of vertices in the object.
             * @param[in] index_count
             * @todo Undestand and document index_count.
             */
            virtual void begin(
              const String &name, const String &material_name,
              size_t vertex_count, size_t index_count
            );

            /**
             * Sets the object position.
             *
             * @param[in] position Position vector.
             * @todo It applies to the object or the current section?
             */
            virtual void position(const Ogre::Vector3 &position);

            /**
             * Sets the object normal vector.
             *
             * @param[in] normal Normal vector.
             * @todo It applies to the object or the current section?
             */
            virtual void normal(const Ogre::Vector3 &normal);

            /**
             * Sets the object colour.
             *
             * @param colour Object colour.
             * @todo It applies to the object or the current section?
             */
            virtual void colour(const Ogre::ColourValue &colour);

            /**
             * Sets the object texture coordinates.
             *
             * @param[in] texture_coordinate Coordinate vector.
             * @todo It applies to the object or the current section?
             */
            virtual void textureCoord(const Ogre::Vector2 &texture_coordinate);

            /**
             * Sets the object index.
             *
             * @param[in] idx Object index.
             * @todo It applies to the object or the current section?
             */
            virtual void index( const uint32 idx );

            /**
             * Adds a bone to the object.
             *
             * @param[in] idx Index of the vertex index to add the bone to.
             * @param[in] bone_handle Index of the bone objct.
             * @param[in] weight Bone weight. In most cases, 1 is ok.
             * @todo It applies to the object or the current section?
             */
            virtual void bone(
              const uint32 idx, const uint16 bone_handle,
              const Ogre::Real weight = 1
            );

            /**
             * End the current section.
             *
             * A section must have been started with {@see begin}
             *
             * @throws Ogre::Exception::ERR_INVALIDPARAMS if a section has not
             * been started.
             */
            virtual void end();

        protected:

            /**
             * Creates a buffer
             *
             * @param[in] binding Buffer binding.
             * @param[in] type Type Buffer type.
             * @param[in] semantic Buffer semantic.
             * @todo I'm no really sure what this function does...
             */
            template<typename BufferType>
            BufferType* createBuffer(
              const BufferBinding binding, Ogre::VertexElementType type,
              Ogre::VertexElementSemantic semantic
            );

            /**
             * Creates a position buffer in {@see BB_POSITION}.
             */
            virtual void createPositionBuffer();

            /**
             * Creates a normal buffer in {@see BB_NORMAL}.
             */
            virtual void createNormalBuffer();

            /**
             * Creates a colour buffer in {@see BB_COLOUR}.
             */
            virtual void createColourBuffer();

            /**
             * Creates a texture coordinate buffer in {@see BB_TEXTURE}.
             */
            virtual void createTextureCoordinateBuffer();

            /**
             * Creates a index buffer in {@see BB_INDEX}.
             */
            virtual void createIndexBuffer();

            /**
             * Resets all buffers in {@see BufferBinding}.
             */
            virtual void resetBuffers();

            /**
             * Resets any of the buffers in {@see BufferBinding}.
             *
             * @param buffer Buffer to reset.
             */
            template<typename BufferSharedPtr>
            void resetBuffer(BufferSharedPtr &buffer) const;

        private:

            typedef Ogre::HardwareVertexBufferSharedPtr VertexBuffer;
            typedef Ogre::HardwareIndexBufferSharedPtr IndexBuffer;

            /**
             * The object mesh.
             */
            Ogre::Mesh *_mesh;

            /**
             * The currently open section.
             */
            Ogre::SubMesh *_section;

            /**
             * @todo
             */
            Ogre::AxisAlignedBox _bbox;

            /**
             * @todo
             */
            Ogre::Real _radius;

            /**
             * The object vetex buffer.
             */
            VertexBuffer _vertex_buffers[BINDING_COUNT];

            /**
             * The index buffer.
             */
            IndexBuffer _indexbuffer_;

            /**
             * The position buffer.
             */
            Ogre::Vector3 *_position;

            /**
             * The normal buffer.
             */
            Ogre::Vector3 *_normal;

            /**
             * The colour buffer.
             */
            uint32 *colour_;

            /**
             * The texture buffer.
             */
            Ogre::Vector2 *_texture_coordinate;

            /**
             * The object index.
             */
            uint16 *_index;

            /**
             * The colour type.
             */
            const Ogre::VertexElementType colour_type_;
    };
}
