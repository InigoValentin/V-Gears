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

#include "common/TypeDefine.h"
#include "common/VGearsStringUtil.h"
#include "core/XmlFile.h"

// TODO: Move implementations to cpp file.

namespace VGears{

    namespace FF7{

        /**
         * Handles Final Fantasy VII Metadata.
         */
        class FF7Metadata : public XmlFile{

            public:

                /**
                 * Constructor.
                 *
                 * @param file[in] The XML file.
                 */
                FF7Metadata(Ogre::String file) : XmlFile(file){
                    TiXmlNode* node = file_.RootElement();
                    if (node == nullptr || node->ValueStr() != "metadata"){
                        throw std::runtime_error(
                          "FF7Metadata: " + file_.ValueStr()
                          + " is not a valid metadata file! "
                          + "No <metadata> in root."
                        );
                    }

                    node = node->FirstChild();
                    while (node){
                        if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "models"
                        ){
                            ReadModels(node->FirstChild());
                        }
                        else if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "animations"
                        ){
                            ReadAnimations(node->FirstChild());
                        }
                        else if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "scripts"
                        ){
                            ReadScripts(node->FirstChild());
                        }
                        node = node->NextSibling();
                    }
                }

                /**
                 * Matches animation names.
                 *
                 * Looks for defined names associated to the original game
                 * animation names. Defined names are more user friendly, more
                 * readable and more descriptive.
                 *
                 * @param key[in] Original game animation name.
                 * @return Friendly name associated to the animation, or the
                 * original name if there is no name associated.
                 */
                const String& Animation(const String &key) const{
                    auto it = animations_.find(key);
                    if (it != std::end(animations_)) return it->second;
                    String base_name;
                    StringUtil::splitBase(key, base_name);
                    it = animations_.find(base_name);
                    if (it != std::end(animations_)) return it->second;
                    return key;
                }

                /**
                 * Matches model names.
                 *
                 * Looks for defined names associated to the original game
                 * model names. Defined names are more user friendly, more
                 * readable and more descriptive.
                 *
                 * @param key[in] Original game model name.
                 * @return Friendly name associated to the model, or the
                 * original name if there is no name associated.
                 */
                const String& Model(const String &key) const{
                    auto it = models_.find(key);
                    if (it != std::end(models_)) return it->second;
                    String base_name;
                    StringUtil::splitBase(key, base_name);
                    it = models_.find(base_name);
                    if (it != std::end(models_)) return it->second;
                    return key;
                }

            private:

                /**
                 * Reads all models from an XML node.
                 *
                 * @param node[in] XML node to read.
                 */
                void ReadModels(TiXmlNode* node){
                    while (node){
                        const auto src = GetString(node, "name");
                        const auto dst = GetString(node, "target");
                        models_[src] = dst;
                        node = node->NextSibling();
                    }
                }

                /**
                 * Reads all models from an XML node.
                 *
                 * @param node[in] XML node to read.
                 */
                void ReadAnimations(TiXmlNode* node){
                    while (node){
                        const auto src = GetString(node, "name");
                        const auto dst = GetString(node, "target");
                        animations_[src] = dst;
                        node = node->NextSibling();
                    }
                }


                /**
                 * Reads all scripts from an XML node.
                 *
                 * @param node[in] XML node to read.
                 */
                void ReadScripts(TiXmlNode* node){
                    while (node){
                        if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "CharacterIds"
                        ){ReadCharacterIds(node->FirstChild());}
                        else if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "var_names"
                        ){ReadVarNames(node->FirstChild());}
                        else if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "entity_names"
                        ){ReadEntityNames(node->FirstChild());}
                        else if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "field"
                        ){
                            const auto name = GetString(node, "name");
                            ReadField(node->FirstChild(), name);
                        }
                        node = node->NextSibling();
                    }
                }

                /**
                 * Reads field information from an XML node.
                 *
                 * @param node[in] XML node to read.
                 */
                void ReadField(TiXmlNode* node, const std::string& name){
                    while (node){
                        if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "function"
                        ){ReadFunction(node, name);}
                        else if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "entity"
                        ){
                            const auto old_name = GetString(node, "name");
                            const auto new_name = GetString(node, "new");
                            field_data_[name].entity_name_map[old_name]
                              = new_name;
                        }
                        node = node->NextSibling();
                    }
                }

                /**
                 * Reads a function of a field from an XML node.
                 *
                 * @param node[in] XML node to read.
                 */
                void ReadFunction(
                  TiXmlNode* node, const std::string& field_name
                ){
                    const auto entity_name = GetString(node, "entity_name");
                    const auto old_name = GetString(node, "name");
                    const auto new_name = GetString(node, "new");
                    const auto comment = GetString(node, "Comment");
                    field_data_[field_name].function_map[entity_name][old_name]
                      = std::make_pair(new_name, comment);
                }

                /**
                 * Reads all entity from an XML node.
                 *
                 * @param node[in] XML node to read.
                 */
                void ReadEntityNames(TiXmlNode* node){
                    while (node){
                        if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "entity"
                        ){
                            const auto old_name = GetString(node, "name");
                            const auto new_name = GetString(node, "new");
                            entity_name_map_[old_name] = new_name;
                        }
                        node = node->NextSibling();
                    }
                }

                /**
                 * Reads all variable names from an XML node.
                 *
                 * @param node[in] XML node to read.
                 */
                void ReadVarNames(TiXmlNode* node){
                    while (node){
                        if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "var"
                        ){
                            const auto bank = GetInt(node, "Bank");
                            const auto address = GetInt(node, "Address");
                            const auto name = GetString(node, "Name");
                            var_map_[bank][address] = name;
                        }
                        node = node->NextSibling();
                    }
                }

                /**
                 * Reads all character identifiers from an XML node.
                 *
                 * @param node[in] XML node to read.
                 */
                void ReadCharacterIds(TiXmlNode* node){
                    while (node){
                        if (
                          node->Type() == TiXmlNode::TINYXML_ELEMENT
                          && node->ValueStr() == "char"
                        ){
                            const auto id = GetInt(node, "Id");
                            const auto name = GetString(node, "Name");
                            character_ids_[id] = name;
                            node = node->NextSibling();
                        }
                        node = node->NextSibling();
                    }
                }

                /**
                 * Retrieves a field script function data.
                 *
                 * @param field_name[in] The name of the field from witch to
                 * get the data.
                 * @param entity_name[in] Name of the entity rom witch to
                 * get the data.
                 * @param old_function_name[in] The original function name.
                 * @return Associated function name pair. An empty pair if the
                 * field, entity or function don't exist.
                 * @todo Verify documentation for old_function name and return
                 * value.
                 */
                std::pair<String,String> FieldScriptFunctionData(
                  const String& field_name, const String& entity_name,
                  const String& old_function_name
                ){
                    // Find a collection of field data for this field.
                    auto field_iterator = field_data_.find(field_name);
                    if (field_iterator != std::end(field_data_)){
                        // See if there is any info for this entity.
                        auto entity_iterator
                          = field_iterator->second.function_map.find(
                            entity_name
                          );
                        if (
                          entity_iterator
                          != std::end(field_iterator->second.function_map)
                        ){
                            // See if there is any info for this function in
                            // this entity.
                            auto function_iterator
                              = entity_iterator->second.find(old_function_name);
                            if (
                              function_iterator
                              != std::end(entity_iterator->second)
                            ){
                                return function_iterator->second;
                            }
                        }
                    }
                    return std::make_pair("", "");
                }


                typedef std::map<String, String> LookupMap;

                /**
                 * The list of models.
                 */
                LookupMap models_;

                /**
                 * The list of animations.
                 */
                LookupMap animations_;

                /**
                 * Character identifier map.
                 */
                std::map<int, String> character_ids_;

                /**
                 * Variable map.
                 */
                std::map<int, std::map<int, String>> var_map_;

                /**
                 * Entity name map.
                 */
                LookupMap entity_name_map_;

                /**
                 * A field metadata representation.
                 */
                struct FieldMetaData{

                    /**
                     * Function map.
                     *
                     * Entity name to old function name finds new function name
                     * and comment.
                     */
                    std::map<String, std::map<String, std::pair<String,String>>>
                      function_map;

                    /**
                     * Entity name map.
                     *
                     * Old name finds new name
                     */
                    LookupMap entity_name_map;
                };
                std::map<String, FieldMetaData> field_data_;

                friend class NameLookup;

        };

        /**
         * Name lookup convenience class.
         */
        class NameLookup{

            public:

                /**
                 * Constructor, not to use.
                 */
                NameLookup() = delete;

                /**
                 * Retrieves a character name from an ID.
                 */
                static String CharName(int char_id){
                    auto it = Data().character_ids_.find(char_id);
                    if (it != std::end(Data().character_ids_))
                        return it->second;
                    // Empty becomes lua "nil" value for this special case:
                    if (char_id == 254) return "";
                    return std::to_string(char_id);
                }

                /**
                 * Matches animation names.
                 *
                 * Looks for defined names associated to the original game
                 * animation names. Defined names are more user friendly, more
                 * readable and more descriptive.
                 *
                 * @param key[in] Original game animation name.
                 * @return Friendly name associated to the animation, or the
                 * original name if there is no name associated.
                 */
                static const String& Animation(const String &key){
                    return Data().Animation(key);
                }

                /**
                 * Matches model names.
                 *
                 * Looks for defined names associated to the original game
                 * model names. Defined names are more user friendly, more
                 * readable and more descriptive.
                 *
                 * @param key[in] Original game model name.
                 * @return Friendly name associated to the model, or the
                 * original name if there is no name associated.
                 */
                static const String& model(const String &key){
                    return Data().Model(key);
                }

                /**
                 * Retrieves a variable name.
                 *
                 * Variables that are frequently used across the game may
                 * have an associated name, more fiendly and descriptive.
                 *
                 * @param bank[in] Variable bank.
                 * @param addr[in] Variable address.
                 * @return Friendly name associated to the variable, or an
                 * empty string if it has no associated friendly name.
                 */
                static String FieldScriptVarName(int bank, int addr){
                    auto bank_iterator = Data().var_map_.find(bank);
                    if (bank_iterator != std::end(Data().var_map_)){
                        auto address_iterator
                          = bank_iterator->second.find(addr);
                        if (
                          address_iterator != std::end(bank_iterator->second)
                        ){
                            if (address_iterator->second.empty() == false)
                                return address_iterator->second;
                        }
                    }
                    return "";
                }

                /**
                 * Matches entity names.
                 *
                 * Looks for defined names associated to the original game
                 * entity name. Defined names are more user friendly, more
                 * readable and more descriptive.
                 *
                 * @param old_entity_name[in] Original game entity name.
                 * @return Friendly name associated to the entity, or the
                 * original name if there is no name associated.
                 */
                static String FieldScriptEntityName(
                  const String& old_entity_name
                ){
                    auto it = Data().entity_name_map_.find(old_entity_name);
                    if (it != std::end(Data().entity_name_map_))
                        return it->second;
                    return old_entity_name;
                }

                /**
                 * Matches function comments.
                 *
                 * @param field_name[in] The name of the field from witch to
                 * get the data.
                 * @param entity_name[in] Name of the entity rom witch to
                 * get the data.
                 * @param old_function_name[in] The original function name.
                 * @return Associated function name pair. An empty pair if the
                 * field, entity or function don't exist.
                 * @todo Does this maps comments, or functions with comments?
                 */
                static String FieldScriptFunctionComment(
                  const String& field_name, const String& entity_name,
                  const String& old_function_name){
                    return Data().FieldScriptFunctionData(
                      field_name, entity_name, old_function_name
                    ).second;
                }

                /**
                 * Matches function names.
                 *
                 * Looks for defined names associated to the original game
                 * function name. Defined names are more user friendly, more
                 * readable and more descriptive.
                 *
                 * @param field_name[in] The name of the field from witch to
                 * get the data.
                 * @param entity_name[in] Name of the entity rom witch to
                 * get the data.
                 * @param old_function_name[in] The original function name.
                 * @return Associated function name, or the original name if
                 * there is no name associated.
                 */
                static String FieldScriptFunctionName(
                  const String& field_name, const String& entity_name,
                  const String& old_function_name
                ){
                    auto name = Data().FieldScriptFunctionData(
                      field_name, entity_name, old_function_name
                    ).first;
                    if (name.empty()) return old_function_name;
                    return name;
                }

                /**
                 * Retrieves game metadata.
                 *
                 * @return The game metadata.
                 */
                static FF7Metadata& Data(){
                    static FF7Metadata data(
                      "field_models_and_animation_metadata.xml"
                    );
                    return data;
                }
        };
    }
}
