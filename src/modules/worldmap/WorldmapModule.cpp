/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
  static bool IsJunonTexture(uint16 texture_id){
    const size_t texture_count = sizeof(textures) / sizeof(textures[0]);
    if (texture_id >= texture_count) return false;
    const std::string texture_name = textures[texture_id].name;
    return texture_name.compare(0, 3, "jun") == 0
      || texture_name == "jutmpl01";
  }
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

#include "WorldmapModule.h"

#include <algorithm>
#include <map>
#include <memory>
#include <limits>
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <tinyxml.h>
#include <OgreEntity.h>
#include <OgreSceneManager.h>
#include <OgreManualObject.h>
#include <OgreRoot.h>
#include <OgreException.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreLogManager.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreResourceGroupManager.h>
#include "data/VGearsTexFile.h"
#include "core/InputManager.h"
#include "core/Logger.h"
#include "core/EntityManager.h"
#include "core/EntityModel.h"
#include "core/DialogsManager.h"
#include "core/ScriptManager.h"
#include "data/worldmap/WorldmapFile.h"
#include "data/worldmap/WorldmapFileManager.h"
#include "data/worldmap/WorldmapFileSerializer.h"
#include "core/CameraManager.h"

/**
 * World map module singleton.
 */
template<> VGears::WorldmapModule *Ogre::Singleton< VGears::WorldmapModule >::msSingleton = nullptr;

namespace VGears {

    using namespace Ogre;

    /**
     * World map hardcoded texture information.
     */
    struct HarcodedTextureInfo{

        /**
         * Texture id.
         */
        uint32 id;

        /**
         * Texture name.
         */
        const char* name;

        /**
         * Texture width.
         */
        uint32 width;

        /**
         * Texture height.
         */
        uint32 height;

        /**
         * Texture U component.
         */
        uint32 u;

        /**
         * Texture V component
         */
        uint32 v;
    };

