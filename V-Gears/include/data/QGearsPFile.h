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

#include <OgreAxisAlignedBox.h>
#include <OgreColourValue.h>
#include <OgreMesh.h>
#include <OgreResource.h>
#include <Ogre.h>
#include "common/TypeDefine.h"
#include "common/QGearsManualObject.h"
#include "data/QGearsRSDFile.h"

namespace QGears{

    /**
     * Handles P files.
     *
     * P files are binary files containing data which form 3D model. The files
     * specify model's vertices, polygons, colors, texture coordinates and
     * model sub-groups. The files do not specify references to the texture
     * files, animations, model skeleton or anything else. P-files are used as
     * parts of field models, battle models, battle locations on PC version of
     * FF7.
     */
    class PFile : public Ogre::Resource{

        public:

            /**
             * Constructor.
             *
             * @param creator[in] Pointer to the ResourceManager that is
             * creating this resource.
             * @param name[in] The unique name of the resource.
             * @param handle[in] @todo Understand and document.
             * @param group[in] The name of the resource group to which this
             * resource belong.
             * @param is_manual[in] True if the resource is manually loaded,
             * false otherwise.
             * @param loader[in] Pointer to a ManualResourceLoader
             * implementation which will be called when the Resource wishes to
             * load (should be supplied if is_manual is set to true). It can be
             * null, but the Resource will never be able to reload if anything
             * ever causes it to unload. Therefore provision of a proper
             * ManualResourceLoader instance is strongly recommended.
             */
            PFile(
              Ogre::ResourceManager* creator, const String &name,
              Ogre::ResourceHandle handle, const String& group,
              bool is_manual = false,
              Ogre::ManualResourceLoader* loader = nullptr
            );

            /**
             * Destructor.
             */
            virtual ~PFile();

            /**
             * Indicates if the file is a valid P file.
             *
             * @return True if the file is a valid P file, false otherwise.
             */
            virtual bool IsValid();

            /**
             * Indicates if the poligons definitions are valid.
             *
             * @return True if all the poligon definitions are valid, false
             * otherwise.
             */
            virtual bool IsPolygonDefinitionListValid();

            /**
             * Adds a resource group.
             *
             * A resource group includes a bone, a mesh and a graphical
             * resource.
             *
             * @param mesh[in] The mesh to add to the group.
             * @param bone_name[in] The bone in the skeleton to which to add
             * the group. The bone must be in a skeleton assigned to MESH.
             * @param rsd[in] File with the resource to add to the group.
             */
            virtual void AddGroups(
              Ogre::Mesh *mesh, const String &bone_name, const RSDFilePtr &rsd
            ) const;

            /**
             * An edge.
             */
            struct Edge{

                /**
                 * Each of the edge vertices.
                 */
                uint16 index[2];
            };

            /**
             * A polygon definition in the P file.
             */
            struct PolygonDefinition{

                /**
                 * Unknown data.
                 */
                uint16 unknown_00;

                /**
                 * Polygon vertices (3, always a triangle).
                 */
                uint16 vertex[3];

                /**
                 * The polygon normal (three components).
                 */
                uint16 normal[3];

                /**
                 * The edges of the polygon (3, always a triangle).
                 */
                uint16 edge[3];

                /**
                 * Unknown data.
                 */
                uint16 unknown_14;

                /**
                 * Unknown data.
                 */
                uint16 unknown_16;
            };

            /**
             * Material information in a P file.
             */
            struct MaterialInformation{

                /**
                 * Unknown data.
                 */
                uint8 unknown[100];
            };

            /**
             * Group data in a P file.
             */
            struct Group{

                /**
                 * The primitive type of the group.
                 *
                 * @todo Understand and document.
                 */
                uint32 primitive_type;

                /**
                 * @todo Understand and document.
                 */
                uint32 polygon_start_index;

                /**
                 * The number of polygons in the group.
                 */
                uint32 num_polygons;

                /**
                 * @todo Understand and document.
                 */
                uint32 vertex_start_index;

                /**
                 * The number of vertices in the group.
                 */
                uint32 nuvertices_;

                /**
                 * @todo Understand and document.
                 */
                uint32 edge_start_index;

                /**
                 * The number of edges in the group.
                 */
                uint32 nuedges_;

                /**
                 * Unknown data.
                 */
                uint32 unknown_1C;

                /**
                 * Unknown data.
                 */
                uint32 unknown_20;

                /**
                 * Unknown data.
                 */
                uint32 unknown_24;

                /**
                 * Unknown data.
                 */
                uint32 unknown_28;

                /**
                 * @todo Understand and document.
                 */
                uint32 texture_coordinate_start_index;

                /**
                 * Indicates if the group has an assigned texture.
                 */
                uint32 has_texture;

                /**
                 * Index of the assigned texture (if any).
                 */
                uint32 texture_index;
            };

            /**
             * A bounding box entry in a P file.
             */
            struct BBoxEntry{

