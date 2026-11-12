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

#include <cstdint>
#include <string>
#include <vector>

/**
 * Finding and running the external programs the installer uses to convert audio: FFmpeg (sounds and music) and
 * TiMidity (to render the MIDI music).
 *
 * Programs are searched in this order: the path the user chose, a list of directories (for example a "tools" folder
 * in the installation directory), and the system PATH. Every candidate is run to check that it really is the
 * program and that it can do what the installer needs, so a broken file doesn't hide a good one further down.
 *
 * Programs are run directly with a list of arguments, not through a shell: there is no quoting to get wrong, paths
 * can have spaces, and it works the same on Windows.
 */
namespace ExternalTools{

    /**
     * What is needed to search for an executable.
     */
    struct Environment{

        /**
         * The PATH variable.
         */
        std::string path;

        /**
         * The PATHEXT variable (Windows): extensions that make a file executable, separated by ';'.
         */
        std::string pathext;

        /**
         * True to search with the rules of Windows (';' separates the PATH entries, a file is executable if it has
         * one of the PATHEXT extensions) and false for the rules of Unix (':' and the executable permission).
         */
        bool windows = false;

        /**
         * Gets the environment of the running system.
         */
        static Environment FromSystem();
    };

    /**
     * Where to look for the programs.
     */
    struct Locations{

        /**
         * Path of FFmpeg that the user chose. Empty if none.
         */
        std::string ffmpeg;

        /**
         * Path of TiMidity that the user chose. Empty if none.
         */
        std::string timidity;

        /**
         * Directories searched before the PATH.
         */
        std::vector<std::string> directories;
    };

    /**
     * Gets the locations used by the installer. Set them before an installation.
     */
    Locations& CurrentLocations();

    /**
     * Lists the executables that could be a program, in the order they are tried.
     *
     * @param[in] name Name of the program, without extension.
     * @param[in] saved_path Path chosen by the user. It is the first candidate if it is a file.
     * @param[in] directories Directories searched before the PATH.
     * @param[in] env The environment.
     */
    std::vector<std::string> Candidates(
      const std::string& name, const std::string& saved_path, const std::vector<std::string>& directories,
      const Environment& env
    );

    /**
     * A program found in the system.
     */
    struct Tool{

        /**
         * Path of the program. Empty if it wasn't found.
         */
        std::string path;

        /**
         * First line of the version of the program.
         */
        std::string version;

        /**
         * True if a working program was found.
         */
        bool usable = false;

        /**
         * Why it can't be used. Empty if it is usable. It's about the first candidate that failed if there were any,
         * and says it wasn't found if there were none.
         */
        std::string problem;
    };

    /**
     * Gets the version number of a tool for the window: "9.0.2-1" from "ffmpeg version 9.0.2-1 Copyright ...", and
     * "2.14.0" from "TiMidity++ version 2.14.0". If there is no "version" word it is the whole version line.
     * Empty if the tool has no version.
     */
    std::string ShortVersion(const Tool& tool);

    /**
     * Result of running a program.
     */
    struct RunResult{
        bool started = false;
        bool finished = false;
        int exit_code = -1;

        /**
         * Standard output and error.
         */
        std::string output;
    };

    /**
     * Runs a program and waits for it.
     *
     * @param[in] program Path of the program.
     * @param[in] arguments Arguments, without quoting.
     * @param[in] timeout_ms Milliseconds to wait before killing it. Negative to wait forever.
     */
    RunResult Run(const std::string& program, const std::vector<std::string>& arguments, int timeout_ms);

    /**
     * Checks that a file is FFmpeg and that it can write ogg files (it has the Vorbis encoder).
     */
    Tool ProbeFfmpeg(const std::string& path);

    /**
     * Checks that a file is TiMidity.
     */
    Tool ProbeTimidity(const std::string& path);

    /**
     * Finds a working FFmpeg.
     */
    Tool FindFfmpeg(const Locations& locations, const Environment& env = Environment::FromSystem());

    /**
     * Finds a working TiMidity.
     */
    Tool FindTimidity(const Locations& locations, const Environment& env = Environment::FromSystem());

    /**
     * Arguments to convert an audio file to ogg with FFmpeg.
     */
    std::vector<std::string> FfmpegArguments(const std::string& input, const std::string& output);

    /**
     * Arguments to convert the audio FFmpeg reads from its standard input to ogg.
     */
    std::vector<std::string> FfmpegStdinArguments(const std::string& output);

    /**
     * Arguments to render a MIDI file with TiMidity to a wav file written to the standard output.
     */
    std::vector<std::string> TimidityArguments(const std::string& midi);

    /**
     * Converts an audio file (wav) to ogg.
     *
     * @param[in] ffmpeg Path of FFmpeg.
     * @param[in] input File to convert.
     * @param[in] output File to create. It is removed if the conversion fails.
     * @param[out] error Reason of the failure. Optional.
     * @return True if the output was created.
     */
    bool ConvertToOgg(
      const std::string& ffmpeg, const std::string& input, const std::string& output, std::string* error = nullptr
    );

    /**
     * Converts a movie to what the game plays: a folder with its frames as JPEG files (00000.jpg,
     * 00001.jpg...), an ogg file with its sound, and movie.xml with the number of frames and the
     * frames per second.
     *
     * @param[in] ffmpeg Path of FFmpeg.
     * @param[in] input The movie.
     * @param[in] directory Folder to create. It is removed if the conversion fails.
     * @param[in] name Name of the movie, which is the name of the ogg file.
     * @param[in] camera Content of the camera file of the movie, which the game uses to draw the
     * models over it. It's written as camera.cam, and the first frame that has a camera in
     * movie.xml. Empty if the movie has no camera.
     * @param[out] error Reason of the failure. Optional.
     * @return True if the frames were created. A movie without sound is converted anyway.
     */
    bool ConvertMovie(
      const std::string& ffmpeg, const std::string& input, const std::string& directory,
      const std::string& name, const std::vector<uint8_t>& camera = {}, std::string* error = nullptr
    );

    /**
     * Converts a MIDI file to ogg: TiMidity renders it and its output is sent to FFmpeg.
     *
     * @param[in] timidity Path of TiMidity.
     * @param[in] ffmpeg Path of FFmpeg.
     * @param[in] midi File to convert.
     * @param[in] output File to create. It is removed if the conversion fails.
     * @param[out] error Reason of the failure. Optional.
     * @return True if the output was created.
     */
    bool ConvertMidiToOgg(
      const std::string& timidity, const std::string& ffmpeg, const std::string& midi, const std::string& output,
      std::string* error = nullptr
    );

}
