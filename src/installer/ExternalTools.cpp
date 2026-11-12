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

#include "ExternalTools.h"
#include "MovieCamera.h"
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <cstdlib>

namespace fs = boost::filesystem;

namespace{

    std::string Lower(std::string text){
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){return std::tolower(c);});
        return text;
    }

    std::vector<std::string> Split(const std::string& text, char separator){
        std::vector<std::string> parts;
        std::string current;
        for (char c : text){
            if (c == separator){
                parts.push_back(current);
                current.clear();
            }
            else current += c;
        }
        parts.push_back(current);
        return parts;
    }

    /**
     * Checks that a path is a file that can be run.
     */
    bool IsExecutable(const std::string& path, const ExternalTools::Environment& env){
        boost::system::error_code ec;
        if (!fs::is_regular_file(path, ec)) return false;
        if (!env.windows){
            // The permission to run it, for the owner, the group or others.
            const fs::perms permissions = fs::status(path, ec).permissions();
            return !ec && (permissions & (fs::owner_exe | fs::group_exe | fs::others_exe)) != 0;
        }
        // In Windows a file is executable by its extension.
        const std::string extension = Lower(fs::path(path).extension().string());
        for (const std::string& allowed : Split(Lower(env.pathext), ';')){
            if (!allowed.empty() && allowed == extension) return true;
        }
        return false;
    }

    /**
     * Gets the files in a directory that could be a program with a name.
     */
    std::vector<std::string> InDirectory(
      const std::string& directory, const std::string& name, const ExternalTools::Environment& env
    ){
        std::vector<std::string> found;
        if (directory.empty()) return found;
        const fs::path base = fs::path(directory) / name;
        if (!env.windows){
            if (IsExecutable(base.string(), env)) found.push_back(base.string());
            return found;
        }
        // The name with each of the executable extensions, in the order of PATHEXT.
        for (const std::string& extension : Split(env.pathext, ';')){
            if (extension.empty()) continue;
            const std::string candidate = base.string() + extension;
            if (IsExecutable(candidate, env)) found.push_back(candidate);
            // Windows file names are case insensitive: the file may be called "FFMPEG.EXE".
            else{
                boost::system::error_code ec;
                for (fs::directory_iterator it(directory, ec), end; !ec && it != end; it.increment(ec)){
                    const std::string file = it->path().filename().string();
                    if (Lower(file) == Lower(name + extension) && IsExecutable(it->path().string(), env)){
                        found.push_back(it->path().string());
                        break;
                    }
                }
            }
        }
        return found;
    }

    std::string FirstLine(const std::string& text){
        const std::size_t end = text.find_first_of("\r\n");
        return end == std::string::npos ? text : text.substr(0, end);
    }

    /**
     * Runs the candidates of a program until one is usable.
     */
    ExternalTools::Tool FindTool(
      const std::string& name, const std::string& saved, const ExternalTools::Locations& locations,
      const ExternalTools::Environment& env, ExternalTools::Tool (*probe)(const std::string&)
    ){
        const std::vector<std::string> candidates
          = ExternalTools::Candidates(name, saved, locations.directories, env);
        ExternalTools::Tool first_failure;
        for (const std::string& candidate : candidates){
            ExternalTools::Tool tool = probe(candidate);
            if (tool.usable) return tool;
            if (first_failure.path.empty()) first_failure = tool;
        }
        if (!first_failure.path.empty()) return first_failure;
        ExternalTools::Tool missing;
        missing.problem = name + " was not found";
        return missing;
    }

    /**
     * Gets the text of a failed run for a message.
     */
    std::string Describe(const ExternalTools::RunResult& result){
        if (!result.started) return "it could not be started";
        if (!result.finished) return "it did not finish";
        std::string text = "exit code " + std::to_string(result.exit_code);
        const std::string line = FirstLine(result.output);
        if (!line.empty()) text += ": " + line;
        return text;
    }

}

namespace ExternalTools{

    Environment Environment::FromSystem(){
        Environment env;
        const char* path = std::getenv("PATH");
        const char* pathext = std::getenv("PATHEXT");
        env.path = path ? path : "";
#ifdef _WIN32
        env.windows = true;
        env.pathext = pathext ? pathext : ".COM;.EXE;.BAT;.CMD";
#else
        env.windows = false;
        env.pathext = pathext ? pathext : "";
#endif
        return env;
    }

