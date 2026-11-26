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

#include <sstream>
#include "InstallCommandLine.h"

namespace{

    /**
     * Name of a preset in the command line, and the preset.
     */
    struct PresetName{
        const char* name;
        InstallPreset preset;
    };

    const PresetName kPresets[] = {
      {"everything", InstallPreset::EVERYTHING},
      {"fields", InstallPreset::FIELDS},
      {"world-map", InstallPreset::WORLD_MAP},
      {"battle", InstallPreset::BATTLE},
      {"media", InstallPreset::MEDIA},
      {"custom", InstallPreset::CUSTOM}
    };

    /**
     * Splits a comma separated list, skipping empty items.
     */
    std::vector<std::string> SplitList(const std::string& list){
        std::vector<std::string> items;
        std::stringstream stream(list);
        std::string item;
        while (std::getline(stream, item, ',')) if (!item.empty()) items.push_back(item);
        return items;
    }

    /**
     * Finds a part by name.
     *
     * @return The part, or nullptr if there is no part with that name.
     */
    const InstallCommandLine::Part* FindPart(const std::string& name){
        for (const InstallCommandLine::Part& part : InstallCommandLine::Parts()) if (name == part.name) return &part;
        return nullptr;
    }

    /**
     * Gets the names of the parts, separated by commas.
     */
    std::string PartNames(){
        std::string names;
        for (const InstallCommandLine::Part& part : InstallCommandLine::Parts())
            names += (names.empty() ? "" : ", ") + std::string(part.name);
        return names;
    }

    /**
     * Gets the names of the presets, separated by commas.
     */
    std::string PresetNames(){
        std::string names;
        for (const PresetName& preset : kPresets) names += (names.empty() ? "" : ", ") + std::string(preset.name);
        return names;
    }

    /**
     * Marks the command line as wrong.
     */
    InstallCommandLine::Arguments Invalid(const std::string& error){
        InstallCommandLine::Arguments arguments;
        arguments.mode = InstallCommandLine::Mode::INVALID;
        arguments.error = error;
        return arguments;
    }

}

namespace InstallCommandLine{

    const std::vector<Part>& Parts(){
        static const std::vector<Part> parts = {
          {"kernel", "Kernel data: items, materia, characters...", &InstallOptions::kernel},
          {"fields", "Field maps", &InstallOptions::fields},
          {"field-models", "Field 3D models (needs fields)", &InstallOptions::field_models},
          {"world-map", "World map", &InstallOptions::wm},
          {"world-map-models", "World map 3D models (needs world-map)", &InstallOptions::wm_models},
          {"battle-data", "Battle scenes", &InstallOptions::battle_data},
          {"battle-models", "Battle 3D models", &InstallOptions::battle_models},
          {"images", "Images", &InstallOptions::images},
          {"sounds", "Sound effects (needs FFmpeg)", &InstallOptions::sounds},
          {"music", "Music (needs TiMidity and FFmpeg)", &InstallOptions::music},
          {"videos", "Videos (only from game disks 1, 2 and 3; needs FFmpeg)", &InstallOptions::videos}
        };
        return parts;
    }

