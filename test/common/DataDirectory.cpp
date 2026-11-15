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
            Write(dir / "data" / "config" / "config.cfg", "template config");
            Write(dir / "data" / "scripts" / "menu" / "begin.lua", "template script");
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
    fs::create_directories(box.root / "data");
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
    BOOST_CHECK_EQUAL(box.Read(target / "data" / "scripts" / "menu" / "begin.lua"), "template script");
}

BOOST_AUTO_TEST_CASE(SeedingKeepsExistingFiles){
    Sandbox box;
    const fs::path source = box.MakeTemplate("template");
    // An installation where the installer extracted the game data but the configuration was not copied.
    const fs::path target = box.root / "install";
    box.Write(target / "data" / "config" / "config.cfg", "user config");
    box.Write(target / "data" / "fields" / "md1_1.xml", "game data");

    const dd::Resolution r = dd::Resolve(target.string(), box.root, box.root / "default", {source});
    BOOST_CHECK_EQUAL(r.error, "");
    BOOST_CHECK(r.seeded);
    BOOST_CHECK_EQUAL(r.copied_files, 3u);
    BOOST_CHECK_EQUAL(box.Read(target / "data" / "config" / "config.cfg"), "user config");
    BOOST_CHECK_EQUAL(box.Read(target / "data" / "fields" / "md1_1.xml"), "game data");
    BOOST_CHECK_EQUAL(box.Read(target / "plugins.cfg"), "template plugins");
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
