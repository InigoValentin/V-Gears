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

#include <iostream>
#include <fstream>
#include <tinyxml.h>
#include <OgreRoot.h>
#include <OgreMesh.h>
#include <OgreMeshSerializer.h>
#include <boost/filesystem.hpp>
#include "WorldInstaller.h"
#include "TexFile.h"
#include "common/Lzs.h"
#include "common/VGearsStringUtil.h"
#include "data/VGearsLGPArchive.h"
#include "data/WorldMapWalkmesh.h"
#include "data/VGearsHRCFileManager.h"
#include "data/FF7Data.h"

std::string WorldInstaller::ELEMENT_MODELS_DIR("models/world/element");

std::string WorldInstaller::TERRAIN_MODELS_DIR("models/world/terrain");

WorldInstaller::WorldInstaller(
  const std::string input_dir, const std::string output_dir,
  const bool keep_originals, Ogre::ResourceGroupManager* res_mgr
):
  input_dir_(input_dir), output_dir_(output_dir), keep_originals_(keep_originals), res_mgr_(res_mgr)
{}

WorldInstaller::~WorldInstaller(){}

unsigned int WorldInstaller::Initialize(){
    if (wm_map_.size() > 0) wm_map_.clear();
    processed_maps_ = 0;
    wm_map_.push_back(File(input_dir_ + "/data/wm/WM0.MAP"));
    wm_map_.push_back(File(input_dir_ + "/data/wm/WM2.MAP"));
    wm_map_.push_back(File(input_dir_ + "/data/wm/WM3.MAP"));
    return wm_map_.size();
}

