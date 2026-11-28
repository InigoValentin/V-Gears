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

#include <algorithm>
#include <memory>
#include <utility>
#include <OgreStringConverter.h>
#include "common/StringUtil.h"
#include "FieldMapWriter.h"

const float FieldMapWriter::LINE_SCALE_FACTOR(0.0078124970964f);

namespace{

    /**
     * Creates an element with a file name attribute.
     *
     * @param[in] name Name of the element.
     * @param[in] file_name The file name.
     * @return The element.
     */
    std::unique_ptr<TiXmlElement> CreateFileLink(const std::string& name, const std::string& file_name){
        std::unique_ptr<TiXmlElement> element(new TiXmlElement(name));
        element->SetAttribute("file_name", file_name);
        return element;
    }

    /**
     * Creates the element of a trigger line.
     *
     * @param[in] name Name of the entity of the trigger.
     * @param[in] point1 First point of the line.
     * @param[in] point2 Second point of the line.
     * @return The element.
     */
    std::unique_ptr<TiXmlElement> CreateTriggerLine(
      const std::string& name, const std::string& point1, const std::string& point2
    ){
        std::unique_ptr<TiXmlElement> element(new TiXmlElement("entity_trigger"));
        element->SetAttribute("name", name);
        element->SetAttribute("point1", point1);
        element->SetAttribute("point2", point2);
        element->SetAttribute("enabled", "true");
        return element;
    }

    /**
     * Converts a vertex of a trigger line to the units of the field.
     *
     * @param[in] vertex The vertex.
     * @param[in] downscaler What the coordinates are divided by.
     * @return The vertex, as text.
     */
    std::string TriggerPoint(const VGears::TriggersFile::TriggerVertex& vertex, const float downscaler){
        return Ogre::StringConverter::toString(Ogre::Vector3(vertex.x, vertex.y, vertex.z) / downscaler);
    }

    /**
     * Converts a point of a script line to the units of the field.
     *
     * @param[in] point The point.
     * @param[in] scale What the coordinates are multiplied by.
     * @return The point, as text.
     */
    template<typename Point> std::string LinePoint(const Point& point, const float scale){
        return std::to_string(point[0] * scale) + " " + std::to_string(point[1] * scale)
          + " " + std::to_string(point[2] * scale);
    }

    /**
     * Names the spawn point of an entrance to a field.
     *
     * @param[in] record The entrance.
     * @param[in] map_list The names of the fields, by ID.
     * @return The name of the spawn point.
     */
    std::string SpawnPointName(const SpawnPointDb::Record& record, const MapList& map_list){
        // Spawn points from map jumps have a another algorithm.
        if (record.from_script){
            return map_list.at(record.field_id) + "_" + record.entity_name + "_"
              + record.script_function_name + "_addr_"
              + std::to_string(record.gateway_index_or_map_jump_address);
        }
        // Must also include the gateway index for the case where 2 fields have more than one door
        // linking to each other.
        return "Spawn_" + map_list.at(record.field_id) + "_"
          + std::to_string(record.gateway_index_or_map_jump_address);
    }
}

FieldMapWriter::FieldMapWriter(
  const std::string& data_dir, const MapList& map_list, const FieldSpawnPointsMap& spawn_points,
  const FieldScaleFactorMap& scale_factors, ModelsAndAnimationsDb& used_models,
  std::function<void(std::string)> log_output_line
):
  data_dir_(data_dir), map_list_(map_list), spawn_points_(spawn_points),
  scale_factors_(scale_factors), used_models_(used_models), write_output_line_(log_output_line)
{}

void FieldMapWriter::WriteMap(
  const VGears::FLevelFilePtr& field, const FieldDecompiler::DecompiledScript& decompiled
){
    TiXmlDocument doc;
    std::unique_ptr<TiXmlElement> map(new TiXmlElement("map"));
    WriteFileLinks(field->getName(), *map);
    // Forward direction (angle player moves when "up" is pressed)
    std::unique_ptr<TiXmlElement> forward_direction(new TiXmlElement("movement_rotation"));
    forward_direction->SetAttribute("degree", std::to_string(field->GetTriggers()->MovementRotation()));
    map->LinkEndChild(forward_direction.release());
    WriteEncounters(field->GetRawEncounter(), *map);
    const Ogre::Vector3 first_entity_point = WriteEntityPoints(field, *map);
    WriteEntities(field, decompiled, first_entity_point, *map);
    WriteTriggerLines(field, *map);
    WriteTracks(field, *map);
    doc.LinkEndChild(map.release());
    doc.SaveFile(data_dir_ + "/" + FieldData::MAPS_DIR + "/" + field->getName() + "/map.xml");
}

