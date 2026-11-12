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
#include "installer/InstallLayout.h"
#include "installer/InstallLog.h"
#include "installer/InstallOptions.h"
#include "installer/InstallVerifier.h"
#include "installer/TaskPool.h"
#include "installer/VideoFiles.h"
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

BOOST_AUTO_TEST_CASE(DataAndTempAreInsideTheInstallationDirectory){
    BOOST_CHECK_EQUAL(InstallLayout::DataDir("/home/u/.v-gears/"), "/home/u/.v-gears/data/");
    BOOST_CHECK_EQUAL(InstallLayout::TempDir("/home/u/.v-gears/"), "/home/u/.v-gears/temp/");
}

BOOST_AUTO_TEST_CASE(DataSubdirsAreRelativeAndUnique){
    const auto& dirs = InstallLayout::DataSubdirs();
    for (const std::string& dir : dirs){
        BOOST_CHECK_MESSAGE(!dir.empty() && dir[0] != '/', "not relative: " << dir);
        BOOST_CHECK_MESSAGE(!StartsWith(dir, "data/") && !StartsWith(dir, "temp"), "wrong root: " << dir);
        BOOST_CHECK_MESSAGE(std::count(dirs.begin(), dirs.end(), dir) == 1, "duplicated: " << dir);
    }
}

BOOST_AUTO_TEST_CASE(EveryDirectoryTheGameReadsIsCreated){
    // The game reads "./data/<x>" and "./temp/<x>"; the installer must create them (or be able to write in them).
    for (const std::string& resource : InstallLayout::GameResourceDirs()){
        if (StartsWith(resource, "data/")){
            const std::string dir = resource.substr(5);
            bool created = false;
            for (const std::string& sub : InstallLayout::DataSubdirs())
                created = created || sub == dir || StartsWith(sub, dir + "/");
            BOOST_CHECK_MESSAGE(created, "the installer doesn't create " << resource);
        }
        else{
            BOOST_REQUIRE_MESSAGE(StartsWith(resource, "temp/"), "unexpected root: " << resource);
            BOOST_CHECK_MESSAGE(Contains(InstallLayout::TempSubdirs(), resource.substr(5)), "not created: " << resource);
        }
    }
}

BOOST_AUTO_TEST_CASE(CleaningKeepsWhatTheGameNeeds){
    const auto removable = InstallLayout::RemovableTempSubdirs();
    BOOST_CHECK(!Contains(removable, "world_models"));
    for (const std::string& kept : InstallLayout::KeptTempSubdirs()){
        BOOST_CHECK(Contains(InstallLayout::TempSubdirs(), kept));
        BOOST_CHECK(!Contains(removable, kept));
    }
    BOOST_CHECK_EQUAL(removable.size() + InstallLayout::KeptTempSubdirs().size(), InstallLayout::TempSubdirs().size());
    // The game reads temp/world_models, so it must be a kept directory.
    BOOST_CHECK(Contains(InstallLayout::KeptTempSubdirs(), "world_models"));
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
      "World map", "World map models", "Sound effects", "Music", "Images"
    }) BOOST_CHECK_MESSAGE(HasProblem(result, group), "no problem reported for " << group);
    BOOST_CHECK_GT(result.checked, 0);
}

BOOST_AUTO_TEST_CASE(ConfigurationAndPlugins){
    TempDir dir;
    dir.Touch("plugins.cfg");
    dir.Touch("resources.cfg");
    BOOST_CHECK(HasProblem(VerifyInstallation(dir.Root()), "Configuration"));
    // The plugins have a version suffix.
    dir.Touch("RenderSystem_GL.so.1.12.10");
    dir.Touch("Codec_STBI.so.1.12.10");
    BOOST_CHECK(!HasProblem(VerifyInstallation(dir.Root()), "Configuration"));
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
    reference.Touch("data/scripts/a.lua");
    BOOST_CHECK(HasProblem(VerifyInstallation(installation.Root(), reference.Root()), "Copied data files"));
    installation.Touch("plugins.cfg");
    BOOST_CHECK(HasProblem(VerifyInstallation(installation.Root(), reference.Root()), "Copied data files"));
    installation.Touch("data/scripts/a.lua");
    BOOST_CHECK(!HasProblem(VerifyInstallation(installation.Root(), reference.Root()), "Copied data files"));
    // Without a reference there is nothing to compare.
    BOOST_CHECK(!HasProblem(VerifyInstallation(installation.Root()), "Copied data files"));
}

BOOST_AUTO_TEST_CASE(CountsFilesInsteadOfJustLookingForOne){
    TempDir dir;
    for (int i = 0; i < 3; ++ i) dir.Touch("data/fields/f" + std::to_string(i) + "/map.xml");
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