void WorldInstaller::GenerateMaterials(){
    // First, populate the texture list with static data.
    texture_.push_back({0, "pond", 32, 32, 0, 352});
    texture_.push_back({1, "riv_m2", 32, 32, 128, 64});
    texture_.push_back({2, "was_gs", 64, 64, 64, 192});
    texture_.push_back({3, "wpcltr", 32, 128, 0, 256});
    texture_.push_back({4, "wpcltr2", 32, 32, 160, 64});
    texture_.push_back({5, "bzdun", 64, 64, 192, 192});
    texture_.push_back({6, "bone", 32, 32, 224, 384});
    texture_.push_back({7, "bone2", 32, 32, 224, 416});
    texture_.push_back({8, "bornwd", 64, 64, 160, 320});
    texture_.push_back({9, "bridge", 32, 64, 192, 0});
    texture_.push_back({10, "bridge2", 32, 32, 224, 0});
    texture_.push_back({11, "cave", 32, 32, 224, 448});
    texture_.push_back({12, "cave2", 32, 32, 224, 320});
    texture_.push_back({13, "cave_s", 32, 32, 160, 224});
    texture_.push_back({14, "cdl_cl2", 64, 32, 96, 96});
    texture_.push_back({15, "cf01", 64, 32, 192, 288});
    texture_.push_back({16, "clf_bgs", 64, 32, 192, 384});
    texture_.push_back({17, "clf_ggl", 64, 64, 128, 256});
    texture_.push_back({18, "clf_ggs", 64, 32, 192, 352});
    texture_.push_back({19, "clf_l", 64, 64, 0, 0});
    texture_.push_back({20, "clf_ld", 64, 64, 64, 0});
    texture_.push_back({21, "clf_lf", 64, 64, 128, 0});
    texture_.push_back({22, "clf_lg", 32, 64, 0, 96});
    texture_.push_back({23, "clf_lr", 32, 64, 128, 0});
    texture_.push_back({24, "clf_lsg", 32, 64, 64, 64});
    texture_.push_back({25, "clf_r", 32, 32, 0, 96});
    texture_.push_back({26, "clf_s", 64, 32, 192, 0});
    texture_.push_back({27, "clf_sd", 64, 32, 192, 32});
    texture_.push_back({28, "clf_sf", 64, 32, 0, 64});
    texture_.push_back({29, "clf_sg", 32, 32, 32, 96});
    texture_.push_back({30, "clf_sg2", 32, 32, 0, 160});
    texture_.push_back({31, "clf_sr", 32, 32, 32, 96});
    texture_.push_back({32, "clf_ss", 32, 32, 32, 128});
    texture_.push_back({33, "clf_ssd", 32, 32, 0, 224});
    texture_.push_back({34, "clf_ssw", 32, 32, 224, 32});
    texture_.push_back({35, "clf_sw", 32, 32, 192, 32});
    texture_.push_back({36, "clf_w02", 64, 64, 128, 64});
    texture_.push_back({37, "clf_w03", 64, 64, 192, 64});
    texture_.push_back({38, "clf_was", 64, 32, 64, 64});
    texture_.push_back({39, "clfeg", 32, 32, 192, 320});
    texture_.push_back({40, "clfegd", 32, 32, 0, 320});
    texture_.push_back({41, "clftop", 64, 32, 192, 64});
    texture_.push_back({42, "clftop2", 32, 32, 128, 64});
    texture_.push_back({43, "cndl_cl", 64, 32, 96, 128});
    texture_.push_back({44, "cndlf", 64, 64, 160, 64});
    texture_.push_back({45, "cndlf02", 64, 64, 208, 64});
    texture_.push_back({46, "comtr", 16, 32, 144, 96});
    texture_.push_back({47, "cosinn", 32, 32, 224, 416});
    texture_.push_back({48, "cosinn2", 32, 32, 192, 448});
    texture_.push_back({49, "csmk", 32, 32, 64, 64});
    texture_.push_back({50, "csmk2", 32, 32, 96, 64});
    texture_.push_back({51, "cstds01", 32, 32, 224, 160});
    texture_.push_back({52, "cstds02", 64, 64, 0, 448});
    texture_.push_back({53, "des01", 32, 32, 160, 320});
    texture_.push_back({54, "desert", 64, 64, 128, 128});
    texture_.push_back({55, "desor", 64, 32, 160, 64});
    texture_.push_back({56, "ds1", 32, 32, 0, 256});
    texture_.push_back({57, "ds_s1", 32, 32, 192, 288});
    texture_.push_back({58, "dsee1", 32, 32, 96, 288});
    texture_.push_back({59, "dsrt_d", 32, 32, 224, 0});
    texture_.push_back({60, "dsrt_e", 64, 128, 64, 128});
    texture_.push_back({61, "edes01", 32, 32, 224, 320});
    texture_.push_back({62, "elm01", 32, 32, 160, 0});
    texture_.push_back({63, "elm02", 32, 32, 64, 96});
    texture_.push_back({64, "elm_gro", 64, 64, 0, 96});
    texture_.push_back({65, "elm_r", 32, 32, 192, 0});
    texture_.push_back({66, "elm_r2", 32, 32, 224, 0});
    texture_.push_back({67, "fall1", 32, 32, 128, 256});
    texture_.push_back({68, "farm01", 32, 32, 160, 32});
    texture_.push_back({69, "farm02", 32, 32, 192, 32});
    texture_.push_back({70, "farm_g", 32, 32, 128, 64});
    texture_.push_back({71, "farm_r", 32, 16, 128, 48});
    texture_.push_back({72, "fld", 64, 64, 64, 96});
    texture_.push_back({73, "fld_02", 64, 64, 0, 64});
    texture_.push_back({74, "fld_s", 64, 64, 0, 160});
    texture_.push_back({75, "fld_s2", 32, 32, 224, 256});
    texture_.push_back({76, "fld_sw", 64, 64, 128, 192});
    texture_.push_back({77, "fld_v", 128, 128, 0, 128});
    texture_.push_back({78, "fld_vd", 32, 64, 96, 128});
    texture_.push_back({79, "fld_vd2", 32, 64, 192, 416});
    texture_.push_back({80, "fvedge", 32, 64, 0, 0});
    texture_.push_back({81, "gclf_d", 128, 64, 128, 128});
    texture_.push_back({82, "gclf_g", 32, 64, 224, 128});
    texture_.push_back({83, "gclfwa", 128, 64, 64, 320});
    texture_.push_back({84, "gclfwa2", 32, 64, 160, 320});
    texture_.push_back({85, "gclfwag", 32, 64, 32, 320});
    texture_.push_back({86, "gg_gro", 64, 64, 64, 448});
    texture_.push_back({87, "gg_mts", 64, 128, 0, 128});
    texture_.push_back({88, "ggmk", 64, 64, 128, 448});
    texture_.push_back({89, "ggmt", 128, 128, 0, 0});
    texture_.push_back({90, "ggmt_e", 128, 32, 0, 96});
    texture_.push_back({91, "ggmt_ed", 128, 32, 128, 96});
    texture_.push_back({92, "ggmt_eg", 32, 32, 96, 224});
    texture_.push_back({93, "ggmtd", 128, 128, 128, 0});
    texture_.push_back({94, "ggs_g", 32, 32, 32, 64});
    texture_.push_back({95, "ggshr", 32, 32, 192, 96});
    texture_.push_back({96, "ggshrg", 32, 32, 224, 96});
    texture_.push_back({97, "gia", 64, 32, 64, 224});
    texture_.push_back({98, "gia2", 64, 32, 0, 224});
    texture_.push_back({99, "gia_d", 64, 32, 128, 224});
    texture_.push_back({100, "gia_d2", 64, 32, 64, 224});
    texture_.push_back({101, "gia_g", 32, 32, 192, 224});
    texture_.push_back({102, "gia_g2", 32, 32, 128, 224});
    texture_.push_back({103, "gmt_eda", 32, 32, 0, 352});
    texture_.push_back({104, "gonclf", 128, 64, 96, 64});
    texture_.push_back({105, "gredge", 32, 32, 192, 192});
    texture_.push_back({106, "hyouga", 64, 64, 192, 64});
    texture_.push_back({107, "iceclf", 64, 32, 64, 96});
    texture_.push_back({108, "iceclfd", 64, 32, 128, 96});
    texture_.push_back({109, "iceclfg", 32, 32, 32, 224});
    texture_.push_back({110, "jun", 64, 64, 192, 192});
    texture_.push_back({111, "jun_d", 64, 64, 128, 192});
    texture_.push_back({112, "jun_e", 64, 16, 0, 240});
    texture_.push_back({113, "jun_gro", 64, 64, 0, 64});
    texture_.push_back({114, "junmk", 32, 32, 0, 96});
    texture_.push_back({115, "junn01", 32, 32, 160, 112});
    texture_.push_back({116, "junn02", 32, 32, 192, 112});
    texture_.push_back({117, "junn03", 32, 32, 224, 112});
    texture_.push_back({118, "junn04", 32, 32, 64, 128});
    texture_.push_back({119, "jutmpl01", 64, 64, 128, 192});
    texture_.push_back({120, "lake-e", 32, 32, 96, 192});
    texture_.push_back({121, "lake_ef", 32, 32, 128, 224});
    texture_.push_back({122, "lake_fl", 128, 32, 160, 224});
    texture_.push_back({123, "lostclf", 32, 64, 128, 384});
    texture_.push_back({124, "lostmt", 128, 32, 128, 448});
    texture_.push_back({125, "lostmtd", 128, 32, 128, 480});
    texture_.push_back({126, "lostmts", 64, 32, 160, 384});
    texture_.push_back({127, "lostwd_e", 32, 32, 64, 480});
    texture_.push_back({128, "lostwod", 64, 64, 0, 448});
    texture_.push_back({129, "lst1", 32, 32, 192, 256});
    texture_.push_back({130, "lstwd_e2", 32, 32, 96, 480});
    texture_.push_back({131, "mzes", 32, 32, 224, 128});
    texture_.push_back({132, "mzmt_e", 128, 64, 128, 64});
    texture_.push_back({133, "mzmt_ed", 128, 32, 128, 128});
    texture_.push_back({134, "mzmt_edw", 128, 32, 128, 160});
    texture_.push_back({135, "mzmt_ew", 128, 32, 0, 128});
    texture_.push_back({136, "mzmt_o", 128, 32, 64, 416});
    texture_.push_back({137, "mzmt_od", 128, 32, 64, 448});
    texture_.push_back({138, "mzmt_s", 128, 32, 0, 192});
    texture_.push_back({139, "mzmt_sd", 128, 32, 0, 160});
    texture_.push_back({140, "md01", 32, 32, 96, 16});
    texture_.push_back({141, "md02", 128, 128, 0, 0});
    texture_.push_back({142, "md03", 16, 16, 112, 64});
    texture_.push_back({143, "md04", 32, 32, 128, 16});
    texture_.push_back({144, "md05", 64, 16, 96, 0});
    texture_.push_back({145, "md06", 16, 32, 96, 48});
    texture_.push_back({146, "md07", 16, 16, 112, 48});
    texture_.push_back({147, "md_mt", 128, 128, 128, 0});
    texture_.push_back({148, "md_mtd", 128, 128, 0, 0});
    texture_.push_back({149, "md_mts", 64, 128, 64, 160});
    texture_.push_back({150, "md_snow", 128, 32, 128, 0});
    texture_.push_back({151, "md_snw2", 128, 32, 128, 32});
    texture_.push_back({152, "md_snwd", 128, 64, 0, 128});
    texture_.push_back({153, "md_snwe", 64, 64, 96, 320});
    texture_.push_back({154, "md_snws", 64, 64, 128, 128});
    texture_.push_back({155, "md_snwt", 128, 32, 0, 192});
    texture_.push_back({156, "md_snww", 32, 32, 224, 224});
    texture_.push_back({157, "md_sw_s", 128, 128, 0, 0});
    texture_.push_back({158, "md_swd2", 32, 32, 192, 256});
    texture_.push_back({159, "md_swnp", 128, 128, 0, 96});
    texture_.push_back({160, "mdsrt_e", 128, 32, 128, 192});
    texture_.push_back({161, "mdsrt_ed", 128, 32, 128, 224});
    texture_.push_back({162, "mdsrt_eg", 32, 32, 64, 224});
    texture_.push_back({163, "midil", 32, 32, 32, 192});
    texture_.push_back({164, "midild", 32, 32, 224, 192});
    texture_.push_back({165, "mt_ewg", 32, 32, 64, 96});
    texture_.push_back({166, "mt_road", 64, 64, 192, 128});
    texture_.push_back({167, "mt_se", 32, 32, 160, 416});
    texture_.push_back({168, "mt_se2", 64, 64, 128, 256});
    texture_.push_back({169, "mt_sg01", 32, 32, 0, 224});
    texture_.push_back({170, "mt_sg02", 32, 32, 32, 224});
    texture_.push_back({171, "mt_sg03", 32, 32, 0, 192});
    texture_.push_back({172, "mt_sg04", 32, 32, 160, 384});
    texture_.push_back({173, "mtcoin", 64, 64, 0, 256});
    texture_.push_back({174, "mtwas_e", 128, 32, 0, 224});
    texture_.push_back({175, "mtwas_ed", 128, 32, 0, 224});
    texture_.push_back({176, "ncol_gro", 64, 64, 64, 384});
    texture_.push_back({177, "ncole01", 32, 32, 224, 384});
    texture_.push_back({178, "ncole02", 32, 32, 192, 416});
    texture_.push_back({179, "nivl_gro", 64, 64, 128, 384});
    texture_.push_back({180, "nivl_mt", 128, 64, 0, 0});
    texture_.push_back({181, "nivl_top", 32, 32, 0, 64});
    texture_.push_back({182, "nivlr", 32, 32, 192, 384});
    texture_.push_back({183, "port", 32, 32, 96, 224});
    texture_.push_back({184, "port_d", 32, 32, 160, 0});
    texture_.push_back({185, "rzclf02", 64, 64, 128, 128});
    texture_.push_back({186, "rct_gro", 64, 128, 0, 416});
    texture_.push_back({187, "riv_cls", 64, 64, 64, 0});
    texture_.push_back({188, "riv_l1", 32, 32, 96, 320});
    texture_.push_back({189, "riv_m", 32, 32, 0, 224});
    texture_.push_back({190, "rivr", 32, 32, 64, 224});
    texture_.push_back({191, "rivrclf", 64, 64, 128, 192});
    texture_.push_back({192, "rivs1", 32, 32, 128, 320});
    texture_.push_back({193, "rivshr", 64, 64, 192, 192});
    texture_.push_back({194, "rivssr", 64, 32, 192, 224});
    texture_.push_back({195, "rivstrt", 32, 32, 192, 160});
    texture_.push_back({196, "rm1", 32, 32, 32, 288});
    texture_.push_back({197, "rocet", 32, 32, 128, 160});
    texture_.push_back({198, "rs_ss", 32, 32, 96, 224});
    texture_.push_back({199, "sango", 32, 32, 224, 320});
    texture_.push_back({200, "sango2", 32, 32, 224, 352});
    texture_.push_back({201, "sango3", 32, 32, 128, 384});
    texture_.push_back({202, "sango4", 64, 64, 0, 384});
    texture_.push_back({203, "sdun", 64, 64, 0, 160});
    texture_.push_back({204, "sdun02", 64, 64, 64, 160});
    texture_.push_back({205, "sh1", 32, 32, 32, 256});
    texture_.push_back({206, "sh_s1", 32, 32, 224, 288});
    texture_.push_back({207, "shedge", 32, 64, 160, 160});
    texture_.push_back({208, "shlm_1", 32, 32, 192, 320});
    texture_.push_back({209, "shol", 128, 128, 128, 96});
    texture_.push_back({210, "shol_s", 64, 64, 192, 192});
    texture_.push_back({211, "shor", 128, 128, 0, 0});
    texture_.push_back({212, "shor_s", 64, 64, 128, 192});
    texture_.push_back({213, "shor_s2", 32, 32, 224, 416});
    texture_.push_back({214, "shor_v", 32, 32, 192, 0});
    texture_.push_back({215, "silo", 32, 32, 224, 32});
    texture_.push_back({216, "slope", 128, 32, 0, 384});
    texture_.push_back({217, "snow_es", 32, 32, 192, 480});
    texture_.push_back({218, "snow_es2", 32, 32, 224, 480});
    texture_.push_back({219, "snow_es3", 32, 32, 224, 448});
    texture_.push_back({220, "snw_mt", 128, 128, 0, 0});
    texture_.push_back({221, "snw_mtd", 128, 128, 128, 0});
    texture_.push_back({222, "snw_mte", 64, 32, 0, 96});
    texture_.push_back({223, "snw_mted", 64, 32, 64, 96});
    texture_.push_back({224, "snw_mts", 64, 128, 64, 0});
    texture_.push_back({225, "snw_mts2", 64, 32, 128, 192});
    texture_.push_back({226, "snwfld", 64, 64, 0, 64});
    texture_.push_back({227, "snwfld_s", 64, 32, 128, 128});
    texture_.push_back({228, "snwgra", 64, 64, 192, 192});
    texture_.push_back({229, "snwhm01", 32, 32, 32, 0});
    texture_.push_back({230, "snwhm02", 32, 32, 32, 32});
    texture_.push_back({231, "snwods", 32, 32, 224, 192});
    texture_.push_back({232, "snwood", 64, 64, 192, 128});
    texture_.push_back({233, "snwtrk", 32, 64, 96, 256});
    texture_.push_back({234, "sse_s1", 32, 32, 32, 320});
    texture_.push_back({235, "ssee1", 32, 32, 64, 288});
    texture_.push_back({236, "sst1", 32, 32, 224, 256});
    texture_.push_back({237, "stown_r", 32, 32, 192, 256});
    texture_.push_back({238, "stw_gro", 64, 64, 0, 384});
    texture_.push_back({239, "subrg2", 32, 32, 224, 160});
    texture_.push_back({240, "susbrg", 64, 64, 192, 96});
    texture_.push_back({241, "sw_se", 64, 64, 0, 0});
    texture_.push_back({242, "swclf_l", 64, 64, 64, 128});
    texture_.push_back({243, "swclf_ld", 64, 64, 192, 128});
    texture_.push_back({244, "swclf_lg", 32, 64, 0, 192});
    texture_.push_back({245, "swclf_s", 64, 32, 128, 96});
    texture_.push_back({246, "swclf_sd", 64, 32, 192, 96});
    texture_.push_back({247, "swclf_sg", 32, 32, 32, 192});
    texture_.push_back({248, "swclf_wg", 32, 32, 192, 192});
    texture_.push_back({249, "swfld_s2", 64, 32, 128, 160});
    texture_.push_back({250, "swfld_s3", 32, 32, 160, 192});
    texture_.push_back({251, "swmd_cg", 32, 32, 128, 192});
    texture_.push_back({252, "swmd_clf", 64, 32, 64, 192});
    texture_.push_back({253, "swp1", 32, 32, 0, 288});
    texture_.push_back({254, "trk", 64, 64, 128, 0});
    texture_.push_back({255, "tyo_f", 128, 128, 128, 128});
    texture_.push_back({256, "tyosnw", 64, 128, 64, 384});
    texture_.push_back({257, "uf1", 32, 32, 160, 256});
    texture_.push_back({258, "utai01", 32, 32, 32, 96});
    texture_.push_back({259, "utai02", 32, 32, 224, 64});
    texture_.push_back({260, "utai_gro", 64, 64, 128, 96});
    texture_.push_back({261, "utaimt", 32, 32, 0, 128});
    texture_.push_back({262, "utaimtd", 32, 32, 96, 96});
    texture_.push_back({263, "utaimtg", 32, 32, 96, 128});
    texture_.push_back({264, "wa1", 32, 32, 192, 320});
    texture_.push_back({265, "wzs1", 32, 32, 128, 288});
    texture_.push_back({266, "wzshr", 32, 32, 160, 32});
    texture_.push_back({267, "wzshr2", 32, 32, 32, 128});
    texture_.push_back({268, "wzshrs", 32, 32, 32, 160});
    texture_.push_back({269, "was", 128, 128, 0, 96});
    texture_.push_back({270, "was_d", 64, 32, 0, 224});
    texture_.push_back({271, "was_g", 64, 64, 0, 192});
    texture_.push_back({272, "was_s", 128, 128, 128, 0});
    texture_.push_back({273, "wasfld", 64, 64, 64, 256});
    texture_.push_back({274, "wdedge", 64, 64, 64, 160});
    texture_.push_back({275, "we1", 32, 32, 96, 256});
    texture_.push_back({276, "we_s1", 32, 32, 160, 288});
    texture_.push_back({277, "wedged", 32, 64, 128, 160});
    texture_.push_back({278, "wod-e2", 32, 32, 64, 224});
    texture_.push_back({279, "wood", 64, 64, 192, 0});
    texture_.push_back({280, "wood_d", 64, 64, 192, 160});
    texture_.push_back({281, "wtrk", 32, 64, 64, 96});
    Ogre::MaterialSerializer mSer;
    for (Texture t : texture_){
        Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().create(t.name, "General", true);
        Ogre::Technique* tech = mat->getTechnique(0);
        Ogre::Pass* pass = tech->getPass(0);
        pass->setDiffuse(0.7, 0.7, 0.7, 0.7);
        std::cout << "SAVING MATERIAL: " << (output_dir_ + "models/world/" + t.name + ".material") << std::endl;
        mSer.exportMaterial(mat, output_dir_ + "models/world/" + t.name + ".material");
    }
}