    Locations& CurrentLocations(){
        static Locations locations;
        return locations;
    }

    std::vector<std::string> Candidates(
      const std::string& name, const std::string& saved_path, const std::vector<std::string>& directories,
      const Environment& env
    ){
        std::vector<std::string> candidates;
        const auto add = [&candidates](const std::string& path){
            if (std::find(candidates.begin(), candidates.end(), path) == candidates.end()) candidates.push_back(path);
        };
        if (!saved_path.empty() && IsExecutable(saved_path, env)) add(saved_path);
        for (const std::string& directory : directories)
            for (const std::string& file : InDirectory(directory, name, env)) add(file);
        for (const std::string& directory : Split(env.path, env.windows ? ';' : ':')){
            // Empty entries mean "the current directory", and searching there is a security risk.
            if (directory.empty() || !fs::path(directory).is_absolute()) continue;
            for (const std::string& file : InDirectory(directory, name, env)) add(file);
        }
        return candidates;
    }

    std::string ShortVersion(const Tool& tool){
        const std::string lower = Lower(tool.version);
        const std::size_t word = lower.find("version ");
        if (word == std::string::npos) return tool.version;
        const std::size_t start = word + 8;
        const std::size_t end = tool.version.find_first_of(" \t", start);
        return tool.version.substr(start, end == std::string::npos ? std::string::npos : end - start);
    }

    RunResult Run(const std::string& program, const std::vector<std::string>& arguments, int timeout_ms){
        RunResult result;
        QStringList list;
        for (const std::string& argument : arguments) list << QString::fromStdString(argument);
        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start(QString::fromStdString(program), list);
        result.started = process.waitForStarted(10000);
        if (!result.started) return result;
        result.finished = process.waitForFinished(timeout_ms);
        if (!result.finished){
            process.kill();
            process.waitForFinished(5000);
        }
        result.output = QString::fromLocal8Bit(process.readAll()).toStdString();
        result.exit_code = process.exitStatus() == QProcess::NormalExit ? process.exitCode() : -1;
        return result;
    }

    Tool ProbeFfmpeg(const std::string& path){
        Tool tool;
        tool.path = path;
        const RunResult version = Run(path, {"-version"}, 15000);
        if (!version.started || !version.finished || version.exit_code != 0
            || Lower(version.output).find("ffmpeg version") == std::string::npos){
            tool.problem = path + " is not FFmpeg (" + Describe(version) + ")";
            return tool;
        }
        tool.version = FirstLine(version.output);
        // The music and the sounds are ogg files, which need the Vorbis encoder.
        const RunResult encoders = Run(path, {"-hide_banner", "-encoders"}, 15000);
        if (encoders.output.find("libvorbis") == std::string::npos){
            tool.problem = tool.version + " can't write ogg files: it was built without libvorbis";
            return tool;
        }
        tool.usable = true;
        return tool;
    }

    Tool ProbeTimidity(const std::string& path){
        Tool tool;
        tool.path = path;
        const RunResult version = Run(path, {"-v"}, 15000);
        if (!version.started || !version.finished || Lower(version.output).find("timidity") == std::string::npos){
            tool.problem = path + " is not TiMidity (" + Describe(version) + ")";
            return tool;
        }
        tool.version = FirstLine(version.output);
        tool.usable = true;
        return tool;
    }

    Tool FindFfmpeg(const Locations& locations, const Environment& env){
        return FindTool("ffmpeg", locations.ffmpeg, locations, env, ProbeFfmpeg);
    }

    Tool FindTimidity(const Locations& locations, const Environment& env){
        return FindTool("timidity", locations.timidity, locations, env, ProbeTimidity);
    }

    std::vector<std::string> FfmpegArguments(const std::string& input, const std::string& output){
        return {"-hide_banner", "-loglevel", "panic", "-y", "-i", input, output};
    }

    std::vector<std::string> FfmpegStdinArguments(const std::string& output){
        return FfmpegArguments("-", output);
    }

    std::vector<std::string> TimidityArguments(const std::string& midi){
        return {"--quiet=3", midi, "-Ow", "-o", "-"};
    }

    namespace{
        std::string FrameName(const int frame){
            char number[16];
            snprintf(number, sizeof(number), "%05d", frame);
            return std::string(number) + ".jpg";
        }
    }

