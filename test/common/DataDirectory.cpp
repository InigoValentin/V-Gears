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
#include <algorithm>
#include <fstream>
#include <sstream>
#include "common/DataDirectory.h"

namespace fs = boost::filesystem;
namespace dd = VGears::DataDirectory;

namespace{

    /**
     * A temporary directory that is removed on destruction.
     */
    struct Sandbox{
        fs::path root;
        Sandbox(): root(fs::temp_directory_path() / fs::unique_path("vgears-datadir-%%%%-%%%%")){
            fs::create_directories(root);
        }
        ~Sandbox(){
            boost::system::error_code ec;
            fs::remove_all(root, ec);
        }
        void Write(const fs::path& file, const std::string& content) const{
            fs::create_directories(file.parent_path());
            std::ofstream(file.string()) << content;
        }
        std::string Read(const fs::path& file) const{
            std::ifstream in(file.string());
            std::stringstream text;
            text << in.rdbuf();
            return text.str();
        }
        /**
         * Creates a template with the V-Gears files under root/name.
         */
        fs::path MakeTemplate(const std::string& name) const{
            const fs::path dir = root / name;
            Write(dir / "plugins.cfg", "template plugins");
            Write(dir / "resources.cfg", "template resources");
            Write(dir / "defaults" / "config.cfg", "template config");
            Write(dir / "engine" / "scripts" / "menu" / "begin.lua", "template script");
            return dir;
        }
    };
}

BOOST_AUTO_TEST_SUITE(DataDirectoryTests)

BOOST_AUTO_TEST_CASE(IsDataDirectoryNeedsPluginsAndData){
    Sandbox box;
    BOOST_CHECK(!dd::IsDataDirectory(box.root));
    box.Write(box.root / "plugins.cfg", "");
    BOOST_CHECK(!dd::IsDataDirectory(box.root));
    fs::create_directories(box.root / "engine");
    BOOST_CHECK(dd::IsDataDirectory(box.root));
    BOOST_CHECK(!dd::IsDataDirectory(box.root / "missing"));
}

BOOST_AUTO_TEST_CASE(RequestedDirectoryWins){
    Sandbox box;
    const fs::path current = box.MakeTemplate("current");
    const fs::path requested = box.MakeTemplate("requested");
    const dd::Resolution r = dd::Resolve(requested.string(), current, box.root / "default", {});
    BOOST_CHECK_EQUAL(r.error, "");
    BOOST_CHECK_EQUAL(r.directory, requested);
    BOOST_CHECK(!r.seeded);
}

BOOST_AUTO_TEST_CASE(RelativeRequestIsRelativeToCurrent){
    Sandbox box;
    const fs::path requested = box.MakeTemplate("requested");
    const dd::Resolution r = dd::Resolve("requested", box.root, box.root / "default", {});
    BOOST_CHECK_EQUAL(r.error, "");
    BOOST_CHECK_EQUAL(r.directory, requested);
}

BOOST_AUTO_TEST_CASE(CurrentDirectoryIsUsedIfItHasTheFiles){
    Sandbox box;
    const fs::path current = box.MakeTemplate("current");
    const dd::Resolution r = dd::Resolve("", current, box.root / "default", {box.MakeTemplate("template")});
    BOOST_CHECK_EQUAL(r.error, "");
    BOOST_CHECK_EQUAL(r.directory, current);
    BOOST_CHECK(!r.seeded);
    BOOST_CHECK(!fs::exists(box.root / "default"));
}

