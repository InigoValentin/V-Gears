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
#include <fstream>
#include "data/AbFile.h"

AbFile::AbFile(File file, bool enemy): ab_file_(file), is_enemy_(enemy){
    scripts_.clear();
    file.SetOffset(0);
    Read();
}

unsigned int AbFile::GenerateScripts(std::string model_id, std::string path) const{
    std::string file_name = path + model_id + ".lua";
    std::fstream file(file_name, std::ios::out);
    if (!file.is_open()){
        std::cerr << "Unable to open file " << file_name
          << " for animation script generation." << std::endl;
        return 0;
    }
    file << model_id << " = {}" << std::endl << std::endl;
    for (int script = 0; script < scripts_.size(); script ++){
        file << model_id << ".script" << script << " = function()" << std::endl;
        file << scripts_[script];
        file << "end" << std::endl << std::endl;
    }
    file.close();
    return scripts_.size();
}

void AbFile::Read(){
    ab_file_.SetOffset(0x68); // 104 / 0x68 is the first script index.
    int total = is_enemy_ ? 74 : 32; // 32 for enemies, 74 for characters.
    for (int a = 0; a < total; a ++){
        scripts_.push_back(DecompileScript(ab_file_.readU16LE()));
        ab_file_.readU16LE(); // Offset are 16 bits, in 32 bit blocks. 3rd and 4th bytes discarded.
    }
}

