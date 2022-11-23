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

#include <algorithm>
#include <cstdio>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <Ogre/OgreDataStream.h>
#include "data/VGearsTexFile.h"

using std::back_inserter;
using std::copy;
using std::cout;
using std::ifstream;
using std::endl;
using std::vector;
using std::string;

namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;

void pintFileInfo(const bfs::path &tex_path){
    cout << endl << "TX Image File Info"
      << endl << " filename: " << tex_path.filename().string();
    bfs::ifstream *ifs(OGRE_NEW_T(bfs::ifstream, Ogre::MEMCATEGORY_GENERAL)(tex_path, bfs::ifstream::binary));
    Ogre::FileStreamDataStream *stream(OGRE_NEW Ogre::FileStreamDataStream(ifs));
    Ogre::DataStreamPtr input(stream);
    VGears::TexFile tex;
    tex.readHeader(input);
    cout << endl << " size: "
      << tex.m_header.image_data.width
      << "x" << tex.m_header.image_data.height
      << "x" << tex.m_header.image_data.bit_depth
      << endl << " pitch: " << tex.m_header.image_data.pitch
      << endl << " color_key_flag: " << tex.m_header.color_key_flag
      << endl << "Palette:"
      << endl << " type              : " << tex.m_header.palette_type
      << endl << " count             : " << tex.m_header.palette_count
      << endl << " index_bits        : "
      << tex.m_header.palette_data.index_bits
      << endl << " flag              : " << tex.m_header.palette_data.flag
      << endl << " total_color_count : "
      << tex.m_header.palette_total_color_count
      << endl << " total_color_count : "
      << tex.m_header.palette_data.total_color_count
      << endl << " colors_per_palette: "
      << tex.m_header.palette_data.colors_per_palette
      << endl << " unknown_CC        : " << tex.m_header.unknown_0xCC;
    if (tex.m_header.palette_data.flag){
        cout << endl
          << " mask_red   : " << tex.m_header.pixel_format.bit_mask.red
          << endl << " mask_green : "
          << tex.m_header.pixel_format.bit_mask.green
          << endl << " mask_blue  : " << tex.m_header.pixel_format.bit_mask.blue
          << endl << " mask_alpha : "
          << tex.m_header.pixel_format.bit_mask.alpha;
    }
    cout << endl;
}

int main(int ac, char *av[]){
    const char* PO_HELP("help");
    const char* PO_INPUT_PATHS("input-paths");
    const string TEX_EXT(".tex");

    typedef vector<bfs::path> t_InputPaths;

    try {
        bpo::options_description generic("Generic options");
        generic.add_options()
            (PO_HELP, "produce help message")
            ;

        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        bpo::options_description hidden("Hidden options");
        hidden.add_options()
            (PO_INPUT_PATHS, bpo::value< t_InputPaths >()->required(), "input paths to files or directories")
            ;

        bpo::positional_options_description p;
        p.add(PO_INPUT_PATHS, -1);

        bpo::options_description cmdline_options;
        cmdline_options.add(generic).add(hidden);
        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(ac, av).options(cmdline_options).positional(p).run(), vm);

        if (vm.count(PO_HELP)){
            bfs::path self(av[0]);
            cout << "Usage: " << self.stem().string() << " [options] file|path...\n";
            cout << generic << "\n";
            return 0;
        }

        bpo::notify(vm);

        if (vm.count(PO_INPUT_PATHS)){
            t_InputPaths input(vm[ PO_INPUT_PATHS ].as< t_InputPaths >());
            t_InputPaths paths;

            for (t_InputPaths::const_iterator it(input.begin()); it != input.end(); ++it){
                if(bfs::exists(*it)){
                    if(bfs::is_directory(*it)){
                        copy(bfs::recursive_directory_iterator(*it), bfs::recursive_directory_iterator(), back_inserter(paths));
                    }
                    else{
                        paths.push_back(*it);
                    }
                }
            }

            for (t_InputPaths::const_iterator it(paths.begin()); it != paths.end(); ++it){
                if(bfs::is_regular_file(*it)){
                    string ext(it->extension().string());
                    if(boost::iequals(ext, TEX_EXT)){
                        pintFileInfo(*it);
                    }
                }
            }
        }

    }
    catch(const bfs::filesystem_error& e){
        cout << e.what() << endl;
        return 1;
    }
    return 0;
}
