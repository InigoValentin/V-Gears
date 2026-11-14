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
#include "common/OgrePlugins.h"

namespace fs = boost::filesystem;
namespace op = VGears::OgrePlugins;

namespace{

    const std::string VERSION("1.12.10");

    struct Sandbox{
        fs::path root;
        Sandbox(): root(fs::temp_directory_path() / fs::unique_path("vgears-plugins-%%%%-%%%%")){
            fs::create_directories(root);
        }
        ~Sandbox(){
            boost::system::error_code ec;
            fs::remove_all(root, ec);
        }
        fs::path Write(const fs::path& file, const std::string& content = "x") const{
            fs::create_directories(file.parent_path());
            std::ofstream(file.string()) << content;
            return file;
        }
        /**
         * Creates the file of a plugin, with the name this system uses.
         */
        fs::path Plugin(const fs::path& directory, const std::string& name) const{
            return Write(directory / op::FileNames(name, VERSION).front());
        }
    };

    size_t IndexOf(const std::vector<fs::path>& list, const fs::path& item){
        return std::find(list.begin(), list.end(), item.lexically_normal()) - list.begin();
    }
}

BOOST_AUTO_TEST_SUITE(OgrePluginsTests)

BOOST_AUTO_TEST_CASE(ReadConfigSkipsCommentsAndBlanks){
    Sandbox box;
    const fs::path file = box.Write(
      box.root / "plugins.cfg",
      "# Defines plugins to load\n\n# PluginFolder=/nowhere\n PluginFolder = plugins \r\n"
      "# Plugin=RenderSystem_Direct3D9\n Plugin=RenderSystem_GL\r\nPlugin=\nPlugin=Codec_STBI\n"
    );
    op::Config config;
    BOOST_REQUIRE(op::ReadConfig(file, config));
    BOOST_CHECK_EQUAL(config.folder, "plugins");
    BOOST_REQUIRE_EQUAL(config.plugins.size(), 2u);
    BOOST_CHECK_EQUAL(config.plugins[0], "RenderSystem_GL");
    BOOST_CHECK_EQUAL(config.plugins[1], "Codec_STBI");
}

BOOST_AUTO_TEST_CASE(ReadConfigFailsForMissingFile){
    Sandbox box;
    op::Config config;
    BOOST_CHECK(!op::ReadConfig(box.root / "missing.cfg", config));
}

BOOST_AUTO_TEST_CASE(TheRepositoryConfigListsThePlugins){
    op::Config config;
    BOOST_REQUIRE(op::ReadConfig(fs::path(VGEARS_SOURCE_DIR) / "data" / "plugins.cfg", config));
    BOOST_CHECK_EQUAL(config.folder, "");
    BOOST_CHECK(std::find(config.plugins.begin(), config.plugins.end(), "RenderSystem_GL") != config.plugins.end());
    BOOST_CHECK(std::find(config.plugins.begin(), config.plugins.end(), "Codec_STBI") != config.plugins.end());
}

BOOST_AUTO_TEST_CASE(SearchOrder){
    const fs::path data("/home/user/.v-gears");
    const fs::path exe("/opt/v-gears/bin");
    const std::vector<fs::path> dirs = op::SearchDirectories(data, "plugins", exe, "/build/ogre/plugins");
    const size_t folder = IndexOf(dirs, data / "plugins");
    const size_t config = IndexOf(dirs, data);
    const size_t executable = IndexOf(dirs, exe);
    const size_t bundle = IndexOf(dirs, "/opt/v-gears/lib/OGRE");
    const size_t build = IndexOf(dirs, "/build/ogre/plugins");
    BOOST_REQUIRE(build < dirs.size());
    BOOST_CHECK(folder < config);
    BOOST_CHECK(config < executable);
    BOOST_CHECK(executable < bundle);
    BOOST_CHECK(bundle < build);
}

BOOST_AUTO_TEST_CASE(SearchDirectoriesHaveNoDuplicates){
    const fs::path data("/home/user/.v-gears");
    const std::vector<fs::path> dirs = op::SearchDirectories(data, ".", data, data.string());
    BOOST_CHECK_EQUAL(std::count(dirs.begin(), dirs.end(), data), 1);
}

BOOST_AUTO_TEST_CASE(FindPrefersTheFirstDirectory){
    Sandbox box;
    const fs::path first = box.root / "first";
    const fs::path second = box.root / "second";
    box.Plugin(second, "RenderSystem_GL");
    BOOST_CHECK_EQUAL(op::Find("RenderSystem_GL", {first, second}, VERSION), box.Plugin(second, "RenderSystem_GL"));
    const fs::path in_first = box.Plugin(first, "RenderSystem_GL");
    BOOST_CHECK_EQUAL(op::Find("RenderSystem_GL", {first, second}, VERSION), in_first);
    BOOST_CHECK(op::Find("Codec_STBI", {first, second}, VERSION).empty());
}

#if !defined(_WIN32) && !defined(__APPLE__)
BOOST_AUTO_TEST_CASE(FindIgnoresPluginsOfOtherOgreVersions){
    Sandbox box;
    box.Write(box.root / "RenderSystem_GL.so.1.12.9");
    box.Write(box.root / "RenderSystem_GL.so");
    BOOST_CHECK(op::Find("RenderSystem_GL", {box.root}, VERSION).empty());
    box.Write(box.root / "RenderSystem_GL.so.1.12.10");
    BOOST_CHECK_EQUAL(op::Find("RenderSystem_GL", {box.root}, VERSION), box.root / "RenderSystem_GL.so.1.12.10");
}
#endif

BOOST_AUTO_TEST_CASE(FindAcceptsAbsolutePaths){
    Sandbox box;
    const fs::path file = box.Write(box.root / "custom" / "MyPlugin.so");
    BOOST_CHECK_EQUAL(op::Find(file.string(), {}, VERSION), file);
    BOOST_CHECK(op::Find((box.root / "nothing.so").string(), {}, VERSION).empty());
}

BOOST_AUTO_TEST_SUITE_END()