    bool ConvertToOgg(
      const std::string& ffmpeg, const std::string& input, const std::string& output, std::string* error
    ){
        const RunResult result = Run(ffmpeg, FfmpegArguments(input, output), -1);
        boost::system::error_code ec;
        if (result.started && result.finished && result.exit_code == 0 && fs::file_size(output, ec) > 0 && !ec)
            return true;
        fs::remove(output, ec);
        if (error) *error = "ffmpeg failed: " + Describe(result);
        return false;
    }

    bool ConvertMovie(
      const std::string& ffmpeg, const std::string& input, const std::string& directory,
      const std::string& name, const std::vector<uint8_t>& camera, std::string* error
    ){
        boost::system::error_code ec;
        fs::remove_all(directory, ec);
        fs::create_directories(directory, ec);
        // The movies of the original game have 15 frames per second.
        const float frames_per_second = 15.0f;
        const RunResult frames = Run(
          ffmpeg,
          {"-hide_banner", "-loglevel", "panic", "-y", "-i", input, "-an", "-q:v", "3",
           "-start_number", "0", directory + "/%05d.jpg"},
          -1
        );
        int frame_count = 0;
        while (fs::exists(directory + "/" + FrameName(frame_count), ec)) ++ frame_count;
        if (!frames.started || !frames.finished || frames.exit_code != 0 || frame_count == 0){
            fs::remove_all(directory, ec);
            if (error) *error = "ffmpeg failed: " + Describe(frames);
            return false;
        }
        // The sound. Some movies have none.
        const std::string ogg = directory + "/" + name + ".ogg";
        const RunResult sound = Run(
          ffmpeg, {"-hide_banner", "-loglevel", "panic", "-y", "-i", input, "-vn", ogg}, -1
        );
        if (!sound.started || !sound.finished || sound.exit_code != 0 || fs::file_size(ogg, ec) == 0 || ec)
            fs::remove(ogg, ec);
        std::ofstream info(directory + "/movie.xml");
        info << "<movie frames=\"" << frame_count << "\" fps=\"" << frames_per_second << "\"";
        const int first_camera_frame = MovieCamera::FirstCameraFrame(camera);
        if (first_camera_frame >= 0){
            info << " camera_frame=\"" << first_camera_frame << "\"";
            std::ofstream camera_file(directory + "/camera.cam", std::ios::binary);
            camera_file.write(reinterpret_cast<const char*>(camera.data()), camera.size());
        }
        info << "/>\n";
        return true;
    }

    bool ConvertMidiToOgg(
      const std::string& timidity, const std::string& ffmpeg, const std::string& midi, const std::string& output,
      std::string* error
    ){
        QProcess render;
        QProcess encode;
        // render | encode, without a shell.
        render.setStandardOutputProcess(&encode);
        render.setStandardErrorFile(QProcess::nullDevice());
        encode.setStandardOutputFile(QProcess::nullDevice());
        encode.setStandardErrorFile(QProcess::nullDevice());
        const auto to_list = [](const std::vector<std::string>& arguments){
            QStringList list;
            for (const std::string& argument : arguments) list << QString::fromStdString(argument);
            return list;
        };
        // The source is started first, as Qt documents for piped processes.
        render.start(QString::fromStdString(timidity), to_list(TimidityArguments(midi)));
        encode.start(QString::fromStdString(ffmpeg), to_list(FfmpegStdinArguments(output)));
        const bool render_started = render.waitForStarted(10000);
        const bool encode_started = encode.waitForStarted(10000);
        bool ok = false;
        std::string reason;
        if (!render_started || !encode_started){
            render.kill();
            encode.kill();
            render.waitForFinished(2000);
            encode.waitForFinished(2000);
            reason = std::string("could not start ") + (!render_started ? "timidity" : "ffmpeg");
        }
        else{
            render.waitForFinished(-1);
            encode.waitForFinished(-1);
            if (render.exitStatus() != QProcess::NormalExit || render.exitCode() != 0)
                reason = "timidity failed with exit code " + std::to_string(render.exitCode());
            else if (encode.exitStatus() != QProcess::NormalExit || encode.exitCode() != 0)
                reason = "ffmpeg failed with exit code " + std::to_string(encode.exitCode());
            else ok = true;
        }
        boost::system::error_code ec;
        if (ok && !(fs::file_size(output, ec) > 0 && !ec)){
            ok = false;
            reason = "no audio was produced";
        }
        if (!ok){
            fs::remove(output, ec);
            if (error) *error = reason;
        }
        return ok;
    }

}