void WorldInstaller::ProcessModels(){
    // Open world_us.lgp
    File world_file(input_dir_ + "data/wm/world_us.lgp");
    VGears::LGPArchive world_lgp(input_dir_ + "data/wm/world_us.lgp", "LGP");
    world_lgp.open(input_dir_ + "data/wm/world_us.lgp", true);
    world_lgp.load();
    VGears::LGPArchive::FileList file_list = world_lgp.GetFiles();
    for (int i = 0; i < file_list.size(); i ++){
        VGears::LGPArchive::FileEntry f = file_list.at(i);
        if (f.data_offset + f.data_size <= world_file.GetFileSize()){
            File w_lgp_file(&world_file, f.data_offset, f.data_size);
            std::string file_name = f.file_name;
            w_lgp_file.WriteFile(output_dir_ + "temp/world_models/" + file_name);
            if (file_name.substr(4, 3) == "hrc"){

                std::string id = file_name.substr(0, 3);
                std::string model_name = FF7Data::GetWorldMapModelName(id);
                std::cout << "CONVERTING WORLD MODEL: " << file_name << " " << i
                  << "/" << file_list.size() << std::endl;



                // TODO: Can this be done with declare resource?
                // TODO: If not, do aloop to save all the hrc files, then call this once, then
                // another loop to process them.
                /*res_mgr_->declareResource(
                  output_dir_ + "temp/world_models/" + file_name, "Skeleton", "FFVII"
                );*/
                std::cout << "1\n";
                res_mgr_->removeResourceLocation(output_dir_ + "temp/world_models/", "FFVII");
                std::cout << "2\n";
                res_mgr_->addResourceLocation(
                  output_dir_ + "temp/world_models/", "FileSystem", "FFVII", true, true
                );



                // TODO: This needs work to assemble all the pieces. Similar to field models.


                std::cout << "3\n";
                Ogre::ResourcePtr hrc = VGears::HRCFileManager::GetSingleton().load(
                  file_name, "FFVII"
                );
                std::cout << "4\n";
                auto mesh_name = model_name + ".mesh";
                std::cout << "5\n";
                Ogre::MeshPtr mesh(Ogre::MeshManager::getSingleton().load(mesh_name, "FFVII"));
                std::cout << "6\n";
                Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
                // TODO: a files??
                /*for (std::string anim : model.a){
                    VGears::AFileManager &afl_mgr(VGears::AFileManager::GetSingleton());
                    Ogre::String a_base_name;
                    VGears::StringUtil::splitBase(anim, a_base_name);
                    VGears::AFilePtr a
                      = afl_mgr.load(a_base_name + ".a", "FFVII").staticCast<VGears::AFile>();
                    // Convert the FF7 name to a more readable name set in the meta data.
                    VGears::StringUtil::splitBase(anim, base_name);
                    a->AddTo(skeleton, VGears::NameLookup::Animation(base_name));
                }*/
                std::cout << "EXPORT MESH: " << (output_dir_ + "data/models/world/" + mesh_name) << std::endl;
                ExportMesh(output_dir_ + "data/models/world/" + mesh_name, mesh);
                std::cout << "    EXPORTED" << std::endl;
            }
        }
    }
}

