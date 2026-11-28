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

#pragma once

/**
 * An image cut out of a texture of menu_us.lgp.
 */
struct MenuImage{

    /**
     * Name of the texture.
     */
    const char* texture;

    /**
     * Name of the image, in the folder of the interface images.
     */
    const char* image;

    /**
     * Position and size of the image in the texture.
     */
    unsigned int x, y, width, height;

    /**
     * Palette of the texture to use.
     */
    unsigned int palette;
};

/**
 * An image made of two pieces of a texture, the second one at the right of the first.
 */
struct MenuImagePair{

    /**
     * Name of the texture.
     */
    const char* texture;

    /**
     * Name of the image, in the folder of the interface images.
     */
    const char* image;

    /**
     * Position of both pieces, their widths and their height, as used by TexFile::SavePng.
     */
    unsigned int x1, x2, y1, y2, width1, width2, height;

    /**
     * Palette of the texture to use.
     */
    unsigned int palette;
};

/**
 * The images of the interface. If an image is listed twice, the last one is the one installed.
 *
 * The cursors of btl_win_b_h.tex are not used: they don't have shadows and are too narrow.
 *
 * TODO: Extract arena reels from coloa.tex, colob.tex and coloc.tex
 */
static const MenuImage MENU_IMAGES[] = {
    // barre.tex
    {"barre.tex", "characters/1.png", 0, 0, 83, 95, 0},
    // bins.tex
    {"bins.tex", "characters/7.png", 0, 0, 83, 95, 0},
    // cido.tex
    {"cido.tex", "characters/8.png", 0, 0, 83, 95, 0},
    // cloud.tex
    {"cloud.tex", "characters/0.png", 0, 0, 83, 95, 0},
    // earith.tex
    {"earith.tex", "characters/3.png", 0, 0, 83, 95, 0},
    // ketc.tex
    {"ketc.tex", "characters/6.png", 0, 0, 83, 95, 0},
    // pcefi.tex
    {"pcefi.tex", "characters/10.png", 0, 0, 83, 95, 0},
    // pcloud.tex
    {"pcloud.tex", "characters/9.png", 0, 0, 83, 95, 0},
    // red.tex
    {"red.tex", "characters/4.png", 0, 0, 83, 95, 0},
    // tifa.tex
    {"tifa.tex", "characters/2.png", 0, 0, 83, 95, 0},
    // yufi.tex
    {"yufi.tex", "characters/5.png", 0, 0, 83, 95, 0},
    // buster.tex
    {"buster.tex", "other/begin_menu.png", 0, 0, 150, 150, 0},
    // choco.tex
    {"choco.tex", "other/choco.png", 0, 0, 83, 95, 0},
    // btl_win_c_l.tex
    {"btl_win_c_l.tex", "icons/item_weapon_1.png", 192, 0, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/item_weapon_2.png", 224, 0, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/item_weapon_3.png", 192, 32, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/item_weapon_4.png", 224, 32, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/item_weapon_5.png", 192, 64, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/item_weapon_6.png", 224, 64, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/item_weapon_7.png", 192, 96, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/item_weapon_8.png", 224, 96, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/item_armor.png", 192, 128, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/item_accessory.png", 224, 128, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/status_2_0.png", 128, 188, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/status_2_1.png", 160, 188, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/status_2_2.png", 192, 188, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/status_2_3.png", 224, 188, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/status_7_0.png", 128, 219, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/status_7_1.png", 160, 219, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/status_7_2.png", 192, 219, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/status_7_3.png", 224, 219, 32, 32, 1},
    {"btl_win_c_l.tex", "icons/pointer_gateway_0.png", 96, 160, 18, 12, 9},
    {"btl_win_c_l.tex", "icons/pointer_gateway_1.png", 128, 160, 18, 12, 9},
    {"btl_win_c_l.tex", "icons/pointer_gateway_2.png", 160, 160, 18, 12, 9},
    {"btl_win_c_l.tex", "icons/pointer_gateway_3.png", 192, 160, 18, 12, 9},
    {"btl_win_c_l.tex", "icons/pointer_gateway_4.png", 224, 160, 18, 12, 9},
    {"btl_win_c_l.tex", "icons/pointer_ladder_0.png", 96, 160, 18, 12, 4},
    {"btl_win_c_l.tex", "icons/pointer_ladder_1.png", 128, 160, 18, 12, 4},
    {"btl_win_c_l.tex", "icons/pointer_ladder_2.png", 160, 160, 18, 12, 4},
    {"btl_win_c_l.tex", "icons/pointer_ladder_3.png", 192, 160, 18, 12, 4},
    {"btl_win_c_l.tex", "icons/pointer_ladder_4.png", 224, 160, 18, 12, 4},
    {"btl_win_c_l.tex", "icons/battle_death.png", 64, 192, 64, 20, 2},
    {"btl_win_c_l.tex", "window/b_t.png", 0, 192, 32, 16, 0},
    {"btl_win_c_l.tex", "window/b_b.png", 32, 208, 32, 16, 0},
    {"btl_win_c_l.tex", "window/b_l.png", 0, 224, 16, 32, 0},
    {"btl_win_c_l.tex", "window/b_r.png", 48, 224, 16, 32, 0},
    {"btl_win_c_l.tex", "window/c_tl.png", 0, 208, 16, 16, 0},
    {"btl_win_c_l.tex", "window/c_tr.png", 16, 208, 16, 16, 0},
    {"btl_win_c_l.tex", "window/c_bl.png", 32, 192, 16, 16, 0},
    {"btl_win_c_l.tex", "window/c_br.png", 48, 192, 16, 16, 0},
    // btl_win_a_h.tex
    {"btl_win_a_h.tex", "icons/item_item.png", 192, 223, 32, 32, 1},
    {"btl_win_a_h.tex", "icons/item_weapon_0.png", 220, 223, 32, 32, 1},
    {"btl_win_a_h.tex", "icons/battle_bar.png", 32, 0, 76, 18, 1},
    {"btl_win_a_h.tex", "icons/battle_limit.png", 112, 0, 48, 10, 1},
    {"btl_win_a_h.tex", "icons/battle_hp.png", 160, 0, 28, 10, 1},
    {"btl_win_a_h.tex", "icons/battle_mp.png", 192, 0, 30, 10, 1},
    {"btl_win_a_h.tex", "icons/battle_barrier.png", 112, 16, 76, 10, 1},
    {"btl_win_a_h.tex", "icons/battle_wait.png", 192, 16, 44, 10, 1},
    {"btl_win_a_h.tex", "icons/battle_1.png", 130, 34, 10, 12, 1},
    {"btl_win_a_h.tex", "icons/battle_2.png", 146, 34, 10, 12, 1},
    {"btl_win_a_h.tex", "icons/battle_3.png", 162, 34, 10, 12, 1},
    {"btl_win_a_h.tex", "icons/battle_4.png", 178, 34, 10, 12, 1},
    {"btl_win_a_h.tex", "icons/battle_5.png", 194, 34, 10, 12, 1},
    {"btl_win_a_h.tex", "icons/battle_mp_needed.png", 0, 40, 80, 24, 1},
    {"btl_win_a_h.tex", "icons/battle_all.png", 240, 32, 16, 16, 7},
    {"btl_win_a_h.tex", "icons/battle_x.png", 240, 16, 16, 16, 7},
    {"btl_win_a_h.tex", "icons/materia_slot_no_growth.png", 224, 63, 24, 24, 1},
    {"btl_win_a_h.tex", "icons/materia_slot_link.png", 227, 95, 14, 24, 1},
    {"btl_win_a_h.tex", "icons/bt_up.png", 160, 70, 26, 20, 7},
    {"btl_win_a_h.tex", "icons/bt_down.png", 192, 70, 26, 20, 7},
    {"btl_win_a_h.tex", "icons/bt_right.png", 160, 102, 26, 20, 7},
    {"btl_win_a_h.tex", "icons/bt_left.png", 192, 102, 26, 20, 7},
    {"btl_win_a_h.tex", "icons/bt_3.png", 192, 134, 26, 20, 7},
    {"btl_win_a_h.tex", "icons/bt_2.png", 224, 134, 26, 20, 7},
    {"btl_win_a_h.tex", "icons/bt_6.png", 192, 166, 26, 20, 7},
    {"btl_win_a_h.tex", "icons/bt_8.png", 224, 166, 26, 20, 7},
    {"btl_win_a_h.tex", "icons/bt_9.png", 192, 198, 26, 20, 7},
    {"btl_win_a_h.tex", "icons/bt_10.png", 224, 198, 26, 20, 7},
    {"btl_win_a_h.tex", "icons/cursor_crossed.png", 80, 32, 48, 32, 7},
    {"btl_win_a_h.tex", "icons/settings_music.png", 101, 131, 30, 30, 1},
    {"btl_win_a_h.tex", "icons/settings_fx.png", 133, 131, 30, 30, 1},
    // btl_win_b_h.tex
    {"btl_win_b_h.tex", "icons/slash.png", 176, 0, 8, 16, 1},
    {"btl_win_b_h.tex", "icons/battle_name.png", 0, 32, 52, 10, 1},
    {"btl_win_b_h.tex", "icons/battle_status.png", 0, 48, 64, 10, 1},
    {"btl_win_b_h.tex", "icons/battle_level.png", 144, 48, 52, 10, 1},
    {"btl_win_b_h.tex", "icons/pointer_up.png", 198, 0, 22, 16, 1},
    {"btl_win_b_h.tex", "icons/pointer_down.png", 230, 0, 22, 16, 1},
    {"btl_win_b_h.tex", "icons/materia_slot.png", 0, 95, 24, 24, 1},
    {"btl_win_b_h.tex", "icons/materia_1.png", 3, 67, 18, 18, 4},
    {"btl_win_b_h.tex", "icons/materia_2.png", 3, 67, 18, 18, 5},
    {"btl_win_b_h.tex", "icons/materia_3.png", 3, 67, 18, 18, 8},
    {"btl_win_b_h.tex", "icons/materia_4.png", 3, 67, 18, 18, 9},
    {"btl_win_b_h.tex", "icons/materia_5.png", 3, 67, 18, 18, 10},
    {"btl_win_b_h.tex", "icons/materia_star_1.png", 32, 64, 23, 26, 4},
    {"btl_win_b_h.tex", "icons/materia_star_2.png", 32, 64, 23, 26, 5},
    {"btl_win_b_h.tex", "icons/materia_star_3.png", 32, 64, 23, 26, 8},
    {"btl_win_b_h.tex", "icons/materia_star_4.png", 32, 64, 23, 26, 9},
    {"btl_win_b_h.tex", "icons/materia_star_5.png", 32, 64, 23, 26, 10},
    {"btl_win_b_h.tex", "icons/materia_star_empty_1.png", 32, 96, 23, 26, 4},
    {"btl_win_b_h.tex", "icons/materia_star_empty_2.png", 32, 96, 23, 26, 5},
    {"btl_win_b_h.tex", "icons/materia_star_empty_3.png", 32, 96, 23, 26, 8},
    {"btl_win_b_h.tex", "icons/materia_star_empty_4.png", 32, 96, 23, 26, 9},
    {"btl_win_b_h.tex", "icons/materia_star_empty_5.png", 32, 96, 23, 26, 10},
    {"btl_win_b_h.tex", "icons/bt_1.png", 0, 134, 26, 20, 7},
    {"btl_win_b_h.tex", "icons/bt_4.png", 32, 134, 26, 20, 7},
    {"btl_win_b_h.tex", "icons/bt_5.png", 0, 166, 26, 20, 7},
    {"btl_win_b_h.tex", "icons/bt_7.png", 32, 16, 26, 20, 7},
    {"btl_win_b_h.tex", "icons/battle_inf.png", 36, 194, 26, 14, 7},
    {"btl_win_b_h.tex", "icons/pointer_right.png", 4, 16, 10, 16, 7},
    {"btl_win_b_h.tex", "icons/battle_bar_barriers.png", 64, 32, 76, 24, 1},
    {"btl_win_b_h.tex", "icons/menu_bar.png", 16, 16, 126, 16, 1},
    {"btl_win_b_h.tex", "reels/yeah.png", 64, 64, 64, 30, 11},
    {"btl_win_b_h.tex", "reels/luck.png", 64, 98, 64, 30, 11},
    {"btl_win_b_h.tex", "reels/hit.png", 128, 64, 64, 30, 11},
    {"btl_win_b_h.tex", "reels/bar.png", 128, 97, 64, 30, 11},
    {"btl_win_b_h.tex", "reels/miss.png", 192, 64, 64, 30, 11},
    {"btl_win_b_h.tex", "reels/choso.png", 192, 98, 64, 30, 11},
    // btl_win_b_l.tex
    {"btl_win_b_l.tex", "icons/cursor.png", 192, 16, 48, 32, 1},
    {"btl_win_b_l.tex", "icons/cursor_left.png", 144, 16, 48, 32, 1},
    {"btl_win_b_l.tex", "icons/pointer_position.png", 0, 192, 32, 26, 1},
};

/**
 * The images of the interface that are made of two pieces.
 */
static const MenuImagePair MENU_IMAGE_PAIRS[] = {
    {"btl_win_c_l.tex", "icons/battle_recovery.png", 64, 64, 212, 232, 64, 36, 20, 4},
    {"btl_win_a_h.tex", "icons/battle_time.png", 224, 162, 0, 16, 32, 14, 10, 1},
};