                /**
                 * Unknown data.
                 */
                uint32 unknown;

                /**
                 * The bounding box.
                 */
                Ogre::AxisAlignedBox bbox;
            };

            typedef Ogre::ColourValue Colour;

            typedef std::vector<Ogre::Vector3> VertexList;

            typedef std::vector<Ogre::Vector3> NormalList;

            typedef std::vector<Ogre::Vector3> Unkown1List;

            typedef std::vector<Ogre::Vector2> TextureCoordinateList;

            typedef std::vector<Colour> VertexColorList;

            typedef std::vector<Colour> PolygonColorList;

            typedef std::vector<Edge> EdgeList;

            typedef std::vector<PolygonDefinition> PolygonDefinitionList;

            typedef std::vector<Group> GroupList;

            typedef std::vector<BBoxEntry> BBoxList;

            /**
             * Retrieves the vertices in the file.
             *
             * @return The vertices in the file.
             */
            virtual VertexList& GetVertices(){return vertices_;}

            /**
             * Retrieves the normals in the file.
             *
             * @return The normals in the file.
             */
            virtual NormalList& GetNormals(){return normals_;}

            /**
             * Retrieves unknown data from the file.
             *
             * @return Unknown data.
             */
            virtual Unkown1List& GetUnknown1(){return unknown_1_;}

            /**
             * Retrieves the texture coordinates in the file.
             *
             * @return The texture coordinates in the file.
             */
            virtual TextureCoordinateList& GetTextureCoordinates(){
                return texture_coordinates_;
            }

            /**
             * Retrieves the vertex colurs in the file.
             *
             * @return The vertex colurs in the file.
             */
            virtual VertexColorList& GetVertexColors(){return vertex_colours_;}

            /**
             * Retrieves the polygon colours in the file.
             *
             * @return The polygon colours in the file.
             */
            virtual PolygonColorList& GetPolygonColors(){
                return polygon_colours_;
            }

            /**
             * Retrieves the edges in the file.
             *
             * @return The edgesvertices in the file.
             */
            virtual EdgeList& GetEdges(){return edges_;}

            /**
             * Retrieves the polygon definitions in the file.
             *
             * @return The polygon definitions in the file.
             */
            virtual PolygonDefinitionList& GetPolygonDefinitions(){
                return polygon_definitions_;
            }

            /**
             * Retrieves the groups in the file.
             *
             * A group includes a bone, a mesh and a graphical resource.
             *
             * @return The groups in the file.
             */
            virtual GroupList& GetGroups(){return groups_;}

            /**
             * Retrieves the bounding boxes in the file.
             *
             * @return The vertices in the file.
             */
            virtual BBoxList& GetBBoxes(){return bounding_boxes_;}

            static const String RESOURCE_TYPE;

        protected:
            /**
             * Loads the file.
             */
            virtual void loadImpl() override;

            /**
             * Unloads the file.
             */
            virtual void unloadImpl() override;

            /**
             * Calculates the size of the palette.
             *
             * @return The size of the palette.
             * @todo Units?
             */
            virtual size_t calculateSize() const override;

            /**
             * Add a group to the file.
             *
             * A resource group includes a bone, a mesh and a graphical
             * resource.
             *
             * @param group[in] The group to add to the file.
             * @param mo[in|out] The object to add to the group.
             * @param sub_name[in] Name for mo.
             * @param material_base_name[in] Name of the material for mo.
             * @param bone[in] The bone to assign to bo.
             */
            virtual void AddGroup(
              const Group &group, ManualObject &mo, const String &sub_name,
              const String &material_base_name, const Ogre::Bone *bone
            ) const;

            /**
             * Retrieves the position of a bone.
             *
             * @param bone[in] The bone.
             * @return The position of the bone.
             */
            virtual Ogre::Vector3 GetPosition(const Ogre::Bone *bone) const;

            /**
             * Static rotation.
             */
            static const Ogre::Quaternion STATIC_ROTATION;

            /**
             * Creates a static rotation.
             */
            static Ogre::Quaternion CreateStaticRotation();

        private:

            /**
             * The list of vertices.
             */
            VertexList vertices_;

            /**
             * The list of normals.
             */
            NormalList normals_;

            /**
             * List of unknown data.
             */
            Unkown1List unknown_1_;

            /**
             * The list of texture coordinates.
             */
            TextureCoordinateList texture_coordinates_;

            /**
             * The list of vertex colours.
             */
            VertexColorList vertex_colours_;

            /**
             * The list of polygon colours.
             */
            PolygonColorList polygon_colours_;

            /**
             * The list of edges.
             */
            EdgeList edges_;

            /**
             * The list of polygon definitions.
             */
            PolygonDefinitionList polygon_definitions_;

            /**
             * The list of groups.
             */
            GroupList groups_;

            /**
             * The list of bounding boxes.
             */
            BBoxList bounding_boxes_;
    };

    typedef Ogre::SharedPtr<PFile> PFilePtr;
}