void FieldMapWriter::WriteWalkmesh(const VGears::FLevelFilePtr& field){
    const VGears::WalkmeshFilePtr& walkmesh = field->GetWalkmesh();
    // Scripts, gateways and the camera use the field's scale, so the walkmesh must too.
    const float field_scale
      = FieldData::ScaleFactor(scale_factors_, FieldData::FieldId(field->getName(), map_list_));
    TiXmlDocument doc;
    std::unique_ptr<TiXmlElement> walkmesh_element(new TiXmlElement("walkmesh"));
    for (const VGears::WalkmeshFile::Triangle& triangle : walkmesh->GetTriangles()){
        std::unique_ptr<TiXmlElement> element(new TiXmlElement("triangle"));
        element->SetAttribute("a", Ogre::StringConverter::toString(triangle.a / field_scale));
        element->SetAttribute("b", Ogre::StringConverter::toString(triangle.b / field_scale));
        element->SetAttribute("c", Ogre::StringConverter::toString(triangle.c / field_scale));
        element->SetAttribute("a_b", std::to_string(triangle.access_side[0]));
        element->SetAttribute("b_c", std::to_string(triangle.access_side[1]));
        element->SetAttribute("c_a", std::to_string(triangle.access_side[2]));
        walkmesh_element->LinkEndChild(element.release());
    }
    doc.LinkEndChild(walkmesh_element.release());
    doc.SaveFile(data_dir_ + "/" + FieldData::MAPS_DIR + "/" + field->getName() + "/wm.xml");
}

std::vector<int> FieldMapWriter::ExtractMusicTrackIds(const std::vector<u8>& raw_script){
    std::vector<int> tracks;
    for (size_t i = 0; i + 4 < raw_script.size(); i ++){
        if (
          raw_script[i] == 'A' && raw_script[i + 1] == 'K'
          && raw_script[i + 2] == 'A' && raw_script[i + 3] == 'O'
        ){
            tracks.push_back(static_cast<int>(raw_script[i + 4]) - 2);
        }
    }
    return tracks;
}

void FieldMapWriter::WriteFileLinks(const std::string& field_name, TiXmlElement& map){
    map.LinkEndChild(CreateFileLink("script", field_name + "/script.lua").release());
    map.LinkEndChild(CreateFileLink("background2d", field_name + "/bg.xml").release());
    map.LinkEndChild(CreateFileLink("texts", field_name + "/text.xml").release());
    map.LinkEndChild(CreateFileLink("walkmesh", field_name + "/wm.xml").release());
}

void FieldMapWriter::WriteEncounters(const std::vector<u8>& raw_encounter, TiXmlElement& map){
    std::unique_ptr<TiXmlElement> encounters(new TiXmlElement("encounters"));
    bool any_table = false;
    for (int table = 0; table < 2 && raw_encounter.size() >= static_cast<size_t>(24 * (table + 1)); table ++){
        const u8* data = raw_encounter.data() + 24 * table;
        if (data[0] == 0) continue;
        std::unique_ptr<TiXmlElement> xml_table(new TiXmlElement("table"));
        xml_table->SetAttribute("id", table + 1);
        xml_table->SetAttribute("rate", static_cast<int>(data[1]));
        for (int e = 0; e < 10; e ++){
            const unsigned int value = data[2 + 2 * e] | (data[3 + 2 * e] << 8);
            const int battle = value & 0x3FF;
            const int probability = value >> 10;
            if (probability == 0) continue;
            std::unique_ptr<TiXmlElement> encounter(new TiXmlElement(e < 6 ? "battle" : "special"));
            encounter->SetAttribute("id", battle);
            encounter->SetAttribute("probability", probability);
            xml_table->LinkEndChild(encounter.release());
        }
        encounters->LinkEndChild(xml_table.release());
        any_table = true;
    }
    if (any_table) map.LinkEndChild(encounters.release());
}