std::string AbFile::DecompileScript(u32 offset){
    u32 intial_offset = ab_file_.GetCurrentOffset();
    ab_file_.SetOffset(offset);
    bool label_set = false; // Indicates if at least one 0xC9 (jump label) opcode has been found.
    bool end = false;
    int total_opcodes = 0;

    std::string str_string = "";
    while (true){
        std::stringstream line;
        if (end) break;
        if (total_opcodes > 100){
            // Give up if there are more than 100
            line << "    return 0 -- More than 100 opcodes in script, discard script.\n";
            end = true;
            break;
        }
        total_opcodes ++;
        u8 opcode = ab_file_.readU8();
        if (opcode >= 0x00 && opcode <= 0x8D){
            line << "    play_animation(" << static_cast<int>(opcode) << ")";
        }
        else{
            switch (opcode){
                case 0x91: // Play effect, one 1 byte parameter.
                    line << "    play_effect(" << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0xA9: // Increase script counter by two. Two 1 byte parameters, ignored
                    ab_file_.readU8();
                    ab_file_.readU8();
                    break;
                case 0xAA: // Unpause camera, no parameters.
                    line << "    unpause_macera()";
                    break;
                case 0xAD:
                     // Set effect, four parameters:
                     //   joint (1 byte), skeleton joint at which to set the effect.
                     //   distance (2 bytes), distance from the joint.
                     //   start (1 byte), effect start.
                     //   end (1 byte), effect end.
                    line << "    set_effect(" << static_cast<int>(ab_file_.readU8()) << ", "
                      << ab_file_.readU16LE() << ", " << static_cast<int>(ab_file_.readU8()) << ", "
                      << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0xB6: // Pause camera and finish animation. One byte parameter, animation ID.
                    line << "    pause_camera_finish_animation("
                      << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0xB9: // Set camera. One byte parameter, camera ID.
                    line << "    set_camera(" << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0x06:
                     // Play hurt animation, including action, effect and sound. It will not display
                     // damage and barrier effect. One byte parameter, the amount of frames to wait
                     // before starting.
                    line << "    execute_hurt(" << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0xC1: // Jump to label set by 0xC9 (::jump::). No parameters.
                    line << "    goto jump";
                    break;
                case 0xC2: // Show damage. One byte parameter, frames to wait before showing.
                    line << "    execute_damage(" << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0xC5:
                     // Set value from 0x800f8374 (unit fade time) as wait time for action script.
                     // No parameters. I have no idea what this is used for.
                    line << "    set_unit_fade_wait()";
                    break;
                case 0xC6:
                     // Set value to 0x800f8374 (unit fade time) for futher use. One byte parameter,
                     // wait time. I have no idea what this is used for.
                    line
                      << "    set_unit_fade_time(" << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0xC9: // Set a jump label. Only do it once per script. No prameters
                    if (!label_set){
                        line << "    ::jump::";
                        label_set = true;
                    }
                    else{
                        line << "    -- Invalid jump label";
                    }
                    break;
                case 0xCA:
                     // Jump to label set by 0xC9 (::jump::) if nothing is being loaded in the
                     // the background. No parameters. I don't think this will be necessary.
                    line << "    -- goto jump";
                    break;
                case 0xD0:
                     // Jump to enemy, two parameters
                     //   (2 bytes), I don't know what it is.
                     //   (1 byte), I don't know what it is.
                    line << "    jump_to_enemy(" << ab_file_.readU16LE() << ", "
                      << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0xD1:
                     // Move to enemy using function 0x800cf5bc by number of steps. Stop distance is
                     // distance to target collision radius. Three parameters:
                     //   distance (2 byte), unknown.
                     //   unknown (2 bytes), unknown.
                     //   steps (1 byte), number of steps.
                    line << "    move_to_target(" << ab_file_.readU16LE() << ", "
                      << ab_file_.readU16LE() << ", " << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0xD8:
                    // Play sound using attacker settings after waiting given number of frames.
                    // Two parameters:
                    //   wait (1 byte), frames to wait before the sound.
                    //   sound (2 bytes), sound ID.
                    line << "    play_sound_for_attacker("
                      << static_cast<int>(ab_file_.readU8()) << ", "
                      << ab_file_.readU16LE() <<  ")";
                    break;
                case 0xE5:
                    // Set initial (idle) direction for current unit according to situation. No
                    // parameters.
                    line << "    return_direction()";
                    break;
                case 0xE8:
                    // Start load effect requested during attack (attack type id and attack id are
                    // used to determinate what effect to load). No parameters.
                    line << "    load_additional_effect()";
                    break;
                case 0xEA: // Show the current action name in the title window.
                    line << "    show_action_name()";
                    break;
                case 0xEC:
                    // if effect not loaded, call this opcode until it does. For magic, summon,
                    // limit, enemy skill and enemy attack. execute loaded effect. All effects are
                    // hardcoded so they can do whatever they want (play sounds, display damage,
                    // request hurt for target and so on).
                    line << "    exectute_aditional_effect()";
                    break;
                case 0xF0: // Set effect.
                    line << "    set_effect()";
                    break;
                case 0xF3:
                    // Repeat reading this opcode until wait time for script not reach 0.
                    // It decreases by 1 each call. Waiting time is set with 0xF4
                    line << "    wait()";
                    break;
                case 0xF4: // Set frames to wait. 1 parameter, frames to wait (1 byte).
                    line << "    set_wait(" << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0xF6:
                    // Play die effect (depends on die type) if unit is dead. Used in enemy hurt
                    // actions.
                    line << "    play_die_if_dead()";
                    break;
                case 0xF7:
                    // After wait time ends execute hurt action, effect, sound. This will display
                    // damage and barriers effect. One byte parameter, wait time, in frames.
                    line << "    execute_attack(" << static_cast<int>(ab_file_.readU8()) << ")";
                    break;
                case 0xFA: // Instantly set default position for units. No parameters.
                    line << "    return_position()";
                    break;
                case 0xFC:
                    // Set direction for targets (delayed) and attacker according to situation. No
                    // parameters.
                    line << "    set_direction()";
                    break;
                case 0x9E: // Return.
                case 0xF1: // Return.
                case 0xEE: // Return.
                case 0xFF: // Return.
                //case 0xC1: // Return.
                case 0xA2: // Return.
                    line << "    return 0";
                    end = true;
                    break;
                case 0xFE:
                    // Special return opcode. Return only if next byte is 0xC0.
                    {
                        u8 param = ab_file_.readU8();
                        if (param == 0xC0){
                            line << "    return 0";
                            end = true;
                        }
                        else line << "    -- 0xFE not paired with 0xC0 for return.";
                    }
                    break;
                default:
                    line << "    -- Unknown/unused opcode.";


            }
        }
        // Add a comment and a new line.
        line << " -- 0x";
        std::stringstream hex_opcode;
        hex_opcode << std::hex << std::uppercase << static_cast<int>(opcode);
        str_string += line.str() + hex_opcode.str() + "\n";
    }
    ab_file_.SetOffset(intial_offset);
    return str_string;
}
