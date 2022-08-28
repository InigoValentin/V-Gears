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

#include <cstring>
#include <OgreException.h>
#include <OgreHardwareBufferManager.h>
#include <OgreRoot.h>
#include "common/QGearsManualObject.h"

namespace QGears{
    ManualObject::ManualObject(Ogre::Mesh *mesh) :
      _mesh(mesh),
      _section(nullptr),
      _bbox(mesh->getBounds()),
      _radius(mesh->getBoundingSphereRadius()),
      _position(nullptr),
      _normal(nullptr),
      colour_(nullptr),
      _texture_coordinate(nullptr),
      _index(nullptr),
      colour_type_(Ogre::VertexElement::getBestColourVertexElementType())
    {}

    ManualObject::~ManualObject(){}

    template<typename BufferType>
    BufferType* ManualObject::createBuffer(
      const BufferBinding binding, Ogre::VertexElementType type,
      Ogre::VertexElementSemantic semantic
    ){
        Ogre::VertexDeclaration* decl =
          _section->vertexData->vertexDeclaration;
        Ogre::VertexBufferBinding* bind =
          _section->vertexData->vertexBufferBinding;
        decl->addElement(binding, 0, type, semantic);
        size_t vertex_size(decl->getVertexSize(binding));

        VertexBuffer buffer =
          Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
            vertex_size, _section->vertexData->vertexCount,
            Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY
          );
        bind->setBinding(binding, buffer);
        _vertex_buffers[binding] = buffer;
        return static_cast<BufferType*>(
          buffer->lock(Ogre::HardwareBuffer::HBL_DISCARD)
        );
    }

    void ManualObject::createPositionBuffer(){
        _position = createBuffer<Ogre::Vector3>(
          BB_POSITION, Ogre::VET_FLOAT3, Ogre::VES_POSITION
        );
    }

    void ManualObject::createNormalBuffer(){
        _normal = createBuffer<Ogre::Vector3>(
          BB_NORMAL, Ogre::VET_FLOAT3, Ogre::VES_NORMAL
        );
    }

    void ManualObject::createColourBuffer(){
        colour_ = createBuffer<uint32>(
          BB_COLOUR, colour_type_, Ogre::VES_DIFFUSE
        );
    }

    void ManualObject::createTextureCoordinateBuffer(){
        _texture_coordinate = createBuffer<Ogre::Vector2>(
          BB_TEXTURE, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES
        );
    }

    void ManualObject::createIndexBuffer(){
        _indexbuffer_ =
          Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
            Ogre::HardwareIndexBuffer::IT_16BIT,
            _section->indexData->indexCount,
            Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY
          );

        void *data(_indexbuffer_->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        _index = static_cast< uint16* >(data);
        _section->indexData->indexBuffer = _indexbuffer_;
    }

    template<typename BufferSharedPtr>
    void ManualObject::resetBuffer(BufferSharedPtr &buffer) const{
        if(buffer == nullptr) return;
        if(buffer->isLocked()) buffer->unlock();
        buffer.reset();
    }

    void ManualObject::resetBuffers(){
        for(int i(BINDING_COUNT); i--;) resetBuffer(_vertex_buffers[i]);
        resetBuffer(_indexbuffer_);
        _position = nullptr;
        _normal = nullptr;
        colour_ = nullptr;
        _texture_coordinate = nullptr;
        _index = nullptr;
    }

    void ManualObject::begin(
      const String &name, const String &material_name,
      size_t vertex_count, size_t index_count
    ){
        if (_mesh == nullptr){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
                "A section can't be started without a mesh.",
                "ManualObject::begin"
              );
        }

        if (_section != nullptr){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "A section can't be started while other one is not ended",
              "ManualObject::begin"
            );
        }

        if (vertex_count == 0){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "You cannot begin with a vertex count of 0",
              "ManualObject::begin"
            );
        }

        _section = _mesh->createSubMesh(name);
        _section->setMaterialName(material_name);
        _section->useSharedVertices = false;
        _section->operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
        _section->vertexData = new Ogre::VertexData();
        _section->vertexData->vertexStart = 0;
        _section->vertexData->vertexCount = vertex_count;
        _section->indexData = new Ogre::IndexData();
        _section->indexData->indexStart = 0;
        _section->indexData->indexCount = index_count;
    }

    void ManualObject::position(const Ogre::Vector3 &position){
        if (_position == nullptr) createPositionBuffer();
        *(_position ++) = position;
        _bbox.merge(position);
        _radius = std::max(_radius, position.length());
    }

    void ManualObject::normal(const Ogre::Vector3 &normal){
        if (_normal == nullptr) createNormalBuffer();
        *(_normal ++) = normal;
    }

    void ManualObject::colour(const Ogre::ColourValue &colour) {
        if(colour_ == nullptr) createColourBuffer();
        *(colour_ ++) =
           Ogre::VertexElement::convertColourValue(colour, colour_type_);
    }

    void ManualObject::textureCoord(const Ogre::Vector2 &texture_coordinate){
        if (_texture_coordinate == nullptr) createTextureCoordinateBuffer();
        *(_texture_coordinate ++) = texture_coordinate;
    }

    void ManualObject::index(const uint32 idx){
        if (_index == nullptr) createIndexBuffer();
        *(_index ++) = idx;
    }

    void ManualObject::bone(
      const uint32 idx, const uint16 bone_handle, const Ogre::Real weight
    ){
        Ogre::VertexBoneAssignment vba;
        vba.vertexIndex = idx;
        vba.boneIndex = bone_handle;
        vba.weight = weight;
        _section->addBoneAssignment(vba);
    }

    void ManualObject::end(){
        if (_section == nullptr){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "A section can't be ended without having been started.",
              "ManualObject::end"
            );
        }
        _mesh->_setBounds(_bbox);
        _mesh->_setBoundingSphereRadius(_radius);
        _section = nullptr;
        resetBuffers();
    }
}