Ogre::Vector3 FieldMapWriter::WriteEntityPoints(const VGears::FLevelFilePtr& field, TiXmlElement& map){
    const size_t field_id = FieldData::FieldId(field->getName(), map_list_);
    Ogre::Vector3 first_entity_point = Ogre::Vector3::ZERO;
    // The gateways and map jumps of the other fields that lead to this one. If there are none, no
    // other field has doors to this one.
    auto spawn_iterator = spawn_points_.find(static_cast<u16>(field_id));
    if (spawn_iterator == std::end(spawn_points_)) return first_entity_point;
    const float scale = FieldData::ScaleFactor(scale_factors_, field_id);
    const auto& triangles = field->GetWalkmesh()->GetTriangles();
    for (const SpawnPointDb::Record& record : spawn_iterator->second.gateways_to_this_field){
        const VGears::TriggersFile::Gateway& gateway = record.gateway;
        std::unique_ptr<TiXmlElement> entity_point(new TiXmlElement("entity_point"));
        entity_point->SetAttribute("name", SpawnPointName(record, map_list_));
        const float downscaler
          = 128.0f * FieldData::ScaleFactor(scale_factors_, gateway.destination_field_id);
        const float x = gateway.destination.x / downscaler;
        const float y = gateway.destination.y / downscaler;
        const unsigned int triangle = FindEntranceTriangle(field, record, x, y);
        const Ogre::Vector3 position(x, y, triangles.at(triangle).a.z / scale);
        if (position != Ogre::Vector3::ZERO && first_entity_point == Ogre::Vector3::ZERO){
            first_entity_point = position;
        }
        entity_point->SetAttribute("position", Ogre::StringConverter::toString(position));
        const float rotation = (360.0f * static_cast<float>(gateway.dir)) / 256.0f;
        entity_point->SetAttribute("rotation", std::to_string(rotation));
        map.LinkEndChild(entity_point.release());
    }
    return first_entity_point;
}

unsigned int FieldMapWriter::FindEntranceTriangle(
  const VGears::FLevelFilePtr& field, const SpawnPointDb::Record& record, const float x, const float y
){
    const VGears::TriggersFile::Gateway& gateway = record.gateway;
    const unsigned int triangle_index = static_cast<unsigned int>(gateway.destination.z);
    const auto& triangles = field->GetWalkmesh()->GetTriangles();
    if (triangle_index < triangles.size()) return triangle_index;
    const float scale
      = FieldData::ScaleFactor(scale_factors_, FieldData::FieldId(field->getName(), map_list_));
    const auto side = [x, y](const Ogre::Vector3& a, const Ogre::Vector3& b){
        return (x - b.x) * (a.y - b.y) - (a.x - b.x) * (y - b.y);
    };
    for (size_t t = 0; t < triangles.size(); t ++){
        const Ogre::Vector3 a = triangles[t].a / scale, b = triangles[t].b / scale;
        const Ogre::Vector3 c = triangles[t].c / scale;
        const float d1 = side(a, b), d2 = side(b, c), d3 = side(c, a);
        const bool negative = d1 < 0 || d2 < 0 || d3 < 0, positive = d1 > 0 || d2 > 0 || d3 > 0;
        if (!(negative && positive)) return static_cast<unsigned int>(t);
    }
    // Only reported when the position is not on the walkmesh either: those jumps are of the debug
    // rooms (blackbg1, blackbg4) or of scenes that place the player themselves.
    const std::string source = record.from_script
      ? "the script " + record.entity_name + ":" + record.script_function_name + " (address "
        + std::to_string(record.gateway_index_or_map_jump_address) + ")"
      : "the door " + std::to_string(record.gateway_index_or_map_jump_address);
    write_output_line_(
      "[WARNING] In field " + field->getName() + ": the entrance from "
      + map_list_.at(record.field_id) + " (" + source + ") is on the walkmesh triangle "
      + std::to_string(triangle_index) + ", which the walkmesh doesn't have (it has "
      + std::to_string(triangles.size()) + "), at (" + std::to_string(gateway.destination.x) + ", "
      + std::to_string(gateway.destination.y) + "), which is not on the walkmesh."
    );
    return 0;
}

void FieldMapWriter::WriteEntities(
  const VGears::FLevelFilePtr& field, const FieldDecompiler::DecompiledScript& decompiled,
  const Ogre::Vector3& first_entity_point, TiXmlElement& map
){
    const size_t field_id = FieldData::FieldId(field->getName(), map_list_);
    std::vector<std::pair<uint, std::unique_ptr<TiXmlElement>>> ordered_entities;
    // The lines. Their entities are not written again as scripts.
    std::vector<std::string> line_entities;
    const float line_scale = LINE_SCALE_FACTOR / FieldData::ScaleFactor(scale_factors_, field_id);
    for (const FieldDecompiler::Line& line : decompiled.lines){
        line_entities.push_back(line.name);
        ordered_entities.emplace_back(
          line.index,
          CreateTriggerLine(line.name, LinePoint(line.point_a, line_scale), LinePoint(line.point_b, line_scale))
        );
    }
    const VGears::ModelListFilePtr& models = field->GetModelList();
    for (const FieldDecompiler::FieldEntity& entity : decompiled.entities){
        if (std::find(line_entities.begin(), line_entities.end(), entity.name) != line_entities.end())
            continue;
        if (entity.char_id != -1){
            ordered_entities.emplace_back(
              entity.index,
              CreateModelEntity(entity, models->GetModels().at(entity.char_id), first_entity_point)
            );
        }
        else{
            std::unique_ptr<TiXmlElement> script(new TiXmlElement("entity_script"));
            script->SetAttribute("name", entity.name);
            ordered_entities.emplace_back(entity.index, std::move(script));
        }
    }
    std::stable_sort(
      ordered_entities.begin(), ordered_entities.end(),
      [](const auto& a, const auto& b){return a.first < b.first;}
    );
    for (auto& ordered : ordered_entities) map.LinkEndChild(ordered.second.release());
}