    /**
     * World map hardcoded textures
     */
    static HarcodedTextureInfo textures[] ={
        {0, "pond", 32, 32, 0, 352},
        {1, "riv_m2", 32, 32, 128, 64},
        {2, "was_gs", 64, 64, 64, 192},
        {3, "wpcltr", 32, 128, 0, 256},
        {4, "wpcltr2", 32, 32, 160, 64},
        {5, "bzdun", 64, 64, 192, 192},
        {6, "bone", 32, 32, 224, 384},
        {7, "bone2", 32, 32, 224, 416},
        {8, "bornwd", 64, 64, 160, 320},
        {9, "bridge", 32, 64, 192, 0},
        {10, "bridge2", 32, 32, 224, 0},
        {11, "cave", 32, 32, 224, 448},
        {12, "cave2", 32, 32, 224, 320},
        {13, "cave_s", 32, 32, 160, 224},
        {14, "cdl_cl2", 64, 32, 96, 96},
        {15, "cf01", 64, 32, 192, 288},
        {16, "clf_bgs", 64, 32, 192, 384},
        {17, "clf_ggl", 64, 64, 128, 256},
        {18, "clf_ggs", 64, 32, 192, 352},
        {19, "clf_l", 64, 64, 0, 0},
        {20, "clf_ld", 64, 64, 64, 0},
        {21, "clf_lf", 64, 64, 128, 0},
        {22, "clf_lg", 32, 64, 0, 96},
        {23, "clf_lr", 32, 64, 128, 0},
        {24, "clf_lsg", 32, 64, 64, 64},
        {25, "clf_r", 32, 32, 0, 96},
        {26, "clf_s", 64, 32, 192, 0},
        {27, "clf_sd", 64, 32, 192, 32},
        {28, "clf_sf", 64, 32, 0, 64},
        {29, "clf_sg", 32, 32, 32, 96},
        {30, "clf_sg2", 32, 32, 0, 160},
        {31, "clf_sr", 32, 32, 32, 96},
        {32, "clf_ss", 32, 32, 32, 128},
        {33, "clf_ssd", 32, 32, 0, 224},
        {34, "clf_ssw", 32, 32, 224, 32},
        {35, "clf_sw", 32, 32, 192, 32},
        {36, "clf_w02", 64, 64, 128, 64},
        {37, "clf_w03", 64, 64, 192, 64},
        {38, "clf_was", 64, 32, 64, 64},
        {39, "clfeg", 32, 32, 192, 320},
        {40, "clfegd", 32, 32, 0, 320},
        {41, "clftop", 64, 32, 192, 64},
        {42, "clftop2", 32, 32, 128, 64},
        {43, "cndl_cl", 64, 32, 96, 128},
        {44, "cndlf", 64, 64, 160, 64},
        {45, "cndlf02", 64, 64, 208, 64},
        {46, "comtr", 16, 32, 144, 96},
        {47, "cosinn", 32, 32, 224, 416},
        {48, "cosinn2", 32, 32, 192, 448},
        {49, "csmk", 32, 32, 64, 64},
        {50, "csmk2", 32, 32, 96, 64},
        {51, "cstds01", 32, 32, 224, 160},
        {52, "cstds02", 64, 64, 0, 448},
        {53, "des01", 32, 32, 160, 320},
        {54, "desert", 64, 64, 128, 128},
        {55, "desor", 64, 32, 160, 64},
        {56, "ds1", 32, 32, 0, 256},
        {57, "ds_s1", 32, 32, 192, 288},
        {58, "dsee1", 32, 32, 96, 288},
        {59, "dsrt_d", 32, 32, 224, 0},
        {60, "dsrt_e", 64, 128, 64, 128},
        {61, "edes01", 32, 32, 224, 320},
        {62, "elm01", 32, 32, 160, 0},
        {63, "elm02", 32, 32, 64, 96},
        {64, "elm_gro", 64, 64, 0, 96},
        {65, "elm_r", 32, 32, 192, 0},
        {66, "elm_r2", 32, 32, 224, 0},
        {67, "fall1", 32, 32, 128, 256},
        {68, "farm01", 32, 32, 160, 32},
        {69, "farm02", 32, 32, 192, 32},
        {70, "farm_g", 32, 32, 128, 64},
        {71, "farm_r", 32, 16, 128, 48},
        {72, "fld", 64, 64, 64, 96},
        {73, "fld_02", 64, 64, 0, 64},
        {74, "fld_s", 64, 64, 0, 160},
        {75, "fld_s2", 32, 32, 224, 256},
        {76, "fld_sw", 64, 64, 128, 192},
        {77, "fld_v", 128, 128, 0, 128},
        {78, "fld_vd", 32, 64, 96, 128},
        {79, "fld_vd2", 32, 64, 192, 416},
        {80, "fvedge", 32, 64, 0, 0},
        {81, "gclf_d", 128, 64, 128, 128},
        {82, "gclf_g", 32, 64, 224, 128},
        {83, "gclfwa", 128, 64, 64, 320},
        {84, "gclfwa2", 32, 64, 160, 320},
        {85, "gclfwag", 32, 64, 32, 320},
        {86, "gg_gro", 64, 64, 64, 448},
        {87, "gg_mts", 64, 128, 0, 128},
        {88, "ggmk", 64, 64, 128, 448},
        {89, "ggmt", 128, 128, 0, 0},
        {90, "ggmt_e", 128, 32, 0, 96},
        {91, "ggmt_ed", 128, 32, 128, 96},
        {92, "ggmt_eg", 32, 32, 96, 224},
        {93, "ggmtd", 128, 128, 128, 0},
        {94, "ggs_g", 32, 32, 32, 64},
        {95, "ggshr", 32, 32, 192, 96},
        {96, "ggshrg", 32, 32, 224, 96},
        {97, "gia", 64, 32, 64, 224},
        {98, "gia2", 64, 32, 0, 224},
        {99, "gia_d", 64, 32, 128, 224},
        {100, "gia_d2", 64, 32, 64, 224},
        {101, "gia_g", 32, 32, 192, 224},
        {102, "gia_g2", 32, 32, 128, 224},
        {103, "gmt_eda", 32, 32, 0, 352},
        {104, "gonclf", 128, 64, 96, 64},
        {105, "gredge", 32, 32, 192, 192},
        {106, "hyouga", 64, 64, 192, 64},
        {107, "iceclf", 64, 32, 64, 96},
        {108, "iceclfd", 64, 32, 128, 96},
        {109, "iceclfg", 32, 32, 32, 224},
        {110, "jun", 64, 64, 192, 192},
        {111, "jun_d", 64, 64, 128, 192},
        {112, "jun_e", 64, 16, 0, 240},
        {113, "jun_gro", 64, 64, 0, 64},
        {114, "junmk", 32, 32, 0, 96},
        {115, "junn01", 32, 32, 160, 112},
        {116, "junn02", 32, 32, 192, 112},
        {117, "junn03", 32, 32, 224, 112},
        {118, "junn04", 32, 32, 64, 128},
        {119, "jutmpl01", 64, 64, 128, 192},
        {120, "lake-e", 32, 32, 96, 192},
        {121, "lake_ef", 32, 32, 128, 224},
        {122, "lake_fl", 128, 32, 160, 224},
        {123, "lostclf", 32, 64, 128, 384},
        {124, "lostmt", 128, 32, 128, 448},
        {125, "lostmtd", 128, 32, 128, 480},
        {126, "lostmts", 64, 32, 160, 384},
        {127, "lostwd_e", 32, 32, 64, 480},
        {128, "lostwod", 64, 64, 0, 448},
        {129, "lst1", 32, 32, 192, 256},
        {130, "lstwd_e2", 32, 32, 96, 480},
        {131, "mzes", 32, 32, 224, 128},
        {132, "mzmt_e", 128, 64, 128, 64},
        {133, "mzmt_ed", 128, 32, 128, 128},
        {134, "mzmt_edw", 128, 32, 128, 160},
        {135, "mzmt_ew", 128, 32, 0, 128},
        {136, "mzmt_o", 128, 32, 64, 416},
        {137, "mzmt_od", 128, 32, 64, 448},
        {138, "mzmt_s", 128, 32, 0, 192},
        {139, "mzmt_sd", 128, 32, 0, 160},
        {140, "md01", 32, 32, 96, 16},
        {141, "md02", 128, 128, 0, 0},
        {142, "md03", 16, 16, 112, 64},
        {143, "md04", 32, 32, 128, 16},
        {144, "md05", 64, 16, 96, 0},
        {145, "md06", 16, 32, 96, 48},
        {146, "md07", 16, 16, 112, 48},
        {147, "md_mt", 128, 128, 128, 0},
        {148, "md_mtd", 128, 128, 0, 0},
        {149, "md_mts", 64, 128, 64, 160},
        {150, "md_snow", 128, 32, 128, 0},
        {151, "md_snw2", 128, 32, 128, 32},
        {152, "md_snwd", 128, 64, 0, 128},
        {153, "md_snwe", 64, 64, 96, 320},
        {154, "md_snws", 64, 64, 128, 128},
        {155, "md_snwt", 128, 32, 0, 192},
        {156, "md_snww", 32, 32, 224, 224},
        {157, "md_sw_s", 128, 128, 0, 0},
        {158, "md_swd2", 32, 32, 192, 256},
        {159, "md_swnp", 128, 128, 0, 96},
        {160, "mdsrt_e", 128, 32, 128, 192},
        {161, "mdsrt_ed", 128, 32, 128, 224},
        {162, "mdsrt_eg", 32, 32, 64, 224},
        {163, "midil", 32, 32, 32, 192},
        {164, "midild", 32, 32, 224, 192},
        {165, "mt_ewg", 32, 32, 64, 96},
        {166, "mt_road", 64, 64, 192, 128},
        {167, "mt_se", 32, 32, 160, 416},
        {168, "mt_se2", 64, 64, 128, 256},
        {169, "mt_sg01", 32, 32, 0, 224},
        {170, "mt_sg02", 32, 32, 32, 224},
        {171, "mt_sg03", 32, 32, 0, 192},
        {172, "mt_sg04", 32, 32, 160, 384},
        {173, "mtcoin", 64, 64, 0, 256},
        {174, "mtwas_e", 128, 32, 0, 224},
        {175, "mtwas_ed", 128, 32, 0, 224},
        {176, "ncol_gro", 64, 64, 64, 384},
        {177, "ncole01", 32, 32, 224, 384},
        {178, "ncole02", 32, 32, 192, 416},
        {179, "nivl_gro", 64, 64, 128, 384},
        {180, "nivl_mt", 128, 64, 0, 0},
        {181, "nivl_top", 32, 32, 0, 64},
        {182, "nivlr", 32, 32, 192, 384},
        {183, "port", 32, 32, 96, 224},
        {184, "port_d", 32, 32, 160, 0},
        {185, "rzclf02", 64, 64, 128, 128},
        {186, "rct_gro", 64, 128, 0, 416},
        {187, "riv_cls", 64, 64, 64, 0},
        {188, "riv_l1", 32, 32, 96, 320},
        {189, "riv_m", 32, 32, 0, 224},
        {190, "rivr", 32, 32, 64, 224},
        {191, "rivrclf", 64, 64, 128, 192},
        {192, "rivs1", 32, 32, 128, 320},
        {193, "rivshr", 64, 64, 192, 192},
        {194, "rivssr", 64, 32, 192, 224},
        {195, "rivstrt", 32, 32, 192, 160},
        {196, "rm1", 32, 32, 32, 288},
        {197, "rocet", 32, 32, 128, 160},
        {198, "rs_ss", 32, 32, 96, 224},
        {199, "sango", 32, 32, 224, 320},
        {200, "sango2", 32, 32, 224, 352},
        {201, "sango3", 32, 32, 128, 384},
        {202, "sango4", 64, 64, 0, 384},
        {203, "sdun", 64, 64, 0, 160},
        {204, "sdun02", 64, 64, 64, 160},
        {205, "sh1", 32, 32, 32, 256},
        {206, "sh_s1", 32, 32, 224, 288},
        {207, "shedge", 32, 64, 160, 160},
        {208, "shlm_1", 32, 32, 192, 320},
        {209, "shol", 128, 128, 128, 96},
        {210, "shol_s", 64, 64, 192, 192},
        {211, "shor", 128, 128, 0, 0},
        {212, "shor_s", 64, 64, 128, 192},
        {213, "shor_s2", 32, 32, 224, 416},
        {214, "shor_v", 32, 32, 192, 0},
        {215, "silo", 32, 32, 224, 32},
        {216, "slope", 128, 32, 0, 384},
        {217, "snow_es", 32, 32, 192, 480},
        {218, "snow_es2", 32, 32, 224, 480},
        {219, "snow_es3", 32, 32, 224, 448},
        {220, "snw_mt", 128, 128, 0, 0},
        {221, "snw_mtd", 128, 128, 128, 0},
        {222, "snw_mte", 64, 32, 0, 96},
        {223, "snw_mted", 64, 32, 64, 96},
        {224, "snw_mts", 64, 128, 64, 0},
        {225, "snw_mts2", 64, 32, 128, 192},
        {226, "snwfld", 64, 64, 0, 64},
        {227, "snwfld_s", 64, 32, 128, 128},
        {228, "snwgra", 64, 64, 192, 192},
        {229, "snwhm01", 32, 32, 32, 0},
        {230, "snwhm02", 32, 32, 32, 32},
        {231, "snwods", 32, 32, 224, 192},
        {232, "snwood", 64, 64, 192, 128},
        {233, "snwtrk", 32, 64, 96, 256},
        {234, "sse_s1", 32, 32, 32, 320},
        {235, "ssee1", 32, 32, 64, 288},
        {236, "sst1", 32, 32, 224, 256},
        {237, "stown_r", 32, 32, 192, 256},
        {238, "stw_gro", 64, 64, 0, 384},
        {239, "subrg2", 32, 32, 224, 160},
        {240, "susbrg", 64, 64, 192, 96},
        {241, "sw_se", 64, 64, 0, 0},
        {242, "swclf_l", 64, 64, 64, 128},
        {243, "swclf_ld", 64, 64, 192, 128},
        {244, "swclf_lg", 32, 64, 0, 192},
        {245, "swclf_s", 64, 32, 128, 96},
        {246, "swclf_sd", 64, 32, 192, 96},
        {247, "swclf_sg", 32, 32, 32, 192},
        {248, "swclf_wg", 32, 32, 192, 192},
        {249, "swfld_s2", 64, 32, 128, 160},
        {250, "swfld_s3", 32, 32, 160, 192},
        {251, "swmd_cg", 32, 32, 128, 192},
        {252, "swmd_clf", 64, 32, 64, 192},
        {253, "swp1", 32, 32, 0, 288},
        {254, "trk", 64, 64, 128, 0},
        {255, "tyo_f", 128, 128, 128, 128},
        {256, "tyosnw", 64, 128, 64, 384},
        {257, "uf1", 32, 32, 160, 256},
        {258, "utai01", 32, 32, 32, 96},
        {259, "utai02", 32, 32, 224, 64},
        {260, "utai_gro", 64, 64, 128, 96},
        {261, "utaimt", 32, 32, 0, 128},
        {262, "utaimtd", 32, 32, 96, 96},
        {263, "utaimtg", 32, 32, 96, 128},
        {264, "wa1", 32, 32, 192, 320},
        {265, "wzs1", 32, 32, 128, 288},
        {266, "wzshr", 32, 32, 160, 32},
        {267, "wzshr2", 32, 32, 32, 128},
        {268, "wzshrs", 32, 32, 32, 160},
        {269, "was", 128, 128, 0, 96},
        {270, "was_d", 64, 32, 0, 224},
        {271, "was_g", 64, 64, 0, 192},
        {272, "was_s", 128, 128, 128, 0},
        {273, "wasfld", 64, 64, 64, 256},
        {274, "wdedge", 64, 64, 64, 160},
        {275, "we1", 32, 32, 96, 256},
        {276, "we_s1", 32, 32, 160, 288},
        {277, "wedged", 32, 64, 128, 160},
        {278, "wod-e2", 32, 32, 64, 224},
        {279, "wood", 64, 64, 192, 0},
        {280, "wood_d", 64, 64, 192, 160},
        {281, "wtrk", 32, 64, 64, 96}
    };

