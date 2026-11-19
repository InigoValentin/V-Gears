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
#include "core/UiManager.h"
#include "core/UiWidget.h"

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
     * World map IDs and the files of the original game they load. The original game has no WM1.
     * 0 is the overworld, 1 is WM2 and 2 is WM3.
     */
    static const int WORLD_MAP_FILE_NUMBERS[] = {0, 2, 3};
    static const int WORLD_MAP_COUNT = sizeof(WORLD_MAP_FILE_NUMBERS) / sizeof(int);

    /**
     * Size and position of a map in blocks. The blocks of a MAP file are stored in order, row by
     * row. The maps are placed in the 9x7 block grid of the world, since the scripts index the
     * meshes with the position in that grid, and the maps share coordinates: the underwater map
     * WM2 is a part of the world of WM0, so a position of WM0 inside it is the same in WM2.
     *
     * The layout of WM2 was deduced from the edges of its blocks and the mesh coordinates in its
     * script. The one of WM3 is a guess.
     */
    struct MapLayout{
        int width;
        int height;
        int origin_x;
        int origin_y;
    };

    static MapLayout GetMapLayout(const int map_id){
        switch (map_id){
            case 1: return {3, 4, 3, 2};
            case 2: return {2, 2, 0, 0};
            default: return {9, 7, 0, 0};
        }
    }

    /**
     * Gets if a position of the world is inside the blocks of a map.
     */
    static bool IsInsideMap(const int map_id, const float x, const float z){
        const MapLayout layout = GetMapLayout(map_id);
        const float block_size = (8182.0f / 4096.0f) * 4.0f;
        const float bx = x / block_size - layout.origin_x;
        const float bz = z / block_size - layout.origin_y;
        return bx >= 0 && bz >= 0 && bx < layout.width && bz < layout.height;
    }

    /// Rotation speed of the camera with L1 and R1, in radians per second.
    static const float CAMERA_ROTATION_SPEED = 1.5f;

    /// Turning speed of the flying vehicles, in radians per second.
    static const float FLIGHT_TURN_SPEED = 1.5f;

    /// Speed up and down of the flying vehicles, in world units per second.
    static const float FLIGHT_CLIMB_SPEED = 1.0f;

    /// Closest distance of the flying vehicles to the ground.
    static const float FLIGHT_MIN_CLEARANCE = 0.1f;

    /// How much the model of the submarine is under the surface of the water in WM0, in world
    /// units. Found by looking at it.
    static const float SUBMARINE_SUBMERGED_DEPTH = 0.09f;

    /**
     * Moves the model of an entity up or down, without moving the entity.
     *
     * The model is scaled to the size of the world, so the offset has to be divided by that scale
     * to be in world units.
     *
     * @param[in] entity The entity.
     * @param[in] offset Distance in world units. Negative is down.
     */
    static void SetModelHeightOffset(EntityModel* entity, const float offset){
        Ogre::SceneNode* model_node = entity->GetModel()->getParentSceneNode();
        float scale = 1.0f;
        if (model_node->getParentSceneNode() != nullptr){
            scale = model_node->getParentSceneNode()->_getDerivedScale().y;
        }
        if (scale <= 0.0f) scale = 1.0f;
        model_node->setPosition(Ogre::Vector3(0.0f, offset / scale, 0.0f));
    }

    /// Height over the ground where the submarine starts, so it floats.
    static const float SUBMARINE_START_CLEARANCE = 0.3f;

    /// Highest height over the ground where the Highwind can be unboarded. It lands there.
    static const float HIGHWIND_MAX_LANDING_HEIGHT = 1.0f;

    /// Highest point the Highwind can reach.
    static const float HIGHWIND_MAX_HEIGHT = 1.0f;

    /// How deep the submarine goes under the surface of WM0 when diving, in world units.
    static const float DIVE_SINK_DEPTH = 0.5f;

    /// Time the submarine takes to go under the surface of WM0, or to come up to it.
    static const float DIVE_SINK_TIME = 1.0f;

    /// Height where the submarine appears in WM2 when diving, and where it goes when surfacing.
    /// It's over the top layer of the map.
    static const float DIVE_HIGH_HEIGHT = 1.0f;

    /// Time the submarine takes to go down from, or up to, the height above.
    static const float DIVE_HIGH_TIME = 1.5f;

    /// Time of the fade to black before the map changes.
    static const float DIVE_FADE_OUT_TIME = 0.5f;

    /**
     * Highest terrain height the submarine can reach in WM2. The terrain above it is the top
     * layer. The submarine can move over all terrain types below this height.
     *
     * @todo The value was chosen from the height distribution of the WM2 triangles, and not
     * verified against the original game.
     */
    static const float UNDERWATER_MAX_HEIGHT = -0.3f;

    /**
     * Enables the submarine height limit of WM2.
     *
     * The submarine can't go over terrain that is closer than FLIGHT_MIN_CLEARANCE to the
     * ceiling.
     */
    static const bool UNDERWATER_HEIGHT_LIMIT_ENABLED = true;

    /**
     * Width in meshes of the grid that the world map scripts use to index meshes (mesh_y * 36 +
     * mesh_x). It is the same in every map, regardless of the size of the map.
     */
    static const int WORLD_MAP_SCRIPT_MESH_STRIDE = 36;

    /**
     * Loads and parses an installed world map file.
     *
     * @param[in] file_name Name of the MAP file, like "WM0.MAP".
     * @return The map blocks.
     */
    static std::vector<VGears::WorldmapFileSerializer::Block> LoadWorldMap(
      const Ogre::String& file_name
    ){
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
              file_name, WORLD_MAP_RESOURCE_GROUP, nullptr, true
            );
        TestFile file;
        VGears::WorldmapFileSerializer serializer;
        serializer.ImportWorldmapFile(stream, file);

        Ogre::LogManager::getSingleton().logMessage(
          "Loaded " + file_name + " with " + std::to_string(serializer.blocks.size())
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
        const std::string material_name
          = std::string("WorldMap_") + textures[texture_id].name;
        MaterialPtr material
          = Ogre::MaterialManager::getSingleton().getByName(material_name);
        // The textures and materials are kept when a map is replaced, and the maps share them.
        if (material.isNull()){
            TexturePtr texture = LoadWorldMapTexture(texture_id);
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

    /**
     * Gets if a position of the overworld (WM0) is over deep water. Used when the submarine
     * comes back from WM2, when WM0 is not loaded. Only the part of the world that WM2 covers is
     * read, and it's done once.
     */
    static bool IsOverworldDeepWater(const float x, const float z){
        struct Triangle{
            float x[3];
            float z[3];
        };
        static std::vector<Triangle> triangles;
        static bool loaded = false;
        if (!loaded){
            loaded = true;
            const MapLayout overworld = GetMapLayout(0);
            const MapLayout underwater = GetMapLayout(1);
            const float space = From16BitFixedPoint(8182);
            const std::vector<VGears::WorldmapFileSerializer::Block> blocks = LoadWorldMap("WM0.MAP");
            for (size_t block_index = 0; block_index < std::min(blocks.size(), size_t(63)); ++block_index){
                const int map_x = block_index % overworld.width;
                const int map_y = block_index / overworld.width;
                if (map_x < underwater.origin_x || map_x >= underwater.origin_x + underwater.width
                  || map_y < underwater.origin_y || map_y >= underwater.origin_y + underwater.height){
                    continue;
                }
                const VGears::WorldmapFileSerializer::Block& block = blocks[block_index];
                for (size_t mesh_index = 0; mesh_index < block.meshes.size(); ++mesh_index){
                    const VGears::WorldmapFileSerializer::BlockPart& part = block.meshes[mesh_index];
                    const float pos_x = ((mesh_index % 4) + map_x * 4) * space;
                    const float pos_z = ((mesh_index / 4) + map_y * 4) * space;
                    for (const VGears::WorldmapFileSerializer::BlockTriangle& triangle : part.triangles){
                        if (triangle.walkability_info_ != VGears::WorldmapFileSerializer::SEA_DEEP_WATER){
                            continue;
                        }
                        const size_t indices[3] = {
                          triangle.vertex_0_index, triangle.vertex_1_index, triangle.vertex_2_index
                        };
                        Triangle result;
                        for (int i = 0; i < 3; ++i){
                            const VGears::WorldmapFileSerializer::Vertex& vertex = part.vertices.at(indices[i]);
                            result.x[i] = From16BitFixedPoint(vertex.x) + pos_x;
                            result.z[i] = From16BitFixedPoint(vertex.z) + pos_z;
                        }
                        triangles.push_back(result);
                    }
                }
            }
        }
        for (const Triangle& t : triangles){
            const float denominator
              = (t.z[1] - t.z[2]) * (t.x[0] - t.x[2]) + (t.x[2] - t.x[1]) * (t.z[0] - t.z[2]);
            if (Ogre::Math::Abs(denominator) < 1e-6f) continue;
            const float a = ((t.z[1] - t.z[2]) * (x - t.x[2]) + (t.x[2] - t.x[1]) * (z - t.z[2]))
              / denominator;
            const float b = ((t.z[2] - t.z[0]) * (x - t.x[2]) + (t.x[0] - t.x[2]) * (z - t.z[2]))
              / denominator;
            const float c = 1.0f - a - b;
            if (a >= -1e-4f && b >= -1e-4f && c >= -1e-4f) return true;
        }
        return false;
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
      map_id_(-1),
      grid_width_(9),
      grid_height_(7),
      camera_yaw_(0),
      flight_active_(false),
      flight_entity_(nullptr),
      flight_heading_(0),
      flight_y_(0),
      origin_x_(0),
      origin_y_(0),
      player_height_offset_(0),
      active_function_mesh_index_(-1),
      active_function_id_(-1),
      initialized_(false),
      frame_listener_attached_(false),
      cancel_was_pressed_(false),
      confirm_was_pressed_(false),
      transition_stage_(TransitionStage::NONE),
      transition_dive_(true),
      transition_timer_(0),
      transition_start_y_(0),
      transition_target_y_(0),
      surface_requested_(false),
      surface_position_(Ogre::Vector3::ZERO),
      surface_heading_(0),
      dive_requested_(false),
      dive_position_(Ogre::Vector3::ZERO),
      dive_heading_(0),
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
      // The node is kept when a map is replaced.
      if (world_map_node_ == nullptr){
        world_map_node_ = scene_manager_->getRootSceneNode()
          ->createChildSceneNode("WorldMapTerrain");
      }
      world_map_node_->setVisible(true);
        const std::vector<VGears::WorldmapFileSerializer::Block> blocks
          = LoadWorldMap("WM" + std::to_string(WORLD_MAP_FILE_NUMBERS[map_id_]) + ".MAP");
        const size_t texture_count = sizeof(textures) / sizeof(textures[0]);
        // WM0 has extra blocks after the 9x7 grid: the alternate shapes of the story progress.
        const size_t base_block_count = map_id_ == 0
          ? std::min(blocks.size(), size_t(63)) : blocks.size();
        const float space = From16BitFixedPoint(8182);
        std::map<uint16, ManualObject*> texture_objects;
        std::set<uint16> unavailable_texture_ids;
        unsigned int manual_index = 0;

        for (size_t block_index = 0; block_index < base_block_count; ++block_index){
            const VGears::WorldmapFileSerializer::Block& block = blocks[block_index];
            const size_t map_x = origin_x_ + block_index % grid_width_;
            const size_t map_y = origin_y_ + block_index / grid_width_;
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

        const float map_center_x = space * (origin_x_ * 4.0f + grid_width_ * 2.0f);
        const float map_center_z = space * (origin_y_ * 4.0f + grid_height_ * 2.0f);
        Ogre::Camera* camera = CameraManager::getSingleton().GetCurrentCamera();
        camera->setPosition(Ogre::Vector3(map_center_x, 80.0f, map_center_z + 90.0f));
        camera->lookAt(Ogre::Vector3(map_center_x, 0.0f, map_center_z));
    }

    void WorldmapModule::CreatePlayer(){
        Ogre::Vector3 requested_spawn = GetSpawn("Cloud", Ogre::Vector3(42.0f, 0.0f, 36.5f));
        Ogre::Vector3 spawn_position;
        bool spawn_found;
        if (map_id_ == 0){
            // A given spawn point (from a field exit) can be on any terrain, like a town or a forest.
            const bool given_spawn = spawn_overrides_.find("Cloud") != spawn_overrides_.end();
            spawn_found = given_spawn
              ? FindWalkableSurface(requested_spawn, spawn_position, false, false)
              : FindWalkableSurface(requested_spawn, spawn_position, true);
        }
        else{
            // Spawn points of the other maps are unknown: use the walkable triangle closest to the
            // center of the map, or to the position where the submarine dived.
            const Ogre::Real space = From16BitFixedPoint(8182);
            const Ogre::Vector3 center = dive_requested_ ? dive_position_ : Ogre::Vector3(
              space * (origin_x_ * 4.0f + grid_width_ * 2.0f), 0.0f,
              space * (origin_y_ * 4.0f + grid_height_ * 2.0f)
            );
            Ogre::Real best_distance = std::numeric_limits<Ogre::Real>::max();
            // The exact position of the dive, if the submarine can be there.
            const bool exact_dive_position = dive_requested_
              && FindWalkableSurface(dive_position_, spawn_position, false, true);
            if (exact_dive_position) best_distance = 0;
            else for (const TerrainTriangle& triangle : terrain_triangles_){
                const Ogre::Vector3 centroid
                  = (triangle.vertices[0] + triangle.vertices[1] + triangle.vertices[2]) / 3.0f;
                if (map_id_ == 1 ? (UNDERWATER_HEIGHT_LIMIT_ENABLED
                  && centroid.y + FLIGHT_MIN_CLEARANCE > UNDERWATER_MAX_HEIGHT)
                  : !IsActiveTerrainAllowed(triangle.walkability_type)) continue;
                const Ogre::Real distance = Ogre::Vector2(
                  centroid.x - center.x, centroid.z - center.z
                ).squaredLength();
                if (distance < best_distance){
                    best_distance = distance;
                    requested_spawn = centroid;
                }
            }
            spawn_found = exact_dive_position || (
              best_distance < std::numeric_limits<Ogre::Real>::max()
              && FindWalkableSurface(requested_spawn, spawn_position, false)
            );
        }
        if (!spawn_found){
            LogManager::getSingleton().logMessage(
              "World map " + std::to_string(map_id_) + " has no walkable surface for Cloud's spawn."
            );
            return;
        }

        if (map_id_ == 1){
            // The underwater map is played with the submarine only.
            AddWorldModel(
              "Submarine", "ddd.mesh", spawn_position, From16BitFixedPoint(8182) * 0.1125f,
              Ogre::Quaternion::IDENTITY, 1.0f, false
            );
            submarine_entity_ = static_cast<EntityModel*>(
              EntityManager::getSingleton().GetEntity("WorldMapSubmarine")
            );
            if (submarine_entity_ == nullptr) return;
            player_entity_ = submarine_entity_;
            player_height_offset_ = 0;
            if (dive_requested_){
              // Same heading as in WM0.
              submarine_entity_->GetModel()->getParentSceneNode()->getParentSceneNode()
                ->setOrientation(Ogre::Quaternion(Ogre::Radian(dive_heading_), Ogre::Vector3::UNIT_Y));
              dive_requested_ = false;
            }
            // It floats over the seabed.
            submarine_entity_->SetPosition(
              submarine_entity_->GetPosition() + Ogre::Vector3(0, SUBMARINE_START_CLEARANCE, 0)
            );
            EntityManager::getSingleton().ScriptSetPlayerEntity("WorldMapSubmarine");
            UpdateCamera(0, true);
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
            // Only solid entities can interact with the vehicles.
            player_entity_->SetSolid(true);
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
            // Vehicles and companions are only placed in the overworld.
            if (map_id_ == 0){
                CreateCompanionModels();
                // Coming back from WM2: everything is where it was, except for the submarine.
                if (surface_requested_){
                    RestoreOverworldSnapshot();
                    PlaceSurfacedSubmarine();
                }
                overworld_snapshot_.clear();
            }
            UpdateCamera(0, true);
        }
    }

    void WorldmapModule::AddWorldModel(
      const Ogre::String& name, const Ogre::String& mesh_name,
      const Ogre::Vector3& desired_position, Ogre::Real target_size,
      const Ogre::Quaternion& orientation, Ogre::Real collision_radius_scale,
      bool talkable, Ogre::Real explicit_scale
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

        // The scale of the model: the size of its largest dimension, or the one that is given.
        const Ogre::Real model_scale = explicit_scale > 0 ? explicit_scale
          : target_size / largest_dimension;
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
        // The submarine of the overworld is partially under the surface of the water. Only the
        // model is lowered, so its position and collision are the same.
        if (map_id_ == 0 && name == "Submarine"){
            SetModelHeightOffset(entity, -SUBMARINE_SUBMERGED_DEPTH);
        }
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
          GetSpawn("Chocobo", Ogre::Vector3(42.5f, 0.0f, 37.5f)),
          space * 0.14f,
          // The animations of the model (asc) already have it the right way up.
          Ogre::Quaternion::IDENTITY, 1.0f, true
        );
        AddWorldModel(
          "Highwind", "cgd.mesh",
          GetSpawn("Highwind", Ogre::Vector3(43.0f, 0.0f, 36.5f)),
          space * 0.25f,
          // The animation of the model (cic) already has it the right way up.
          Ogre::Quaternion::IDENTITY, 0.5f, true
        );
        AddWorldModel(
          "Buggy", "aba.mesh",
          GetSpawn("Buggy", Ogre::Vector3(42.0f, 0.0f, 36.0f)),
          space * 0.3f, Ogre::Quaternion::IDENTITY, 0.5f, true
        );
        AddWorldModel(
          "Submarine", "ddd.mesh",
          GetSpawn("Submarine", Ogre::Vector3(41.5f, 0.0f, 36.5f)),
          space * 0.1125f, Ogre::Quaternion::IDENTITY, 1.0f, true
        );
        AddWorldModel(
          "TinyBronco", "dva.mesh",
          GetSpawn("TinyBronco", Ogre::Vector3(42.0f, 0.0f, 37.37f)),
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
        for (const Ogre::String& name : hidden_vehicles_){
            ::Entity* entity = EntityManager::getSingleton().GetEntity("WorldMap" + name);
            if (entity == nullptr) continue;
            entity->SetVisible(false);
            entity->SetSolid(false);
            entity->SetTalkable(false);
        }
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
            found = !require_walkable || (map_id_ == 1
              // Underwater: every terrain type is allowed, but there is a height limit.
              ? (!UNDERWATER_HEIGHT_LIMIT_ENABLED
                || height + FLIGHT_MIN_CLEARANCE <= UNDERWATER_MAX_HEIGHT)
              : IsActiveTerrainAllowed(triangle.walkability_type));
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

    bool WorldmapModule::DiveToUnderwaterMap(EntityModel* submarine){
        const Ogre::Vector3 position = submarine->GetPosition();
        Ogre::Vector3 surface;
        uint8 walkability_type = 0;
        if (!FindWalkableSurface(position, surface, false, false, &walkability_type)
          || walkability_type != VGears::WorldmapFileSerializer::SEA_DEEP_WATER){
            return false;
        }
        // WM2 only covers a part of the world.
        if (!IsInsideMap(1, position.x, position.z)){
            LogManager::getSingleton().logMessage(
              "The submarine can't dive here: it's outside the underwater map."
            );
            return false;
        }
        dive_requested_ = true;
        dive_position_ = position;
        dive_heading_ = submarine->GetModel()->getParentSceneNode()->getParentSceneNode()
          ->getOrientation().getYaw().valueRadians();
        BeginTransition(true, submarine);
        return true;
    }

    /**
     * Sets how opaque the black cover of the screen is.
     *
     * @param[in] alpha 0 shows the world map, 1 is all black.
     */
    static void SetScreenCover(const float alpha){
        UiWidget* cover = UiManager::getSingleton().GetWidget("Fade.Screen");
        if (cover == nullptr) return;
        cover->SetColour(0, 0, 0);
        cover->SetAlpha(std::max(0.0f, std::min(1.0f, alpha)));
    }

    /// Eases a progress from 0 to 1.
    static float Smooth(const float t){
        const float clamped = std::max(0.0f, std::min(1.0f, t));
        return clamped * clamped * (3.0f - 2.0f * clamped);
    }

    void WorldmapModule::BeginTransition(const bool dive, EntityModel* submarine){
        transition_stage_ = TransitionStage::LEAVING;
        transition_dive_ = dive;
        transition_timer_ = 0;
        transition_start_y_ = submarine->GetPosition().y;
        // Diving: it sinks under the surface. Surfacing: it goes up, out of the map.
        transition_target_y_ = dive ? transition_start_y_ - DIVE_SINK_DEPTH : DIVE_HIGH_HEIGHT;
        EntityManager::getSingleton().ScriptPlayerLock(true);
    }

    void WorldmapModule::EndTransition(){
        transition_stage_ = TransitionStage::NONE;
        SetScreenCover(0);
        EntityManager::getSingleton().ScriptPlayerLock(false);
    }

    void WorldmapModule::UpdateTransition(const Ogre::Real delta_time){
        transition_timer_ += delta_time;
        ::Entity* submarine = EntityManager::getSingleton().ScriptGetPlayerEntity();
        if (submarine == nullptr){
            EndTransition();
            return;
        }
        const auto set_height = [submarine](const float y){
            Ogre::Vector3 position = submarine->GetPosition();
            position.y = y;
            submarine->SetPosition(position);
        };

        if (transition_stage_ == TransitionStage::LEAVING){
            const float duration = transition_dive_ ? DIVE_SINK_TIME : DIVE_HIGH_TIME;
            set_height(
              transition_start_y_
              + (transition_target_y_ - transition_start_y_) * Smooth(transition_timer_ / duration)
            );
            if (transition_timer_ >= duration){
                transition_stage_ = TransitionStage::FADING_OUT;
                transition_timer_ = 0;
            }
        }
        else if (transition_stage_ == TransitionStage::FADING_OUT){
            SetScreenCover(transition_timer_ / DIVE_FADE_OUT_TIME);
            if (transition_timer_ >= DIVE_FADE_OUT_TIME){
                // The map changes under the black screen. The entities are not valid after it.
                Init(transition_dive_ ? 1 : 0);
                ::Entity* arrived = EntityManager::getSingleton().GetEntity("WorldMapSubmarine");
                if (arrived == nullptr){
                    EndTransition();
                    return;
                }
                transition_target_y_ = arrived->GetPosition().y;
                // Diving: it comes from high up. Surfacing: it comes from under the surface.
                transition_start_y_ = transition_dive_
                  ? DIVE_HIGH_HEIGHT : transition_target_y_ - DIVE_SINK_DEPTH;
                Ogre::Vector3 position = arrived->GetPosition();
                position.y = transition_start_y_;
                arrived->SetPosition(position);
                transition_stage_ = TransitionStage::ARRIVING;
                transition_timer_ = 0;
                EntityManager::getSingleton().ScriptPlayerLock(true);
                UpdateCamera(0, true);
            }
        }
        else if (transition_stage_ == TransitionStage::ARRIVING){
            const float duration = transition_dive_ ? DIVE_HIGH_TIME : DIVE_SINK_TIME;
            set_height(
              transition_start_y_
              + (transition_target_y_ - transition_start_y_) * Smooth(transition_timer_ / duration)
            );
            // The screen is shown again by the time the submarine is at its height.
            SetScreenCover(1.0f - transition_timer_ / duration);
            if (transition_timer_ >= duration){
                set_height(transition_target_y_);
                EndTransition();
            }
        }
    }

    /**
     * Animations of a vehicle in the world map, from the A files of the model: when it's parked,
     * when it's boarded, and when it's boarded and moving. An empty name is no animation (the
     * model in its pose). The roles were found by playing them, and for the models with only one
     * animation it's the one of the model.
     */
    struct VehicleAnimations{
        const char* parked;
        const char* boarded;
        const char* moving;
        /// The parked animation shows its first frame and doesn't move.
        bool parked_paused;
    };

    static const VehicleAnimations CHOCOBO_ANIMATIONS = {"asc", "asc", "asd", false};
    static const VehicleAnimations BUGGY_ANIMATIONS = {"age", "aha", "aha", false};
    static const VehicleAnimations TINY_BRONCO_ANIMATIONS = {"dya", "dya", "dya", true};
    static const VehicleAnimations HIGHWIND_ANIMATIONS = {"cic", "cic", "cic", true};
    static const VehicleAnimations SUBMARINE_ANIMATIONS = {"dfe", "dfe", "dfe", true};

    /**
     * Plays the animation of a vehicle in its current state.
     */
    static void PlayVehicleAnimation(
      EntityModel* vehicle, const ::Entity* active_entity, const bool moving,
      const VehicleAnimations& animations
    ){
        if (vehicle == nullptr) return;
        const bool parked = vehicle != active_entity;
        const char* name = parked ? animations.parked
          : (moving ? animations.moving : animations.boarded);
        if (name[0] == '\0'){
            vehicle->StopAnimation();
            return;
        }
        if (parked && animations.parked_paused){
            vehicle->PlayAnimationPaused(name);
            return;
        }
        vehicle->UnfreezeAnimation();
        if (vehicle->HasAnimation(name)) vehicle->PlayAnimationContinue(name);
    }

    void WorldmapModule::UpdateVehicleAnimations(const bool moving){
        const ::Entity* active_entity = EntityManager::getSingleton().ScriptGetPlayerEntity();
        PlayVehicleAnimation(chocobo_entity_, active_entity, moving, CHOCOBO_ANIMATIONS);
        PlayVehicleAnimation(buggy_entity_, active_entity, moving, BUGGY_ANIMATIONS);
        PlayVehicleAnimation(tiny_bronco_entity_, active_entity, moving, TINY_BRONCO_ANIMATIONS);
        PlayVehicleAnimation(highwind_entity_, active_entity, moving, HIGHWIND_ANIMATIONS);
        PlayVehicleAnimation(submarine_entity_, active_entity, moving, SUBMARINE_ANIMATIONS);
    }

    void WorldmapModule::AddWorldModelFromScript(
      const char* name, const char* hrc, const float x, const float z, const int raw_scale,
      const int direction, const float rotation_x, const float rotation_y, const float rotation_z
    ){
        if (name == nullptr || hrc == nullptr) return;
        // The game scale is for coordinates of 4096 units per world unit, and 0x200 is 1 (15.2).
        // The 1.1 is what the models that were adjusted by looking at them have more.
        const Ogre::Real scale = (static_cast<Ogre::Real>(raw_scale) / 512.0f) * 15.2f / 4096.0f * 1.1f;
        AddWorldModel(
          name, Ogre::String(hrc) + ".mesh", Ogre::Vector3(x, 0.0f, z), 1.0f,
          Ogre::Quaternion::IDENTITY, 1.0f, false, scale
        );
        ::Entity* entity = EntityManager::getSingleton().GetEntity(Ogre::String("WorldMap") + name);
        if (entity == nullptr) return;
        entity->SetSolid(false);
        // The rotation of the model itself.
        entity->setRootOrientation(
          Ogre::Quaternion(Ogre::Degree(rotation_x), Ogre::Vector3::UNIT_X)
            * Ogre::Quaternion(Ogre::Degree(rotation_y), Ogre::Vector3::UNIT_Y)
            * Ogre::Quaternion(Ogre::Degree(rotation_z), Ogre::Vector3::UNIT_Z)
        );
        // The direction of the script is a full turn in 256. How it relates to the heading of the
        // module (0 faces +Z) is a guess.
        const Ogre::Radian heading(Ogre::Degree(-direction * 360.0f / 256.0f));
        static_cast<EntityModel*>(entity)->GetModel()->getParentSceneNode()->getParentSceneNode()
          ->setOrientation(Ogre::Quaternion(heading, Ogre::Vector3::UNIT_Y));
    }

    void WorldmapModule::SetVehicleHidden(const char* entity_name){
        if (entity_name != nullptr) hidden_vehicles_.insert(entity_name);
    }

    void WorldmapModule::SetSpawn(const char* entity_name, const float x, const float z){
        if (entity_name == nullptr) return;
        spawn_overrides_[entity_name] = Ogre::Vector2(x, z);
    }

    Ogre::Vector3 WorldmapModule::GetSpawn(
      const Ogre::String& entity_name, const Ogre::Vector3& default_spawn
    ) const{
        const auto spawn = spawn_overrides_.find(entity_name);
        if (spawn == spawn_overrides_.end()) return default_spawn;
        return Ogre::Vector3(spawn->second.x, 0.0f, spawn->second.y);
    }

    void WorldmapModule::SetModelOffset(const char* entity_name, const float y){
        if (entity_name == nullptr) return;
        ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return;
        SetModelHeightOffset(static_cast<EntityModel*>(entity), y);
    }

    void WorldmapModule::SetEntityAltitude(const char* entity_name, const float y){
        if (entity_name == nullptr) return;
        ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return;
        const Ogre::Vector3 position = entity->GetPosition();
        entity->SetPosition(Ogre::Vector3(position.x, y, position.z));
    }

    void WorldmapModule::SetModelOrientation(
      const char* entity_name, const float x, const float y, const float z
    ){
        if (entity_name == nullptr) return;
        ::Entity* entity = EntityManager::getSingleton().GetEntity(entity_name);
        if (entity == nullptr) return;
        entity->setRootOrientation(
          Ogre::Quaternion(Ogre::Degree(x), Ogre::Vector3::UNIT_X)
            * Ogre::Quaternion(Ogre::Degree(y), Ogre::Vector3::UNIT_Y)
            * Ogre::Quaternion(Ogre::Degree(z), Ogre::Vector3::UNIT_Z)
        );
    }

    bool WorldmapModule::SurfaceToOverworld(EntityModel* submarine){
        const Ogre::Vector3 position = submarine->GetPosition();
        // The same position of WM0 has to be deep water.
        if (!IsOverworldDeepWater(position.x, position.z)){
            LogManager::getSingleton().logMessage(
              "The submarine can't surface here: it's not deep water in the overworld."
            );
            return false;
        }
        surface_requested_ = true;
        surface_position_ = position;
        surface_heading_ = submarine->GetModel()->getParentSceneNode()->getParentSceneNode()
          ->getOrientation().getYaw().valueRadians();
        BeginTransition(false, submarine);
        return true;
    }

    void WorldmapModule::PlaceSurfacedSubmarine(){
        surface_requested_ = false;
        if (submarine_entity_ == nullptr) return;
        Ogre::Vector3 surface;
        if (!FindWalkableSurface(surface_position_, surface, false, false)) return;
        submarine_entity_->SetPosition(surface);
        submarine_entity_->GetModel()->getParentSceneNode()->getParentSceneNode()->setOrientation(
          Ogre::Quaternion(Ogre::Radian(surface_heading_), Ogre::Vector3::UNIT_Y)
        );
        // Cloud boards the submarine, like the boarding script does.
        player_entity_->SetVisible(false);
        player_entity_->SetSolid(false);
        player_entity_->SetTalkable(false);
        ClearVehicleTerrains();
        for (const char* terrain : {"SEA_DEEP_WATER", "SEA", "SUBMARINE_PEN"}) AddVehicleTerrain(terrain);
        EntityManager::getSingleton().ScriptSetPlayerEntity("WorldMapSubmarine");
        UpdateCamera(0, true);
    }

    void WorldmapModule::EnterTriangle(const TerrainTriangle& triangle){
        const int mesh_index = triangle.mesh_y * WORLD_MAP_SCRIPT_MESH_STRIDE + triangle.mesh_x;
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
    }

    void WorldmapModule::UpdateFlight(
      EntityModel* entity, const Ogre::Real delta_time, const bool highwind
    ){
        Ogre::SceneNode* node = entity->GetModel()->getParentSceneNode()->getParentSceneNode();
        if (flight_entity_ != entity){
            // Starts flying: continue from the current heading and height.
            flight_entity_ = entity;
            flight_heading_ = node->getOrientation().getYaw().valueRadians();
            flight_y_ = entity->GetPosition().y;
            camera_yaw_ = 0;
        }

        float turn = 0;
        float climb = 0;
        bool forward = false;
        // Scripts lock the player during events and transitions.
        if (!EntityManager::getSingleton().IsPlayerLocked()){
            InputManager& input = InputManager::getSingleton();
            // The heading decreases to turn right: the model faces +Z at heading 0 and the
            // right side of the camera behind it is -X.
            if (input.IsGameEventPressed("walk_left")) turn += 1.0f;
            if (input.IsGameEventPressed("walk_right")) turn -= 1.0f;
            // Inverted, like the controls of a plane: down climbs and up descends.
            if (input.IsGameEventPressed("walk_up")) climb -= 1.0f;
            if (input.IsGameEventPressed("walk_down")) climb += 1.0f;
            forward = input.IsGameEventPressed("CONFIRM");
        }

        flight_heading_ += turn * FLIGHT_TURN_SPEED * delta_time;
        if (flight_heading_ > Ogre::Math::PI) flight_heading_ -= Ogre::Math::TWO_PI;
        else if (flight_heading_ < -Ogre::Math::PI) flight_heading_ += Ogre::Math::TWO_PI;
        node->setOrientation(Ogre::Quaternion(Ogre::Radian(flight_heading_), Ogre::Vector3::UNIT_Y));

        const Ogre::Vector3 current_position = entity->GetPosition();
        Ogre::Vector3 requested_position = current_position;
        if (forward){
            const Ogre::Vector3 direction(
              Ogre::Math::Sin(Ogre::Radian(flight_heading_)), 0.0f,
              Ogre::Math::Cos(Ogre::Radian(flight_heading_))
            );
            requested_position += direction * From16BitFixedPoint(8182) * 0.45f * 2.0f * delta_time;
        }

        // The terrain below. If the vehicle can't go there, it stays but can still go up or down.
        Ogre::Vector3 surface;
        size_t triangle_index = 0;
        if (!FindWalkableSurface(requested_position, surface, false, true, nullptr, &triangle_index)){
            requested_position = current_position;
            if (!FindWalkableSurface(
              requested_position, surface, false, false, nullptr, &triangle_index
            )) return;
            forward = false;
        }

        // The ground has priority over the maximum height, so the vehicle never goes through it.
        const Ogre::Real ceiling = highwind ? HIGHWIND_MAX_HEIGHT : UNDERWATER_MAX_HEIGHT;
        Ogre::Real new_y = flight_y_ + climb * FLIGHT_CLIMB_SPEED * delta_time;
        new_y = std::min(new_y, ceiling);
        new_y = std::max(new_y, surface.y + FLIGHT_MIN_CLEARANCE);
        const Ogre::Vector3 new_position(requested_position.x, new_y, requested_position.z);
        if (EntityManager::getSingleton().IsWorldPositionBlocked(entity, new_position)) return;
        flight_y_ = new_y;
        entity->SetPosition(new_position);
        if (forward) EnterTriangle(terrain_triangles_[triangle_index]);
    }

    void WorldmapModule::TriggerTriangleFunction(const TerrainTriangle& triangle){
        ScriptEntity* world_map_script
          = ScriptManager::getSingleton().GetScriptEntityByName(
              ScriptManager::SYSTEM, "WorldMap"
            );
        if (world_map_script == nullptr) return;

        const int mesh_index = triangle.mesh_y * WORLD_MAP_SCRIPT_MESH_STRIDE + triangle.mesh_x;
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

          if (vehicle_entity == highwind_entity_){
            // The Highwind lands. It can't be left higher up, Cloud couldn't board it again.
            Ogre::Vector3 ground;
            if (!FindWalkableSurface(vehicle_position, ground, false, false)) return false;
            if (vehicle_position.y - ground.y > HIGHWIND_MAX_LANDING_HEIGHT) return false;
            vehicle_entity->SetPosition(Ogre::Vector3(vehicle_position.x, ground.y, vehicle_position.z));
          }
          player_entity_->SetPosition(surface);
          // The boarding scripts hide Cloud and make him non-solid.
          player_entity_->SetVisible(true);
          player_entity_->SetSolid(true);
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

      // Diving or surfacing: the submarine moves by itself and the screen fades.
      if (transition_stage_ != TransitionStage::NONE){
        UpdateTransition(event.timeSinceLastFrame);
        if (initialized_ && player_entity_ != nullptr){
          UpdateVehicleAnimations(false);
          UpdateCamera(event.timeSinceLastFrame, false);
        }
        return true;
      }

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
        const bool cancel_edge = cancel_pressed && !cancel_was_pressed_;
        // The cancel button makes the submarine of WM2 go back to the surface.
        if (
          cancel_edge && map_id_ == 1 && active_entity == submarine_entity_
          && !EntityManager::getSingleton().IsPlayerLocked()
        ){
            cancel_was_pressed_ = true;
            // Changes the map: the entities are not valid after it.
            if (SurfaceToOverworld(submarine_entity_)) return true;
        }
        if (vehicle_active && cancel_pressed && !cancel_was_pressed_){
            ScriptManager::getSingleton().ScriptRequest(
              ScriptManager::ENTITY, active_entity->GetName().c_str(), "unboard", 0
            );
        }
        cancel_was_pressed_ = cancel_pressed;
        // The action button makes the submarine dive from WM0 to WM2.
        const bool confirm_pressed = InputManager::getSingleton().IsGameEventPressed("CONFIRM");
        const bool dive_pressed = confirm_pressed && !confirm_was_pressed_;
        confirm_was_pressed_ = confirm_pressed;
        if (
          dive_pressed && map_id_ == 0 && active_entity == submarine_entity_
          && !EntityManager::getSingleton().IsPlayerLocked()
        ){
            // Changes the map: the entities are not valid after it.
            if (DiveToUnderwaterMap(submarine_entity_)) return true;
        }
      EntityModel* active_model = active_entity == player_entity_ ? player_entity_
        : (current_vehicle_ == VehicleType::CHOCOBO ? chocobo_entity_
        : (current_vehicle_ == VehicleType::BUGGY ? buggy_entity_
          : (highwind_active ? highwind_entity_
            : (current_vehicle_ == VehicleType::SUBMARINE
              ? submarine_entity_ : tiny_bronco_entity_))));

        // Flying vehicles (the Highwind, and the submarine of WM2) have their own controls.
        flight_active_ = highwind_active || (map_id_ == 1 && active_entity == player_entity_);
        if (flight_active_){
            UpdateFlight(active_model, event.timeSinceLastFrame, highwind_active);
            UpdateVehicleAnimations(false);
            UpdateCamera(event.timeSinceLastFrame, false);
            return true;
        }
        flight_entity_ = nullptr;

        // L1 and R1 rotate the camera around the active entity.
        {
            InputManager& camera_input = InputManager::getSingleton();
            if (camera_input.IsGameEventPressed("L1")) camera_yaw_ -= CAMERA_ROTATION_SPEED * event.timeSinceLastFrame;
            if (camera_input.IsGameEventPressed("R1")) camera_yaw_ += CAMERA_ROTATION_SPEED * event.timeSinceLastFrame;
            if (camera_yaw_ > Ogre::Math::PI) camera_yaw_ -= Ogre::Math::TWO_PI;
            else if (camera_yaw_ < -Ogre::Math::PI) camera_yaw_ += Ogre::Math::TWO_PI;
        }

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
          // The movement is relative to the camera.
          movement = Ogre::Quaternion(Ogre::Radian(camera_yaw_), Ogre::Vector3::UNIT_Y) * movement;
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
                    EnterTriangle(triangle);
                    // There are no encounters underwater.
                    if (current_vehicle_ == VehicleType::ON_FOOT && map_id_ != 1){
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

        // The submarine of WM2 has no walking animations.
        if (!vehicle_active && map_id_ != 1){
            player_entity_->PlayAnimationContinue(is_walking ? "Walk" : "Idle");
        }

        UpdateVehicleAnimations(is_walking);
        UpdateCamera(event.timeSinceLastFrame, false);
        return true;
    }

      void WorldmapModule::UpdateCamera(Ogre::Real delta_time, bool snap_to_player){
        ::Entity* active_entity = EntityManager::getSingleton().ScriptGetPlayerEntity();
        if (active_entity == nullptr) return;
        const Ogre::Vector3 target
          = active_entity->GetPosition() + Ogre::Vector3(0, active_entity->GetHeight() * 0.5f, 0);
        // The camera of a flying vehicle stays behind it. L1 and R1 rotate it from there.
        const float yaw = camera_yaw_ + (flight_active_ ? flight_heading_ + Ogre::Math::PI : 0.0f);
        const Ogre::Vector3 camera_offset
          = Ogre::Quaternion(Ogre::Radian(yaw), Ogre::Vector3::UNIT_Y)
            * Ogre::Vector3(0.0f, 1.5f, 2.25f);
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

    /// Names of the entities of the overworld whose state is kept when the map is replaced.
    static const char* const OVERWORLD_ENTITY_NAMES[] = {
      "WorldMapCloud", "WorldMapChocobo", "WorldMapHighwind", "WorldMapBuggy",
      "WorldMapSubmarine", "WorldMapTinyBronco"
    };

    void WorldmapModule::SaveOverworldSnapshot(){
      overworld_snapshot_.clear();
      for (const char* name : OVERWORLD_ENTITY_NAMES){
        ::Entity* entity = EntityManager::getSingleton().GetEntity(name);
        if (entity == nullptr) continue;
        EntityState state;
        state.name = name;
        state.position = entity->GetPosition();
        state.orientation = static_cast<EntityModel*>(entity)->GetModel()->getParentSceneNode()
          ->getParentSceneNode()->getOrientation();
        state.visible = entity->IsVisible();
        state.solid = entity->IsSolid();
        state.talkable = entity->IsTalkable();
        overworld_snapshot_.push_back(state);
      }
    }

    void WorldmapModule::RestoreOverworldSnapshot(){
      for (const EntityState& state : overworld_snapshot_){
        ::Entity* entity = EntityManager::getSingleton().GetEntity(state.name);
        if (entity == nullptr) continue;
        entity->SetPosition(state.position);
        static_cast<EntityModel*>(entity)->GetModel()->getParentSceneNode()->getParentSceneNode()
          ->setOrientation(state.orientation);
        entity->SetVisible(state.visible);
        entity->SetSolid(state.solid);
        entity->SetTalkable(state.talkable);
      }
      overworld_snapshot_.clear();
    }

    void WorldmapModule::DestroyWorldMap(){
      // The overworld is built again when the submarine surfaces: keep where everything was.
      if (map_id_ == 0) SaveOverworldSnapshot();
      Suspend();
      if (world_map_node_ != nullptr && scene_manager_ != nullptr){
        // Destroy the surfaces. The node is kept and reused.
        while (world_map_node_->numAttachedObjects() > 0){
          Ogre::MovableObject* object = world_map_node_->detachObject(static_cast<unsigned short>(0));
          scene_manager_->destroyMovableObject(object);
        }
      }
      terrain_triangles_.clear();
      encounter_records_.clear();
      // The entities of the old map (player, vehicles, models and scripts).
      EntityManager::getSingleton().ClearWorld();
      initialized_ = false;
      map_id_ = -1;
    }

    void WorldmapModule::Enter(const int map_id){
      if (initialized_) DestroyWorldMap();
      Init(map_id);
    }

    void WorldmapModule::Init(const int map_id){
      if (map_id < 0 || map_id >= WORLD_MAP_COUNT){
        LogManager::getSingleton().logMessage(
          "Invalid world map " + std::to_string(map_id) + ". Valid maps are 0 to "
            + std::to_string(WORLD_MAP_COUNT - 1) + "."
        );
        return;
      }
      if (initialized_ && map_id != map_id_){
        DestroyWorldMap();
      }
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
      map_id_ = map_id;
      camera_yaw_ = 0;
      const MapLayout layout = GetMapLayout(map_id_);
      grid_width_ = layout.width;
      grid_height_ = layout.height;
      origin_x_ = layout.origin_x;
      origin_y_ = layout.origin_y;
      EntityManager::getSingleton().SetWorldModule();
      DialogsManager::getSingleton().SetWorldModule();
      CreateWorldMap(Root::getSingleton().getSceneManager("Scene"));
      ScriptManager::getSingleton().RunFile(
        "scripts/worldmap/wm" + std::to_string(WORLD_MAP_FILE_NUMBERS[map_id_]) + ".lua"
      );
      // The debug spawn points, if debug_data.lua has them.
      spawn_overrides_.clear();
      hidden_vehicles_.clear();
      ScriptManager::getSingleton().RunString(
        "if WorldMapDebug and WorldMapDebug.spawn then "
        "for name, point in pairs(WorldMapDebug.spawn) do "
        "world_map_module:set_spawn(name, point[1], point[2]) end end"
      );
      // The positions that the scripts give to the vehicles, for the story (debug ones have priority).
      ScriptManager::getSingleton().RunString(
        "if WorldVM then WorldVM.prepare(" + std::to_string(map_id_) + ") end"
      );
      CreatePlayer();
      // The models that the scripts place (scenery).
      ScriptManager::getSingleton().RunString(
        "if WorldVM then WorldVM.place_models(" + std::to_string(map_id_) + ") end"
      );
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
      flight_active_ = false;
      flight_entity_ = nullptr;
      cancel_was_pressed_ = false;
      confirm_was_pressed_ = false;
    }

} // namespace VGears
