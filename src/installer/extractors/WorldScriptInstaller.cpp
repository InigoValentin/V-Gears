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
#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>
#include "decompiler/ControlFlow.h"
#include "decompiler/DecompilerException.h"
#include "decompiler/world/WorldEngine.h"
#include "WorldScriptInstaller.h"

const size_t WorldScriptInstaller::FIELD_TABLE_SIZE = 64 * 24;

namespace{

    /**
     * Number of functions of an event file.
     */
    const unsigned int FUNCTION_COUNT = 256;

    /**
     * Reads a little endian 16 bit value.
     *
     * @param[in] data The data.
     * @param[in] offset Offset of the value.
     * @return The value.
     */
    uint16 ReadU16(const std::vector<u8>& data, const size_t offset){
        return static_cast<uint16>(data[offset] | (data[offset + 1] << 8));
    }

    /**
     * Reads the type of a function of an event file.
     *
     * @param[in] event_data The event file.
     * @param[in] generated Which functions are in the script.
     * @param[in] index Index of the function.
     * @param[in] kind The wanted kind of function (the two upper bits of the type).
     * @param[out] type The type of the function.
     * @return True if the function is in the script and is of the wanted kind.
     */
    bool FunctionType(
      const std::vector<u8>& event_data, const std::vector<bool>& generated, const size_t index,
      const unsigned int kind, uint16& type
    ){
        if (!generated[index] || index * 4 + 1 >= event_data.size()) return false;
        type = ReadU16(event_data, index * 4);
        return (type >> 14) == kind;
    }
}

WorldScriptInstaller::WorldScriptInstaller(const std::string& data_dir):
  script_dir_(data_dir + "world/scripts")
{}

bool WorldScriptInstaller::Install(
  WorldArchive& archive, const std::vector<std::string>& field_names,
  const std::map<u16, float>& field_scale_factors,
  const std::vector<std::string>& converted_fields, std::string& error_message
){
    std::vector<const VGears::LGPArchive::FileEntry*> event_entries;
    const VGears::LGPArchive::FileEntry* field_table_entry = nullptr;
    bool has_wm0 = false;
    for (const VGears::LGPArchive::FileEntry& entry : archive.Entries()){
        const std::string name = WorldArchive::LowerName(entry);
        if (IsEventFile(name)){
            event_entries.push_back(&entry);
            if (name == "wm0.ev") has_wm0 = true;
        }
        else if (name == "field.tbl") field_table_entry = &entry;
    }
    if (event_entries.empty() || !has_wm0 || field_table_entry == nullptr){
        error_message = "WM0.EV, other world event files, or FIELD.TBL were not found inside WORLD_US.LGP.";
        return false;
    }
    if (field_table_entry->data_size != FIELD_TABLE_SIZE){
        error_message = "FIELD.TBL has an unexpected size.";
        return false;
    }
    const std::vector<u8> field_table = archive.Read(*field_table_entry);
    boost::filesystem::create_directories(script_dir_);
    for (const VGears::LGPArchive::FileEntry* event_entry : event_entries){
        if (!WriteScript(
          archive, *event_entry, field_table, field_names, field_scale_factors, converted_fields,
          error_message
        )) return false;
    }
    return true;
}

bool WorldScriptInstaller::IsEventFile(const std::string& name){
    if (name.size() <= 5 || name.compare(0, 2, "wm") != 0) return false;
    if (name.compare(name.size() - 3, 3, ".ev") != 0) return false;
    return std::all_of(
      name.begin() + 2, name.end() - 3,
      [](char character){return character >= '0' && character <= '9';}
    );
}

bool WorldScriptInstaller::WriteScript(
  WorldArchive& archive, const VGears::LGPArchive::FileEntry& event_entry,
  const std::vector<u8>& field_table, const std::vector<std::string>& field_names,
  const std::map<u16, float>& field_scale_factors,
  const std::vector<std::string>& converted_fields, std::string& error_message
){
    const std::vector<u8> event_data = archive.Read(event_entry);
    if (event_data.empty()){
        error_message = event_entry.file_name + " inside WORLD_US.LGP is empty.";
        return false;
    }
    const std::string name = WorldArchive::LowerName(event_entry);
    const std::string map_name = name.substr(0, name.size() - 3);
    const std::string path = script_dir_ + "/" + map_name + ".lua";
    std::ofstream script(path);
    if (!script){
        error_message = "Cannot create script output in " + script_dir_ + ".";
        return false;
    }
    script << "-- Decompiled from the user's " << event_entry.file_name << " data.\n\n";
    const std::vector<bool> generated = WriteFunctions(script, event_data, map_name);
    script << "-- " << map_name << " mesh-function call-table mapping.\n";
    WriteMeshFunctions(script, event_data, generated);
    WriteEntityFunctions(script, event_data, generated);
    WriteFieldTable(script, field_table, field_names, field_scale_factors, converted_fields);
    script.flush();
    if (!script){
        error_message = "Failed while writing " + path + ".";
        return false;
    }
    if (std::find(generated.begin(), generated.end(), true) == generated.end()){
        error_message = "No functions in " + event_entry.file_name + " could be decompiled.";
        return false;
    }
    return true;
}