    static const char* WORLD_MAP_RESOURCE_GROUP = "WORLD_MAP";

    static bool IsFootWalkable(uint8 walkability_type){
      using WalkType = VGears::WorldmapFileSerializer::TriangleWalkMapTypes;
      switch (walkability_type){
        case WalkType::GRASS:
        case WalkType::FOREST:
        case WalkType::SWAMP:
        case WalkType::DESERT:
        case WalkType::WASTELAND:
        case WalkType::SNOW:
        case WalkType::RIVERSIDE:
        case WalkType::CLIFF:
        case WalkType::COREL_BRIDGE:
        case WalkType::WUTAI_BRIDGE:
        case WalkType::HILL_SIDE:
        case WalkType::BEACH:
        case WalkType::CANYON:
        case WalkType::MOUNTAIN_PASS:
        case WalkType::GOLD_SAUCER_DESERT:
        case WalkType::JUNGLE:
        case WalkType::NORTHERND_CAVE:
        case WalkType::GOLD_SAUCER_DESERT_BORDER:
        case WalkType::BRIDGE_HEAD:
        case WalkType::BACK_ENTRANCE:
          return true;
        default:
          return false;
      }
    }

    static const std::map<Ogre::String, uint8>& GetWorldMapTerrainTypes(){
      using WalkType = VGears::WorldmapFileSerializer::TriangleWalkMapTypes;
      static const std::map<Ogre::String, uint8> terrain_types = {
        {"GRASS", static_cast<uint8>(WalkType::GRASS)},
        {"FOREST", static_cast<uint8>(WalkType::FOREST)},
        {"MOUNTAIN", static_cast<uint8>(WalkType::MOUNTAIN)},
        {"SEA_DEEP_WATER", static_cast<uint8>(WalkType::SEA_DEEP_WATER)},
        {"RIVER_CROSSING", static_cast<uint8>(WalkType::RIVER_CROSSING)},
        {"RIVER", static_cast<uint8>(WalkType::RIVER)},
        {"WATER", static_cast<uint8>(WalkType::WATER)},
        {"SWAMP", static_cast<uint8>(WalkType::SWAMP)},
        {"DESERT", static_cast<uint8>(WalkType::DESERT)},
        {"WASTELAND", static_cast<uint8>(WalkType::WASTELAND)},
        {"SNOW", static_cast<uint8>(WalkType::SNOW)},
        {"RIVERSIDE", static_cast<uint8>(WalkType::RIVERSIDE)},
        {"CLIFF", static_cast<uint8>(WalkType::CLIFF)},
        {"COREL_BRIDGE", static_cast<uint8>(WalkType::COREL_BRIDGE)},
        {"WUTAI_BRIDGE", static_cast<uint8>(WalkType::WUTAI_BRIDGE)},
        {"UNUSED_1", static_cast<uint8>(WalkType::UNUSED_1)},
        {"HILL_SIDE", static_cast<uint8>(WalkType::HILL_SIDE)},
        {"BEACH", static_cast<uint8>(WalkType::BEACH)},
        {"SUBMARINE_PEN", static_cast<uint8>(WalkType::SUBMARINE_PEN)},
        {"CANYON", static_cast<uint8>(WalkType::CANYON)},
        {"MOUNTAIN_PASS", static_cast<uint8>(WalkType::MOUNTAIN_PASS)},
        {"UNKNOWN_1", static_cast<uint8>(WalkType::UNKNOWN_1)},
        {"WATERFALL", static_cast<uint8>(WalkType::WATERFALL)},
        {"UNUSED_2", static_cast<uint8>(WalkType::UNUSED_2)},
        {"GOLD_SAUCER_DESERT", static_cast<uint8>(WalkType::GOLD_SAUCER_DESERT)},
        {"JUNGLE", static_cast<uint8>(WalkType::JUNGLE)},
        {"SEA", static_cast<uint8>(WalkType::SEA)},
        {"NORTHERND_CAVE", static_cast<uint8>(WalkType::NORTHERND_CAVE)},
        {"GOLD_SAUCER_DESERT_BORDER", static_cast<uint8>(WalkType::GOLD_SAUCER_DESERT_BORDER)},
        {"BRIDGE_HEAD", static_cast<uint8>(WalkType::BRIDGE_HEAD)},
        {"BACK_ENTRANCE", static_cast<uint8>(WalkType::BACK_ENTRANCE)},
        {"UNUSED_3", static_cast<uint8>(WalkType::UNUSED_3)}
      };
      return terrain_types;
    }

      static Ogre::String GetWorldMapTerrainName(const uint8 walkability_type){
        for (const auto& terrain : GetWorldMapTerrainTypes()){
          if (terrain.second == walkability_type) return terrain.first;
        }
        return "";
      }

    /**
     * Loads and parses the installed WM0 map.
     *
     * @return The map blocks.
     */
    static std::vector<VGears::WorldmapFileSerializer::Block> LoadWorldMap(){
        /**
       * The serializer retains its parsed blocks; the resource argument is
       * currently unused by the MAP serializer.
         */
        class TestFile : public VGears::WorldmapFile{

            public:

                TestFile() : VGears::WorldmapFile(nullptr, "", 0, "") {}
        };

        Ogre::DataStreamPtr stream
          = Ogre::ResourceGroupManager::getSingleton().openResource(
              "WM0.MAP", WORLD_MAP_RESOURCE_GROUP, nullptr, true
            );
        TestFile file;
        VGears::WorldmapFileSerializer serializer;
        serializer.ImportWorldmapFile(stream, file);

        Ogre::LogManager::getSingleton().logMessage(
          "Loaded WM0.MAP with " + std::to_string(serializer.blocks.size())
            + " blocks."
        );
        return serializer.blocks;
    }

