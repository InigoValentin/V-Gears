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

#include "core/AudioManager.h"
#include "core/EntityManager.h"
#include "core/Logger.h"
#include "core/ScriptManager.h"
#include "core/WorldMapManager.h"
#include "core/XmlBackground2DFile.h"
#include "core/XmlWorldMapFile.h"
#include "map/VGearsBackground2DFileManager.h"
#include "map/VGearsWalkmeshFileManager.h"
#include "TextHandler.h"

XmlWorldMapFile::XmlWorldMapFile(const Ogre::String& file): XmlFile(file){}

XmlWorldMapFile::~XmlWorldMapFile(){}

void XmlWorldMapFile::LoadWorldMap(const unsigned int current_progress){
    TiXmlNode* node = file_.RootElement();
    if (node == nullptr || node->ValueStr() != "world_map"){
        LOG_ERROR(file_.ValueStr() + " is not a valid fields map file! No <world_map> in root.");
        return;
    }
    node = node->FirstChild();
    while (node != nullptr){
        if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "walkmesh"){
            Ogre::String name(GetString(node, "file_name"));
            if (!name.empty()){
                VGears::WalkmeshFileManager::getSingleton();
                VGears::WalkmeshFilePtr walkmesh = VGears::WalkmeshFileManager::getSingleton()
                  .load(name, "FIELDS").staticCast<VGears::WalkmeshFile>();
                EntityManager::getSingleton().GetWalkmesh()->load(walkmesh);
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "background2d"){
            Ogre::String name = GetString(node, "file_name");
            if (name != ""){
                // TODO Migrate this code to a Resource and use it's group to load the background.
                VGears::Background2DFilePtr background
                  = VGears::Background2DFileManager::getSingleton()
                    .load(name, "FIELDS").staticCast<VGears::Background2DFile>();
                EntityManager::getSingleton().GetBackground2D()->load(background);
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "texts"){
            TextHandler::getSingleton().LoadFieldText(GetString(node, "file_name"));
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "terrain"){
            TiXmlNode* terrain_node = node->FirstChild();
            while (terrain_node != nullptr){
                if (terrain_node->ValueStr() == "block"){
                    int index(GetInt(terrain_node, "index"));
                    Ogre::String file_name(GetString(terrain_node, "file_name"));
                    Ogre::Vector3 position(GetVector3(node, "position"));
                    Ogre::String alt_file_name(GetString(terrain_node, "alt_file_name", ""));
                    int alt_story_change(GetInt(terrain_node, "alt_story_change", -1));
                    // TODO: Get current story point and determine whether to load alt models.
                    if ("" != alt_file_name && alt_story_change > current_progress)
                        WorldMapManager::getSingleton().AddTerrain(index, alt_file_name, position);
                    else WorldMapManager::getSingleton().AddTerrain(index, file_name, position);


                }
                terrain_node = terrain_node->NextSibling();
            }
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "entity_model"){
            Ogre::String name = GetString(node, "name");
            if (name == ""){
                LOG_ERROR("There is no name specified for <entity_model> tag.");
                continue;
            }
            Ogre::String file_name = GetString(node, "file_name");
            if (file_name == ""){
                LOG_ERROR("There is no file_name specified for <entity_model> tag.");
                continue;
            }
            Ogre::Vector3 position(GetVector3(node, "position"));
            Ogre::Degree direction(Ogre::Degree(GetFloat(node, "direction")));
            Ogre::Vector3 scale(GetVector3(node, "scale", Ogre::Vector3::UNIT_SCALE));
            Ogre::Quaternion orientation(GetQuaternion(node, "root_orientation"));
            int index(GetInt(node, "index"));
            EntityManager::getSingleton().AddEntity(
              name, file_name, position, direction, scale, orientation, index
            );
        }
        else if (
          node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "entity_trigger"
        ){
            Ogre::String name = GetString(node, "name");
            if (name == ""){
                LOG_ERROR("There is no name specified for <entity_trigger> tag.");
                continue;
            }
            Ogre::Vector3 point1 = GetVector3(node, "point1", Ogre::Vector3::ZERO);
            Ogre::Vector3 point2 = GetVector3(node, "point2", Ogre::Vector3::ZERO);
            bool enabled = GetBool(node, "enabled", false);
            EntityManager::getSingleton().AddEntityTrigger(name, point1, point2, enabled);
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "entity_point"){
            Ogre::String name = GetString(node, "name");
            if (name == ""){
                LOG_ERROR("There is no name specified for <entity_point> tag.");
                continue;
            }
            Ogre::Vector3 position = GetVector3(node, "position");
            float rotation = GetFloat(node, "rotation");
            EntityManager::getSingleton().AddEntityPoint(name, position, rotation);
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "entity_script"){
            Ogre::String name = GetString(node, "name");
            if (name == ""){
                LOG_ERROR("There is no name specified for <entity_script> tag.");
                continue;
            }
            EntityManager::getSingleton().AddEntityScript(name);
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "script"){
            Ogre::String file_name = GetString(node, "file_name");
            if (file_name != "") ScriptManager::getSingleton().RunFile("fields/" + file_name);
        }
        else if (node->Type() == TiXmlNode::TINYXML_ELEMENT && node->ValueStr() == "tracks"){
            for (TiXmlNode* track = node->FirstChild(); track; track = track->NextSibling()){
                int id = GetInt(track, "id");
                int track_id = GetInt(track, "track_id");
                AudioManager::getSingleton().AddTrack(id, track_id);
            }
        }
        node = node->NextSibling();
    }
}