    Arguments Parse(const std::vector<std::string>& args){
        Arguments arguments;
        bool cli = false;
        bool verify = false;
        bool list_parts = false;
        bool help = false;
        bool version = false;
        bool preset_given = false;
        std::vector<std::string> added;
        std::vector<std::string> removed;
        std::vector<std::string> unknown;
        for (size_t i = 0; i < args.size(); ++ i){
            std::string arg = args[i];
            // "--option=value" is the same as "--option value".
            std::string inline_value;
            bool has_inline_value = false;
            const size_t equals = arg.find('=');
            if (arg.compare(0, 2, "--") == 0 && equals != std::string::npos){
                inline_value = arg.substr(equals + 1);
                arg = arg.substr(0, equals);
                has_inline_value = true;
            }
            // Reads the value of an option. Returns false if it is missing.
            std::string value;
            const auto take_value = [&](){
                if (has_inline_value){
                    value = inline_value;
                    return true;
                }
                if (i + 1 >= args.size()) return false;
                value = args[++ i];
                return true;
            };
            const auto needs_value = [&](const std::string& name){return "Missing value for " + name + ".";};

            if (arg == "--cli") cli = true;
            else if (arg == "--verify") verify = cli = true;
            else if (arg == "--list-parts") list_parts = true;
            else if (arg == "-h" || arg == "--help") help = true;
            else if (arg == "-v" || arg == "--version") version = true;
            else if (arg == "-y" || arg == "--yes") arguments.yes = cli = true;
            else if (arg == "--no-copy-data"){
                arguments.copy_data = false;
                cli = true;
            }
            else if (arg == "--keep-originals"){
                arguments.keep_originals = true;
                cli = true;
            }
            else if (arg == "--clean"){
                arguments.clean = true;
                cli = true;
            }
            else if (arg == "--strict"){
                arguments.strict = true;
                cli = true;
            }
            else if (arg == "-i" || arg == "--iso"){
                if (!take_value()) return Invalid(needs_value(arg));
                arguments.iso = value;
                cli = true;
            }
            else if (arg == "-o" || arg == "--output"){
                if (!take_value()) return Invalid(needs_value(arg));
                arguments.output = value;
                cli = true;
            }
            else if (arg == "--data-dir"){
                if (!take_value()) return Invalid(needs_value(arg));
                arguments.data_dir = value;
                cli = true;
            }
            else if (arg == "--ffmpeg"){
                if (!take_value()) return Invalid(needs_value(arg));
                arguments.ffmpeg = value;
                cli = true;
            }
            else if (arg == "--timidity"){
                if (!take_value()) return Invalid(needs_value(arg));
                arguments.timidity = value;
                cli = true;
            }
            else if (arg == "-p" || arg == "--preset"){
                if (!take_value()) return Invalid(needs_value(arg));
                bool found = false;
                for (const PresetName& preset : kPresets){
                    if (value == preset.name){
                        arguments.preset = preset.preset;
                        found = true;
                    }
                }
                if (!found) return Invalid("Unknown preset '" + value + "'. The presets are: " + PresetNames() + ".");
                preset_given = cli = true;
            }
            else if (arg == "--with" || arg == "--without"){
                if (!take_value()) return Invalid(needs_value(arg));
                for (const std::string& name : SplitList(value)){
                    if (FindPart(name) == nullptr)
                        return Invalid("Unknown part '" + name + "'. The parts are: " + PartNames() + ".");
                    (arg == "--with" ? added : removed).push_back(name);
                }
                cli = true;
            }
            else unknown.push_back(args[i]);
        }

        // Requests for information are answered whatever else is in the command line.
        if (help){
            arguments.mode = Mode::HELP;
            return arguments;
        }
        if (version){
            arguments.mode = Mode::VERSION;
            return arguments;
        }
        if (list_parts){
            arguments.mode = Mode::LIST_PARTS;
            return arguments;
        }
        // Without installer options, the window is opened, and Qt gets the rest of the arguments.
        if (!cli){
            arguments.mode = Mode::GUI;
            return arguments;
        }
        if (!unknown.empty()) return Invalid("Unknown argument '" + unknown.front() + "'.");

        // The parts: those of the preset, plus those added, minus those removed. Like in the window, the presets
        // don't include the videos: they are not on the install disk, and the game disks only have videos.
        arguments.options = InstallRules::Preset(arguments.preset, false);
        for (const std::string& name : added) arguments.options.*(FindPart(name)->option) = true;
        for (const std::string& name : removed) arguments.options.*(FindPart(name)->option) = false;
        const InstallOptions selectable = InstallRules::Selectable(arguments.options);
        for (const std::string& name : added){
            const Part* part = FindPart(name);
            if (arguments.options.*(part->option) && !(selectable.*(part->option)))
                return Invalid("The part '" + name + "' can't be installed: " + part->description + ".");
        }
        InstallRules::ApplyDependencies(arguments.options);

        if (verify){
            if (!arguments.iso.empty()) return Invalid("--verify checks an installation, it doesn't take an ISO.");
            if (preset_given || !added.empty() || !removed.empty())
                return Invalid("--verify checks everything; it doesn't take a preset or parts.");
            if (arguments.clean) return Invalid("--verify checks an installation, it doesn't delete it.");
            arguments.mode = Mode::VERIFY;
            return arguments;
        }
        if (arguments.iso.empty()) return Invalid("No ISO image given. Use --iso <file>.");
        arguments.mode = Mode::INSTALL;
        return arguments;
    }

    std::string Usage(const std::string& program){
        std::stringstream usage;
        usage
          << "Usage:\n"
          << "  " << program << "                      Open the installer window.\n"
          << "  " << program << " --cli --iso <file> [options]\n"
          << "                                         Install from the command line.\n"
          << "  " << program << " --verify [--output <dir>]\n"
          << "                                         Check that an installation has everything the game needs.\n"
          << "\n"
          << "Installation options:\n"
          << "  -i, --iso <file>         The disk image. The install disk installs the game; game disks 1, 2\n"
          << "                           and 3 only add their videos to an installation.\n"
          << "  -o, --output <dir>       Installation directory. Default: the one the game reads.\n"
          << "  -p, --preset <name>      What to install: " << PresetNames() << ".\n"
          << "                           Default: everything. 'custom' starts with nothing.\n"
          << "      --with <parts>       Install these parts too (comma separated).\n"
          << "      --without <parts>    Don't install these parts (comma separated).\n"
          << "      --no-copy-data       Don't copy the V-Gears data files: the installation directory\n"
          << "                           must already have them.\n"
          << "      --data-dir <dir>     The V-Gears 'data' folder to copy. Default: looked for next to\n"
          << "                           the installer and in the system.\n"
          << "      --clean              Delete the data of a previous installation first. The saves, the\n"
          << "                           controls, the game configuration and the videos are kept.\n"
          << "      --keep-originals     Keep the original data once converted: the extracted disk, the\n"
          << "                           temporary files and the original sounds, music and images.\n"
          << "      --ffmpeg <file>      Path to FFmpeg. Default: looked for in the 'tools' folder of the\n"
          << "                           installation and of the installer, then in the PATH.\n"
          << "      --timidity <file>    Path to TiMidity. Looked for like FFmpeg.\n"
          << "  -y, --yes                Continue when there are warnings or the audio tools are missing.\n"
          << "      --strict             End with status 3 if the installation finished with errors in its log.\n"
          << "\n"
          << "Other options:\n"
          << "      --cli                Use the command line, not the window. Implied by any option above.\n"
          << "      --list-parts         List the parts that can be installed.\n"
          << "  -h, --help               Show this help.\n"
          << "  -v, --version            Show the version.\n"
          << "\n"
          << "Parts: " << PartNames() << ".\n"
          << "\n"
          << "Exit status: 0 if everything went well, 1 if the installation or the verification failed,\n"
          << "2 if the command line is wrong, 3 if it finished with errors and --strict was given, 130 if it\n"
          << "was interrupted.\n";
        return usage.str();
    }

}