std::unique_ptr<TiXmlElement> FieldMapWriter::CreateModelEntity(
  const FieldDecompiler::FieldEntity& entity, const VGears::ModelListFile::ModelDescription& model,
  const Ogre::Vector3& first_entity_point
){
    auto& animations = used_models_.ModelAnimations(model.hrc_name);
    for (const auto& animation : model.animations){
        animations.insert(used_models_.NormalizeAnimationName(animation.name));
    }
    std::unique_ptr<TiXmlElement> element(new TiXmlElement("entity_model"));
    element->SetAttribute("name", entity.name);
    auto lower_case_hrc_name = model.hrc_name;
    VGears::StringUtil::toLowerCase(lower_case_hrc_name);
    element->SetAttribute("file_name", used_models_.ModelMetaDataName(lower_case_hrc_name));
    element->SetAttribute("index", entity.index);
    // The position of the player models is where the player ends up if the field is loaded
    // without an entrance: the first entity point.
    element->SetAttribute(
      "position",
      Ogre::StringConverter::toString(
        model.type == VGears::ModelListFile::PLAYER ? first_entity_point : Ogre::Vector3::ZERO
      )
    );
    element->SetAttribute("direction", "0");
    // TODO: This isn't quite right, the models animation translation seems to be inverted?
    element->SetAttribute("scale", "0.03125 0.03125 0.03125");
    element->SetAttribute("root_orientation", "0.7071067811865476 0.7071067811865476 0 0");
    return element;
}

void FieldMapWriter::WriteTriggerLines(const VGears::FLevelFilePtr& field, TiXmlElement& map){
    const size_t field_id = FieldData::FieldId(field->getName(), map_list_);
    const float downscaler = 128.0f * FieldData::ScaleFactor(scale_factors_, field_id);
    const VGears::TriggersFilePtr& triggers = field->GetTriggers();
    const auto& gateways = triggers->GetGateways();
    for (size_t i = 0; i < gateways.size(); i ++){
        const VGears::TriggersFile::Gateway& gateway = gateways[i];
        if (gateway.destination_field_id == FieldData::INACTIVE_GATEWAY_ID) continue;
        map.LinkEndChild(CreateTriggerLine(
          "Gateway" + std::to_string(i), TriggerPoint(gateway.exit_line[0], downscaler),
          TriggerPoint(gateway.exit_line[1], downscaler)
        ).release());
    }
    // Background triggers (doors and similar that change tiles).
    const auto& bg_triggers = triggers->GetBackgroundTriggers();
    for (size_t i = 0; i < bg_triggers.size(); i ++){
        const VGears::TriggersFile::Trigger& trigger = bg_triggers[i];
        if (!FieldData::IsBackgroundTriggerUsed(trigger)) continue;
        map.LinkEndChild(CreateTriggerLine(
          "BgTrigger" + std::to_string(i), TriggerPoint(trigger.trigger_line[0], downscaler),
          TriggerPoint(trigger.trigger_line[1], downscaler)
        ).release());
    }
}

void FieldMapWriter::WriteTracks(const VGears::FLevelFilePtr& field, TiXmlElement& map){
    const std::vector<int> tracks = ExtractMusicTrackIds(field->GetRawScript());
    std::unique_ptr<TiXmlElement> xml_tracks(new TiXmlElement("tracks"));
    for (int t = 0; t < tracks.size(); t ++){
        std::unique_ptr<TiXmlElement> track(new TiXmlElement("track"));
        track->SetAttribute("id", t);
        track->SetAttribute("track_id", tracks[t]);
        xml_tracks->LinkEndChild(track.release());
    }
    map.LinkEndChild(xml_tracks.release());
}