bool WorldInstaller::ProcessMap(){
    //std::cout << "[WI] Processing map " << processed_maps_ << "/" << wm_map_.size() << std::endl;
    if (processed_maps_ >= wm_map_.size()) return true;
    // Initialize the xml file.
    TiXmlDocument doc;
    std::unique_ptr<TiXmlElement> xml(new TiXmlElement("world_map"));
    // Script.
    std::unique_ptr<TiXmlElement> xml_script_element(new TiXmlElement("script"));
    xml_script_element->SetAttribute("file_name", std::to_string(processed_maps_) + "/script.lua");
    xml->LinkEndChild(xml_script_element.release());
    // Background.
    std::unique_ptr<TiXmlElement> xml_background_2d(new TiXmlElement("background2d"));
    xml_background_2d->SetAttribute("file_name", std::to_string(processed_maps_) + "/bg.xml");
    xml->LinkEndChild(xml_background_2d.release());
    // Texts.
    std::unique_ptr<TiXmlElement> xml_texts(new TiXmlElement("texts"));
    xml_texts->SetAttribute("file_name", std::to_string(processed_maps_) + "/text.xml");
    xml->LinkEndChild(xml_texts.release());
    // Walkmesh.
    std::unique_ptr<TiXmlElement> xml_walkmesh(new TiXmlElement("walkmesh"));
    xml_walkmesh->SetAttribute("file_name", std::to_string(processed_maps_) + "/wm.xml");
    xml->LinkEndChild(xml_walkmesh.release());
    // Terrain (don't close yet).
    std::unique_ptr<TiXmlElement> xml_terrain(new TiXmlElement("terrain"));

    // Initialize the walkmesh data.
    WorldMapWalkmesh walkmesh(processed_maps_, processed_maps_ == 0 ? true : false);

    // Read WM*.MAP file.
    Map map;
    Ogre::MeshSerializer mesh_serializer;
    boost::filesystem::path p(wm_map_[processed_maps_].GetFileName());
    std::string map_name = p.stem().native(); // w/o path or extension.
    for (int b = 0; b < wm_map_[processed_maps_].GetFileSize() / 0xB800; b ++){
        // Calculate offsets for walkmesh.
        // WM0 is 9x7 blocks, WM2 probably is too, WM3 no idea?
        int wm_offset_x = 32 * (b % 9);
        int wm_offset_y = 32 * (b / 9);
        Block block;
        for (int m = 0; m < 16; m ++){
            // Extract lzss compressed mesh to file.
            std::vector<unsigned char> lzss;
            wm_map_[processed_maps_].SetOffset(0xB800 * b + m * 4);
            u32 mesh_offset = 0xB800 * b + wm_map_[processed_maps_].readU32LE();
            wm_map_[processed_maps_].SetOffset(mesh_offset);
            u32 mesh_size = wm_map_[processed_maps_].readU32LE();
            // Add size to lzss
            wm_map_[processed_maps_].SetOffset(mesh_offset);
            for (int d = 0; d < 4; d ++)
                lzss.push_back(wm_map_[processed_maps_].readU8());
            for (int d = 0; d < mesh_size ; d ++) lzss.push_back(wm_map_[processed_maps_].readU8());
            // Extract the lzss file.
            std::vector<unsigned char> data = Lzs::Decompress(lzss);
            if (data.size() == 0) continue;
            std::string dat_name = output_dir_ + "temp/wm/" + map_name + "_"
              + std::to_string(b) + "_" + std::to_string(m) + ".dat";
            std::ofstream dat(dat_name, std::ios::out | std::ios::binary);
            if (!dat){
                std::cerr << "Cannot create temporary world map file " << dat_name << std::endl;
                processed_maps_ ++;
                if (processed_maps_ >= wm_map_.size()) return true;
                else return false;
            }
            for (int d = 0; d <= data.size(); d ++)
                dat.write((char *) &data[d], sizeof(unsigned char));
            dat.close();

            // Open and read the decompressed file.
            File mesh_data(dat_name);
            block.mesh[m].triangle_count = mesh_data.readU16LE();
            block.mesh[m].vertex_count = mesh_data.readU16LE();
            for (int tri = 0; tri < block.mesh[m].triangle_count; tri ++){
                Triangle t;
                t.vertex_index[0] = mesh_data.readU8();
                t.vertex_index[1] = mesh_data.readU8();
                t.vertex_index[2] = mesh_data.readU8();
                u8 walkability_function = mesh_data.readU8();
                //t.walkability = walkability_function >> 5; // 5 bits
                //t.function_id = walkability_function & 0x7; // 3 bits
                t.walkability =  walkability_function & 0x1F; // Lowest 5 bits
                t.function_id =  walkability_function >> 3; // 3 bits
                // FIXME: I think function_id is wrong. It must be 3 bits
                t.vertex_coord[0].u = mesh_data.readU8();
                t.vertex_coord[0].v = mesh_data.readU8();
                t.vertex_coord[1].u = mesh_data.readU8();
                t.vertex_coord[1].v = mesh_data.readU8();
                t.vertex_coord[2].u = mesh_data.readU8();
                t.vertex_coord[2].v = mesh_data.readU8();
                u16 texture_location = mesh_data.readU16LE();
                t.texture = texture_location >> 9; // 9 bytes
                t.location = texture_location & ((1 << 7) - 1); // 7 bytes.
                block.mesh[m].triangles.push_back(t);
            }
            for (int ver = 0; ver < block.mesh[m].vertex_count; ver ++){
                Vertex v;
                v.x = mesh_data.readU16LE();
                v.z = mesh_data.readU16LE();
                v.y = mesh_data.readU16LE();
                v.unknown = mesh_data.readU16LE();
                block.mesh[m].vertices.push_back(v);
            }
            for (int nor = 0; nor < block.mesh[m].vertex_count; nor ++){
                Vertex v;
                v.x = mesh_data.readU16LE();
                v.z = mesh_data.readU16LE();
                v.y = mesh_data.readU16LE();
                v.unknown = mesh_data.readU16LE();
                block.mesh[m].normals.push_back(v);
            }

            // Add data to the walkmesh
            for (int tri = 0; tri < block.mesh[m].triangle_count; tri ++){
                Triangle t = block.mesh[m].triangles.at(tri);
                walkmesh.addTriangle(
                  Ogre::Vector3(
                    block.mesh[m].vertices.at(t.vertex_index[0]).x + wm_offset_x,
                    block.mesh[m].vertices.at(t.vertex_index[0]).y + wm_offset_y,
                    block.mesh[m].vertices.at(t.vertex_index[0]).z
                  ), // a
                  Ogre::Vector3(
                    block.mesh[m].vertices.at(t.vertex_index[1]).x + wm_offset_x,
                    block.mesh[m].vertices.at(t.vertex_index[1]).y + wm_offset_y,
                    block.mesh[m].vertices.at(t.vertex_index[1]).z
                  ), // b
                  Ogre::Vector3(
                    block.mesh[m].vertices.at(t.vertex_index[2]).x + wm_offset_x,
                    block.mesh[m].vertices.at(t.vertex_index[2]).y + wm_offset_y,
                    block.mesh[m].vertices.at(t.vertex_index[2]).z
                  ), // c
                  t.walkability,
                  t.function_id
                );
            }
        }
        map.blocks.push_back(block);
    }
    // TODO: Generate material.
    // Generate manual meshes.
    for (int b = 0; b < map.blocks.size(); b ++){
        int wm_offset_x = 32 * (b % 9);
        int wm_offset_y = 32 * (b / 9);
        //std::cout << "[MAN] Block " << b << "/" << map.blocks.size() << std::endl;

        std::string name = map_name + "_" + std::to_string(b);
        std::string mat_name = texture_.at(0).name;
        Ogre::ManualObject man = Ogre::ManualObject(name);
        //man.setBoundingBox(Ogre::AxisAlignedBox({-100,-100,0}, {100,100,0})); //TODO;
          //= Ogre::Root::getSingleton().getSceneManager("Scene")->createManualObject(name);
        man.begin(mat_name, Ogre::RenderOperation::OT_TRIANGLE_LIST);
        for (int m = 0; m < 16; m ++){
            //std::cout << "[MAN]        Mesh " << m << " triangles: "
            //  << map.blocks[b].mesh[m].triangle_count << "\n";
            // Add vertices.
            for (int v = 0; v < map.blocks[b].mesh[m].vertex_count; v ++){
                man.position(
                  map.blocks[b].mesh[m].vertices[v].x, map.blocks[b].mesh[m].vertices[v].y,
                  map.blocks[b].mesh[m].vertices[v].z
                );
                man.normal(
                  map.blocks[b].mesh[m].normals[v].x, map.blocks[b].mesh[m].normals[v].y,
                  map.blocks[b].mesh[m].normals[v].z
                );
                // TODO: How to add texture coordinates at this point?
                //man->textureCoord(0, 0);
            }
            // Conform faces from vertices
            for (int t = 0; t < map.blocks[b].mesh[m].triangle_count; t ++){
                //std::cout << "Triangle " << t << " vertizes: "
                //  << (int) map.blocks[b].mesh[m].triangles[t].vertex_index[0] << ", "
                //  << (int) map.blocks[b].mesh[m].triangles[t].vertex_index[1] << ", "
                //  << (int) map.blocks[b].mesh[m].triangles[t].vertex_index[2] << std::endl;
                // TODO: El index deberia incrementar por triangulo por cada material
                man.setMaterialName(
                  0, texture_.at(map.blocks[b].mesh[m].triangles[t].texture).name
                );
                man.triangle(
                  map.blocks[b].mesh[m].triangles[t].vertex_index[0],
                  map.blocks[b].mesh[m].triangles[t].vertex_index[1],
                  map.blocks[b].mesh[m].triangles[t].vertex_index[2]
                );
            }
        }
        // Export the mesh.
        man.end();
        Ogre::MeshPtr mesh = man.convertToMesh(name);
        //std::cout << "[WI] Export mesh " << (output_dir_ + "wm/" + mesh->getName() + ".mesh")
        //  << std::endl;
        mesh_serializer.exportMesh(
          mesh.getPointer(),
          output_dir_ + TERRAIN_MODELS_DIR + "/" + std::to_string(processed_maps_)
            + "/" + mesh->getName() + ".mesh"
        );
        // WM0 is te main worldmap, its shape changes according to the history progress, it has
        // 68 blocks but only 63 are used at a time
        if (processed_maps_ > 0 || b < 63){
            std::unique_ptr<TiXmlElement> xml_block(new TiXmlElement("block"));
            xml_block->SetAttribute("index", std::to_string(b));
            xml_block->SetAttribute(
              "file_name",
              "terrain/" + std::to_string(processed_maps_) + "/" + mesh->getName() + ".mesh"
            );
            xml_block->SetAttribute(
              "position", std::to_string(wm_offset_x) + " " + std::to_string(wm_offset_y) + " 0"
            );
            if (processed_maps_ == 0){
                // TODO: Correct history points for each block.
                switch (b){
                    case 50: // Block 50 replaced by block 63
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_63.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                    case 41: // Block 41 replaced by block 64
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_64.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                    case 42: // Block 42 replaced by block 65
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_65.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                    case 60: // Block 60 replaced by block 66
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_66.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                    case 47: // Block 50 replaced by block 67
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_67.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                    case 48: // Block 50 replaced by block 68
                        xml_block->SetAttribute("alt_file_name", "terrain/0/WM0_68.mesh");
                        xml_block->SetAttribute("alt_story_change", "1000");
                        break;
                }
            }
            xml_terrain->LinkEndChild(xml_block.release());
        }
    }
    // Add other data to the XML file.
    // Close and save XML file,
    xml->LinkEndChild(xml_terrain.release());
    doc.LinkEndChild(xml.release());
    // TODO: Add models, entities...
    doc.SaveFile(
      output_dir_ + "/world/" + std::to_string(processed_maps_)
      + "/world" + std::to_string(processed_maps_) + ".xml"
    );
    // Generate the walkmesh xml file
    walkmesh.generate(output_dir_ + "/world/" + std::to_string(processed_maps_) + "/wm.xml");
    // TODO: Generate other files: background, script, texts...
    // Ready for next map or next step.
    processed_maps_ ++;
    if (processed_maps_ >= wm_map_.size()) return true;
    else return false;
}