BOOST_AUTO_TEST_CASE(DefaultDirectoryIsSeededFromTheFirstValidTemplate){
    Sandbox box;
    const fs::path not_a_template = box.root / "empty";
    fs::create_directories(not_a_template);
    const fs::path source = box.MakeTemplate("template");
    const fs::path second = box.MakeTemplate("second");
    box.Write(second / "plugins.cfg", "second plugins");
    const fs::path target = box.root / "home" / ".v-gears";

    const dd::Resolution r = dd::Resolve("", box.root / "elsewhere", target, {not_a_template, source, second});
    BOOST_CHECK_EQUAL(r.error, "");
    BOOST_CHECK_EQUAL(r.directory, target);
    BOOST_CHECK(r.seeded);
    BOOST_CHECK_EQUAL(r.template_directory, source);
    BOOST_CHECK_EQUAL(r.copied_files, 4u);
    BOOST_CHECK(dd::IsDataDirectory(target));
    BOOST_CHECK_EQUAL(box.Read(target / "plugins.cfg"), "template plugins");
    BOOST_CHECK_EQUAL(box.Read(target / "engine" / "scripts" / "menu" / "begin.lua"), "template script");
}

BOOST_AUTO_TEST_CASE(SeedingKeepsExistingFiles){
    Sandbox box;
    const fs::path source = box.MakeTemplate("template");
    // An installation where the installer extracted the game data but the configuration was not copied.
    const fs::path target = box.root / "install";
    box.Write(target / "user" / "config.cfg", "user config");
    box.Write(target / "game" / "fields" / "md1_1.xml", "game data");

    const dd::Resolution r = dd::Resolve(target.string(), box.root, box.root / "default", {source});
    BOOST_CHECK_EQUAL(r.error, "");
    BOOST_CHECK(r.seeded);
    BOOST_CHECK_EQUAL(r.copied_files, 4u);
    BOOST_CHECK_EQUAL(box.Read(target / "user" / "config.cfg"), "user config");
    BOOST_CHECK_EQUAL(box.Read(target / "game" / "fields" / "md1_1.xml"), "game data");
    BOOST_CHECK_EQUAL(box.Read(target / "plugins.cfg"), "template plugins");
}

BOOST_AUTO_TEST_CASE(NewInstallationGetsTheConfigurationAndTheDirectoriesOfThePlayer){
    Sandbox box;
    const fs::path source = box.MakeTemplate("template");
    // The template can have player files and logs if the game was run from it: they are not copied.
    box.Write(source / "user" / "saves" / "0.xml", "template save");
    box.Write(source / "logs" / "v-gears.log", "template log");
    const fs::path target = box.root / ".v-gears";
    const dd::Resolution r = dd::Resolve("", box.root / "elsewhere", target, {source});
    BOOST_CHECK_EQUAL(r.error, "");
    BOOST_CHECK_EQUAL(box.Read(target / "user" / "config.cfg"), "template config");
    BOOST_CHECK(fs::is_directory(target / "user" / "screenshots"));
    BOOST_CHECK(fs::is_directory(target / "logs"));
    BOOST_CHECK(!fs::exists(target / "user" / "saves" / "0.xml"));
    BOOST_CHECK(!fs::exists(target / "logs" / "v-gears.log"));
}

BOOST_AUTO_TEST_CASE(OldInstallationIsMigrated){
    Sandbox box;
    const fs::path source = box.MakeTemplate("template");
    const fs::path target = box.root / "old";
    box.Write(target / "plugins.cfg", "old plugins");
    box.Write(target / "data" / "config" / "config.cfg", "old config");
    box.Write(target / "data" / "video" / "opening" / "movie.xml", "movie");
    box.Write(target / "save" / "0.xml", "save");
    box.Write(target / "controls.cfg", "controls");
    box.Write(target / "console_history.txt", "history");
    box.Write(target / "screenshot_1.png", "image");

    const dd::Resolution r = dd::Resolve(target.string(), box.root, box.root / "default", {source});
    BOOST_CHECK_EQUAL(r.error, "");
    BOOST_CHECK_EQUAL(r.migrated_files, 6u);
    BOOST_CHECK(r.seeded);
    BOOST_CHECK_EQUAL(box.Read(target / "user" / "config.cfg"), "old config");
    BOOST_CHECK_EQUAL(box.Read(target / "user" / "saves" / "0.xml"), "save");
    BOOST_CHECK_EQUAL(box.Read(target / "user" / "controls.cfg"), "controls");
    BOOST_CHECK_EQUAL(box.Read(target / "user" / "console_history.txt"), "history");
    BOOST_CHECK_EQUAL(box.Read(target / "user" / "screenshots" / "screenshot_1.png"), "image");
    BOOST_CHECK_EQUAL(box.Read(target / "game" / "video" / "opening" / "movie.xml"), "movie");
    BOOST_CHECK(!fs::exists(target / "save"));
    BOOST_CHECK_EQUAL(box.Read(target / "plugins.cfg"), "old plugins");
}

