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

#include <fstream>
#include <stdexcept>
#include "FieldScriptFormatters.h"
#include "FieldScriptInstaller.h"

const std::vector<int> FieldScriptInstaller::TEMPORARY_BANKS{5};

FieldScriptInstaller::FieldScriptInstaller(
  const std::string& data_dir, const MapList& map_list, FieldSpawnPointsMap& spawn_points,
  std::function<void(std::string)> log_output_line
):
  data_dir_(data_dir), map_list_(map_list), spawn_points_(spawn_points),
  write_output_line_(log_output_line)
{}

FieldDecompiler::DecompiledScript FieldScriptInstaller::Install(const VGears::FLevelFilePtr& field){
    const std::string trigger_scripts = CreateTriggerScripts(field);
    const VGears::ModelListFilePtr& models = field->GetModelList();
    FieldDecompiler::DecompiledScript decompiled;
    FF7FieldScriptFormatter formatter(field->getName(), models, map_list_, spawn_points_);
    try{
        const std::vector<u8> raw_script = field->GetRawScript();
        decompiled = FieldDecompiler::Decompile(
          field->getName(), raw_script, formatter, trigger_scripts,
          FieldScriptHeader(static_cast<int>(FieldData::FieldId(field->getName(), map_list_)))
        );
        const std::string dir = data_dir_ + "/" + FieldData::MAPS_DIR + "/" + field->getName();
        std::ofstream script_file(dir + "/script.lua");
        if (script_file.is_open()){
            script_file << decompiled.luaScript;
            WriteTexts(field, raw_script);
        }
        else{
            write_output_line_(
              "[ERROR] Failed to open script file from field " + field->getName() + " for writing."
            );
        }
    }
    catch (const ::DecompilerException& ex){
        write_output_line_(
          "[ERROR] Internal decompiler error in field " + field->getName() + ": " + ex.what()
        );
    }
    return decompiled;
}

std::string FieldScriptInstaller::FieldScriptHeader(const int field_id){
    std::string header = "EntityContainer = {}\n\n";
    if (field_id >= 0){
        // LSTMP reads the ID of the field the player comes from. The field being left is the
        // previous one, unless the field was just reloaded (after a battle, for example).
        header += "-- The IDs of this field and of the previous one, for LSTMP.\n"
          "if Data.FieldId ~= " + std::to_string(field_id) + " then\n"
          "    Data.PreviousFieldId = Data.FieldId or 0\n"
          "    Data.FieldId = " + std::to_string(field_id) + "\n"
          "end\n\n";
    }
    for (const int bank : TEMPORARY_BANKS){
        header += "-- Temporary variables start at zero in every field.\n"
          "for address = 0, Banks.SIZE do\n"
          "    Banks[" + std::to_string(bank) + "][address] = 0\n"
          "end\n\n";
    }
    return header;
}

std::string FieldScriptInstaller::CreateTriggerScripts(const VGears::FLevelFilePtr& field) const{
    std::string scripts;
    const VGears::TriggersFilePtr& triggers = field->GetTriggers();
    const auto& gateways = triggers->GetGateways();
    for (size_t i = 0; i < gateways.size(); i ++){
        const VGears::TriggersFile::Gateway& gateway = gateways[i];
        if (gateway.destination_field_id == FieldData::INACTIVE_GATEWAY_ID) continue;
        scripts += CreateGatewayScript(
          "Gateway" + std::to_string(i), map_list_.at(gateway.destination_field_id),
          "Spawn_" + field->getName() + "_" + std::to_string(i)
        );
    }
    const auto& bg_triggers = triggers->GetBackgroundTriggers();
    for (size_t i = 0; i < bg_triggers.size(); i ++){
        if (!FieldData::IsBackgroundTriggerUsed(bg_triggers[i])) continue;
        scripts += CreateBackgroundTriggerScript("BgTrigger" + std::to_string(i), bg_triggers[i]);
    }
    return scripts;
}

std::string FieldScriptInstaller::CreateGatewayScript(
  const std::string& gateway_entity_name, const std::string& target_map_name,
  const std::string& source_spawn_point_name
){
    return
      "\nEntityContainer[ \"" + gateway_entity_name + "\" ] = {\n\n"
      "    on_start = function(self)\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_approach = function(self, entity)\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_cross = function(self, entity)\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_near = function(self, entity)\n"
      "        if entity ~= nil then\n"
      "            if not Data.DisableGateways then\n"
      "                load_field_map_request(\"" + target_map_name + "\", \"" + source_spawn_point_name + "\")\n"
      "            end\n"
      "        end\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_leave = function(self, entity)\n"
      "        return 0\n"
      "    end,\n"
      "}\n\n";
}

std::string FieldScriptInstaller::CreateBackgroundTriggerScript(
  const std::string& name, const VGears::TriggersFile::Trigger& trigger
){
    const std::string area = std::to_string(trigger.background_parameter);
    const std::string state = std::to_string(trigger.background_state);
    const std::string on = "background2d:area_on(" + area + ", " + state + ")";
    const std::string off = "background2d:area_off(" + area + ", " + state + ")";
    std::string enter, leave;
    switch (trigger.behavior){
        case 0: enter = on; break;
        case 1: enter = off; break;
        case 2: case 4: enter = on; leave = off; break;
        case 3: case 5: enter = off; leave = on; break;
        default: break;
    }
    return
      "\nEntityContainer[ \"" + name + "\" ] = {\n\n"
      "    on_start = function(self)\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_approach = function(self, entity)\n"
      "        if entity ~= nil then " + enter + " end\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_cross = function(self, entity)\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_near = function(self, entity)\n"
      "        return 0\n"
      "    end,\n\n"
      "    on_leave = function(self, entity)\n"
      "        if entity ~= nil then " + leave + " end\n"
      "        return 0\n"
      "    end,\n"
      "}\n\n";
}

void FieldScriptInstaller::WriteTexts(
  const VGears::FLevelFilePtr& field, const std::vector<u8>& raw_script
){
    text_writer_.Begin(data_dir_ + "/" + FieldData::MAPS_DIR + "/" + field->getName() + "/text.xml");
    try{text_writer_.Write(raw_script, field->getName());}
    catch (const std::out_of_range& ex){
        write_output_line_(
          "[ERROR] Failed to read texts from field " + field->getName() + ": " + ex.what()
        );
    }
    text_writer_.End();
}