void WorldInstaller::ExportMesh(const std::string outdir, const Ogre::MeshPtr &mesh){

    // TODO: Share function with pc model exporter
    VGears::String base_mesh_name;
    VGears::StringUtil::splitFull(mesh->getName(), base_mesh_name);
    Ogre::MeshSerializer mesh_serializer;
    Ogre::SkeletonPtr skeleton(mesh->getSkeleton());
    Ogre::SkeletonSerializer skeleton_serializer;
    skeleton_serializer.exportSkeleton(
      skeleton.getPointer(), outdir + base_mesh_name + ".skeleton"
    );
    mesh->setSkeletonName(base_mesh_name + ".skeleton");
    mesh_serializer.exportMesh(mesh.getPointer(), outdir + mesh->getName());
    Ogre::Mesh::SubMeshIterator it(mesh->getSubMeshIterator());
    Ogre::MaterialSerializer mat_ser;
    size_t i(0);
    std::set<std::string> textures;
    while (it.hasMoreElements()){
        Ogre::SubMesh *sub_mesh(it.getNext());
        Ogre::MaterialPtr mat(Ogre::MaterialManager::getSingleton().getByName(
          sub_mesh->getMaterialName())
        );
        if (mat != nullptr){
            for (size_t techs = 0; techs < mat->getNumTechniques(); techs ++){
                Ogre::Technique* tech = mat->getTechnique(techs);
                if (tech){
                    for (size_t pass_num = 0; pass_num < tech->getNumPasses(); pass_num ++){
                        Ogre::Pass* pass = tech->getPass(pass_num);
                        if (pass){
                            for (
                              size_t texture_unit_num = 0;
                              texture_unit_num < pass->getNumTextureUnitStates();
                              texture_unit_num ++
                            ){
                                Ogre::TextureUnitState* unit
                                  = pass->getTextureUnitState(texture_unit_num);
                                if (unit && unit->getTextureName().empty() == false){
                                    // Convert the texture from .tex to .png.
                                    TexFile tex(
                                      output_dir_ + "temp/wm/" + unit->getTextureName()
                                    );

                                    // Ensure the output material script references png files
                                    // rather than tex files.
                                    Ogre::String base_name;
                                    VGears::StringUtil::splitBase(
                                      unit->getTextureName(), base_name
                                    );
                                    unit->setTextureName(base_mesh_name + "_" + base_name + ".png");

                                    tex.SavePng(
                                      output_dir_ + ELEMENT_MODELS_DIR + "/" + base_name + ".png", 0
                                    );
                                    // Copy subtexture (xxxx.png) to model_xxxx.png
                                    // TODO: obtain the "data" folder
                                    // programatically.
                                    boost::filesystem::copy_file(
                                        output_dir_ + ELEMENT_MODELS_DIR + "/" + base_name + ".png",
                                        output_dir_ + ELEMENT_MODELS_DIR + "/"
                                        + base_mesh_name + "_" + base_name + ".png",
                                      boost::filesystem::copy_option::overwrite_if_exists
                                    );
                                    textures.insert(unit->getTextureName());
                                }
                            }
                        }
                    }
                }
            }
            // TODO: Check what to do with materials
            /**
            if (std::count(materials_.begin(), materials_.end(), sub_mesh->getMaterialName()) == 0){
                mat_ser.queueForExport(mat);
                materials_.push_back(sub_mesh->getMaterialName());
            }*/

        }
        ++ i;
    }
    mat_ser.exportQueued(outdir + base_mesh_name + VGears::EXT_MATERIAL);
    for (auto& texture_name : textures){
        std::string tex_name = texture_name.c_str();
        try{
            Ogre::TexturePtr texture_ptr
              = Ogre::TextureManager::getSingleton().load(tex_name, "FFVIITextures" /*"FFVII"*/);
            Ogre::Image image;
            texture_ptr->convertToImage(image);
            Ogre::String base_name;
            VGears::StringUtil::splitBase(texture_name, base_name);
            image.save(outdir + base_mesh_name + "_" + base_name + ".png");
        }
        catch (std::exception const& ex){
            std::cerr << "[ERROR] Exception: " << ex.what() << std::endl;
        }
    }
}

