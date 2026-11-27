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

#define BOOST_TEST_MODULE VGearsInstallerLogic
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <fstream>
#include "installer/cli/InstallCommandLine.h"
#include "installer/steps/InstallLayout.h"
#include "installer/steps/InstallLog.h"
#include "installer/steps/InstallOptions.h"
#include "installer/steps/InstallVerifier.h"
#include "common/InstallStamp.h"
#include "common/OgrePlugins.h"
#include "installer/steps/TaskPool.h"
#include "installer/steps/VideoFiles.h"
#include <atomic>
#include <chrono>
#include <set>

namespace fs = boost::filesystem;

namespace{

    bool Contains(const std::vector<std::string>& list, const std::string& value){
        return std::find(list.begin(), list.end(), value) != list.end();
    }

    bool StartsWith(const std::string& text, const std::string& prefix){return text.compare(0, prefix.size(), prefix) == 0;}

    /**
     * A temporary directory that is removed on destruction.
     */
    struct TempDir{
        fs::path path;
        TempDir(): path(fs::temp_directory_path() / fs::unique_path("vgears-test-%%%%-%%%%")){
            fs::create_directories(path);
        }
        ~TempDir(){
            boost::system::error_code ec;
            fs::remove_all(path, ec);
        }
        std::string Root() const{return path.string() + "/";}
        void Touch(const std::string& relative) const{
            fs::create_directories((path / relative).parent_path());
            std::ofstream(( path / relative).string()) << "x";
        }
    };

    bool HasProblem(const VerifyResult& result, const std::string& group){
        for (const VerifyProblem& problem : result.problems) if (problem.group == group) return true;
        return false;
    }

}

// ---------------------------------------------------------------------------------------------------------------
// Layout: where things go.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(InstallLayoutTests)

BOOST_AUTO_TEST_CASE(GameAndTempAreInsideTheInstallationDirectory){
    BOOST_CHECK_EQUAL(InstallLayout::GameDir("/home/u/.v-gears/"), "/home/u/.v-gears/game/");
    BOOST_CHECK_EQUAL(InstallLayout::TempDir("/home/u/.v-gears/"), "/home/u/.v-gears/temp/");
}

BOOST_AUTO_TEST_CASE(GameSubdirsAreRelativeAndUnique){
    const auto& dirs = InstallLayout::GameSubdirs();
    for (const std::string& dir : dirs){
        BOOST_CHECK_MESSAGE(!dir.empty() && dir[0] != '/', "not relative: " << dir);
        BOOST_CHECK_MESSAGE(!StartsWith(dir, "game/") && !StartsWith(dir, "temp"), "wrong root: " << dir);
        BOOST_CHECK_MESSAGE(std::count(dirs.begin(), dirs.end(), dir) == 1, "duplicated: " << dir);
    }
}

BOOST_AUTO_TEST_CASE(EveryDirectoryTheGameReadsIsCreated){
    // The game reads "./engine/<x>", which is shipped with V-Gears, and "./game/<x>", which the installer must create.
    for (const std::string& resource : InstallLayout::GameResourceDirs()){
        if (StartsWith(resource, "engine/")){
            BOOST_CHECK_MESSAGE(
              fs::is_directory(fs::path(VGEARS_SOURCE_DIR) / "data" / resource), "not shipped: " << resource
            );
        }
        else{
            BOOST_REQUIRE_MESSAGE(StartsWith(resource, "game/"), "unexpected root: " << resource);
            const std::string dir = resource.substr(5);
            bool created = false;
            for (const std::string& sub : InstallLayout::GameSubdirs())
                created = created || sub == dir || StartsWith(sub, dir + "/");
            BOOST_CHECK_MESSAGE(created, "the installer doesn't create " << resource);
        }
    }
}