BOOST_AUTO_TEST_CASE(MigrationNeverOverwrites){
    Sandbox box;
    const fs::path target = box.root / "install";
    box.Write(target / "plugins.cfg", "plugins");
    box.Write(target / "save" / "0.xml", "old save");
    box.Write(target / "save" / "1.xml", "other save");
    box.Write(target / "user" / "saves" / "0.xml", "new save");
    BOOST_CHECK_EQUAL(dd::MigrateLayout(target), 1u);
    BOOST_CHECK_EQUAL(box.Read(target / "user" / "saves" / "0.xml"), "new save");
    BOOST_CHECK_EQUAL(box.Read(target / "user" / "saves" / "1.xml"), "other save");
    BOOST_CHECK_EQUAL(box.Read(target / "save" / "0.xml"), "old save");
}

BOOST_AUTO_TEST_CASE(MigrationIgnoresOtherDirectories){
    Sandbox box;
    box.Write(box.root / "documents" / "save" / "0.xml", "not ours");
    BOOST_CHECK_EQUAL(dd::MigrateLayout(box.root / "documents"), 0u);
    BOOST_CHECK(fs::exists(box.root / "documents" / "save" / "0.xml"));
}

BOOST_AUTO_TEST_CASE(UnrelatedNonEmptyDirectoryIsNotFilled){
    Sandbox box;
    const fs::path source = box.MakeTemplate("template");
    const fs::path target = box.root / "documents";
    box.Write(target / "letter.txt", "hello");
    const dd::Resolution r = dd::Resolve(target.string(), box.root, box.root / "default", {source});
    BOOST_CHECK(!r.error.empty());
    BOOST_CHECK(r.directory.empty());
    BOOST_CHECK(!fs::exists(target / "plugins.cfg"));
}

BOOST_AUTO_TEST_CASE(MissingTemplateIsAnError){
    Sandbox box;
    const dd::Resolution r = dd::Resolve("", box.root, box.root / "default", {box.root / "nowhere"});
    BOOST_CHECK(!r.error.empty());
    BOOST_CHECK(r.directory.empty());
}

BOOST_AUTO_TEST_CASE(EmptyDefaultIsAnError){
    Sandbox box;
    const dd::Resolution r = dd::Resolve("", box.root, fs::path(), {box.MakeTemplate("template")});
    BOOST_CHECK(!r.error.empty());
}

BOOST_AUTO_TEST_CASE(TemplateIsNeverTheTargetItself){
    Sandbox box;
    const fs::path source = box.MakeTemplate("template");
    BOOST_CHECK(dd::FindTemplate({source}, source).empty());
    BOOST_CHECK_EQUAL(dd::FindTemplate({source}, box.root / "other"), source);
}

BOOST_AUTO_TEST_CASE(TemplateCandidatesCoverBuildTreeAndPackage){
    const fs::path exe("/opt/repo/build/bin");
    const std::vector<fs::path> c = dd::TemplateCandidates(exe);
    auto has = [&](const fs::path& p){return std::find(c.begin(), c.end(), p) != c.end();};
    BOOST_CHECK(has("/opt/repo/build/bin/data"));
    BOOST_CHECK(has("/opt/repo/data"));
    BOOST_CHECK(has("/opt/repo/build/share/v-gears"));
    const std::vector<fs::path> installed = dd::TemplateCandidates("/usr/games");
    BOOST_CHECK(std::find(installed.begin(), installed.end(), fs::path("/usr/share/v-gears")) != installed.end());
}

BOOST_AUTO_TEST_SUITE_END()
