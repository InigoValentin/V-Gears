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

#include <OgreLogManager.h>
#include <OgreException.h>
#include "map/QGearsWalkmeshFileXMLSerializer.h"

namespace QGears{

    WalkmeshFileXMLSerializer::WalkmeshFileXMLSerializer() : XMLSerializer(){}
    
    WalkmeshFileXMLSerializer::~WalkmeshFileXMLSerializer(){}
    
    void WalkmeshFileXMLSerializer::ReadHeader(TiXmlNode *node){
        if(node == nullptr || node->ValueStr() != "walkmesh"){
            OGRE_EXCEPT(
              Ogre::Exception::ERR_INVALIDPARAMS,
              "not a valid walkmesh file, no <walkmesh> in root",
              "WalkmeshFileXMLSerializer::readHeader"
            );
        }
    }

    void WalkmeshFileXMLSerializer::ImportWalkmeshFile(
      Ogre::DataStreamPtr &stream, WalkmeshFile *dest
    ){
        TiXmlDocument document;
        Parse(stream, document);
        TiXmlNode *root_node(document.RootElement());
        ReadHeader(root_node);
        TiXmlNode &node(*root_node);
        ReadVector(node, dest->GetTriangles(), "triangle");
    }

    void WalkmeshFileXMLSerializer::readObject(
      TiXmlNode &node, WalkmeshFileXMLSerializer::Triangle &dest
    ){
        ReadAttribute(node, "a", dest.a);
        ReadAttribute(node, "b", dest.b);
        ReadAttribute(node, "c", dest.c);
        ReadAttribute(node, "a_b", dest.access_side[0]);
        ReadAttribute(node, "b_c", dest.access_side[1]);
        ReadAttribute(node, "c_a", dest.access_side[2]);
    }

}