std::vector<bool> WorldScriptInstaller::WriteFunctions(
  std::ostream& script, const std::vector<u8>& event_data, const std::string& map_name
){
    std::vector<bool> generated(FUNCTION_COUNT, false);
    for (unsigned int index = 0; index < FUNCTION_COUNT; ++ index){
        try{
            WorldEngine engine(index);
            InstVec instructions;
            std::unique_ptr<Disassembler> disassembler = engine.GetDisassembler(instructions);
            disassembler->Open(event_data);
            disassembler->Disassemble();
            if (instructions.empty()) continue;
            ControlFlow control_flow(instructions, engine);
            control_flow.CreateGroups();
            std::ostringstream function;
            std::unique_ptr<CodeGenerator> code_generator
              = engine.GetCodeGenerator(instructions, function);
            code_generator->Generate(instructions, control_flow.GetGraph());
            script << function.str() << "\n";
            generated[index] = true;
        }
        catch (const DecompilerException&){
            std::cerr << "[WARNING] World function " << index << " of " << map_name
              << " could not be decompiled." << std::endl;
            script << "-- world function " << index << " could not be decompiled.\n";
        }
        catch (const std::exception& exception){
            std::cerr << "[WARNING] World function " << index << " of " << map_name
              << " failed: " << exception.what() << std::endl;
            script << "-- world function " << index << " failed: " << exception.what() << ".\n";
        }
    }
    return generated;
}

void WorldScriptInstaller::WriteMeshFunctions(
  std::ostream& script, const std::vector<u8>& event_data, const std::vector<bool>& generated
){
    for (size_t index = 0; index < generated.size(); ++ index){
        uint16 type = 0;
        if (!FunctionType(event_data, generated, index, 2, type)) continue;
        const uint16 mesh = (type >> 4) & 0x03FF;
        const unsigned int function = type & 0x0F;
        if (mesh >= 36 * 28 || function > 4) continue;
        script << "System.WorldMap.triangle_functions[" << mesh << "]"
          << " = System.WorldMap.triangle_functions[" << mesh << "] or {}\n"
          << "System.WorldMap.triangle_functions[" << mesh << "][" << function << "] = "
          << index << "\n";
    }
}

void WorldScriptInstaller::WriteEntityFunctions(
  std::ostream& script, const std::vector<u8>& event_data, const std::vector<bool>& generated
){
    // The type of the entity is in the bits 8 to 13 of the type of the function, and the event in
    // the low byte.
    script << "System.WorldMap.entity_functions = {}\n";
    for (size_t index = 0; index < generated.size(); ++ index){
        uint16 type = 0;
        if (!FunctionType(event_data, generated, index, 1, type)) continue;
        const unsigned int entity = (type >> 8) & 0x3F;
        const unsigned int event = type & 0xFF;
        script << "System.WorldMap.entity_functions[" << entity << "] = "
          << "System.WorldMap.entity_functions[" << entity << "] or {}\n"
          << "System.WorldMap.entity_functions[" << entity << "][" << event << "] = "
          << index << "\n";
    }
}

void WorldScriptInstaller::WriteFieldTable(
  std::ostream& script, const std::vector<u8>& field_table,
  const std::vector<std::string>& field_names, const std::map<u16, float>& field_scale_factors,
  const std::vector<std::string>& converted_fields
){
    script << "System.WorldMap.field_table = {}\n";
    for (size_t record = 0; record < 64; ++ record){
        for (size_t scenario = 0; scenario < 2; ++ scenario){
            const size_t offset = record * 24 + scenario * 12;
            const uint16 field_id = ReadU16(field_table, offset + 6);
            if (field_id >= field_names.size()) continue;
            const std::string& field_name = field_names[field_id];
            if (std::find(converted_fields.begin(), converted_fields.end(), field_name)
              == converted_fields.end()) continue;
            const auto scale = field_scale_factors.find(field_id);
            if (scale == field_scale_factors.end() || scale->second <= 0) continue;
            const float downscale = 128.0f * scale->second;
            const float x = static_cast<int16>(ReadU16(field_table, offset)) / downscale;
            const float y = static_cast<int16>(ReadU16(field_table, offset + 2)) / downscale;
            const int16 triangle = static_cast<int16>(ReadU16(field_table, offset + 4));
            const float direction
              = 360.0f * static_cast<float>(field_table[offset + 8]) / 256.0f;
            const size_t table_id = record + 1;
            script << "System.WorldMap.field_table[" << table_id << "]"
              << " = System.WorldMap.field_table[" << table_id << "] or {}\n"
              << "System.WorldMap.field_table[" << table_id << "][" << scenario << "] = {"
              << "field = \"" << field_name << "\", x = " << x << ", y = " << y
              << ", triangle = " << triangle << ", direction = " << direction << "}\n";
        }
    }
}
