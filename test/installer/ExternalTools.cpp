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

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <QtCore/QCoreApplication>
#include <fstream>
#include <sstream>
#include "installer/ExternalTools.h"

namespace fs = boost::filesystem;
using namespace ExternalTools;

namespace{

    /**
     * The programs are run with Qt, which needs an application object.
     */
    struct QtApplication{
        int argc = 1;
        char name[6] = "tests";
        char* argv[2] = {name, nullptr};
        QCoreApplication application;
        QtApplication(): application(argc, argv){}
    };

    /**
     * A temporary directory that is removed on destruction.
     */
    struct Dir{
        fs::path path;
        explicit Dir(const std::string& name = ""): path(fs::temp_directory_path() / fs::unique_path("vgears-tools-%%%%-%%%%")){
            if (!name.empty()) path /= name;
            fs::create_directories(path);
        }
        ~Dir(){
            boost::system::error_code ec;
            fs::remove_all(path.parent_path().filename().string().compare(0, 13, "vgears-tools-") == 0 ? path.parent_path() : path, ec);
        }
        std::string Str() const{return path.string();}

        /**
         * Creates a file. With a mode of executable it can be run.
         */
        std::string File(const std::string& name, const std::string& content = "x", bool executable = true) const{
            const fs::path file = path / name;
            fs::create_directories(file.parent_path());
            std::ofstream(file.string()) << content;
            fs::permissions(file, executable ? fs::owner_all | fs::group_read | fs::others_read : fs::owner_read | fs::owner_write);
            return file.string();
        }
    };

    Environment Unix(const std::string& path){
        Environment env;
        env.path = path;
        env.windows = false;
        return env;
    }

    Environment Windows(const std::string& path, const std::string& pathext = ".COM;.EXE;.BAT;.CMD"){
        Environment env;
        env.path = path;
        env.pathext = pathext;
        env.windows = true;
        return env;
    }

#ifndef _WIN32
    /**
     * A shell script that behaves like FFmpeg.
     *
     * @param[in] vorbis True if it says it has the Vorbis encoder.
     */
    std::string FakeFfmpeg(const Dir& dir, bool vorbis = true, const std::string& name = "ffmpeg"){
        std::ostringstream script;
        script << "#!/bin/sh\n"
          << "if [ \"$1\" = \"-version\" ]; then echo \"ffmpeg version 6.0 Copyright (c) the FFmpeg developers\"; exit 0; fi\n"
          << "if [ \"$2\" = \"-encoders\" ]; then echo \" A....D " << (vorbis ? "libvorbis" : "libmp3lame")
          << "  encoder\"; exit 0; fi\n"
          // A conversion: -hide_banner -loglevel panic -y -i INPUT OUTPUT.
          << "input=\"$6\"\nfor last; do :; done\n"
          << "if [ \"$input\" = \"-\" ]; then cat > \"$last\"; else cat \"$input\" > \"$last\"; fi\n";
        return dir.File(name, script.str());
    }

    /**
     * A shell script that behaves like TiMidity: it "renders" the file by printing it with a header.
     */
    std::string FakeTimidity(const Dir& dir, bool works = true){
        std::ostringstream script;
        script << "#!/bin/sh\n"
          << "if [ \"$1\" = \"-v\" ]; then echo \"TiMidity++ version 2.15.0\"; exit 0; fi\n"
          << (works ? "echo RENDERED; cat \"$2\"; exit 0\n" : "exit 3\n");
        return dir.File("timidity", script.str());
    }

