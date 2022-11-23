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

#include <iostream>

#include "FieldTextWriter.h"
#include "Characters.h"
#include "common/VGearsStringUtil.h"




void FieldTextWriter::Begin(std::string file_name){
    data_.clear();
    logger_.reset(new Logger(file_name));
    // Write BOM
    /* FIX ME - It needs to write data as UTF8.
    std::vector< unsigned char > bomBytes;
    bomBytes.push_back(0xFF);
    bomBytes.push_back(0xFE);
    logger_->Log(bomBytes);
    */
    tag_open_ = false;
    logger_->Log("<texts>\n");
}

u16 FieldTextWriter::GetU16LE(u32 offset){
    if (offset+1 >= data_.size()) throw std::out_of_range("");
    return *reinterpret_cast<u16*>(&data_[offset]);
}

u8 FieldTextWriter::GetU8(u32 offset){
    if (offset >= data_.size()) throw std::out_of_range("");
    return data_[offset];
}

void FieldTextWriter::Write(
  const std::vector<u8>& script_section_buffer, std::string field_name, bool english
){
    data_ = script_section_buffer;
    // Get sector 1 offset (scripts and dialog).
    u32 offset_to_sector = 0; // Our buffer IS the start of scripts and dialog.
    /*
    u16 unknown1; // Always 0x0502.
    char nEntities; // Number of entities.
    char nModels; // Number of models.
    u16 wStringOffset; // Offset to strings code below extracts this offset.
    */
    u16 offset_to_dialogs = GetU16LE(offset_to_sector + 0x04);
    std::vector< unsigned char > dialog; // text line to write out
    // HACK: If this function throws then this iteration generates invalid XML
    // so close the unclosed XML tag - this function should be made thread safe
    // and/or fix the fields that fail to convert correctly.
    if (tag_open_){
        logger_->Log("[ERROR exception]");
        logger_->Log("</dialog>\n");
    }
    const u16 dialogCount = GetU16LE(offset_to_sector + offset_to_dialogs);
    for (u16 i = 0; i <dialogCount; ++ i){
        // gGt offset of string data.
        u32 offset = offset_to_sector + offset_to_dialogs + GetU16LE(
            offset_to_sector + offset_to_dialogs
            + 0x02 +  // +2 to skip dialog count
            i * 0x02 // *2 because each char is 2 bytes
          );
        AddText(dialog);
        logger_->Log(
          "<dialog name=\"" + field_name + "_" + std::to_string(i) + "\">"
        );
        tag_open_ = true;
        // Some maps have unclosed color tags.
        // Use this counter to close them at the end.
        int colour_opened = 0;
        for (unsigned char temp = 0x00;; ++ offset){
            temp = GetU8(offset);
            if (temp == 0xFF) break;
            if (temp == 0xE0 && english == true){
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
            }
            else if (temp == 0xE1 && english == true){
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
            }
            else if (temp == 0xE2 && english == true){
                dialog.push_back(ENGLISH_CHARS[0x0C]);
                dialog.push_back(ENGLISH_CHARS[0x00]);
            }
            else if (temp == 0xE8){
                AddText(dialog);
                logger_->Log("<next_page />");
            }
            else if (temp == 0xEA){
                AddText(dialog);
                logger_->Log("<character id=\"0\" />");
            }
            else if (temp == 0xEB){
                AddText(dialog);
                logger_->Log("<character id=\"1\" />");
            }
            else if (temp == 0xEC){
                AddText(dialog);
                logger_->Log("<character id=\"2\" />");
            }
            else if (temp == 0xED){
                AddText(dialog);
                logger_->Log("<character id=\"3\" />");
            }
            else if (temp == 0xEE){
                AddText(dialog);
                logger_->Log("<character id=\"4\" />");
            }
            else if (temp == 0xEF){
                AddText(dialog);
                logger_->Log("<character id=\"5\" />");
            }
            else if (temp == 0xF0){
                AddText(dialog);
                logger_->Log("<character id=\"6\" />");
            }
            else if (temp == 0xF1){
                AddText(dialog);
                logger_->Log("<character id=\"7\" />");
            }
            else if (temp == 0xF2){
                AddText(dialog);
                logger_->Log("<character id=\"8\" />");
            }
            else if (temp == 0xF3){
                AddText(dialog);
                logger_->Log("<party pos=\"0\" />");
            }
            else if (temp == 0xF4){
                AddText(dialog);
                logger_->Log("<party pos=\"1\" />");
            }
            else if (temp == 0xF5){
                AddText(dialog);
                logger_->Log("<party pos=\"2\" />");
            }
            else if (temp == 0xF6){
                AddText(dialog);
                logger_->Log("<image sprite=\"ButtonCircle\" />");
            }
            else if (temp == 0xF7){
                AddText(dialog);
                logger_->Log("<image sprite=\"ButtonTriangle\" />");
            }
            else if (temp == 0xF8){
                AddText(dialog);
                logger_->Log("<image sprite=\"ButtonSquare\" />");
            }
            else if (temp == 0xF9){
                AddText(dialog);
                logger_->Log("<image sprite=\"ButtonCross\" />");
            }
            else if (
              (temp == 0xFA || temp == 0xFB || temp == 0xFC || temp == 0xFD) && english == false
            ){
                ++ offset;
                unsigned char temp2 = GetU8(offset);
                if (temp == 0xFA){
                    if (JAPANESE_CHARS_FA[temp2] != 0x0000){
                        dialog.push_back(JAPANESE_CHARS_FA[temp2] >> 8);
                        dialog.push_back(JAPANESE_CHARS_FA[temp2] & 0xFF);
                    }
                    else{
                        AddText(dialog);
                        logger_->Log("[MISSING 0xFA " + HexToString(temp2, 2, '0') + "]");
                    }
                }
                else if (temp == 0xFB){
                    if (JAPANESE_CHARS_FB[temp2] != 0x0000){
                        dialog.push_back(JAPANESE_CHARS_FB[temp2] >> 8);
                        dialog.push_back(JAPANESE_CHARS_FB[temp2] & 0xFF);
                    }
                    else{
                        AddText(dialog);
                        logger_->Log("[MISSING 0xFB " + HexToString(temp2, 2, '0') + "]");
                    }
                }
                else if (temp == 0xFC){
                    if (JAPANESE_CHARS_FC[temp2] != 0x0000){
                        dialog.push_back(JAPANESE_CHARS_FC[temp2] >> 8);
                        dialog.push_back(JAPANESE_CHARS_FC[temp2] & 0xFF);
                    }
                    else{
                        AddText(dialog);
                        logger_->Log("[MISSING 0xFC " + HexToString(temp2, 2, '0') + "]");
                    }
                }
                else if (temp == 0xFD){
                    if (JAPANESE_CHARS_FD[temp2] != 0x0000){
                        dialog.push_back(JAPANESE_CHARS_FD[temp2] >> 8);
                        dialog.push_back(JAPANESE_CHARS_FD[temp2] & 0xFF);
                    }
                    else{
                        AddText(dialog);
                        logger_->Log("[MISSING 0xFD " + HexToString(temp2, 2, '0') + "]");
                    }
                }
            }
            else if (temp == 0xFE){
                ++ offset;
                unsigned char temp2 = GetU8(offset);

                // TODO: Maybe value must be RGB?
                // TODO: anfrst map fails to terminate a colour breaking all of
                // the XML, so colours turned off for now.
                if (temp2 == 0xD2){ // Gray.
                    AddText(dialog);
                    logger_->Log("<colour value=\"0.6 0.6 0.6\">");
                    colour_opened ++;
                }
                else if (temp2 == 0xD3){ // Blue.
                    AddText(dialog);
                    logger_->Log("<colour value=\"0.0 0.0 1.0\">");
                    colour_opened ++;
                }
                else if (temp2 == 0xD4){ // Red.
                    AddText(dialog);
                    logger_->Log("<colour value=\"1.0 0.0 0.0\">");
                    colour_opened ++;
                }
                else if (temp2 == 0xD5){ // Purple
                    AddText(dialog);
                    logger_->Log("<colour value=\"1.0 0.2 0.8\" >");
                    colour_opened ++;
                }
                else if (temp2 == 0xD6){ // Green.
                    AddText(dialog);
                    logger_->Log("<colour value=\"0.0 1.0 0.0\" >");
                    colour_opened ++;
                }
                else if (temp2 == 0xD7){ // Cyan.
                    AddText(dialog);
                    logger_->Log("<colour value=\"0.0 1.0 0.8\" >");
                    colour_opened ++;
                }
                else if (temp2 == 0xD8){ // Yellow
                    AddText(dialog);
                    logger_->Log("<colour value=\"1.0 1.0 0.4\">");
                    colour_opened ++;
                }
                else if (temp2 == 0xD9){ // White, or back to normal.
                    AddText(dialog);
                    logger_->Log("</colour>");
                    colour_opened --;
                }
                else if (temp2 == 0xDA){ // Flash (blink), global for a window.
                    // TODO: Implement
                }
                else if (temp2 == 0xDB){ // Rainbow, global for a window.
                    // TODO: Implement
                }
                else if (temp2 == 0xDC){
                    AddText(dialog);
                    logger_->Log("<pause_ok >");
                }
                else if (temp2 == 0xDD){
                    u16 wait = GetU16LE(offset + 1);
                    ++ offset;
                    ++ offset;
                    AddText(dialog);
                    logger_->Log("<pause time=\"" + IntToString(wait) + "\" />");
                }
                else{
                    if (JAPANESE_CHARS_FE[temp2] != 0x0000 && english == false){
                        dialog.push_back(JAPANESE_CHARS_FE[temp2] >> 8);
                        dialog.push_back(JAPANESE_CHARS_FE[temp2] & 0xFF);
                    }
                    else{
                        AddText(dialog);
                        logger_->Log("[MISSING 0xFE " + HexToString(temp2, 2, '0') + "]");
                    }
                }
            }
            else{
                if (japanese_chars[temp] != 0x0000 && english == false){
                   // dialog.push_back(japanese_chars[temp]);
                }
                else if (ENGLISH_CHARS[temp] != 0x0000 && english == true){
                    dialog.push_back(ENGLISH_CHARS[temp]);
                }
                else{
                    AddText(dialog);
                    if (temp == 0xa9) logger_->Log("..."); // TODO Verify.
                    else logger_->Log("[MISSING CHAR " + HexToString(temp, 2, '0') + "]");
                }
            }
        }
        // Close unclosed color tags;
        for (; colour_opened > 0; colour_opened --) logger_->Log("</colour>");
        AddText(dialog);
        logger_->Log("</dialog>\n\n");
        tag_open_ = false;
    }

}

void FieldTextWriter::End(){
    if (logger_) logger_->Log("</texts>\n");
    logger_.reset();
}

void FieldTextWriter::AddText(std::vector< unsigned char >& text){
    if (text.empty()) return;
    std::string str(reinterpret_cast<char*>(text.data()), text.size());
    str = Ogre::StringUtil::replaceAll(str, "&", "&amp;");
    str = Ogre::StringUtil::replaceAll(str, "\"", "&quot;");
    str = Ogre::StringUtil::replaceAll(str, "'", "&apos;");
    str = Ogre::StringUtil::replaceAll(str, "<", "&lt;");
    str = Ogre::StringUtil::replaceAll(str, ">", "&gt;");
    logger_->Log(str);
    text.clear();
}