BOOST_AUTO_TEST_CASE(OldSettingsAreMigrated){
    // The data directory used to be <installation directory>/data.
    BOOST_CHECK_EQUAL(InstallLayout::MigrateInstallDir("/home/u/.v-gears/data", "", "/def"), "/home/u/.v-gears");
    BOOST_CHECK_EQUAL(InstallLayout::MigrateInstallDir("/home/u/.v-gears/data/", "/other", "/def"), "/home/u/.v-gears");
    // A data directory that isn't called "data" is kept as is.
    BOOST_CHECK_EQUAL(InstallLayout::MigrateInstallDir("/games/ff7", "", "/def"), "/games/ff7");
    // Without a data directory the configuration directory is used, and then the default.
    BOOST_CHECK_EQUAL(InstallLayout::MigrateInstallDir("", "/home/u/.v-gears", "/def"), "/home/u/.v-gears");
    BOOST_CHECK_EQUAL(InstallLayout::MigrateInstallDir("", "", "/def"), "/def");
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------------------------------------------------------------------------------
// Options: dependencies, presets, estimates.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(InstallOptionsTests)

BOOST_AUTO_TEST_CASE(ModelsNeedTheirMaps){
    InstallOptions o;
    o.field_models = true;
    o.wm_models = true;
    InstallRules::ApplyDependencies(o);
    BOOST_CHECK(!o.field_models);
    BOOST_CHECK(!o.wm_models);

    o.fields = o.wm = o.field_models = o.wm_models = true;
    InstallRules::ApplyDependencies(o);
    BOOST_CHECK(o.field_models);
    BOOST_CHECK(o.wm_models);
}

BOOST_AUTO_TEST_CASE(OneMapDoesNotEnableTheOtherModels){
    InstallOptions o;
    o.fields = true;
    o.wm_models = true;
    InstallRules::ApplyDependencies(o);
    BOOST_CHECK(!o.wm_models);
}

BOOST_AUTO_TEST_CASE(SelectableMatchesDependencies){
    // An option is selectable exactly when applying the rules to it keeps it.
    for (int bits = 0; bits < (1 << 2); ++ bits){
        InstallOptions o;
        o.fields = bits & 1;
        o.wm = bits & 2;
        o.field_models = o.wm_models = true;
        const InstallOptions selectable = InstallRules::Selectable(o);
        InstallRules::ApplyDependencies(o);
        BOOST_CHECK_EQUAL(o.field_models, selectable.field_models);
        BOOST_CHECK_EQUAL(o.wm_models, selectable.wm_models);
    }
}

BOOST_AUTO_TEST_CASE(PresetsRespectTheRules){
    for (InstallPreset preset : {
      InstallPreset::EVERYTHING, InstallPreset::FIELDS, InstallPreset::WORLD_MAP, InstallPreset::BATTLE,
      InstallPreset::MEDIA, InstallPreset::CUSTOM
    }){
        for (bool videos : {false, true}){
            InstallOptions o = InstallRules::Preset(preset, videos);
            InstallOptions checked = o;
            InstallRules::ApplyDependencies(checked);
            BOOST_CHECK(o.field_models == checked.field_models && o.wm_models == checked.wm_models);
            // Videos are only chosen when the disk has them.
            if (!videos) BOOST_CHECK(!o.videos);
        }
    }
}

BOOST_AUTO_TEST_CASE(PresetContents){
    const InstallOptions all = InstallRules::Preset(InstallPreset::EVERYTHING, true);
    BOOST_CHECK(all.battle_data && all.battle_models && all.kernel && all.images && all.sounds && all.music);
    BOOST_CHECK(all.fields && all.field_models && all.wm && all.wm_models && all.videos);

    const InstallOptions world = InstallRules::Preset(InstallPreset::WORLD_MAP, true);
    BOOST_CHECK(world.wm && world.wm_models);
    BOOST_CHECK(!world.fields && !world.field_models && !world.music && !world.videos);

    const InstallOptions fields = InstallRules::Preset(InstallPreset::FIELDS, true);
    BOOST_CHECK(fields.fields && fields.field_models && !fields.wm);

    const InstallOptions none = InstallRules::Preset(InstallPreset::CUSTOM, true);
    BOOST_CHECK(!none.fields && !none.wm && !none.music && !none.sounds && !none.videos);
}

BOOST_AUTO_TEST_CASE(ToolsNeededByTheAudio){
    InstallOptions none;
    BOOST_CHECK(!InstallRules::ToolsNeeded(none).ffmpeg && !InstallRules::ToolsNeeded(none).timidity);

    InstallOptions sounds;
    sounds.sounds = true;
    BOOST_CHECK(InstallRules::ToolsNeeded(sounds).ffmpeg);
    BOOST_CHECK(!InstallRules::ToolsNeeded(sounds).timidity);

    // The music needs both: TiMidity renders the MIDI and FFmpeg encodes it.
    InstallOptions music;
    music.music = true;
    BOOST_CHECK(InstallRules::ToolsNeeded(music).ffmpeg && InstallRules::ToolsNeeded(music).timidity);

    // Nothing else needs them.
    InstallOptions other;
    other.fields = other.field_models = other.wm = other.battle_data = other.images = other.kernel = true;
    BOOST_CHECK(!InstallRules::ToolsNeeded(other).ffmpeg && !InstallRules::ToolsNeeded(other).timidity);
}

BOOST_AUTO_TEST_CASE(WhatIsLostWithoutTheTools){
    InstallOptions all;
    all.sounds = all.music = true;
    // Nothing is lost when the programs work, or when nothing needs them.
    BOOST_CHECK(InstallRules::ToolConsequences(all, true, true).empty());
    BOOST_CHECK(InstallRules::ToolConsequences(InstallOptions(), false, false).empty());

    // Without FFmpeg, both the sounds and the music are lost.
    const auto no_ffmpeg = InstallRules::ToolConsequences(all, false, true);
    BOOST_REQUIRE_EQUAL(no_ffmpeg.size(), 2u);
    BOOST_CHECK(no_ffmpeg[0].find("sound") != std::string::npos);
    BOOST_CHECK(no_ffmpeg[1].find("music") != std::string::npos);

    // Without TiMidity only the music is lost.
    const auto no_timidity = InstallRules::ToolConsequences(all, true, false);
    BOOST_REQUIRE_EQUAL(no_timidity.size(), 1u);
    BOOST_CHECK(no_timidity[0].find("music") != std::string::npos);
    BOOST_CHECK(no_timidity[0].find("TiMidity") != std::string::npos);

    // Without both it is not said twice.
    BOOST_CHECK_EQUAL(InstallRules::ToolConsequences(all, false, false).size(), 2u);

    // Sound effects don't need TiMidity.
    InstallOptions sounds;
    sounds.sounds = true;
    BOOST_CHECK(InstallRules::ToolConsequences(sounds, true, false).empty());
    BOOST_CHECK_EQUAL(InstallRules::ToolConsequences(sounds, false, true).size(), 1u);

    // Music alone, without FFmpeg.
    InstallOptions music;
    music.music = true;
    const auto music_only = InstallRules::ToolConsequences(music, false, true);
    BOOST_REQUIRE_EQUAL(music_only.size(), 1u);
    BOOST_CHECK(music_only[0].find("music") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(WorldMapNeedsFields){
    InstallOptions o;
    o.wm = true;
    BOOST_CHECK_EQUAL(InstallRules::Problems(o, false).size(), 1u);
    // Fine if the fields are installed now or were installed before.
    BOOST_CHECK(InstallRules::Problems(o, true).empty());
    o.fields = true;
    BOOST_CHECK(InstallRules::Problems(o, false).empty());
    // No world map, no problem.
    BOOST_CHECK(InstallRules::Problems(InstallOptions(), false).empty());
}

BOOST_AUTO_TEST_CASE(Estimates){
    uint64_t temporary = 99;
    BOOST_CHECK_EQUAL(InstallRules::EstimateMegabytes(InstallOptions(), &temporary), 0u);
    BOOST_CHECK_EQUAL(temporary, 0u);

    // Selecting more never makes it smaller.
    const InstallOptions all = InstallRules::Preset(InstallPreset::EVERYTHING, false);
    uint64_t all_temporary = 0;
    const uint64_t all_size = InstallRules::EstimateMegabytes(all, &all_temporary);
    for (InstallPreset preset : {InstallPreset::FIELDS, InstallPreset::WORLD_MAP, InstallPreset::BATTLE, InstallPreset::MEDIA}){
        uint64_t t = 0;
        BOOST_CHECK_LE(InstallRules::EstimateMegabytes(InstallRules::Preset(preset, false), &t), all_size);
        BOOST_CHECK_LE(t, all_temporary);
    }
    // The pointer is optional.
    BOOST_CHECK_EQUAL(InstallRules::EstimateMegabytes(all), all_size);
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------------------------------------------------------------------------------
// Verifier.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(InstallVerifierTests)

BOOST_AUTO_TEST_CASE(EmptyDirectoryMissesEverything){
    TempDir dir;
    const VerifyResult result = VerifyInstallation(dir.Root());
    BOOST_CHECK(!result.Ok());
    for (const char* group : {
      "Configuration", "Game files", "Kernel data", "Battle data", "Battle models", "Field maps", "Field models",
      "World map", "World map models", "Sound effects", "Music", "Images", "Fonts"
    }) BOOST_CHECK_MESSAGE(HasProblem(result, group), "no problem reported for " << group);
    BOOST_CHECK_GT(result.checked, 0);
}

BOOST_AUTO_TEST_CASE(ConfigurationAndPlugins){
    TempDir dir;
    BOOST_CHECK(HasProblem(VerifyInstallation(dir.Root()), "Configuration"));
    // A plugin that can't be in the system, so only the given directories count.
    const std::string plugin = "Plugin_VGearsVerifierTest";
    VerifyPlugins search;
    search.ogre_version = "1.2.3";
    const std::string file_name = VGears::OgrePlugins::FileNames(plugin, search.ogre_version).front();
    std::ofstream(dir.path.string() + "/plugins.cfg") << "Plugin=" << plugin << "\n";
    dir.Touch("resources.cfg");
    BOOST_CHECK(HasProblem(VerifyInstallation(dir.Root(), "", search), "Configuration"));
    // Without the Ogre version the plugins are not checked.
    BOOST_CHECK(!HasProblem(VerifyInstallation(dir.Root()), "Configuration"));
    // Next to the executable, as the game looks for it there too.
    TempDir executable;
    executable.Touch(file_name);
    search.executable_dir = executable.path.string();
    BOOST_CHECK(!HasProblem(VerifyInstallation(dir.Root(), "", search), "Configuration"));
    // In the installation directory.
    search.executable_dir.clear();
    dir.Touch(file_name);
    BOOST_CHECK(!HasProblem(VerifyInstallation(dir.Root(), "", search), "Configuration"));
}

BOOST_AUTO_TEST_CASE(RootWithoutTrailingSlash){
    TempDir dir;
    dir.Touch("plugins.cfg");
    const VerifyResult with = VerifyInstallation(dir.Root());
    const VerifyResult without = VerifyInstallation(dir.path.string());
    BOOST_CHECK_EQUAL(with.problems.size(), without.problems.size());
}

BOOST_AUTO_TEST_CASE(FilesOfTheDataFolderMustBeCopied){
    TempDir installation;
    TempDir reference;
    reference.Touch("plugins.cfg");
    reference.Touch("engine/scripts/a.lua");
    BOOST_CHECK(HasProblem(VerifyInstallation(installation.Root(), reference.Root()), "Copied data files"));
    installation.Touch("plugins.cfg");
    BOOST_CHECK(HasProblem(VerifyInstallation(installation.Root(), reference.Root()), "Copied data files"));
    installation.Touch("engine/scripts/a.lua");
    BOOST_CHECK(!HasProblem(VerifyInstallation(installation.Root(), reference.Root()), "Copied data files"));
    // Without a reference there is nothing to compare.
    BOOST_CHECK(!HasProblem(VerifyInstallation(installation.Root()), "Copied data files"));
}

BOOST_AUTO_TEST_CASE(CountsFilesInsteadOfJustLookingForOne){
    TempDir dir;
    for (int i = 0; i < 3; ++ i) dir.Touch("game/field/maps/f" + std::to_string(i) + "/map.xml");
    // Three fields are not an installation of the fields.
    BOOST_CHECK(HasProblem(VerifyInstallation(dir.Root()), "Field maps"));
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------------------------------------------------------------------------------
// Log.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(InstallLogTests)

BOOST_AUTO_TEST_CASE(Severity){
    BOOST_CHECK_EQUAL(InstallLog::Classify("[ERROR] Skip field: x"), InstallLog::ERROR_LEVEL);
    BOOST_CHECK_EQUAL(InstallLog::Classify("Exception: Bone not found"), InstallLog::ERROR_LEVEL);
    BOOST_CHECK_EQUAL(InstallLog::Classify("[WARNING] Texture not found"), InstallLog::WARNING);
    BOOST_CHECK_EQUAL(InstallLog::Classify("Converting field models..."), InstallLog::INFO);
    BOOST_CHECK_EQUAL(InstallLog::Classify(""), InstallLog::INFO);
}

BOOST_AUTO_TEST_CASE(CountsFileAndSink){
    TempDir dir;
    InstallLog& log = InstallLog::Get();
    std::vector<std::string> shown;
    log.Open(dir.Root() + "installer.log");
    log.SetSink([&shown](InstallLog::Level, const std::string& message){shown.push_back(message);});
    log.Add("Starting");
    log.Add("[WARNING] careful");
    log.Add("[ERROR] broken");
    // The same problem reported twice in a row counts once.
    log.Add("[ERROR] broken");
    log.Add("hidden", false);
    // What the installer prints is captured too, and only problems reach the sink.
    std::cerr << "[ERROR] printed" << std::endl;
    std::cout << "plain output" << std::endl;
    BOOST_CHECK_EQUAL(log.Count(InstallLog::ERROR_LEVEL), 2u);
    BOOST_CHECK_EQUAL(log.Count(InstallLog::WARNING), 1u);
    BOOST_CHECK(Contains(shown, "Starting"));
    BOOST_CHECK(!Contains(shown, "hidden"));
    BOOST_CHECK(!Contains(shown, "plain output"));
    BOOST_CHECK(Contains(shown, "[ERROR] printed"));
    BOOST_REQUIRE_EQUAL(log.First(InstallLog::ERROR_LEVEL, 1).size(), 1u);
    BOOST_CHECK_EQUAL(log.First(InstallLog::ERROR_LEVEL, 1)[0], "[ERROR] broken");
    log.SetSink(nullptr);
    log.Close();

    std::ifstream file(dir.Root() + "installer.log");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    BOOST_CHECK(content.find("careful") != std::string::npos);
    BOOST_CHECK(content.find("hidden") != std::string::npos);
    BOOST_CHECK(content.find("plain output") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------------------------------------------------------------------------------
// Steps of the log.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(InstallLogStepTests)

BOOST_AUTO_TEST_CASE(StepsReportTheirOwnProblems){
    TempDir dir;
    InstallLog& log = InstallLog::Get();
    std::vector<std::string> shown;
    log.Open(dir.Root() + "installer.log");
    log.SetSink([&shown](InstallLog::Level, const std::string& message){shown.push_back(message);});

    log.BeginStep("Clean step");
    log.Add("fine");
    log.BeginStep("Bad step");
    log.Add("[ERROR] one");
    log.Add("[ERROR] two");
    log.Add("[WARNING] three");
    log.BeginStep("Next step");
    log.Add("[WARNING] four");
    log.EndStep();
    log.SetSink(nullptr);
    log.Close();

    auto summary = [&shown](const std::string& step){
        for (const std::string& line : shown) if (line.find("\"" + step + "\" finished") != std::string::npos) return line;
        return std::string();
    };
    // Steps without problems are not mentioned, the others only count their own.
    BOOST_CHECK(summary("Clean step").empty());
    BOOST_CHECK(summary("Bad step").find("2 error(s) and 1 warning(s)") != std::string::npos);
    BOOST_CHECK(summary("Next step").find("0 error(s) and 1 warning(s)") != std::string::npos);
    // The summaries don't count as problems themselves.
    BOOST_CHECK_EQUAL(InstallLog::Classify(summary("Bad step")), InstallLog::INFO);
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------------------------------------------------------------------------------
// Task pool.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(TaskPoolTests)

BOOST_AUTO_TEST_CASE(RunsEveryTask){
    std::atomic<int> done(0);
    TaskPool pool(4);
    for (int i = 0; i < 200; ++ i) pool.Run([&done](){++ done;});
    pool.Wait();
    BOOST_CHECK_EQUAL(done.load(), 200);
}

BOOST_AUTO_TEST_CASE(WaitWithoutTasksReturns){
    TaskPool pool(2);
    pool.Wait();
    BOOST_CHECK_EQUAL(pool.Threads(), 2u);
}

BOOST_AUTO_TEST_CASE(RunsInParallel){
    // Tasks that wait for each other can only finish if they really run at the same time.
    std::atomic<int> arrived(0);
    TaskPool pool(3);
    for (int i = 0; i < 3; ++ i){
        pool.Run([&arrived](){
            ++ arrived;
            const auto limit = std::chrono::steady_clock::now() + std::chrono::seconds(5);
            while (arrived < 3 && std::chrono::steady_clock::now() < limit) std::this_thread::yield();
        });
    }
    pool.Wait();
    BOOST_CHECK_EQUAL(arrived.load(), 3);
}

BOOST_AUTO_TEST_CASE(UsesSeveralThreads){
    std::mutex mutex;
    std::set<std::thread::id> threads;
    TaskPool pool(4);
    for (int i = 0; i < 50; ++ i){
        pool.Run([&](){
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            std::lock_guard<std::mutex> lock(mutex);
            threads.insert(std::this_thread::get_id());
        });
    }
    pool.Wait();
    BOOST_CHECK_GT(threads.size(), 1u);
    BOOST_CHECK_LE(threads.size(), 4u);
}

BOOST_AUTO_TEST_CASE(DestructorFinishesPendingTasks){
    std::atomic<int> done(0);
    {
        TaskPool pool(2);
        for (int i = 0; i < 20; ++ i) pool.Run([&done](){
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            ++ done;
        });
    }
    BOOST_CHECK_EQUAL(done.load(), 20);
}

BOOST_AUTO_TEST_CASE(ATaskThatThrowsDoesNotStopThePool){
    std::atomic<int> done(0);
    TaskPool pool(2);
    pool.Run([](){throw std::runtime_error("boom");});
    for (int i = 0; i < 10; ++ i) pool.Run([&done](){++ done;});
    pool.Wait();
    BOOST_CHECK_EQUAL(done.load(), 10);
}

BOOST_AUTO_TEST_CASE(DefaultNumberOfThreadsIsReasonable){
    TaskPool pool;
    BOOST_CHECK_GE(pool.Threads(), 1u);
    BOOST_CHECK_LE(pool.Threads(), 8u);
}

BOOST_AUTO_TEST_SUITE_END()

// ---------------------------------------------------------------------------------------------------------------
// Videos of the game disks.
// ---------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(VideoFilesTests)

BOOST_AUTO_TEST_CASE(RecognizesMovies){
    std::string name;
    BOOST_CHECK(VideoFiles::IsMovie("ff7/movies/bike.avi", &name));
    BOOST_CHECK_EQUAL(name, "bike.avi");
    // The disks mix the case, and ISO9660 adds version numbers and dots.
    BOOST_CHECK(VideoFiles::IsMovie("FF7/MOVIES/SQLOGO.AVI;1", &name));
    BOOST_CHECK_EQUAL(name, "sqlogo.avi");
    BOOST_CHECK(VideoFiles::IsMovie("/ff7/Movies/Explode.avi", &name));
    BOOST_CHECK_EQUAL(name, "explode.avi");
    BOOST_CHECK(VideoFiles::IsMovie("ff7\\movies\\ending1.avi", &name));
    BOOST_CHECK_EQUAL(name, "ending1.avi");
    // Other files of the folder are copied too (the camera data of the movies).
    BOOST_CHECK(VideoFiles::IsMovie("ff7/movies/moviecam.lgp", &name));
    BOOST_CHECK_EQUAL(name, "moviecam.lgp");
}

BOOST_AUTO_TEST_CASE(IgnoresEverythingElse){
    BOOST_CHECK(!VideoFiles::IsMovie("ff7/movies"));
    BOOST_CHECK(!VideoFiles::IsMovie("ff7/movies/"));
    BOOST_CHECK(!VideoFiles::IsMovie("ff7/movies/sub/bike.avi"));
    BOOST_CHECK(!VideoFiles::IsMovie("ff7/field/char.lgp"));
    BOOST_CHECK(!VideoFiles::IsMovie("ff7/moviesx/bike.avi"));
    BOOST_CHECK(!VideoFiles::IsMovie("data/movies/bike.avi"));
    BOOST_CHECK(!VideoFiles::IsMovie(""));
    std::string name = "untouched";
    BOOST_CHECK(!VideoFiles::IsMovie("ff7/wm/x.avi", &name));
    BOOST_CHECK_EQUAL(name, "untouched");
}

BOOST_AUTO_TEST_CASE(NormalizesPaths){
    BOOST_CHECK_EQUAL(VideoFiles::NormalizePath("/FF7/Movies/A.AVI;1"), "ff7/movies/a.avi");
    BOOST_CHECK_EQUAL(VideoFiles::NormalizePath("FF7/MOVIES/A."), "ff7/movies/a");
    // A semicolon that is not a version stays.
    BOOST_CHECK_EQUAL(VideoFiles::NormalizePath("a;b"), "a;b");
}

BOOST_AUTO_TEST_CASE(LogCanBeAppended){
    TempDir dir;
    InstallLog& log = InstallLog::Get();
    log.Open(dir.Root() + "installer.log");
    log.Add("first run");
    log.Close();
    log.Open(dir.Root() + "installer.log", true);
    log.Add("second run");
    log.Close();
    std::ifstream file(dir.Root() + "installer.log");
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    BOOST_CHECK(content.find("first run") != std::string::npos);
    BOOST_CHECK(content.find("second run") != std::string::npos);
    // Without append the file starts over.
    log.Open(dir.Root() + "installer.log");
    log.Close();
    std::ifstream again(dir.Root() + "installer.log");
    std::string content2((std::istreambuf_iterator<char>(again)), std::istreambuf_iterator<char>());
    BOOST_CHECK(content2.empty());
}

BOOST_AUTO_TEST_SUITE_END()

// Command line.

BOOST_AUTO_TEST_CASE(CommandLineWithoutInstallerOptionsOpensTheWindow){
    using namespace InstallCommandLine;
    BOOST_CHECK(Parse({}).mode == Mode::GUI);
    // Qt's own arguments are left for the window.
    BOOST_CHECK(Parse({"-platform", "xcb"}).mode == Mode::GUI);
}

BOOST_AUTO_TEST_CASE(CommandLineInstallDefaults){
    using namespace InstallCommandLine;
    const Arguments a = Parse({"--cli", "--iso", "disk.iso"});
    BOOST_REQUIRE(a.mode == Mode::INSTALL);
    BOOST_CHECK_EQUAL(a.iso, "disk.iso");
    BOOST_CHECK(a.output.empty());
    BOOST_CHECK(a.copy_data);
    BOOST_CHECK(!a.keep_originals);
    BOOST_CHECK(!a.clean);
    BOOST_CHECK(!a.strict);
    BOOST_CHECK(!a.yes);
    // Everything, except the videos, which are not on the install disk.
    const InstallOptions o = a.options;
    BOOST_CHECK(o.kernel && o.fields && o.field_models && o.wm && o.wm_models && o.battle_data && o.battle_models);
    BOOST_CHECK(o.images && o.sounds && o.music);
    BOOST_CHECK(!o.videos);
}

BOOST_AUTO_TEST_CASE(CommandLineInstallerOptionsImplyCli){
    using namespace InstallCommandLine;
    const Arguments a = Parse({"-i", "disk.iso", "-o", "/tmp/vg", "-y", "--no-copy-data", "--keep-originals", "--clean"});
    BOOST_REQUIRE(a.mode == Mode::INSTALL);
    BOOST_CHECK_EQUAL(a.output, "/tmp/vg");
    BOOST_CHECK(a.yes);
    BOOST_CHECK(!a.copy_data);
    BOOST_CHECK(a.keep_originals);
    BOOST_CHECK(a.clean);
    // Each of them is enough to use the command line.
    BOOST_CHECK(Parse({"--clean"}).mode == Mode::INVALID); // No ISO, but not the window.
    BOOST_CHECK(Parse({"--keep-originals"}).mode == Mode::INVALID);
    BOOST_CHECK(Parse({"-i", "d.iso", "--strict"}).strict);
    // "--option=value" works too.
    BOOST_CHECK_EQUAL(Parse({"--iso=other.iso"}).iso, "other.iso");
}

BOOST_AUTO_TEST_CASE(CommandLinePresetsAndParts){
    using namespace InstallCommandLine;
    Arguments a = Parse({"-i", "d.iso", "--preset", "battle", "--with", "music,images", "--without", "kernel"});
    BOOST_REQUIRE(a.mode == Mode::INSTALL);
    BOOST_CHECK(a.options.battle_data && a.options.battle_models && a.options.music && a.options.images);
    BOOST_CHECK(!a.options.kernel && !a.options.fields && !a.options.sounds);

    a = Parse({"-i", "d.iso", "-p", "custom", "--with", "fields"});
    BOOST_REQUIRE(a.mode == Mode::INSTALL);
    BOOST_CHECK(a.options.fields && !a.options.field_models && !a.options.kernel);

    // Removing a part removes what depends on it.
    a = Parse({"-i", "d.iso", "--without", "world-map"});
    BOOST_REQUIRE(a.mode == Mode::INSTALL);
    BOOST_CHECK(!a.options.wm && !a.options.wm_models);
}

BOOST_AUTO_TEST_CASE(CommandLineErrors){
    using namespace InstallCommandLine;
    BOOST_CHECK(Parse({"--cli"}).mode == Mode::INVALID);
    BOOST_CHECK(Parse({"--iso"}).mode == Mode::INVALID);
    BOOST_CHECK(Parse({"-i", "d.iso", "--preset", "nope"}).mode == Mode::INVALID);
    BOOST_CHECK(Parse({"-i", "d.iso", "--with", "nope"}).mode == Mode::INVALID);
    BOOST_CHECK(Parse({"-i", "d.iso", "--bogus"}).mode == Mode::INVALID);
    // A part can't be added without what it needs.
    BOOST_CHECK(Parse({"-i", "d.iso", "-p", "custom", "--with", "field-models"}).mode == Mode::INVALID);
    BOOST_CHECK(Parse({"--verify", "-i", "d.iso"}).mode == Mode::INVALID);
    BOOST_CHECK(Parse({"--verify", "--with", "music"}).mode == Mode::INVALID);
    BOOST_CHECK(Parse({"--verify", "--clean"}).mode == Mode::INVALID);
    // The old name of the option is gone.
    BOOST_CHECK(Parse({"-i", "d.iso", "--discard-originals"}).mode == Mode::INVALID);
}

BOOST_AUTO_TEST_CASE(CommandLineOtherModes){
    using namespace InstallCommandLine;
    BOOST_CHECK(Parse({"--verify"}).mode == Mode::VERIFY);
    BOOST_CHECK_EQUAL(Parse({"--verify", "-o", "/x"}).output, "/x");
    BOOST_CHECK(Parse({"--help"}).mode == Mode::HELP);
    BOOST_CHECK(Parse({"-i", "d.iso", "-h"}).mode == Mode::HELP);
    BOOST_CHECK(Parse({"--version"}).mode == Mode::VERSION);
    BOOST_CHECK(Parse({"--list-parts"}).mode == Mode::LIST_PARTS);
    BOOST_CHECK(!Usage("v-gears-installer").empty());
}

// Clean installation.

BOOST_AUTO_TEST_CASE(CleanInstallationDeletesTheDataAndKeepsTheRest){
    TempDir dir;
    const std::string root = dir.Root();
    for (const char* file : {
      "plugins.cfg", "resources.cfg", "v-gears.cfg", "logs/installer.log", "user/saves/0.xml", "user/config.cfg",
      "game/video/opening/movie.xml", "game/field/maps/md1_1/map.xml", "game/field/models/aerisflowers.mesh",
      "game/audio/sounds/1.ogg", "game/scripts/x.lua", "engine/scripts/x.lua", "defaults/config.cfg",
      "temp/char/cbba.rsd", "temp/world_models/a.p", "original_data/ff7/ff7.exe", "install.info",
      // The files of an older version.
      "save/1.xml", "controls.cfg", "data/fields/md1_1/map.xml", "data/video/gold1/movie.xml"
    }) dir.Touch(file);
    const InstallLayout::CleanResult result = InstallLayout::CleanInstallation(root);
    BOOST_CHECK(result.errors.empty());
    BOOST_CHECK(result.removed > 0);
    // Kept, the files of the older version in their new places.
    for (const char* file : {
      "plugins.cfg", "resources.cfg", "v-gears.cfg", "logs/installer.log", "user/saves/0.xml", "user/saves/1.xml",
      "user/controls.cfg", "user/config.cfg", "game/video/opening/movie.xml", "game/video/gold1/movie.xml"
    }) BOOST_CHECK_MESSAGE(fs::exists(dir.path / file), std::string("kept: ") + file);
    // Deleted.
    for (const char* path : {
      "game/field", "game/battle", "game/audio", "game/scripts", "engine", "defaults", "data", "temp",
      "original_data", "install.info"
    }) BOOST_CHECK_MESSAGE(!fs::exists(dir.path / path), std::string("deleted: ") + path);
    BOOST_CHECK(fs::is_directory(dir.path / "game"));
}

BOOST_AUTO_TEST_CASE(CleanInstallationDoesNotTouchOtherDirectories){
    // Without the configuration files it is not an installation, and nothing is deleted.
    TempDir dir;
    dir.Touch("game/my_files/document.txt");
    dir.Touch("temp/notes.txt");
    BOOST_CHECK(!InstallLayout::IsInstallation(dir.Root()));
    const InstallLayout::CleanResult result = InstallLayout::CleanInstallation(dir.Root());
    BOOST_CHECK_EQUAL(result.removed, 0u);
    BOOST_CHECK(fs::exists(dir.path / "game/my_files/document.txt"));
    BOOST_CHECK(fs::exists(dir.path / "temp/notes.txt"));
}

BOOST_AUTO_TEST_CASE(OriginalDataIsInsideTheInstallationDirectory){
    BOOST_CHECK_EQUAL(InstallLayout::OriginalDataDir("/x/"), "/x/original_data/");
}

// Record of the installations.

BOOST_AUTO_TEST_CASE(InstallStampIsWrittenAndRead){
    namespace stamp = VGears::InstallStamp;
    TempDir dir;
    stamp::Stamp written;
    stamp::Start(written, {"kernel", "fields"}, "1.2.3", "2026-11-26 10:00:00", "Install Disk");
    BOOST_REQUIRE(stamp::Write(dir.path, written));
    stamp::Stamp read;
    BOOST_REQUIRE(stamp::Read(dir.path, read));
    BOOST_CHECK(!read.complete);
    BOOST_CHECK_EQUAL(read.installer, "1.2.3");
    BOOST_CHECK_EQUAL(read.date, "2026-11-26 10:00:00");
    BOOST_CHECK_EQUAL(read.disk, "Install Disk");
    BOOST_CHECK(read.installing == std::vector<std::string>({"kernel", "fields"}));
    stamp::Finish(read);
    BOOST_REQUIRE(stamp::Write(dir.path, read));
    BOOST_REQUIRE(stamp::Read(dir.path, read));
    BOOST_CHECK(read.complete);
    BOOST_CHECK(read.installing.empty());
    BOOST_CHECK_EQUAL(read.parts["kernel"], stamp::DATA_VERSION);
    BOOST_CHECK_EQUAL(read.parts["fields"], stamp::DATA_VERSION);
    BOOST_CHECK(stamp::Outdated(read).empty());
    BOOST_CHECK(stamp::Problems(dir.path).empty());
    // Without a file, there is no record.
    TempDir empty;
    BOOST_CHECK(!stamp::Read(empty.path, read));
    bool found = true;
    BOOST_CHECK(stamp::Problems(empty.path, &found).empty());
    BOOST_CHECK(!found);
}

BOOST_AUTO_TEST_CASE(InstallStampReportsUnfinishedAndOldInstallations){
    namespace stamp = VGears::InstallStamp;
    TempDir dir;
    stamp::Stamp record;
    // A complete installation, then one of other parts that is interrupted.
    stamp::Start(record, {"kernel"}, "1", "a", "disk");
    stamp::Finish(record);
    stamp::Start(record, {"battle-data"}, "1", "b", "disk");
    BOOST_REQUIRE(stamp::Write(dir.path, record));
    BOOST_CHECK_EQUAL(stamp::Problems(dir.path).size(), 1u);
    BOOST_CHECK(stamp::Outdated(record) == std::vector<std::string>({"battle-data"}));
    // The next installation doesn't install them again: they stay as not installed.
    stamp::Start(record, {"fields"}, "1", "c", "disk");
    stamp::Finish(record);
    BOOST_CHECK_EQUAL(record.parts["battle-data"], 0);
    BOOST_CHECK(stamp::Outdated(record) == std::vector<std::string>({"battle-data"}));
    BOOST_CHECK_EQUAL(record.parts["kernel"], stamp::DATA_VERSION);
    // A part of an older version of the data.
    record.parts["fields"] = stamp::DATA_VERSION - 1;
    BOOST_REQUIRE(stamp::Write(dir.path, record));
    const std::vector<std::string> problems = stamp::Problems(dir.path);
    BOOST_REQUIRE_EQUAL(problems.size(), 1u);
    BOOST_CHECK(problems[0].find("battle-data") != std::string::npos);
    BOOST_CHECK(problems[0].find("fields") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(VerifierReportsTheRecord){
    namespace stamp = VGears::InstallStamp;
    TempDir dir;
    BOOST_CHECK(HasProblem(VerifyInstallation(dir.Root()), "Installation"));
    stamp::Stamp record;
    stamp::Start(record, {"kernel"}, "1", "a", "disk");
    BOOST_REQUIRE(stamp::Write(dir.path, record));
    BOOST_CHECK(HasProblem(VerifyInstallation(dir.Root()), "Installation"));
    stamp::Finish(record);
    BOOST_REQUIRE(stamp::Write(dir.path, record));
    BOOST_CHECK(!HasProblem(VerifyInstallation(dir.Root()), "Installation"));
}