    std::string Read(const std::string& path){
        std::ifstream file(path);
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
#endif

}

BOOST_GLOBAL_FIXTURE(QtApplication);

// ---------------------------------------------------------------------------------------------------------------
// Searching.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(ExternalToolsSearchTests)

BOOST_AUTO_TEST_CASE(SearchesThePathInOrder){
    Dir first("a");
    Dir second("b");
    const std::string in_first = first.File("ffmpeg");
    const std::string in_second = second.File("ffmpeg");
    const auto found = Candidates("ffmpeg", "", {}, Unix(first.Str() + ":" + second.Str()));
    BOOST_REQUIRE_EQUAL(found.size(), 2u);
    BOOST_CHECK_EQUAL(found[0], in_first);
    BOOST_CHECK_EQUAL(found[1], in_second);
}

BOOST_AUTO_TEST_CASE(DirectoriesComeBeforeThePathAndTheSavedPathBeforeBoth){
    Dir saved;
    Dir tools;
    Dir path_dir;
    const std::string chosen = saved.File("my-ffmpeg");
    const std::string in_tools = tools.File("ffmpeg");
    const std::string in_path = path_dir.File("ffmpeg");
    const auto found = Candidates("ffmpeg", chosen, {tools.Str()}, Unix(path_dir.Str()));
    BOOST_REQUIRE_EQUAL(found.size(), 3u);
    BOOST_CHECK_EQUAL(found[0], chosen);
    BOOST_CHECK_EQUAL(found[1], in_tools);
    BOOST_CHECK_EQUAL(found[2], in_path);
}

BOOST_AUTO_TEST_CASE(SkipsWhatCantBeRun){
    Dir dir;
    dir.File("not-executable", "x", false);
    fs::create_directories(dir.path / "ffmpeg");
    // A directory with the name, and a file without the permission to run it.
    BOOST_CHECK(Candidates("ffmpeg", "", {}, Unix(dir.Str())).empty());
    BOOST_CHECK(Candidates("not-executable", "", {}, Unix(dir.Str())).empty());
    // A saved path that doesn't exist is ignored, not returned.
    BOOST_CHECK(Candidates("ffmpeg", dir.Str() + "/missing", {}, Unix("")).empty());
}

BOOST_AUTO_TEST_CASE(IgnoresEmptyAndRelativePathEntries){
    Dir dir;
    dir.File("ffmpeg");
    const std::string relative = fs::relative(dir.path, fs::current_path()).string();
    // An empty entry means the current directory, and relative ones depend on where the installer is run from.
    BOOST_CHECK(Candidates("ffmpeg", "", {}, Unix(":" + relative + "::")).empty());
    BOOST_CHECK_EQUAL(Candidates("ffmpeg", "", {}, Unix(":" + dir.Str() + ":")).size(), 1u);
}

BOOST_AUTO_TEST_CASE(NoDuplicates){
    Dir dir;
    dir.File("ffmpeg");
    BOOST_CHECK_EQUAL(Candidates("ffmpeg", dir.Str() + "/ffmpeg", {dir.Str()}, Unix(dir.Str())).size(), 1u);
}

BOOST_AUTO_TEST_CASE(PathsWithSpaces){
    Dir dir("Program Files (x86)/with space");
    const std::string file = dir.File("ffmpeg");
    const auto found = Candidates("ffmpeg", "", {}, Unix(dir.Str()));
    BOOST_REQUIRE_EQUAL(found.size(), 1u);
    BOOST_CHECK_EQUAL(found[0], file);
}

BOOST_AUTO_TEST_CASE(WindowsUsesPathExt){
    Dir bin;
    const std::string exe = bin.File("ffmpeg.exe", "x", false);
    bin.File("ffmpeg.txt", "x", false);
    // Windows doesn't have the executable permission: the extension decides.
    const auto found = Candidates("ffmpeg", "", {}, Windows(bin.Str()));
    BOOST_REQUIRE_EQUAL(found.size(), 1u);
    BOOST_CHECK_EQUAL(found[0], exe);
}

BOOST_AUTO_TEST_CASE(WindowsExtensionsAreTriedInOrderAndCaseDoesNotMatter){
    Dir bin;
    const std::string cmd = bin.File("timidity.cmd", "x", false);
    const std::string upper = bin.File("TIMIDITY.EXE", "x", false);
    const auto found = Candidates("timidity", "", {}, Windows(bin.Str(), ".exe;.cmd"));
    BOOST_REQUIRE_EQUAL(found.size(), 2u);
    BOOST_CHECK_EQUAL(found[0], upper);
    BOOST_CHECK_EQUAL(found[1], cmd);
}

BOOST_AUTO_TEST_CASE(WindowsUsesSemicolons){
    Dir a("a");
    Dir b("b");
    a.File("ffmpeg.exe", "x", false);
    b.File("ffmpeg.exe", "x", false);
    // A drive letter has a colon in it: entries are split with ';' only.
    BOOST_CHECK_EQUAL(Candidates("ffmpeg", "", {}, Windows(a.Str() + ";" + b.Str())).size(), 2u);
    BOOST_CHECK_EQUAL(Candidates("ffmpeg", "", {}, Windows("")).size(), 0u);
}

BOOST_AUTO_TEST_CASE(EnvironmentOfTheSystem){
    const Environment env = Environment::FromSystem();
#ifdef _WIN32
    BOOST_CHECK(env.windows);
    BOOST_CHECK(!env.pathext.empty());
#else
    BOOST_CHECK(!env.windows);
#endif
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------------------------------------------------------------------------------
// Arguments.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(ExternalToolsArgumentTests)

BOOST_AUTO_TEST_CASE(FfmpegArgumentsDontNeedQuoting){
    const std::vector<std::string> expected = {
      "-hide_banner", "-loglevel", "panic", "-y", "-i", "/path with spaces/in put.wav", "/out dir/o'ut.ogg"
    };
    BOOST_CHECK(FfmpegArguments("/path with spaces/in put.wav", "/out dir/o'ut.ogg") == expected);
}

BOOST_AUTO_TEST_CASE(FfmpegReadsStdin){
    const std::vector<std::string> expected = {"-hide_banner", "-loglevel", "panic", "-y", "-i", "-", "out.ogg"};
    BOOST_CHECK(FfmpegStdinArguments("out.ogg") == expected);
}

BOOST_AUTO_TEST_CASE(ShortVersions){
    Tool ffmpeg;
    ffmpeg.version = "ffmpeg version 9.0.2-1 Copyright (c) 2000-2026 the FFmpeg developers";
    BOOST_CHECK_EQUAL(ShortVersion(ffmpeg), "9.0.2-1");
    Tool timidity;
    timidity.version = "TiMidity++ version 2.14.0";
    BOOST_CHECK_EQUAL(ShortVersion(timidity), "2.14.0");
    // Builds from git have a long version, and some tools say it with another case.
    Tool git;
    git.version = "FFmpeg Version N-12345-gabcdef Copyright";
    BOOST_CHECK_EQUAL(ShortVersion(git), "N-12345-gabcdef");
    // Without the word it is the line as it is, and nothing is nothing.
    Tool odd;
    odd.version = "something 1.0";
    BOOST_CHECK_EQUAL(ShortVersion(odd), "something 1.0");
    BOOST_CHECK_EQUAL(ShortVersion(Tool()), "");
}

BOOST_AUTO_TEST_CASE(TimidityWritesToStdout){
    const std::vector<std::string> expected = {"--quiet=3", "my song.mid", "-Ow", "-o", "-"};
    BOOST_CHECK(TimidityArguments("my song.mid") == expected);
}

BOOST_AUTO_TEST_SUITE_END()

#ifndef _WIN32

// ---------------------------------------------------------------------------------------------------------------
// Running programs (with scripts that behave like them).
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(ExternalToolsRunTests)

BOOST_AUTO_TEST_CASE(RunsAndCapturesOutput){
    Dir dir;
    const std::string script = dir.File("hello", "#!/bin/sh\necho \"hello $1\"\nexit 4\n");
    const RunResult result = Run(script, {"world"}, 10000);
    BOOST_CHECK(result.started && result.finished);
    BOOST_CHECK_EQUAL(result.exit_code, 4);
    BOOST_CHECK_EQUAL(result.output, "hello world\n");
}

BOOST_AUTO_TEST_CASE(ArgumentsArriveUnchanged){
    Dir dir;
    const std::string script = dir.File("args", "#!/bin/sh\nfor a in \"$@\"; do echo \"[$a]\"; done\n");
    const RunResult result = Run(script, {"a b", "c'd", "$HOME", "e;f"}, 10000);
    // No shell is involved: spaces, quotes and special characters are not interpreted.
    BOOST_CHECK_EQUAL(result.output, "[a b]\n[c'd]\n[$HOME]\n[e;f]\n");
}

BOOST_AUTO_TEST_CASE(ProgramThatDoesNotExist){
    const RunResult result = Run("/nonexistent/program", {}, 10000);
    BOOST_CHECK(!result.started);
}

BOOST_AUTO_TEST_CASE(KillsWhatTakesTooLong){
    Dir dir;
    const std::string script = dir.File("slow", "#!/bin/sh\nsleep 30\n");
    const RunResult result = Run(script, {}, 200);
    BOOST_CHECK(result.started);
    BOOST_CHECK(!result.finished);
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------------------------------------------------------------------------------
// Probing and finding.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(ExternalToolsProbeTests)

BOOST_AUTO_TEST_CASE(AGoodFfmpegIsUsable){
    Dir dir;
    const Tool tool = ProbeFfmpeg(FakeFfmpeg(dir));
    BOOST_CHECK(tool.usable);
    BOOST_CHECK(tool.problem.empty());
    BOOST_CHECK_EQUAL(tool.version, "ffmpeg version 6.0 Copyright (c) the FFmpeg developers");
}

BOOST_AUTO_TEST_CASE(FfmpegWithoutVorbisIsNotUsable){
    Dir dir;
    const Tool tool = ProbeFfmpeg(FakeFfmpeg(dir, false));
    BOOST_CHECK(!tool.usable);
    BOOST_CHECK(tool.problem.find("libvorbis") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(ProgramsThatAreNotFfmpeg){
    Dir dir;
    BOOST_CHECK(!ProbeFfmpeg(dir.File("fake", "#!/bin/sh\nexit 1\n")).usable);
    BOOST_CHECK(!ProbeFfmpeg(dir.File("other", "#!/bin/sh\necho hello\n")).usable);
    BOOST_CHECK(!ProbeFfmpeg(dir.File("garbage", "not a program")).usable);
    BOOST_CHECK(!ProbeFfmpeg(dir.Str() + "/missing").usable);
}

BOOST_AUTO_TEST_CASE(TimidityProbe){
    Dir dir;
    const Tool tool = ProbeTimidity(FakeTimidity(dir));
    BOOST_CHECK(tool.usable);
    BOOST_CHECK_EQUAL(tool.version, "TiMidity++ version 2.15.0");
    BOOST_CHECK(!ProbeTimidity(dir.File("fake", "#!/bin/sh\nexit 1\n")).usable);
    BOOST_CHECK(!ProbeTimidity(FakeFfmpeg(dir)).usable);
}

BOOST_AUTO_TEST_CASE(FindingFollowsThePriorityAndSkipsBrokenOnes){
    Dir broken_dir("broken");
    Dir good_dir("good");
    const std::string broken = FakeFfmpeg(broken_dir, false);
    const std::string good = FakeFfmpeg(good_dir);
    Locations locations;
    locations.ffmpeg = broken;
    // The user's choice doesn't work (no Vorbis), so the next one is used.
    Tool tool = FindFfmpeg(locations, Unix(good_dir.Str()));
    BOOST_CHECK(tool.usable);
    BOOST_CHECK_EQUAL(tool.path, good);
    // A choice that works wins over the PATH.
    locations.ffmpeg = good;
    BOOST_CHECK_EQUAL(FindFfmpeg(locations, Unix(broken_dir.Str())).path, good);
}

BOOST_AUTO_TEST_CASE(FindingInTheToolsDirectories){
    Dir tools;
    const std::string ffmpeg = FakeFfmpeg(tools);
    const std::string timidity = FakeTimidity(tools);
    Locations locations;
    locations.directories = {tools.Str()};
    BOOST_CHECK_EQUAL(FindFfmpeg(locations, Unix("")).path, ffmpeg);
    BOOST_CHECK_EQUAL(FindTimidity(locations, Unix("")).path, timidity);
}

BOOST_AUTO_TEST_CASE(NothingFound){
    Dir empty;
    Locations locations;
    const Tool tool = FindFfmpeg(locations, Unix(empty.Str()));
    BOOST_CHECK(!tool.usable);
    BOOST_CHECK(tool.path.empty());
    BOOST_CHECK(tool.problem.find("not found") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(ABrokenProgramIsReportedInsteadOfNotFound){
    Dir dir;
    FakeFfmpeg(dir, false);
    const Tool tool = FindFfmpeg(Locations(), Unix(dir.Str()));
    BOOST_CHECK(!tool.usable);
    BOOST_CHECK(!tool.path.empty());
    BOOST_CHECK(tool.problem.find("libvorbis") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------------------------------------------------------------------------------
// Converting.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(ExternalToolsConvertTests)

BOOST_AUTO_TEST_CASE(ConvertsToOgg){
    Dir tools;
    Dir files("Final Fantasy VII (PC, 1998)");
    const std::string ffmpeg = FakeFfmpeg(tools);
    const std::string input = files.File("1 sound.wav", "SOUNDDATA");
    const std::string output = (files.path / "1 sound.ogg").string();
    std::string error;
    BOOST_CHECK_MESSAGE(ConvertToOgg(ffmpeg, input, output, &error), error);
    BOOST_CHECK_EQUAL(Read(output), "SOUNDDATA");
}

BOOST_AUTO_TEST_CASE(FailedConversionLeavesNothingBehind){
    Dir tools;
    Dir files;
    const std::string failing = tools.File("ffmpeg", "#!/bin/sh\nfor last; do :; done\necho partial > \"$last\"\nexit 1\n");
    const std::string input = files.File("a.wav", "x");
    const std::string output = (files.path / "a.ogg").string();
    std::string error;
    BOOST_CHECK(!ConvertToOgg(failing, input, output, &error));
    BOOST_CHECK(!fs::exists(output));
    BOOST_CHECK(error.find("exit code 1") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(EmptyOutputIsAFailure){
    Dir tools;
    Dir files;
    const std::string silent = tools.File("ffmpeg", "#!/bin/sh\nfor last; do :; done\n: > \"$last\"\nexit 0\n");
    const std::string output = (files.path / "a.ogg").string();
    BOOST_CHECK(!ConvertToOgg(silent, files.File("a.wav"), output));
    BOOST_CHECK(!fs::exists(output));
}

BOOST_AUTO_TEST_CASE(ConvertsMidiThroughAPipe){
    Dir tools;
    Dir files("my music");
    const std::string timidity = FakeTimidity(tools);
    const std::string ffmpeg = FakeFfmpeg(tools);
    const std::string midi = files.File("5.mid", "MIDIDATA");
    const std::string output = (files.path / "5.ogg").string();
    std::string error;
    BOOST_CHECK_MESSAGE(ConvertMidiToOgg(timidity, ffmpeg, midi, output, &error), error);
    // What TiMidity wrote reached FFmpeg, which wrote the file.
    BOOST_CHECK_EQUAL(Read(output), "RENDERED\nMIDIDATA");
}

BOOST_AUTO_TEST_CASE(MidiConversionFailsIfTimidityFails){
    Dir tools;
    Dir files;
    const std::string timidity = FakeTimidity(tools, false);
    const std::string ffmpeg = FakeFfmpeg(tools);
    const std::string output = (files.path / "5.ogg").string();
    std::string error;
    BOOST_CHECK(!ConvertMidiToOgg(timidity, ffmpeg, files.File("5.mid"), output, &error));
    BOOST_CHECK(!fs::exists(output));
    BOOST_CHECK(error.find("timidity failed") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(MidiConversionFailsIfProgramsAreMissing){
    Dir files;
    const std::string output = (files.path / "5.ogg").string();
    std::string error;
    BOOST_CHECK(!ConvertMidiToOgg("/nonexistent/timidity", "/nonexistent/ffmpeg", files.File("5.mid"), output, &error));
    BOOST_CHECK(!fs::exists(output));
    BOOST_CHECK(!error.empty());
}

BOOST_AUTO_TEST_SUITE_END()

#endif