    /**
     * Creates a 16 bit point from coordinates.
     *
     * @param[in] coord The coordinates.
     * @return Coordinate point.
     */
    static float From16BitFixedPoint(VGears::sint16 coord){
        // return float(coord);
        return (float(coord) / 4096.0f);
    }

    /**
     * Converts a coordinate to a texture U component coordinate.
     *
     * @param[in] coord The coordinate to convert.
     * @param[in] id Texture ID.
     * @return Texture coordinate U component.
     * @todo Understand and document properly.
     */
    static float ToTextureCoordU(uint8 coord, uint16 id){
        return (float(coord) - float(textures[id].u)) / float(textures[id].width);
        //return (float(textures[id].u) + float(coord)) / 256.0f;
    }

    /**
     * Converts a coordinate to a texture V component coordinate.
     *
     * @param[in] coord The coordinate to convert.
     * @param[in] id Texture ID.
     * @return Texture coordinate V component.
     * @todo Understand and document properly.
     */
    static float ToTextureCoordV(uint8 coord, uint16 id){
        return (float(coord) - float(textures[id].v)) / float(textures[id].height);
        //return (float(textures[id].v) + float(coord)) / 640.0f;
    }

    struct ImageDataDeleter{

      void operator()(Ogre::ImageCodec::ImageData* image_data) const{
        OGRE_DELETE image_data;
      }
    };

    static TexturePtr LoadWorldMapTexture(uint16 texture_id){
        const std::string texture_name
          = std::string(textures[texture_id].name) + ".tex";
        Ogre::DataStreamPtr stream
          = ResourceGroupManager::getSingleton().openResource(
              texture_name, WORLD_MAP_RESOURCE_GROUP, nullptr, true
            );
        VGears::TexFile tex_file;
        tex_file.Read(stream);

        Ogre::MemoryDataStreamPtr pixel_data(tex_file.GetPixelData());
        std::unique_ptr<Ogre::ImageCodec::ImageData, ImageDataDeleter> image_data(
          tex_file.GetImageData()
        );
        TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(
          std::string("WorldMapTex_") + textures[texture_id].name,
          ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
          image_data->width, image_data->height, 0, image_data->format, TU_STATIC
        );
        texture->getBuffer()->blitFromMemory(Ogre::PixelBox(
          image_data->width, image_data->height, 1, image_data->format,
          pixel_data->getPtr()
        ));
        return texture;
    }

    static MaterialPtr LoadTextureMaterial(uint16 texture_id){
        TexturePtr texture = LoadWorldMapTexture(texture_id);
        const std::string material_name
          = std::string("WorldMap_") + textures[texture_id].name;
        MaterialPtr material
          = Ogre::MaterialManager::getSingleton().getByName(material_name);
        if (material.isNull()){
            material = Ogre::MaterialManager::getSingleton().create(
              material_name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
            );
            Pass* pass = material->getTechnique(0)->getPass(0);
            pass->createTextureUnitState(texture->getName());
            pass->setTextureFiltering(TFO_NONE);
            pass->setLightingEnabled(false);
        }
        return material;
    }

    WorldmapModule::WorldmapModule():
      scene_manager_(nullptr),
      world_map_node_(nullptr),
      player_entity_(nullptr),
      highwind_entity_(nullptr),
      buggy_entity_(nullptr),
      chocobo_entity_(nullptr),
      submarine_entity_(nullptr),
      tiny_bronco_entity_(nullptr),
      current_vehicle_(VehicleType::ON_FOOT),
      player_height_offset_(0),
      active_function_mesh_index_(-1),
      active_function_id_(-1),
      initialized_(false),
      frame_listener_attached_(false),
      cancel_was_pressed_(false),
      encounter_frames_(0),
      encounter_accumulator_(0),
      encounter_last_battle_(-1),
      encounters_locked_(false),
      paused_for_battle_(false)
    {}

    WorldmapModule::~WorldmapModule(){
      if (frame_listener_attached_) Root::getSingleton().removeFrameListener(this);
      if (scene_manager_ != nullptr && world_map_node_ != nullptr){
        scene_manager_->getRootSceneNode()->removeAndDestroyChild("WorldMapTerrain");
      }
    }

    void WorldmapModule::CreateWorldMap(SceneManager* scene_manager){
      scene_manager_ = scene_manager;
      world_map_node_ = scene_manager_->getRootSceneNode()
        ->createChildSceneNode("WorldMapTerrain");
        const std::vector<VGears::WorldmapFileSerializer::Block> blocks
          = LoadWorldMap();
        const size_t texture_count = sizeof(textures) / sizeof(textures[0]);
        const size_t base_block_count = std::min(blocks.size(), size_t(63));
        const float space = From16BitFixedPoint(8182);
        std::map<uint16, ManualObject*> texture_objects;
        std::set<uint16> unavailable_texture_ids;
        unsigned int manual_index = 0;

        for (size_t block_index = 0; block_index < base_block_count; ++block_index){
            const VGears::WorldmapFileSerializer::Block& block = blocks[block_index];
            const size_t map_x = block_index % 9;
            const size_t map_y = block_index / 9;
            for (size_t mesh_index = 0; mesh_index < block.meshes.size(); ++mesh_index){
                const VGears::WorldmapFileSerializer::BlockPart& part
                  = block.meshes[mesh_index];
                const size_t mesh_x = mesh_index % 4;
                const size_t mesh_y = mesh_index / 4;
                const float pos_x = (mesh_x * space) + (map_x * space * 4);
                const float pos_z = (mesh_y * space) + (map_y * space * 4);

                for (const VGears::WorldmapFileSerializer::BlockTriangle& triangle
                  : part.triangles){
                    const VGears::WorldmapFileSerializer::Vertex& vertex_0
                      = part.vertices.at(triangle.vertex_0_index);
                    const VGears::WorldmapFileSerializer::Vertex& vertex_1
                      = part.vertices.at(triangle.vertex_1_index);
                    const VGears::WorldmapFileSerializer::Vertex& vertex_2
                      = part.vertices.at(triangle.vertex_2_index);
                    TerrainTriangle terrain_triangle;
                    terrain_triangle.vertices[0] = Ogre::Vector3(
                      From16BitFixedPoint(vertex_0.x) + pos_x,
                      From16BitFixedPoint(vertex_0.y),
                      From16BitFixedPoint(vertex_0.z) + pos_z
                    );
                    terrain_triangle.vertices[1] = Ogre::Vector3(
                      From16BitFixedPoint(vertex_1.x) + pos_x,
                      From16BitFixedPoint(vertex_1.y),
                      From16BitFixedPoint(vertex_1.z) + pos_z
                    );
                    terrain_triangle.vertices[2] = Ogre::Vector3(
                      From16BitFixedPoint(vertex_2.x) + pos_x,
                      From16BitFixedPoint(vertex_2.y),
                      From16BitFixedPoint(vertex_2.z) + pos_z
                    );
                    terrain_triangle.walkability_type = triangle.walkability_info_;
                    terrain_triangle.function_id = triangle.function_id;
                    terrain_triangle.mesh_x = static_cast<int>(map_x * 4 + mesh_x);
                    terrain_triangle.mesh_y = static_cast<int>(map_y * 4 + mesh_y);
                    // The 5 lower bits of the location are the region of the random encounters.
                    terrain_triangle.region = std::min<int>(triangle.Location & 0x1F, 15);
                    terrain_triangles_.push_back(terrain_triangle);

                    if (unavailable_texture_ids.find(triangle.TextureInfo)
                      != unavailable_texture_ids.end()){
                        continue;
                    }
                    if (triangle.TextureInfo >= texture_count){
                        if (unavailable_texture_ids.insert(triangle.TextureInfo).second){
                            LogManager::getSingleton().logMessage(
                              "Skipping world-map texture ID "
                                + std::to_string(triangle.TextureInfo)
                                + ": no PC texture mapping is available."
                            );
                        }
                        continue;
                    }

                    ManualObject* manual = nullptr;
                    auto object = texture_objects.find(triangle.TextureInfo);
                    if (object == texture_objects.end()){
                        MaterialPtr material;
                        try{
                            material = LoadTextureMaterial(triangle.TextureInfo);
                        }
                        catch (const Ogre::Exception& exception){
                            unavailable_texture_ids.insert(triangle.TextureInfo);
                            LogManager::getSingleton().logMessage(
                              "Failed to load world-map texture "
                                + std::string(textures[triangle.TextureInfo].name)
                                + ": " + exception.getDescription()
                            );
                            continue;
                        }

                        manual = scene_manager->createManualObject(
                          "world_map_surface_" + std::to_string(++manual_index)
                        );
                        manual->begin(material->getName(), RenderOperation::OT_TRIANGLE_LIST);
                        manual->setDebugDisplayEnabled(false);
                        manual->setDynamic(false);
                        texture_objects[triangle.TextureInfo] = manual;
                    }
                    else manual = object->second;

                    manual->position(
                      terrain_triangle.vertices[2].x,
                      terrain_triangle.vertices[2].y,
                      terrain_triangle.vertices[2].z
                    );
                    manual->textureCoord(
                      ToTextureCoordU(triangle.vertex_2_u, triangle.TextureInfo),
                      ToTextureCoordV(triangle.vertex_2_v, triangle.TextureInfo)
                    );

                    manual->position(
                      terrain_triangle.vertices[1].x,
                      terrain_triangle.vertices[1].y,
                      terrain_triangle.vertices[1].z
                    );
                    manual->textureCoord(
                      ToTextureCoordU(triangle.vertex_1_u, triangle.TextureInfo),
                      ToTextureCoordV(triangle.vertex_1_v, triangle.TextureInfo)
                    );

                    manual->position(
                      terrain_triangle.vertices[0].x,
                      terrain_triangle.vertices[0].y,
                      terrain_triangle.vertices[0].z
                    );
                    manual->textureCoord(
                      ToTextureCoordU(triangle.vertex_0_u, triangle.TextureInfo),
                      ToTextureCoordV(triangle.vertex_0_v, triangle.TextureInfo)
                    );
                }
            }
        }

        for (const auto& texture_object : texture_objects){
            texture_object.second->end();
            world_map_node_->attachObject(texture_object.second);
        }

        const float map_center_x = space * 18.0f;
        const float map_center_z = space * 14.0f;
        Ogre::Camera* camera = CameraManager::getSingleton().GetCurrentCamera();
        camera->setPosition(Ogre::Vector3(map_center_x, 80.0f, map_center_z + 90.0f));
        camera->lookAt(Ogre::Vector3(map_center_x, 0.0f, map_center_z));
    }

    void WorldmapModule::CreatePlayer(){
        const Ogre::Vector3 requested_spawn(42.0f, 0.0f, 36.5f);
        Ogre::Vector3 spawn_position;
        if (!FindWalkableSurface(requested_spawn, spawn_position, true)){
            LogManager::getSingleton().logMessage(
              "WM0 has no GRASS surface at Cloud's requested spawn (42, 36.5)."
            );
            return;
        }

        LogManager::getSingleton().logMessage(
          "Spawning Cloud at "
            + Ogre::StringConverter::toString(spawn_position.x) + ", "
            + Ogre::StringConverter::toString(spawn_position.z)
        );

        player_entity_ = EntityManager::getSingleton().AddWorldMapEntity(
          "WorldMapCloud", "cloud.mesh", spawn_position, Ogre::Degree(0),
          Ogre::Vector3::UNIT_SCALE, "FIELD_MODELS"
        );
        if (player_entity_ == nullptr) return;

        // Cloud's bounds without animations. The bounds in the mesh can't be used for sizing: the
        // installer enlarges them with the animations' movement, so the model is not culled.
        const Ogre::AxisAlignedBox bounds(
          -8.7473f, -8.1671f, -4.1878f, 7.8685f, 5.6837f, 22.8135f
        );
        const Ogre::Vector3 model_size = bounds.getSize();
        const Ogre::Real model_height = model_size.y;
        if (model_height > 0){
            const Ogre::Real scale = From16BitFixedPoint(8182) * 0.45f * 0.25f * 0.25f
              / model_height;
            player_entity_->setScale(Ogre::Vector3::UNIT_SCALE * scale);
            player_entity_->SetHeight(model_height * scale);
            player_entity_->SetSolidRadius(
              std::max(model_size.x, model_size.z) * scale * 0.5f
            );
            player_entity_->SetCollisionDebugOrientation(
              Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_X)
            );
            player_height_offset_ = -bounds.getMinimum().y * scale;
            spawn_position.y += player_height_offset_;
            player_entity_->SetPosition(spawn_position);
            Ogre::AnimationStateSet* animations
              = player_entity_->GetModel()->getAllAnimationStates();
            if (animations == nullptr || !animations->hasAnimationState("Walk")){
                LogManager::getSingleton().logMessage(
                  "Cloud model has no Walk animation state."
                );
            }
            EntityManager::getSingleton().ScriptSetPlayerEntity("WorldMapCloud");
            player_entity_->PlayAnimationContinue("Idle");
            CreateCompanionModels();
            UpdateCamera(0, true);
        }
    }

    void WorldmapModule::AddWorldModel(
      const Ogre::String& name, const Ogre::String& mesh_name,
      const Ogre::Vector3& desired_position, Ogre::Real target_size,
      const Ogre::Quaternion& orientation, Ogre::Real collision_radius_scale,
      bool talkable
    ){
        Ogre::Vector3 surface_position;
        if (!FindWalkableSurface(desired_position, surface_position, false, false)){
            LogManager::getSingleton().logMessage(
              "No terrain surface found for world-map model '" + name + "'."
            );
            return;
        }

        EntityModel* entity = nullptr;
        try{
            entity = EntityManager::getSingleton().AddWorldMapEntity(
              "WorldMap" + name, mesh_name, surface_position,
              Ogre::Degree(0), Ogre::Vector3::UNIT_SCALE, "WORLD_MODELS"
            );
        }
        catch (const Ogre::Exception& exception){
            LogManager::getSingleton().logMessage(
              "Failed to load world-map model '" + mesh_name + "': "
                + exception.getDescription()
            );
            return;
        }
        if (entity == nullptr) return;

        const Ogre::Vector3 bounds_size = entity->GetModel()->getBoundingBox().getSize();
        const Ogre::Real largest_dimension = std::max(
          bounds_size.x, std::max(bounds_size.y, bounds_size.z)
        );
        if (largest_dimension <= 0){
            LogManager::getSingleton().logMessage(
              "World-map model '" + mesh_name + "' has empty bounds."
            );
            return;
        }

        const Ogre::Real model_scale = target_size / largest_dimension;
        const Ogre::Vector3 oriented_x = orientation * Ogre::Vector3::UNIT_X;
        const Ogre::Vector3 oriented_y = orientation * Ogre::Vector3::UNIT_Y;
        const Ogre::Vector3 oriented_z = orientation * Ogre::Vector3::UNIT_Z;
        const Ogre::Real world_width =
          Ogre::Math::Abs(oriented_x.x) * bounds_size.x
          + Ogre::Math::Abs(oriented_y.x) * bounds_size.y
          + Ogre::Math::Abs(oriented_z.x) * bounds_size.z;
        const Ogre::Real world_height =
          Ogre::Math::Abs(oriented_x.y) * bounds_size.x
          + Ogre::Math::Abs(oriented_y.y) * bounds_size.y
          + Ogre::Math::Abs(oriented_z.y) * bounds_size.z;
        const Ogre::Real world_depth =
          Ogre::Math::Abs(oriented_x.z) * bounds_size.x
          + Ogre::Math::Abs(oriented_y.z) * bounds_size.y
          + Ogre::Math::Abs(oriented_z.z) * bounds_size.z;
        entity->setRootOrientation(orientation);
        entity->setScale(Ogre::Vector3::UNIT_SCALE * model_scale);
        entity->SetSolid(true);
        entity->SetHeight(world_height * model_scale);
        entity->SetSolidRadius(
          std::max(world_width, world_depth) * model_scale * 0.5f
            * collision_radius_scale
        );
        entity->SetTalkable(talkable);
        if (talkable) entity->SetTalkRadius(entity->GetSolidRadius());
        entity->SetCollisionDebugOrientation(
          Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_X)
        );
        LogManager::getSingleton().logMessage(
          "Placed world-map model '" + name + "' at "
            + Ogre::StringConverter::toString(surface_position.x) + ", "
            + Ogre::StringConverter::toString(surface_position.z)
        );
    }

    void WorldmapModule::CreateCompanionModels(){
        // These models and their spawn points are now hardcoded, they mmust be
        // loaded according to the story point and save game data.
        const Ogre::Real space = From16BitFixedPoint(8182);
        AddWorldModel(
          "Chocobo", "aja.mesh",
          Ogre::Vector3(42.5f, 0.0f, 37.5f),
          space * 0.14f,
          Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_X), 1.0f, true
        );
        AddWorldModel(
          "Highwind", "cgd.mesh",
          Ogre::Vector3(43.0f, 0.0f, 36.5f),
          space * 0.25f,
          Ogre::Quaternion(Ogre::Degree(-90), Ogre::Vector3::UNIT_X), 0.5f, true
        );
        AddWorldModel(
          "Buggy", "aba.mesh",
          Ogre::Vector3(42.0f, 0.0f, 36.0f),
          space * 0.3f, Ogre::Quaternion::IDENTITY, 0.5f, true
        );
        AddWorldModel(
          "Submarine", "ddd.mesh",
          Ogre::Vector3(41.5f, 0.0f, 36.5f),
          space * 0.1125f, Ogre::Quaternion::IDENTITY, 1.0f, true
        );
        AddWorldModel(
          "TinyBronco", "dva.mesh",
          Ogre::Vector3(42.0f, 0.0f, 37.37f),
          space * 0.15f, Ogre::Quaternion::IDENTITY, 0.75f, true
        );
        highwind_entity_ = static_cast<EntityModel*>(
          EntityManager::getSingleton().GetEntity("WorldMapHighwind")
        );
        buggy_entity_ = static_cast<EntityModel*>(
          EntityManager::getSingleton().GetEntity("WorldMapBuggy")
        );
        chocobo_entity_ = static_cast<EntityModel*>(
          EntityManager::getSingleton().GetEntity("WorldMapChocobo")
        );
        submarine_entity_ = static_cast<EntityModel*>(
          EntityManager::getSingleton().GetEntity("WorldMapSubmarine")
        );
        tiny_bronco_entity_ = static_cast<EntityModel*>(
          EntityManager::getSingleton().GetEntity("WorldMapTinyBronco")
        );
    }

    bool WorldmapModule::FindWalkableSurface(
      const Ogre::Vector3& position, Ogre::Vector3& surface, bool grass_only,
      bool require_walkable, uint8* walkability_type, size_t* triangle_index
    ) const{
        bool found = false;
        Ogre::Real closest_height_distance = std::numeric_limits<Ogre::Real>::max();
        for (size_t i = 0; i < terrain_triangles_.size(); ++i){
            const TerrainTriangle& triangle = terrain_triangles_[i];
            if (grass_only
              && triangle.walkability_type != VGears::WorldmapFileSerializer::GRASS){
                continue;
            }
            const Ogre::Vector3& a = triangle.vertices[0];
            const Ogre::Vector3& b = triangle.vertices[1];
            const Ogre::Vector3& c = triangle.vertices[2];
            const Ogre::Real denominator
              = (b.z - c.z) * (a.x - c.x) + (c.x - b.x) * (a.z - c.z);
            if (Ogre::Math::Abs(denominator) < 1e-6f) continue;

            const Ogre::Real weight_a
              = ((b.z - c.z) * (position.x - c.x)
                + (c.x - b.x) * (position.z - c.z)) / denominator;
            const Ogre::Real weight_b
              = ((c.z - a.z) * (position.x - c.x)
                + (a.x - c.x) * (position.z - c.z)) / denominator;
            const Ogre::Real weight_c = 1.0f - weight_a - weight_b;
            if (weight_a < -1e-4f || weight_b < -1e-4f || weight_c < -1e-4f){
                continue;
            }

            const Ogre::Real height
              = weight_a * a.y + weight_b * b.y + weight_c * c.y;
            const Ogre::Real height_distance = Ogre::Math::Abs(height - position.y);
            if (height_distance >= closest_height_distance) continue;
            closest_height_distance = height_distance;
            found = !require_walkable || IsActiveTerrainAllowed(triangle.walkability_type);
            surface = Ogre::Vector3(position.x, height, position.z);
            if (walkability_type != nullptr) *walkability_type = triangle.walkability_type;
            if (triangle_index != nullptr) *triangle_index = i;
        }
        return found;
    }

    WorldmapModule::VehicleType WorldmapModule::GetCurrentVehicle() const{
      return current_vehicle_;
    }

    void WorldmapModule::ClearVehicleTerrains(){
        active_vehicle_terrain_types_.clear();
    }

    void WorldmapModule::AddVehicleTerrain(const char* terrain_name){
        if (terrain_name == nullptr) return;
        const auto terrain = GetWorldMapTerrainTypes().find(terrain_name);
        if (terrain == GetWorldMapTerrainTypes().end()){
            LogManager::getSingleton().logMessage(
              "Unknown world-map vehicle terrain '" + Ogre::String(terrain_name) + "'."
            );
            return;
        }
        if (std::find(
              active_vehicle_terrain_types_.begin(), active_vehicle_terrain_types_.end(),
              terrain->second
            ) == active_vehicle_terrain_types_.end()){
            active_vehicle_terrain_types_.push_back(terrain->second);
        }
    }

    bool WorldmapModule::IsActiveTerrainAllowed(uint8 walkability_type) const{
        if (current_vehicle_ == VehicleType::ON_FOOT){
            return IsFootWalkable(walkability_type);
        }
        return std::find(
          active_vehicle_terrain_types_.begin(), active_vehicle_terrain_types_.end(),
          walkability_type
        ) != active_vehicle_terrain_types_.end();
    }

    void WorldmapModule::TriggerTriangleFunction(const TerrainTriangle& triangle){
        ScriptEntity* world_map_script
          = ScriptManager::getSingleton().GetScriptEntityByName(
              ScriptManager::SYSTEM, "WorldMap"
            );
        if (world_map_script == nullptr) return;

        const int mesh_index = triangle.mesh_y * 36 + triangle.mesh_x;
        ScriptManager::getSingleton().ScriptRequest(
          world_map_script, "on_triangle_enter", 1,
          std::to_string(mesh_index), std::to_string(triangle.function_id - 3), false, false
        );
    }

      Ogre::String WorldmapModule::GetTerrainTypeUnderEntity(const char* entity_name) const{
        if (entity_name == nullptr) return "";
        const ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return "";

        Ogre::Vector3 surface;
        uint8 walkability_type = 0;
        if (!FindWalkableSurface(entity->GetPosition(), surface, false, false, &walkability_type)){
          return "";
        }
        return GetWorldMapTerrainName(walkability_type);
      }

      int WorldmapModule::GetTerrainTriangleUnderEntity(const char* entity_name) const{
        if (entity_name == nullptr) return -1;
        const ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return -1;

        Ogre::Vector3 surface;
        size_t triangle_index = 0;
        if (!FindWalkableSurface(entity->GetPosition(), surface, false, false, nullptr, &triangle_index)){
          return -1;
        }
        return static_cast<int>(triangle_index);
      }

      bool WorldmapModule::UnboardVehicle(const char* vehicle_name){
        if (vehicle_name == nullptr) return false;

        EntityManager& entity_manager = EntityManager::getSingleton();
        ::Entity* player_entity = entity_manager.ScriptGetPlayerEntity();
        ::Entity* vehicle_entity = entity_manager.GetEntity(vehicle_name);
        if (
          player_entity == nullptr || vehicle_entity == nullptr
          || player_entity != vehicle_entity || player_entity == player_entity_
        ){
          return false;
        }

        const Ogre::Vector3 vehicle_position = vehicle_entity->GetPosition();
        const Ogre::Real unboard_distance
          = vehicle_entity->GetSolidRadius() + player_entity_->GetSolidRadius() + 0.1f;
        const Ogre::Real diagonal_distance = unboard_distance * 0.70710678f;
        const Ogre::Vector3 directions[] = {
          Ogre::Vector3(1, 0, 0), Ogre::Vector3(-1, 0, 0),
          Ogre::Vector3(0, 0, 1), Ogre::Vector3(0, 0, -1),
          Ogre::Vector3(1, 0, 1), Ogre::Vector3(-1, 0, 1),
          Ogre::Vector3(1, 0, -1), Ogre::Vector3(-1, 0, -1)
        };

        for (size_t i = 0; i < sizeof(directions) / sizeof(directions[0]); ++i){
          const Ogre::Real distance = i < 4 ? unboard_distance : diagonal_distance;
          const Ogre::Vector3 requested_position
            = vehicle_position + directions[i] * distance;
          Ogre::Vector3 surface;
          uint8 walkability_type = 0;
          if (!FindWalkableSurface(
              requested_position, surface, false, false, &walkability_type
            ) || !IsFootWalkable(walkability_type)){
            continue;
          }
          surface.y += player_height_offset_;
          if (entity_manager.IsWorldPositionBlocked(player_entity_, surface)) continue;

          player_entity_->SetPosition(surface);
          vehicle_entity->SetTalkable(true);
          entity_manager.ScriptSetPlayerEntity("WorldMapCloud");
          ClearVehicleTerrains();
          current_vehicle_ = VehicleType::ON_FOOT;
          return true;
        }
        return false;
      }

    bool WorldmapModule::frameStarted(const Ogre::FrameEvent& event){
      if (!initialized_ || player_entity_ == nullptr || paused_for_battle_) return true;

        ::Entity* active_entity = EntityManager::getSingleton().ScriptGetPlayerEntity();
      if (active_entity == player_entity_){
        current_vehicle_ = VehicleType::ON_FOOT;
      }
      else if (active_entity == chocobo_entity_){
        current_vehicle_ = VehicleType::CHOCOBO;
      }
      else if (active_entity == buggy_entity_){
        current_vehicle_ = VehicleType::BUGGY;
      }
      else if (active_entity == highwind_entity_){
        current_vehicle_ = VehicleType::HIGHWIND;
      }
      else if (active_entity == submarine_entity_){
        current_vehicle_ = VehicleType::SUBMARINE;
      }
      else if (active_entity == tiny_bronco_entity_){
        current_vehicle_ = VehicleType::TINY_BRONCO;
      }
      else{
        return true;
      }

        const bool highwind_active = active_entity == highwind_entity_;
      const bool vehicle_active = current_vehicle_ != VehicleType::ON_FOOT;
        const bool cancel_pressed = InputManager::getSingleton().IsGameEventPressed("CANCEL");
        if (vehicle_active && cancel_pressed && !cancel_was_pressed_){
            ScriptManager::getSingleton().ScriptRequest(
              ScriptManager::ENTITY, active_entity->GetName().c_str(), "unboard", 0
            );
        }
        cancel_was_pressed_ = cancel_pressed;
      EntityModel* active_model = active_entity == player_entity_ ? player_entity_
        : (current_vehicle_ == VehicleType::CHOCOBO ? chocobo_entity_
        : (current_vehicle_ == VehicleType::BUGGY ? buggy_entity_
          : (highwind_active ? highwind_entity_
            : (current_vehicle_ == VehicleType::SUBMARINE
              ? submarine_entity_ : tiny_bronco_entity_))));

        Ogre::Vector3 movement = Ogre::Vector3::ZERO;
        // Scripts lock the player during events and transitions.
        if (EntityManager::getSingleton().IsPlayerLocked()){
            // Nothing: no movement.
        }
        else{
            // The game events of the movement, so the keys of the controller settings are used.
            InputManager& input = InputManager::getSingleton();
            if (input.IsGameEventPressed("walk_left")) movement.x -= 1.0f;
            if (input.IsGameEventPressed("walk_right")) movement.x += 1.0f;
            if (input.IsGameEventPressed("walk_up")) movement.z -= 1.0f;
            if (input.IsGameEventPressed("walk_down")) movement.z += 1.0f;
        }
        bool is_walking = false;
        if (!movement.isZeroLength()){
          movement.normalise();
            const Ogre::Vector3 current_position = active_entity->GetPosition();
          const Ogre::Real speed = From16BitFixedPoint(8182) * 0.45f
            * (vehicle_active ? 2.0f : 1.0f);
          Ogre::Vector3 requested_position = current_position
            + movement * speed * event.timeSinceLastFrame;
          Ogre::Vector3 surface;
          size_t triangle_index = 0;
          if (FindWalkableSurface(requested_position, surface, false, true, nullptr,
              &triangle_index)){
                if (highwind_active){
                    Ogre::Vector3 current_surface;
                    if (FindWalkableSurface(current_position, current_surface, false, false)){
                        surface.y += current_position.y - current_surface.y;
                    }
                }
                else if (active_entity == player_entity_){
                  surface.y += player_height_offset_;
                }
                if (!EntityManager::getSingleton().IsWorldPositionBlocked(
                  active_entity, surface
                )){
                    active_entity->SetPosition(surface);
                    const TerrainTriangle& triangle = terrain_triangles_[triangle_index];
                    const int mesh_index = triangle.mesh_y * 36 + triangle.mesh_x;
                    if (triangle.function_id >= 3){
                        if (mesh_index != active_function_mesh_index_
                          || triangle.function_id != active_function_id_){
                            TriggerTriangleFunction(triangle);
                        }
                        active_function_mesh_index_ = mesh_index;
                        active_function_id_ = triangle.function_id;
                    }
                    else{
                        active_function_mesh_index_ = -1;
                        active_function_id_ = -1;
                    }
                    if (current_vehicle_ == VehicleType::ON_FOOT){
                        UpdateEncounters(event.timeSinceLastFrame, triangle);
                    }
                    const Ogre::Vector3 model_forward(0.0f, 0.0f, 1.0f);
                    const Ogre::Vector3 move_direction(movement.x, 0.0f, movement.z);
                    const Ogre::Quaternion heading
                      = model_forward.getRotationTo(move_direction, Ogre::Vector3::UNIT_Y);
                    active_model->GetModel()->getParentSceneNode()
                      ->getParentSceneNode()->setOrientation(heading);
                    is_walking = true;
                }
          }
        }

        if (!vehicle_active){
            player_entity_->PlayAnimationContinue(is_walking ? "Walk" : "Idle");
        }

        UpdateCamera(event.timeSinceLastFrame, false);
        return true;
    }

      void WorldmapModule::UpdateCamera(Ogre::Real delta_time, bool snap_to_player){
        ::Entity* active_entity = EntityManager::getSingleton().ScriptGetPlayerEntity();
        if (active_entity == nullptr) return;
        const Ogre::Vector3 target
          = active_entity->GetPosition() + Ogre::Vector3(0, active_entity->GetHeight() * 0.5f, 0);
        const Ogre::Vector3 camera_offset(0.0f, 1.5f, 2.25f);
        Ogre::Camera* camera = CameraManager::getSingleton().GetCurrentCamera();
        const Ogre::Vector3 desired_position = target + camera_offset;
        if (snap_to_player){
          camera->setPosition(desired_position);
        }
        else{
          const Ogre::Real follow_amount = std::min(1.0f, delta_time * 8.0f);
          camera->setPosition(
            camera->getPosition()
            + (desired_position - camera->getPosition()) * follow_amount
          );
        }
        camera->lookAt(target);
      }

    void WorldmapModule::LoadEncounters(){
        encounter_records_.clear();
        TiXmlDocument document("./data/gamedata/world_encounters.xml");
        if (!document.LoadFile()){
            LOG_WARNING("World map encounters were not found. There will be no random battles.");
            return;
        }
        TiXmlHandle handle(&document);
        for (
          TiXmlElement* record = handle.FirstChildElement("world_encounters")
            .FirstChildElement("record").ToElement();
          record != nullptr; record = record->NextSiblingElement("record")
        ){
            EncounterRecord data;
            int enabled = 0;
            data.rate = 0;
            data.terrain = 0;
            record->QueryIntAttribute("enabled", &enabled);
            record->QueryIntAttribute("rate", &data.rate);
            record->QueryIntAttribute("terrain", &data.terrain);
            data.enabled = enabled != 0;
            for (unsigned int i = 0; i < 15; ++ i) data.words[i] = 0;
            std::istringstream words(record->Attribute("words") ? record->Attribute("words") : "");
            for (unsigned int i = 0; i < 15; ++ i){
                unsigned int word = 0;
                if (!(words >> word)) break;
                data.words[i] = static_cast<uint16>(word);
            }
            encounter_records_.push_back(data);
        }
        LOG_TRIVIAL("World map encounters loaded: " + std::to_string(encounter_records_.size()));
    }

    int WorldmapModule::RollEncounterBattle(const EncounterRecord& record) const{
        // Same order as the original game: the special battles first, then the standard ones. The
        // chance of a word is its probability out of 64, as the roll is a 16 bit value and the
        // probability sits in the 6 highest bits.
        const uint16* w = record.words;
        int battle = -1;
        int roll = (std::rand() % 256) << 8;
        // Words 7 and 8 (indices 6 and 7): back attacks.
        if (w[6] > roll) battle = w[6] & 0x3FF;
        else if (w[6] + w[7] > roll) battle = w[7] & 0x3FF;
        // Word 9: side attack. Needs a party of more than one member.
        if (battle < 0){
            roll = (std::rand() % 256) << 8;
            if (w[8] > roll) battle = w[8] & 0x3FF;
        }
        // Word 10: pincer attack.
        if (battle < 0){
            roll = (std::rand() % 256) << 8;
            if (w[9] > roll) battle = w[9] & 0x3FF;
        }
        // Words 1-6: standard battles. The last battle is not repeated (one reroll).
        for (unsigned int attempt = 0; battle < 0 && attempt < 2; ++ attempt){
            roll = (std::rand() % 256) << 8;
            int sum = 0;
            for (unsigned int i = 0; i < 6; ++ i){
                sum += w[i];
                if (sum > roll){
                    battle = w[i] & 0x3FF;
                    break;
                }
            }
            if (battle == encounter_last_battle_ && attempt == 0) battle = -1;
        }
        return battle;
    }

    void WorldmapModule::UpdateEncounters(const float delta_time, const TerrainTriangle& triangle){
        if (encounters_locked_ || encounter_records_.size() < 64) return;
        // The original game checks every 16 frames of walking, at 30 frames per second.
        encounter_frames_ += delta_time * 30.0f;
        if (encounter_frames_ < 16.0f) return;
        encounter_frames_ -= 16.0f;

        // Region and terrain group. The terrain types 0x10 and 0x18 count as 0 and 8.
        const int region = std::min<int>(triangle.region, 15);
        int terrain = triangle.walkability_type;
        if (terrain == 0x10) terrain = 0;
        else if (terrain == 0x18) terrain = 8;
        int group = 0;
        for (int i = 0; i < 4; ++ i){
            if (encounter_records_[region * 4 + i].terrain == terrain){
                group = i;
                break;
            }
        }
        const EncounterRecord& record = encounter_records_[region * 4 + group];

        // The accumulator grows on every check, so the chance gets higher until a battle starts.
        const float multiplier = EntityManager::getSingleton().GetEncounterMultiplier();
        if (multiplier <= 0.0f) return;
        const int speed = static_cast<int>(16 * multiplier);
        encounter_accumulator_ += (record.rate != 0) ? (speed << 10) / record.rate : 0x7FFF;
        const int check = std::rand() % 256;
        if (EntityManager::getSingleton().IsDebugEncounters()){
            LOG_TRIVIAL(
              "World encounter check: region " + std::to_string(region) + ", terrain "
              + std::to_string(terrain) + ", group " + std::to_string(group) + ", rate "
              + std::to_string(record.rate) + ", chance " + std::to_string(encounter_accumulator_ >> 8)
              + "/256, roll " + std::to_string(check)
            );
        }
        if (check >= (encounter_accumulator_ >> 8) || !record.enabled) return;
        encounter_accumulator_ = 0;
        const int battle = RollEncounterBattle(record);
        if (battle < 0) return;
        encounter_last_battle_ = battle;
        LOG_TRIVIAL("World random encounter: battle " + std::to_string(battle));
        ScriptManager::getSingleton().RunString("Encounter.start(" + std::to_string(battle) + ")");
    }

    void WorldmapModule::PauseForBattle(){
        paused_for_battle_ = true;
        if (world_map_node_ != nullptr) world_map_node_->setVisible(false);
    }

    void WorldmapModule::ResumeAfterBattle(){
        paused_for_battle_ = false;
        encounter_frames_ = 0;
        if (world_map_node_ != nullptr) world_map_node_->setVisible(true);
        if (player_entity_ != nullptr) UpdateCamera(0, true);
    }

    void WorldmapModule::SetEncountersLocked(const bool locked){encounters_locked_ = locked;}

    void WorldmapModule::Init(){
      if (initialized_){
        if (frame_listener_attached_) return;
        EntityManager::getSingleton().SetWorldModule();
        DialogsManager::getSingleton().SetWorldModule();
        world_map_node_->setVisible(true);
        CreatePlayer();
        Root::getSingleton().addFrameListener(this);
        frame_listener_attached_ = true;
        return;
      }
      EntityManager::getSingleton().SetWorldModule();
        DialogsManager::getSingleton().SetWorldModule();
        CreateWorldMap(Root::getSingleton().getSceneManager("Scene"));
        ScriptManager::getSingleton().RunFile("scripts/worldmap/wm0.lua");
        CreatePlayer();
        LoadEncounters();
        initialized_ = true;
        Root::getSingleton().addFrameListener(this);
      frame_listener_attached_ = true;
    }

    void WorldmapModule::Suspend(){
      if (!frame_listener_attached_) return;
      Root::getSingleton().removeFrameListener(this);
      frame_listener_attached_ = false;
      if (world_map_node_ != nullptr) world_map_node_->setVisible(false);
      player_entity_ = nullptr;
      highwind_entity_ = nullptr;
      buggy_entity_ = nullptr;
      chocobo_entity_ = nullptr;
      submarine_entity_ = nullptr;
      tiny_bronco_entity_ = nullptr;
      current_vehicle_ = VehicleType::ON_FOOT;
      active_vehicle_terrain_types_.clear();
      active_function_mesh_index_ = -1;
      active_function_id_ = -1;
      cancel_was_pressed_ = false;
    }

} // namespace VGears
