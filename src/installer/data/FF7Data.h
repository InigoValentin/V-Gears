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

#include <algorithm>

/**
 * Class that provides fixed data related to Final Fantasy VII.
 */
class FF7Data{

    public:

        /**
         * Contains playable character IDs.
         */
        class CHAR_ID{

            public:

                /**
                 * Cloud ID.
                 */
                static const int CLOUD = 0;

                /**
                 * Barret ID.
                 */
                static const int BARRET = 1;

                /**
                 * Tifa ID.
                 */
                static const int TIFA = 2;

                /**
                 * Aeris ID.
                 */
                static const int AERIS = 3;

                /**
                 * Red XIII ID.
                 */
                static const int RED_XIII = 4;

                /**
                 * Yuffie ID.
                 */
                static const int YUFFIE = 5;

                /**
                 * Cait Sith ID.
                 */
                static const int CAIT_SITH = 6;

                /**
                 * Vincent ID.
                 */
                static const int VINCENT = 7;

                /**
                 * Cid ID.
                 */
                static const int CID = 8;

                /**
                 * Young Cloud ID.
                 */
                static const int YOUND_CLOUD = 9;

                /**
                 * Sephiroth ID.
                 */
                static const int SEPHIROTH = 10;

                /**
                 * Chocobo ID.
                 */
                static const int CHOCOBO = 11;
        };

        /**
         * Information about a model.
         */
        struct BattleModelInfo{

            /**
             * A numeric ID assigned to the model.
             */
            int numeric_id;

            /**
             * An alphanumeric ID assigned to the model.
             */
            std::string alphanumeric_id;

            /**
             * A name for the model.
             */
            std::string name;

            /**
             * A normalized version of the name.
             */
            std::string name_normal;

            /**
             * Indicates if the model is of an enemy.
             */
            bool is_enemy;

            /**
             * Indicates if the model is of aa playable character.
             */
            bool is_player;

            /**
             * Indicates if the model is of a battle scene.
             */
            bool is_scene;

            /**
             * Constructor. Initializes to default values.
             */
            BattleModelInfo():
              numeric_id(-1), alphanumeric_id(""), name(""), name_normal(""),
              is_enemy(false), is_player(false), is_scene(false)
            {}
        };

        /**
         * Retrieves information about a battle model from it's name.
         *
         * The return structure has the following properties ({@see BattleModelInfo}):
         *
         * - numeric_id: A numeric ID assigned to the model. For enemies, it's a unique enemy ID.
         * For scenes, it's a unique numeric id. For playable characters, it is the id of the
         * character the model references, but since  a character may have many models, it's not
         * guaranteed to be unique. There is a special case where it is -1: for the playable frog,
         * which is related to all characters.
         *
         * - alphanumeric_id: A unique id assigned to the model. Two lowercase letters.
         *
         * - name: A descriptive name for the model. For debugging purposes only, it's not intended
         * to ever be displayed, and it's not suitable to use in filenames.
         *
         * - name_normal: Simplified name, suitable to be used on filenames. Not garanteed to be
         * unique.
         *
         * - is_enemy: Indicates if the models is a battle enemy.
         *
         * - is_enemy: Indicates if the models is a battle playable character.
         *
         * - is_scene: Indicates if the models is a battle scenario.
         *
         * @param[in] model_id ID of the model. It can be two letters, or two letters preceded by
         * "btl_". Case insensitive. If the id contains more than four letters (not counting the
         * optional "btl_" prefix), only the first two sill be considered.
         * @return A battle model info structure. If an invvalid model ID is provided, or if the
         * model doesn't exist, an empty structure will be returned.
         */
        static BattleModelInfo GetBattleModelInfo(std::string model_id){
            BattleModelInfo info;
            std::string alphanumeric_id = model_id;
            std::transform(
              alphanumeric_id.begin(), alphanumeric_id.end(), alphanumeric_id.begin(), ::tolower
            );
            if (alphanumeric_id.size() < 2) return info; // Empty
            if (alphanumeric_id.size() > 4 && alphanumeric_id.substr(0, 4) == "btl_")
                alphanumeric_id = alphanumeric_id.substr(4, alphanumeric_id.size() - 4);
            if (alphanumeric_id.size() > 4) alphanumeric_id = alphanumeric_id.substr(0, 2);
            info.alphanumeric_id = alphanumeric_id;
            info.name = alphanumeric_id;
            info.name_normal = "btl_" + alphanumeric_id;
            if ("aa" == alphanumeric_id){
                info.numeric_id = 0;
                info.name = "Unused Pyramid";
                info.name_normal = "unused_pyramid";
                info.is_enemy = true;
            }
            else if ("ab" == alphanumeric_id){
                info.numeric_id = 1;
                info.name = "Unused Pyramid";
                info.name_normal = "unused_pyramid";
                info.is_enemy = true;
            }
            else if ("ac" == alphanumeric_id){
                info.numeric_id = 2;
                info.name = "Unused Pyramid";
                info.name_normal = "unused_pyramid";
                info.is_enemy = true;
            }
            else if ("ad" == alphanumeric_id){
                info.numeric_id = 3;
                info.name = "Unused Pyramid";
                info.name_normal = "unused_pyramid";
                info.is_enemy = true;
            }
            else if ("ae" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Unused Pyramid";
                info.name_normal = "unused_pyramid";
                info.is_enemy = true;
            }
            else if ("af" == alphanumeric_id){
                info.numeric_id = 5;
                info.name = "Unused Pyramid";
                info.name_normal = "unused_pyramid";
                info.is_enemy = true;
            }
            else if ("ag" == alphanumeric_id){
                info.numeric_id = 6;
                info.name = "Unused Pyramid";
                info.name_normal = "unused_pyramid";
                info.is_enemy = true;
            }
            else if ("ah" == alphanumeric_id){
                info.numeric_id = 7;
                info.name = "Unused Pyramid";
                info.name_normal = "unused_pyramid";
                info.is_enemy = true;
            }
            else if ("ai" == alphanumeric_id){
                info.numeric_id = 8;
                info.name = "Unused Pyramid";
                info.name_normal = "unused_pyramid";
                info.is_enemy = true;
            }
            else if ("aj" == alphanumeric_id){
                info.numeric_id = 9;
                info.name = "Unused Pyramid";
                info.name_normal = "unused_pyramid";
                info.is_enemy = true;
            }
            else if ("ak" == alphanumeric_id){
                info.numeric_id = 10;
                info.name = "Diamond Weapon";
                info.name_normal = "diamond_weapon";
                info.is_enemy = true;
            }
            else if ("al" == alphanumeric_id){
                info.numeric_id = 11;
                info.name = "Ruby Weapon";
                info.name_normal = "ruby_weapon";
                info.is_enemy = true;
            }
            else if ("am" == alphanumeric_id){
                info.numeric_id = 12;
                info.name = "Ruby???s Tentacle";
                info.name_normal = "ruby???s_tentacle";
                info.is_enemy = true;
            }
            else if ("an" == alphanumeric_id){
                info.numeric_id = 13;
                info.name = "Emerald Weapon: Upper Battle Skeleton";
                info.name_normal = "emerald_weapon_upper_battle_skeleton";
                info.is_enemy = true;
            }
            else if ("ao" == alphanumeric_id){
                info.numeric_id = 14;
                info.name = "Emerald Weapon: Lower Battle Skeleton";
                info.name_normal = "emerald_weapon_lower_battle_skeleton";
                info.is_enemy = true;
            }
            else if ("ap" == alphanumeric_id){
                info.numeric_id = 15;
                info.name = "UNKNOWN";
                info.name_normal = "unknown";
                info.is_enemy = true;
            }
            else if ("aq" == alphanumeric_id){
                info.numeric_id = 16;
                info.name = "MP";
                info.name_normal = "mp";
                info.is_enemy = true;
            }
            else if ("ar" == alphanumeric_id){
                info.numeric_id = 17;
                info.name = "Guard Hound";
                info.name_normal = "guard_hound";
                info.is_enemy = true;
            }
            else if ("as" == alphanumeric_id){
                info.numeric_id = 18;
                info.name = "Mono Dive";
                info.name_normal = "mono_dive";
                info.is_enemy = true;
            }
            else if ("at" == alphanumeric_id){
                info.numeric_id = 19;
                info.name = "Grunt";
                info.name_normal = "grunt";
                info.is_enemy = true;
            }
            else if ("au" == alphanumeric_id){
                info.numeric_id = 20;
                info.name = "1st Ray";
                info.name_normal = "1st_ray";
                info.is_enemy = true;
            }
            else if ("av" == alphanumeric_id){
                info.numeric_id = 21;
                info.name = "Sweeper: Unidentified Part";
                info.name_normal = "sweeper_unidentified_part";
                info.is_enemy = true;
            }
            else if ("aw" == alphanumeric_id){
                info.numeric_id = 22;
                info.name = "Guard Scorpion: Battle skeleton";
                info.name_normal = "guard_scorpion_battle_skeleton";
                info.is_enemy = true;
            }
            else if ("ax" == alphanumeric_id){
                info.numeric_id = 23;
                info.name = "Garshtrike";
                info.name_normal = "garshtrike";
                info.is_enemy = true;
            }
            else if ("ay" == alphanumeric_id){
                info.numeric_id = 24;
                info.name = "Rocket Launcher";
                info.name_normal = "rocket_launcher";
                info.is_enemy = true;
            }
            else if ("az" == alphanumeric_id){
                info.numeric_id = 25;
                info.name = "Whole Eater";
                info.name_normal = "whole_eater";
                info.is_enemy = true;
            }
            else if ("ba" == alphanumeric_id){
                info.numeric_id = 26;
                info.name = "Chuse Tank";
                info.name_normal = "chuse_tank";
                info.is_enemy = true;
            }
            else if ("bb" == alphanumeric_id){
                info.numeric_id = 27;
                info.name = "Blugu";
                info.name_normal = "blugu";
                info.is_enemy = true;
            }
            else if ("bc" == alphanumeric_id){
                info.numeric_id = 28;
                info.name = "Hedgehog Pie";
                info.name_normal = "hedgehog_pie";
                info.is_enemy = true;
            }
            else if ("bd" == alphanumeric_id){
                info.numeric_id = 29;
                info.name = "Smogger";
                info.name_normal = "smogger";
                info.is_enemy = true;
            }
            else if ("be" == alphanumeric_id){
                info.numeric_id = 30;
                info.name = "Special Combatant";
                info.name_normal = "special_combatant";
                info.is_enemy = true;
            }
            else if ("bf" == alphanumeric_id){
                info.numeric_id = 31;
                info.name = "Blood Taste";
                info.name_normal = "blood_taste";
                info.is_enemy = true;
            }
            else if ("bg" == alphanumeric_id){
                info.numeric_id = 32;
                info.name = "Proto Machinegun";
                info.name_normal = "proto_machinegun";
                info.is_enemy = true;
            }
            else if ("bh" == alphanumeric_id){
                info.numeric_id = 33;
                info.name = "Air Buster";
                info.name_normal = "air_buster";
                info.is_enemy = true;
            }
            else if ("bi" == alphanumeric_id){
                info.numeric_id = 34;
                info.name = "Vice: Battle Model";
                info.name_normal = "vice_battle_model";
                info.is_enemy = true;
            }
            else if ("bj" == alphanumeric_id){
                info.numeric_id = 35;
                info.name = "Corneo's Lackey";
                info.name_normal = "corneos_lackey";
                info.is_enemy = true;
            }
            else if ("bk" == alphanumeric_id){
                info.numeric_id = 36;
                info.name = "Scotch";
                info.name_normal = "scotch";
                info.is_enemy = true;
            }
            else if ("bl" == alphanumeric_id){
                info.numeric_id = 37;
                info.name = "Aps: Unidentified Part";
                info.name_normal = "aps_unidentified_part";
                info.is_enemy = true;
            }
            else if ("bm" == alphanumeric_id){
                info.numeric_id = 38;
                info.name = "Sahagin";
                info.name_normal = "sahagin";
                info.is_enemy = true;
            }
            else if ("bn" == alphanumeric_id){
                info.numeric_id = 39;
                info.name = "Ceasar";
                info.name_normal = "ceasar";
                info.is_enemy = true;
            }
            else if ("bo" == alphanumeric_id){
                info.numeric_id = 40;
                info.name = "Eligor";
                info.name_normal = "eligor";
                info.is_enemy = true;
            }
            else if ("bp" == alphanumeric_id){
                info.numeric_id = 41;
                info.name = "Ghost";
                info.name_normal = "ghost";
                info.is_enemy = true;
            }
            else if ("bq" == alphanumeric_id){
                info.numeric_id = 42;
                info.name = "Cripshay";
                info.name_normal = "cripshay";
                info.is_enemy = true;
            }
            else if ("br" == alphanumeric_id){
                info.numeric_id = 43;
                info.name = "Deenglow";
                info.name_normal = "deenglow";
                info.is_enemy = true;
            }
            else if ("bs" == alphanumeric_id){
                info.numeric_id = 44;
                info.name = "Hell House (calm) ";
                info.name_normal = "hell_house_calm_";
                info.is_enemy = true;
            }
            else if ("bt" == alphanumeric_id){
                info.numeric_id = 45;
                info.name = "Hell House (angry)";
                info.name_normal = "hell_house_angry";
                info.is_enemy = true;
            }
            else if ("bu" == alphanumeric_id){
                info.numeric_id = 46;
                info.name = "Aero Combatant (Flying)";
                info.name_normal = "aero_combatant_flying";
                info.is_enemy = true;
            }
            else if ("bv" == alphanumeric_id){
                info.numeric_id = 47;
                info.name = "Aero Combatant (Downed)";
                info.name_normal = "aero_combatant_downed";
                info.is_enemy = true;
            }
            else if ("bw" == alphanumeric_id){
                info.numeric_id = 48;
                info.name = "TURKS: Reno";
                info.name_normal = "turks_reno";
                info.is_enemy = true;
            }
            else if ("bx" == alphanumeric_id){
                info.numeric_id = 49;
                info.name = "Reno's Pyramid";
                info.name_normal = "renos_pyramid";
                info.is_enemy = true;
            }
            else if ("by" == alphanumeric_id){
                info.numeric_id = 50;
                info.name = "Warning Board";
                info.name_normal = "warning_board";
                info.is_enemy = true;
            }
            else if ("bz" == alphanumeric_id){
                info.numeric_id = 51;
                info.name = "Machine Gun";
                info.name_normal = "machine_gun";
                info.is_enemy = true;
            }
            else if ("ca" == alphanumeric_id){
                info.numeric_id = 52;
                info.name = "Laser Cannon";
                info.name_normal = "laser_cannon";
                info.is_enemy = true;
            }
            else if ("cb" == alphanumeric_id){
                info.numeric_id = 53;
                info.name = "Hammer Blaster (Upper)";
                info.name_normal = "hammer_blaster_upper";
                info.is_enemy = true;
            }
            else if ("cc" == alphanumeric_id){
                info.numeric_id = 54;
                info.name = "Hammer Blaster (Base)";
                info.name_normal = "hammer_blaster_base";
                info.is_enemy = true;
            }
            else if ("cd" == alphanumeric_id){
                info.numeric_id = 55;
                info.name = "Sword Dance";
                info.name_normal = "sword_dance";
                info.is_enemy = true;
            }
            else if ("ce" == alphanumeric_id){
                info.numeric_id = 56;
                info.name = "SOLDIER 3rd";
                info.name_normal = "soldier_3rd";
                info.is_enemy = true;
            }
            else if ("cf" == alphanumeric_id){
                info.numeric_id = 57;
                info.name = "Mighty Grunt (Outer Shell)";
                info.name_normal = "mighty_grunt_outer_shell";
                info.is_enemy = true;
            }
            else if ("cg" == alphanumeric_id){
                info.numeric_id = 58;
                info.name = "Mighty Grunt (Inner)";
                info.name_normal = "mighty_grunt_inner";
                info.is_enemy = true;
            }
            else if ("ch" == alphanumeric_id){
                info.numeric_id = 59;
                info.name = "Moth Slasher";
                info.name_normal = "moth_slasher";
                info.is_enemy = true;
            }
            else if ("ci" == alphanumeric_id){
                info.numeric_id = 60;
                info.name = "Grenade Combatant";
                info.name_normal = "grenade_combatant";
                info.is_enemy = true;
            }
            else if ("cj" == alphanumeric_id){
                info.numeric_id = 61;
                info.name = "Brain Pod";
                info.name_normal = "brain_pod";
                info.is_enemy = true;
            }
            else if ("ck" == alphanumeric_id){
                info.numeric_id = 62;
                info.name = "Vargid Police";
                info.name_normal = "vargid_police";
                info.is_enemy = true;
            }
            else if ("cl" == alphanumeric_id){
                info.numeric_id = 63;
                info.name = "Zenene";
                info.name_normal = "zenene";
                info.is_enemy = true;
            }
            else if ("cm" == alphanumeric_id){
                info.numeric_id = 64;
                info.name = "Sample H0512";
                info.name_normal = "sample_h0512";
                info.is_enemy = true;
            }
            else if ("cn" == alphanumeric_id){
                info.numeric_id = 65;
                info.name = "Sample H0512-opt ";
                info.name_normal = "sample_h0512opt_";
                info.is_enemy = true;
            }
            else if ("co" == alphanumeric_id){
                info.numeric_id = 66;
                info.name = "Hundred Gunner";
                info.name_normal = "hundred_gunner";
                info.is_enemy = true;
            }
            else if ("cp" == alphanumeric_id){
                info.numeric_id = 67;
                info.name = "Heli Gunner";
                info.name_normal = "heli_gunner";
                info.is_enemy = true;
            }
            else if ("cq" == alphanumeric_id){
                info.numeric_id = 68;
                info.name = "Rufus";
                info.name_normal = "rufus";
                info.is_enemy = true;
            }
            else if ("cr" == alphanumeric_id){
                info.numeric_id = 69;
                info.name = "Dark Nation";
                info.name_normal = "dark_nation";
                info.is_enemy = true;
            }
            else if ("cs" == alphanumeric_id){
                info.numeric_id = 70;
                info.name = "Helicopter";
                info.name_normal = "helicopter";
                info.is_enemy = true;
            }
            else if ("ct" == alphanumeric_id){
                info.numeric_id = 71;
                info.name = "Motor Ball";
                info.name_normal = "motor_ball";
                info.is_enemy = true;
            }
            else if ("cu" == alphanumeric_id){
                info.numeric_id = 72;
                info.name = "Devil Ride";
                info.name_normal = "devil_ride";
                info.is_enemy = true;
            }
            else if ("cv" == alphanumeric_id){
                info.numeric_id = 73;
                info.name = "Custom Sweeper";
                info.name_normal = "custom_sweeper";
                info.is_enemy = true;
            }
            else if ("cw" == alphanumeric_id){
                info.numeric_id = 74;
                info.name = "Kalm Fang";
                info.name_normal = "kalm_fang";
                info.is_enemy = true;
            }
            else if ("cx" == alphanumeric_id){
                info.numeric_id = 75;
                info.name = "Prowler";
                info.name_normal = "prowler";
                info.is_enemy = true;
            }
            else if ("cy" == alphanumeric_id){
                info.numeric_id = 76;
                info.name = "Elfadunk";
                info.name_normal = "elfadunk";
                info.is_enemy = true;
            }
            else if ("cz" == alphanumeric_id){
                info.numeric_id = 77;
                info.name = "Mu";
                info.name_normal = "mu";
                info.is_enemy = true;
            }
            else if ("da" == alphanumeric_id){
                info.numeric_id = 78;
                info.name = "Mu (Rock Only)";
                info.name_normal = "mu_rock_only";
                info.is_enemy = true;
            }
            else if ("db" == alphanumeric_id){
                info.numeric_id = 79;
                info.name = "Mandragora";
                info.name_normal = "mandragora";
                info.is_enemy = true;
            }
            else if ("dc" == alphanumeric_id){
                info.numeric_id = 80;
                info.name = "Levrikon: Unidentified Part";
                info.name_normal = "levrikon_unidentified_part";
                info.is_enemy = true;
            }
            else if ("dd" == alphanumeric_id){
                info.numeric_id = 81;
                info.name = "Midgar Zolom";
                info.name_normal = "midgar_zolom";
                info.is_enemy = true;
            }
            else if ("de" == alphanumeric_id){
                info.numeric_id = 82;
                info.name = "Madouge";
                info.name_normal = "madouge";
                info.is_enemy = true;
            }
            else if ("df" == alphanumeric_id){
                info.numeric_id = 83;
                info.name = "Crawler";
                info.name_normal = "crawler";
                info.is_enemy = true;
            }
            else if ("dg" == alphanumeric_id){
                info.numeric_id = 84;
                info.name = "Ark Dragon";
                info.name_normal = "ark_dragon";
                info.is_enemy = true;
            }
            else if ("dh" == alphanumeric_id){
                info.numeric_id = 85;
                info.name = "Castanets";
                info.name_normal = "castanets";
                info.is_enemy = true;
            }
            else if ("di" == alphanumeric_id){
                info.numeric_id = 86;
                info.name = "Zemzelett";
                info.name_normal = "zemzelett";
                info.is_enemy = true;
            }
            else if ("dj" == alphanumeric_id){
                info.numeric_id = 87;
                info.name = "Nerosuferoth";
                info.name_normal = "nerosuferoth";
                info.is_enemy = true;
            }
            else if ("dk" == alphanumeric_id){
                info.numeric_id = 88;
                info.name = "Hell Rider VR2";
                info.name_normal = "hell_rider_vr2";
                info.is_enemy = true;
            }
            else if ("dl" == alphanumeric_id){
                info.numeric_id = 89;
                info.name = "Formula";
                info.name_normal = "formula";
                info.is_enemy = true;
            }
            else if ("dm" == alphanumeric_id){
                info.numeric_id = 90;
                info.name = "Capparwire";
                info.name_normal = "capparwire";
                info.is_enemy = true;
            }
            else if ("dn" == alphanumeric_id){
                info.numeric_id = 91;
                info.name = "Bottomswell";
                info.name_normal = "bottomswell";
                info.is_enemy = true;
            }
            else if ("do" == alphanumeric_id){
                info.numeric_id = 92;
                info.name = "Waterpolo";
                info.name_normal = "waterpolo";
                info.is_enemy = true;
            }
            else if ("dp" == alphanumeric_id){
                info.numeric_id = 93;
                info.name = "Scrutin Eye";
                info.name_normal = "scrutin_eye";
                info.is_enemy = true;
            }
            else if ("dq" == alphanumeric_id){
                info.numeric_id = 94;
                info.name = "Marine";
                info.name_normal = "marine";
                info.is_enemy = true;
            }
            else if ("dr" == alphanumeric_id){
                info.numeric_id = 95;
                info.name = "Jenova Birth";
                info.name_normal = "jenova_birth";
                info.is_enemy = true;
            }
            else if ("ds" == alphanumeric_id){
                info.numeric_id = 96;
                info.name = "Grangalan";
                info.name_normal = "grangalan";
                info.is_enemy = true;
            }
            else if ("dt" == alphanumeric_id){
                info.numeric_id = 97;
                info.name = "Grangalan Jr";
                info.name_normal = "grangalan_jr";
                info.is_enemy = true;
            }
            else if ("du" == alphanumeric_id){
                info.numeric_id = 98;
                info.name = "Grangalan Jr Jr";
                info.name_normal = "grangalan_jr_jr";
                info.is_enemy = true;
            }
            else if ("dv" == alphanumeric_id){
                info.numeric_id = 99;
                info.name = "Beach Plug";
                info.name_normal = "beach_plug";
                info.is_enemy = true;
            }
            else if ("dw" == alphanumeric_id){
                info.numeric_id = 101;
                info.name = "Search Crown";
                info.name_normal = "search_crown";
                info.is_enemy = true;
            }
            else if ("dx" == alphanumeric_id){
                info.numeric_id = 101;
                info.name = "Needle Kiss";
                info.name_normal = "needle_kiss";
                info.is_enemy = true;
            }
            else if ("dy" == alphanumeric_id){
                info.numeric_id = 102;
                info.name = "Bloatfloat";
                info.name_normal = "bloatfloat";
                info.is_enemy = true;
            }
            else if ("dz" == alphanumeric_id){
                info.numeric_id = 103;
                info.name = "Bagnadrana";
                info.name_normal = "bagnadrana";
                info.is_enemy = true;
            }
            else if ("ea" == alphanumeric_id){
                info.numeric_id = 104;
                info.name = "Cokatolis";
                info.name_normal = "cokatolis";
                info.is_enemy = true;
            }
            else if ("eb" == alphanumeric_id){
                info.numeric_id = 105;
                info.name = "Bomb";
                info.name_normal = "bomb";
                info.is_enemy = true;
            }
            else if ("ec" == alphanumeric_id){
                info.numeric_id = 106;
                info.name = "Death Claw";
                info.name_normal = "death_claw";
                info.is_enemy = true;
            }
            else if ("ed" == alphanumeric_id){
                info.numeric_id = 107;
                info.name = "2-Faced";
                info.name_normal = "2faced";
                info.is_enemy = true;
            }
            else if ("ee" == alphanumeric_id){
                info.numeric_id = 108;
                info.name = "Bandit";
                info.name_normal = "bandit";
                info.is_enemy = true;
            }
            else if ("ef" == alphanumeric_id){
                info.numeric_id = 109;
                info.name = "Bullmotor";
                info.name_normal = "bullmotor";
                info.is_enemy = true;
            }
            else if ("eg" == alphanumeric_id){
                info.numeric_id = 110;
                info.name = "Land Worm";
                info.name_normal = "land_worm";
                info.is_enemy = true;
            }
            else if ("eh" == alphanumeric_id){
                info.numeric_id = 111;
                info.name = "Dyne";
                info.name_normal = "dyne";
                info.is_enemy = true;
            }
            else if ("ei" == alphanumeric_id){
                info.numeric_id = 112;
                info.name = "Bullmotor";
                info.name_normal = "bullmotor";
                info.is_enemy = true;
            }
            else if ("ej" == alphanumeric_id){
                info.numeric_id = 112;
                info.name = "Spencer";
                info.name_normal = "spencer";
                info.is_enemy = true;
            }
            else if ("ek" == alphanumeric_id){
                info.numeric_id = 114;
                info.name = "Joker";
                info.name_normal = "joker";
                info.is_enemy = true;
            }
            else if ("el" == alphanumeric_id){
                info.numeric_id = 115;
                info.name = "Flapbeat";
                info.name_normal = "flapbeat";
                info.is_enemy = true;
            }
            else if ("em" == alphanumeric_id){
                info.numeric_id = 116;
                info.name = "Harpy";
                info.name_normal = "harpy";
                info.is_enemy = true;
            }
            else if ("en" == alphanumeric_id){
                info.numeric_id = 117;
                info.name = "Grand Horn";
                info.name_normal = "grand_horn";
                info.is_enemy = true;
            }
            else if ("eo" == alphanumeric_id){
                info.numeric_id = 118;
                info.name = "Gagighandi";
                info.name_normal = "gagighandi";
                info.is_enemy = true;
            }
            else if ("ep" == alphanumeric_id){
                info.numeric_id = 119;
                info.name = "Touch Me";
                info.name_normal = "touch_me";
                info.is_enemy = true;
            }
            else if ("eq" == alphanumeric_id){
                info.numeric_id = 120;
                info.name = "Crown Lance";
                info.name_normal = "crown_lance";
                info.is_enemy = true;
            }
            else if ("er" == alphanumeric_id){
                info.numeric_id = 121;
                info.name = "Flower Prong (small)";
                info.name_normal = "flower_prong_small";
                info.is_enemy = true;
            }
            else if ("es" == alphanumeric_id){
                info.numeric_id = 122;
                info.name = "Flower Prong (medium)";
                info.name_normal = "flower_prong_medium";
                info.is_enemy = true;
            }
            else if ("et" == alphanumeric_id){
                info.numeric_id = 123;
                info.name = "Flower Prong (large)";
                info.name_normal = "flower_prong_large";
                info.is_enemy = true;
            }
            else if ("eu" == alphanumeric_id){
                info.numeric_id = 124;
                info.name = "Slaps";
                info.name_normal = "slaps";
                info.is_enemy = true;
            }
            else if ("ev" == alphanumeric_id){
                info.numeric_id = 125;
                info.name = "Kimara Bug";
                info.name_normal = "kimara_bug";
                info.is_enemy = true;
            }
            else if ("ew" == alphanumeric_id){
                info.numeric_id = 126;
                info.name = "Heavy Tank";
                info.name_normal = "heavy_tank";
                info.is_enemy = true;
            }
            else if ("ex" == alphanumeric_id){
                info.numeric_id = 127;
                info.name = "TURKS: Reno";
                info.name_normal = "turks_reno";
                info.is_enemy = true;
            }
            else if ("ey" == alphanumeric_id){
                info.numeric_id = 128;
                info.name = "TURKS: Rude";
                info.name_normal = "turks_rude";
                info.is_enemy = true;
            }
            else if ("ez" == alphanumeric_id){
                info.numeric_id = 129;
                info.name = "Skeeskee";
                info.name_normal = "skeeskee";
                info.is_enemy = true;
            }
            else if ("fa" == alphanumeric_id){
                info.numeric_id = 130;
                info.name = "Griffin";
                info.name_normal = "griffin";
                info.is_enemy = true;
            }
            else if ("fb" == alphanumeric_id){
                info.numeric_id = 131;
                info.name = "Golem";
                info.name_normal = "golem";
                info.is_enemy = true;
            }
            else if ("fc" == alphanumeric_id){
                info.numeric_id = 132;
                info.name = "Bagrisk";
                info.name_normal = "bagrisk";
                info.is_enemy = true;
            }
            else if ("fd" == alphanumeric_id){
                info.numeric_id = 133;
                info.name = "Desert Sahagin";
                info.name_normal = "desert_sahagin";
                info.is_enemy = true;
            }
            else if ("fe" == alphanumeric_id){
                info.numeric_id = 134;
                info.name = "Gi Spector";
                info.name_normal = "gi_spector";
                info.is_enemy = true;
            }
            else if ("ff" == alphanumeric_id){
                info.numeric_id = 135;
                info.name = "Sneaky Step";
                info.name_normal = "sneaky_step";
                info.is_enemy = true;
            }
            else if ("fg" == alphanumeric_id){
                info.numeric_id = 139;
                info.name = "Heg";
                info.name_normal = "heg";
                info.is_enemy = true;
            }
            else if ("fh" == alphanumeric_id){
                info.numeric_id = 139;
                info.name = "Stinger";
                info.name_normal = "stinger";
                info.is_enemy = true;
            }
            else if ("fi" == alphanumeric_id){
                info.numeric_id = 139;
                info.name = "Soul Fire";
                info.name_normal = "soul_fire";
                info.is_enemy = true;
            }
            else if ("fj" == alphanumeric_id){
                info.numeric_id = 139;
                info.name = "Gi Nattak";
                info.name_normal = "gi_nattak";
                info.is_enemy = true;
            }
            else if ("fk" == alphanumeric_id){
                info.numeric_id = 140;
                info.name = "Nibel Wolf";
                info.name_normal = "nibel_wolf";
                info.is_enemy = true;
            }
            else if ("fl" == alphanumeric_id){
                info.numeric_id = 141;
                info.name = "Velcher Task";
                info.name_normal = "velcher_task";
                info.is_enemy = true;
            }
            else if ("fm" == alphanumeric_id){
                info.numeric_id = 142;
                info.name = "Bahba Velamyu";
                info.name_normal = "bahba_velamyu";
                info.is_enemy = true;
            }
            else if ("fn" == alphanumeric_id){
                info.numeric_id = 143;
                info.name = "Valron";
                info.name_normal = "valron";
                info.is_enemy = true;
            }
            else if ("fo" == alphanumeric_id){
                info.numeric_id = 144;
                info.name = "Battery Cap";
                info.name_normal = "battery_cap";
                info.is_enemy = true;
            }
            else if ("fp" == alphanumeric_id){
                info.numeric_id = 145;
                info.name = "Mirage";
                info.name_normal = "mirage";
                info.is_enemy = true;
            }
            else if ("fq" == alphanumeric_id){
                info.numeric_id = 146;
                info.name = "Dorkey Face";
                info.name_normal = "dorkey_face";
                info.is_enemy = true;
            }
            else if ("fr" == alphanumeric_id){
                info.numeric_id = 147;
                info.name = "Jersey";
                info.name_normal = "jersey";
                info.is_enemy = true;
            }
            else if ("fs" == alphanumeric_id){
                info.numeric_id = 148;
                info.name = "Black Bat";
                info.name_normal = "black_bat";
                info.is_enemy = true;
            }
            else if ("ft" == alphanumeric_id){
                info.numeric_id = 149;
                info.name = "Ghirofelgo (w/o chain)";
                info.name_normal = "ghirofelgo_wo_chain";
                info.is_enemy = true;
            }
            else if ("fu" == alphanumeric_id){
                info.numeric_id = 150;
                info.name = "Ghirofelgo's Chain";
                info.name_normal = "ghirofelgos_chain";
                info.is_enemy = true;
            }
            else if ("fv" == alphanumeric_id){
                info.numeric_id = 151;
                info.name = "Ying";
                info.name_normal = "ying";
                info.is_enemy = true;
            }
            else if ("fw" == alphanumeric_id){
                info.numeric_id = 152;
                info.name = "Yang";
                info.name_normal = "yang";
                info.is_enemy = true;
            }
            else if ("fx" == alphanumeric_id){
                info.numeric_id = 153;
                info.name = "Ying/Yang (body)";
                info.name_normal = "yingyang_body";
                info.is_enemy = true;
            }
            else if ("fy" == alphanumeric_id){
                info.numeric_id = 154;
                info.name = "Lost Number";
                info.name_normal = "lost_number";
                info.is_enemy = true;
            }
            else if ("fz" == alphanumeric_id){
                info.numeric_id = 155;
                info.name = "Lost Number: Psychic";
                info.name_normal = "lost_number_psychic";
                info.is_enemy = true;
            }
            else if ("ga" == alphanumeric_id){
                info.numeric_id = 156;
                info.name = "Lost Number: Psysic ";
                info.name_normal = "lost_number_psysic_";
                info.is_enemy = true;
            }
            else if ("gb" == alphanumeric_id){
                info.numeric_id = 157;
                info.name = "Dragon";
                info.name_normal = "dragon";
                info.is_enemy = true;
            }
            else if ("gc" == alphanumeric_id){
                info.numeric_id = 158;
                info.name = "Sonic Speed";
                info.name_normal = "sonic_speed";
                info.is_enemy = true;
            }
            else if ("gd" == alphanumeric_id){
                info.numeric_id = 159;
                info.name = "Twin Brain";
                info.name_normal = "twin_brain";
                info.is_enemy = true;
            }
            else if ("ge" == alphanumeric_id){
                info.numeric_id = 160;
                info.name = "Zuu";
                info.name_normal = "zuu";
                info.is_enemy = true;
            }
            else if ("gf" == alphanumeric_id){
                info.numeric_id = 161;
                info.name = "Kyuvilduns";
                info.name_normal = "kyuvilduns";
                info.is_enemy = true;
            }
            else if ("gg" == alphanumeric_id){
                info.numeric_id = 162;
                info.name = "Screamer";
                info.name_normal = "screamer";
                info.is_enemy = true;
            }
            else if ("gh" == alphanumeric_id){
                info.numeric_id = 163;
                info.name = "Materia Keeper";
                info.name_normal = "materia_keeper";
                info.is_enemy = true;
            }
            else if ("gi" == alphanumeric_id){
                info.numeric_id = 164;
                info.name = "Palmer";
                info.name_normal = "palmer";
                info.is_enemy = true;
            }
            else if ("gj" == alphanumeric_id){
                info.numeric_id = 165;
                info.name = "Tiny Bronco";
                info.name_normal = "tiny_bronco";
                info.is_enemy = true;
            }
            else if ("gk" == alphanumeric_id){
                info.numeric_id = 166;
                info.name = "Shinra Truck";
                info.name_normal = "shinra_truck";
                info.is_enemy = true;
            }
            else if ("gl" == alphanumeric_id){
                info.numeric_id = 167;
                info.name = "Thunderbird";
                info.name_normal = "thunderbird";
                info.is_enemy = true;
            }
            else if ("gm" == alphanumeric_id){
                info.numeric_id = 168;
                info.name = "Razor Weed";
                info.name_normal = "razor_weed";
                info.is_enemy = true;
            }
            else if ("gn" == alphanumeric_id){
                info.numeric_id = 169;
                info.name = "Edgehead";
                info.name_normal = "edgehead";
                info.is_enemy = true;
            }
            else if ("go" == alphanumeric_id){
                info.numeric_id = 170;
                info.name = "Bizarre Bug";
                info.name_normal = "bizarre_bug";
                info.is_enemy = true;
            }
            else if ("gp" == alphanumeric_id){
                info.numeric_id = 171;
                info.name = "Tail Vault";
                info.name_normal = "tail_vault";
                info.is_enemy = true;
            }
            else if ("gq" == alphanumeric_id){
                info.numeric_id = 172;
                info.name = "Adamantaimai";
                info.name_normal = "adamantaimai";
                info.is_enemy = true;
            }
            else if ("gr" == alphanumeric_id){
                info.numeric_id = 173;
                info.name = "Attack Squad";
                info.name_normal = "attack_squad";
                info.is_enemy = true;
            }
            else if ("gs" == alphanumeric_id){
                info.numeric_id = 174;
                info.name = "Foulander";
                info.name_normal = "foulander";
                info.is_enemy = true;
            }
            else if ("gt" == alphanumeric_id){
                info.numeric_id = 175;
                info.name = "Garuda";
                info.name_normal = "garuda";
                info.is_enemy = true;
            }
            else if ("gu" == alphanumeric_id){
                info.numeric_id = 176;
                info.name = "Jayjujayme";
                info.name_normal = "jayjujayme";
                info.is_enemy = true;
            }
            else if ("gv" == alphanumeric_id){
                info.numeric_id = 177;
                info.name = "Rapps";
                info.name_normal = "rapps";
                info.is_enemy = true;
            }
            else if ("gw" == alphanumeric_id){
                info.numeric_id = 178;
                info.name = "Gorkii";
                info.name_normal = "gorkii";
                info.is_enemy = true;
            }
            else if ("gx" == alphanumeric_id){
                info.numeric_id = 179;
                info.name = "Shake";
                info.name_normal = "shake";
                info.is_enemy = true;
            }
            else if ("gy" == alphanumeric_id){
                info.numeric_id = 180;
                info.name = "Chekhov";
                info.name_normal = "chekhov";
                info.is_enemy = true;
            }
            else if ("gz" == alphanumeric_id){
                info.numeric_id = 181;
                info.name = "Staniv";
                info.name_normal = "staniv";
                info.is_enemy = true;
            }
            else if ("ha" == alphanumeric_id){
                info.numeric_id = 182;
                info.name = "Godo";
                info.name_normal = "godo";
                info.is_enemy = true;
            }
            else if ("hb" == alphanumeric_id){
                info.numeric_id = 183;
                info.name = "Toxic Frog";
                info.name_normal = "toxic_frog";
                info.is_enemy = true;
            }
            else if ("hc" == alphanumeric_id){
                info.numeric_id = 184;
                info.name = "Toxic Frog";
                info.name_normal = "toxic_frog";
                info.is_enemy = true;
            }
            else if ("hd" == alphanumeric_id){
                info.numeric_id = 185;
                info.name = "Under Lizard";
                info.name_normal = "under_lizard";
                info.is_enemy = true;
            }
            else if ("he" == alphanumeric_id){
                info.numeric_id = 186;
                info.name = "Kelzmelzer";
                info.name_normal = "kelzmelzer";
                info.is_enemy = true;
            }
            else if ("hf" == alphanumeric_id){
                info.numeric_id = 187;
                info.name = "Dual Horn";
                info.name_normal = "dual_horn";
                info.is_enemy = true;
            }
            else if ("hg" == alphanumeric_id){
                info.numeric_id = 188;
                info.name = "Tonadu";
                info.name_normal = "tonadu";
                info.is_enemy = true;
            }
            else if ("hh" == alphanumeric_id){
                info.numeric_id = 189;
                info.name = "Toxic Frog";
                info.name_normal = "toxic_frog";
                info.is_enemy = true;
            }
            else if ("hi" == alphanumeric_id){
                info.numeric_id = 190;
                info.name = "Jemnezmy";
                info.name_normal = "jemnezmy";
                info.is_enemy = true;
            }
            else if ("hj" == alphanumeric_id){
                info.numeric_id = 191;
                info.name = "Doorbull";
                info.name_normal = "doorbull";
                info.is_enemy = true;
            }
            else if ("hk" == alphanumeric_id){
                info.numeric_id = 192;
                info.name = "Ancient Dragon";
                info.name_normal = "ancient_dragon";
                info.is_enemy = true;
            }
            else if ("hl" == alphanumeric_id){
                info.numeric_id = 193;
                info.name = "Red Dragon";
                info.name_normal = "red_dragon";
                info.is_enemy = true;
            }
            else if ("hm" == alphanumeric_id){
                info.numeric_id = 194;
                info.name = "8 eye";
                info.name_normal = "8_eye";
                info.is_enemy = true;
            }
            else if ("hn" == alphanumeric_id){
                info.numeric_id = 195;
                info.name = "Demons Gate";
                info.name_normal = "demons_gate";
                info.is_enemy = true;
            }
            else if ("ho" == alphanumeric_id){
                info.numeric_id = 196;
                info.name = "Jenova LIFE";
                info.name_normal = "jenova_life";
                info.is_enemy = true;
            }
            else if ("hp" == alphanumeric_id){
                info.numeric_id = 197;
                info.name = "Vlakorados";
                info.name_normal = "vlakorados";
                info.is_enemy = true;
            }
            else if ("hq" == alphanumeric_id){
                info.numeric_id = 198;
                info.name = "Trickplay";
                info.name_normal = "trickplay";
                info.is_enemy = true;
            }
            else if ("hr" == alphanumeric_id){
                info.numeric_id = 199;
                info.name = "Trick Play's Attack";
                info.name_normal = "trick_plays_attack";
                info.is_enemy = true;
            }
            else if ("hs" == alphanumeric_id){
                info.numeric_id = 200;
                info.name = "Boundfat";
                info.name_normal = "boundfat";
                info.is_enemy = true;
            }
            else if ("ht" == alphanumeric_id){
                info.numeric_id = 201;
                info.name = "Malldancer";
                info.name_normal = "malldancer";
                info.is_enemy = true;
            }
            else if ("hu" == alphanumeric_id){
                info.numeric_id = 202;
                info.name = "Grimguard";
                info.name_normal = "grimguard";
                info.is_enemy = true;
            }
            else if ("hv" == alphanumeric_id){
                info.numeric_id = 203;
                info.name = "Hungry";
                info.name_normal = "hungry";
                info.is_enemy = true;
            }
            else if ("hw" == alphanumeric_id){
                info.numeric_id = 204;
                info.name = "Acrophies";
                info.name_normal = "acrophies";
                info.is_enemy = true;
            }
            else if ("hx" == alphanumeric_id){
                info.numeric_id = 205;
                info.name = "Ice Golem";
                info.name_normal = "ice_golem";
                info.is_enemy = true;
            }
            else if ("hy" == alphanumeric_id){
                info.numeric_id = 206;
                info.name = "Shred";
                info.name_normal = "shred";
                info.is_enemy = true;
            }
            else if ("hz" == alphanumeric_id){
                info.numeric_id = 207;
                info.name = "Lessaloploth";
                info.name_normal = "lessaloploth";
                info.is_enemy = true;
            }
            else if ("ia" == alphanumeric_id){
                info.numeric_id = 208;
                info.name = "Frozen Nail";
                info.name_normal = "frozen_nail";
                info.is_enemy = true;
            }
            else if ("ib" == alphanumeric_id){
                info.numeric_id = 209;
                info.name = "Jumping";
                info.name_normal = "jumping";
                info.is_enemy = true;
            }
            else if ("ic" == alphanumeric_id){
                info.numeric_id = 210;
                info.name = "Snow";
                info.name_normal = "snow";
                info.is_enemy = true;
            }
            else if ("id" == alphanumeric_id){
                info.numeric_id = 211;
                info.name = "Bandersnatch";
                info.name_normal = "bandersnatch";
                info.is_enemy = true;
            }
            else if ("ie" == alphanumeric_id){
                info.numeric_id = 212;
                info.name = "Magnade";
                info.name_normal = "magnade";
                info.is_enemy = true;
            }
            else if ("if" == alphanumeric_id){
                info.numeric_id = 213;
                info.name = "Magnade's Shield";
                info.name_normal = "magnades_shield";
                info.is_enemy = true;
            }
            else if ("ig" == alphanumeric_id){
                info.numeric_id = 214;
                info.name = "Magnade's Shield";
                info.name_normal = "magnades_shield";
                info.is_enemy = true;
            }
            else if ("ih" == alphanumeric_id){
                info.numeric_id = 215;
                info.name = "Malboro";
                info.name_normal = "malboro";
                info.is_enemy = true;
            }
            else if ("ii" == alphanumeric_id){
                info.numeric_id = 216;
                info.name = "Blue Dragon";
                info.name_normal = "blue_dragon";
                info.is_enemy = true;
            }
            else if ("ij" == alphanumeric_id){
                info.numeric_id = 217;
                info.name = "Icicle";
                info.name_normal = "icicle";
                info.is_enemy = true;
            }
            else if ("ik" == alphanumeric_id){
                info.numeric_id = 218;
                info.name = "Headbomber";
                info.name_normal = "headbomber";
                info.is_enemy = true;
            }
            else if ("il" == alphanumeric_id){
                info.numeric_id = 219;
                info.name = "Stilva";
                info.name_normal = "stilva";
                info.is_enemy = true;
            }
            else if ("im" == alphanumeric_id){
                info.numeric_id = 220;
                info.name = "Zolokalter";
                info.name_normal = "zolokalter";
                info.is_enemy = true;
            }
            else if ("in" == alphanumeric_id){
                info.numeric_id = 221;
                info.name = "Evilhead";
                info.name_normal = "evilhead";
                info.is_enemy = true;
            }
            else if ("io" == alphanumeric_id){
                info.numeric_id = 222;
                info.name = "Cuahl";
                info.name_normal = "cuahl";
                info.is_enemy = true;
            }
            else if ("ip" == alphanumeric_id){
                info.numeric_id = 223;
                info.name = "Gigas";
                info.name_normal = "gigas";
                info.is_enemy = true;
            }
            else if ("iq" == alphanumeric_id){
                info.numeric_id = 224;
                info.name = "Grenade";
                info.name_normal = "grenade";
                info.is_enemy = true;
            }
            else if ("ir" == alphanumeric_id){
                info.numeric_id = 225;
                info.name = "Gremlin";
                info.name_normal = "gremlin";
                info.is_enemy = true;
            }
            else if ("is" == alphanumeric_id){
                info.numeric_id = 226;
                info.name = "Ironite";
                info.name_normal = "ironite";
                info.is_enemy = true;
            }
            else if ("it" == alphanumeric_id){
                info.numeric_id = 227;
                info.name = "Sculpture";
                info.name_normal = "sculpture";
                info.is_enemy = true;
            }
            else if ("iu" == alphanumeric_id){
                info.numeric_id = 228;
                info.name = "SCHIZO";
                info.name_normal = "schizo";
                info.is_enemy = true;
            }
            else if ("iv" == alphanumeric_id){
                info.numeric_id = 229;
                info.name = "SCHIZO's head";
                info.name_normal = "schizos_head";
                info.is_enemy = true;
            }
            else if ("iw" == alphanumeric_id){
                info.numeric_id = 230;
                info.name = "Wind Wing";
                info.name_normal = "wind_wing";
                info.is_enemy = true;
            }
            else if ("ix" == alphanumeric_id){
                info.numeric_id = 231;
                info.name = "Dragon Rider";
                info.name_normal = "dragon_rider";
                info.is_enemy = true;
            }
            else if ("iy" == alphanumeric_id){
                info.numeric_id = 232;
                info.name = "Killbin";
                info.name_normal = "killbin";
                info.is_enemy = true;
            }
            else if ("iz" == alphanumeric_id){
                info.numeric_id = 233;
                info.name = "Tonberry";
                info.name_normal = "tonberry";
                info.is_enemy = true;
            }
            else if ("ja" == alphanumeric_id){
                info.numeric_id = 234;
                info.name = "Jenova DEATH";
                info.name_normal = "jenova_death";
                info.is_enemy = true;
            }
            else if ("jb" == alphanumeric_id){
                info.numeric_id = 235;
                info.name = "Roulette Cannon";
                info.name_normal = "roulette_cannon";
                info.is_enemy = true;
            }
            else if ("jc" == alphanumeric_id){
                info.numeric_id = 236;
                info.name = "Pedestal";
                info.name_normal = "pedestal";
                info.is_enemy = true;
            }
            else if ("jd" == alphanumeric_id){
                info.numeric_id = 237;
                info.name = "SOLDIER:2nd";
                info.name_normal = "soldier2nd";
                info.is_enemy = true;
            }
            else if ("je" == alphanumeric_id){
                info.numeric_id = 238;
                info.name = "Death Machine";
                info.name_normal = "death_machine";
                info.is_enemy = true;
            }
            else if ("jf" == alphanumeric_id){
                info.numeric_id = 239;
                info.name = "Slalom";
                info.name_normal = "slalom";
                info.is_enemy = true;
            }
            else if ("jg" == alphanumeric_id){
                info.numeric_id = 240;
                info.name = "Scissors";
                info.name_normal = "scissors";
                info.is_enemy = true;
            }
            else if ("jh" == alphanumeric_id){
                info.numeric_id = 241;
                info.name = "Scissors(Upper)";
                info.name_normal = "scissors_upper";
                info.is_enemy = true;
            }
            else if ("ji" == alphanumeric_id){
                info.numeric_id = 242;
                info.name = "Scissors(Lower)";
                info.name_normal = "scissors_lower";
                info.is_enemy = true;
            }
            else if ("jj" == alphanumeric_id){
                info.numeric_id = 243;
                info.name = "Guard System";
                info.name_normal = "guard_system";
                info.is_enemy = true;
            }
            else if ("jk" == alphanumeric_id){
                info.numeric_id = 244;
                info.name = "Quick Machine Gun";
                info.name_normal = "quick_machine_gun";
                info.is_enemy = true;
            }
            else if ("jl" == alphanumeric_id){
                info.numeric_id = 245;
                info.name = "Rocket Launcher";
                info.name_normal = "rocket_launcher";
                info.is_enemy = true;
            }
            else if ("jm" == alphanumeric_id){
                info.numeric_id = 246;
                info.name = "Ghost Ship";
                info.name_normal = "ghost_ship";
                info.is_enemy = true;
            }
            else if ("jn" == alphanumeric_id){
                info.numeric_id = 247;
                info.name = "Corvette";
                info.name_normal = "corvette";
                info.is_enemy = true;
            }
            else if ("jo" == alphanumeric_id){
                info.numeric_id = 248;
                info.name = "Diver Nest";
                info.name_normal = "diver_nest";
                info.is_enemy = true;
            }
            else if ("jp" == alphanumeric_id){
                info.numeric_id = 249;
                info.name = "Submarine Crew";
                info.name_normal = "submarine_crew";
                info.is_enemy = true;
            }
            else if ("jq" == alphanumeric_id){
                info.numeric_id = 250;
                info.name = "Captain";
                info.name_normal = "captain";
                info.is_enemy = true;
            }
            else if ("jr" == alphanumeric_id){
                info.numeric_id = 251;
                info.name = "Underwater MP";
                info.name_normal = "underwater_mp";
                info.is_enemy = true;
            }
            else if ("js" == alphanumeric_id){
                info.numeric_id = 252;
                info.name = "Senior Grunt";
                info.name_normal = "senior_grunt";
                info.is_enemy = true;
            }
            else if ("jt" == alphanumeric_id){
                info.numeric_id = 253;
                info.name = "Hard Attacker";
                info.name_normal = "hard_attacker";
                info.is_enemy = true;
            }
            else if ("ju" == alphanumeric_id){
                info.numeric_id = 254;
                info.name = "Guardian";
                info.name_normal = "guardian";
                info.is_enemy = true;
            }
            else if ("jv" == alphanumeric_id){
                info.numeric_id = 255;
                info.name = "Guardian's Hand (Right)";
                info.name_normal = "guardians_hand_right";
                info.is_enemy = true;
            }
            else if ("jw" == alphanumeric_id){
                info.numeric_id = 256;
                info.name = "Guardian's Hand (Left)";
                info.name_normal = "guardians_hand_left";
                info.is_enemy = true;
            }
            else if ("jx" == alphanumeric_id){
                info.numeric_id = 257;
                info.name = "Gun Carrier";
                info.name_normal = "gun_carrier";
                info.is_enemy = true;
            }
            else if ("jy" == alphanumeric_id){
                info.numeric_id = 258;
                info.name = "Carry Armor";
                info.name_normal = "carry_armor";
                info.is_enemy = true;
            }
            else if ("jz" == alphanumeric_id){
                info.numeric_id = 259;
                info.name = "Carry's Left Arm";
                info.name_normal = "carrys_left_arm";
                info.is_enemy = true;
            }
            else if ("ka" == alphanumeric_id){
                info.numeric_id = 260;
                info.name = "Carry's Right Arm";
                info.name_normal = "carrys_right_arm";
                info.is_enemy = true;
            }
            else if ("kb" == alphanumeric_id){
                info.numeric_id = 261;
                info.name = "Rilfsak";
                info.name_normal = "rilfsak";
                info.is_enemy = true;
            }
            else if ("kc" == alphanumeric_id){
                info.numeric_id = 262;
                info.name = "Diablo";
                info.name_normal = "diablo";
                info.is_enemy = true;
            }
            else if ("kd" == alphanumeric_id){
                info.numeric_id = 263;
                info.name = "Epiolnis";
                info.name_normal = "epiolnis";
                info.is_enemy = true;
            }
            else if ("ke" == alphanumeric_id){
                info.numeric_id = 264;
                info.name = "Ho-chu";
                info.name_normal = "hochu";
                info.is_enemy = true;
            }
            else if ("kf" == alphanumeric_id){
                info.numeric_id = 265;
                info.name = "Gas Ducter";
                info.name_normal = "gas_ducter";
                info.is_enemy = true;
            }
            else if ("kg" == alphanumeric_id){
                info.numeric_id = 266;
                info.name = "Wolfmeister";
                info.name_normal = "wolfmeister";
                info.is_enemy = true;
            }
            else if ("kh" == alphanumeric_id){
                info.numeric_id = 267;
                info.name = "Eagle Gun";
                info.name_normal = "eagle_gun";
                info.is_enemy = true;
            }
            else if ("ki" == alphanumeric_id){
                info.numeric_id = 268;
                info.name = "Serpent";
                info.name_normal = "serpent";
                info.is_enemy = true;
            }
            else if ("kj" == alphanumeric_id){
                info.numeric_id = 269;
                info.name = "Poodler";
                info.name_normal = "poodler";
                info.is_enemy = true;
            }
            else if ("kk" == alphanumeric_id){
                info.numeric_id = 270;
                info.name = "Bad Rap";
                info.name_normal = "bad_rap";
                info.is_enemy = true;
            }
            else if ("kl" == alphanumeric_id){
                info.numeric_id = 271;
                info.name = "Unknown (tongue)";
                info.name_normal = "unknown_tongue";
                info.is_enemy = true;
            }
            else if ("km" == alphanumeric_id){
                info.numeric_id = 272;
                info.name = "Unknown3 (creep)";
                info.name_normal = "unknown3_creep";
                info.is_enemy = true;
            }
            else if ("kn" == alphanumeric_id){
                info.numeric_id = 273;
                info.name = "Unknown2 (needle)";
                info.name_normal = "unknown2_needle";
                info.is_enemy = true;
            }
            else if ("ko" == alphanumeric_id){
                info.numeric_id = 274;
                info.name = "TURKS: Reno";
                info.name_normal = "turks_reno";
                info.is_enemy = true;
            }
            else if ("kp" == alphanumeric_id){
                info.numeric_id = 275;
                info.name = "TURKS: Rude";
                info.name_normal = "turks_rude";
                info.is_enemy = true;
            }
            else if ("kq" == alphanumeric_id){
                info.numeric_id = 276;
                info.name = "Hippogriff";
                info.name_normal = "hippogriff";
                info.is_enemy = true;
            }
            else if ("kr" == alphanumeric_id){
                info.numeric_id = 277;
                info.name = "Head Hunter";
                info.name_normal = "head_hunter";
                info.is_enemy = true;
            }
            else if ("ks" == alphanumeric_id){
                info.numeric_id = 278;
                info.name = "Spiral";
                info.name_normal = "spiral";
                info.is_enemy = true;
            }
            else if ("kt" == alphanumeric_id){
                info.numeric_id = 279;
                info.name = "Crysales";
                info.name_normal = "crysales";
                info.is_enemy = true;
            }
            else if ("ku" == alphanumeric_id){
                info.numeric_id = 280;
                info.name = "Sea Worm";
                info.name_normal = "sea_worm";
                info.is_enemy = true;
            }
            else if ("kv" == alphanumeric_id){
                info.numeric_id = 281;
                info.name = "TURKS: Rude";
                info.name_normal = "turks_rude";
                info.is_enemy = true;
            }
            else if ("kw" == alphanumeric_id){
                info.numeric_id = 282;
                info.name = "CMD.Grand Horn";
                info.name_normal = "cmd_grand_horn";
                info.is_enemy = true;
            }
            else if ("kx" == alphanumeric_id){
                info.numeric_id = 283;
                info.name = "CMD.Grand Horn";
                info.name_normal = "cmd_grand_horn";
                info.is_enemy = true;
            }
            else if ("ky" == alphanumeric_id){
                info.numeric_id = 284;
                info.name = "CMD.Grand Horn";
                info.name_normal = "cmd_grand_horn";
                info.is_enemy = true;
            }
            else if ("kz" == alphanumeric_id){
                info.numeric_id = 285;
                info.name = "Behemoth";
                info.name_normal = "behemoth";
                info.is_enemy = true;
            }
            else if ("la" == alphanumeric_id){
                info.numeric_id = 286;
                info.name = "Cromwell";
                info.name_normal = "cromwell";
                info.is_enemy = true;
            }
            else if ("lb" == alphanumeric_id){
                info.numeric_id = 287;
                info.name = "Manhole";
                info.name_normal = "manhole";
                info.is_enemy = true;
            }
            else if ("lc" == alphanumeric_id){
                info.numeric_id = 288;
                info.name = "Manhole (Lid)";
                info.name_normal = "manhole_lid";
                info.is_enemy = true;
            }
            else if ("ld" == alphanumeric_id){
                info.numeric_id = 289;
                info.name = "Crazy Saw";
                info.name_normal = "crazy_saw";
                info.is_enemy = true;
            }
            else if ("le" == alphanumeric_id){
                info.numeric_id = 290;
                info.name = "Shadow Maker";
                info.name_normal = "shadow_maker";
                info.is_enemy = true;
            }
            else if ("lf" == alphanumeric_id){
                info.numeric_id = 291;
                info.name = "Grosspanzer Big";
                info.name_normal = "grosspanzer_big";
                info.is_enemy = true;
            }
            else if ("lg" == alphanumeric_id){
                info.numeric_id = 292;
                info.name = "Grosspanzer Small";
                info.name_normal = "grosspanzer_small";
                info.is_enemy = true;
            }
            else if ("lh" == alphanumeric_id){
                info.numeric_id = 293;
                info.name = "Grosspanzer Mobile";
                info.name_normal = "grosspanzer_mobile";
                info.is_enemy = true;
            }
            else if ("li" == alphanumeric_id){
                info.numeric_id = 294;
                info.name = "Gargoyle stoned";
                info.name_normal = "gargoyle_stoned";
                info.is_enemy = true;
            }
            else if ("lj" == alphanumeric_id){
                info.numeric_id = 295;
                info.name = "Gargoyle";
                info.name_normal = "gargoyle";
                info.is_enemy = true;
            }
            else if ("lk" == alphanumeric_id){
                info.numeric_id = 296;
                info.name = "TURKS: Elena";
                info.name_normal = "turks_elena";
                info.is_enemy = true;
            }
            else if ("ll" == alphanumeric_id){
                info.numeric_id = 297;
                info.name = "TURKS: Reno";
                info.name_normal = "turks_reno";
                info.is_enemy = true;
            }
            else if ("lm" == alphanumeric_id){
                info.numeric_id = 298;
                info.name = "TURKS: Rude";
                info.name_normal = "turks_rude";
                info.is_enemy = true;
            }
            else if ("ln" == alphanumeric_id){
                info.numeric_id = 299;
                info.name = "Proud Clod";
                info.name_normal = "proud_clod";
                info.is_enemy = true;
            }
            else if ("lo" == alphanumeric_id){
                info.numeric_id = 300;
                info.name = "Jamar Armor";
                info.name_normal = "jamar_armor";
                info.is_enemy = true;
            }
            else if ("lp" == alphanumeric_id){
                info.numeric_id = 301;
                info.name = "SOLDIER 1st";
                info.name_normal = "soldier_1st";
                info.is_enemy = true;
            }
            else if ("lq" == alphanumeric_id){
                info.numeric_id = 302;
                info.name = "X-Cannon";
                info.name_normal = "xcannon";
                info.is_enemy = true;
            }
            else if ("lr" == alphanumeric_id){
                info.numeric_id = 303;
                info.name = "bubble";
                info.name_normal = "bubble";
                info.is_enemy = true;
            }
            else if ("ls" == alphanumeric_id){
                info.numeric_id = 304;
                info.name = "Maximum Kimaira";
                info.name_normal = "maximum_kimaira";
                info.is_enemy = true;
            }
            else if ("lt" == alphanumeric_id){
                info.numeric_id = 305;
                info.name = "HOJO";
                info.name_normal = "hojo";
                info.is_enemy = true;
            }
            else if ("lu" == alphanumeric_id){
                info.numeric_id = 306;
                info.name = "HELETIC HOJO";
                info.name_normal = "heletic_hojo";
                info.is_enemy = true;
            }
            else if ("lv" == alphanumeric_id){
                info.numeric_id = 307;
                info.name = "Hojo's Left Arm";
                info.name_normal = "hojos_left_arm";
                info.is_enemy = true;
            }
            else if ("lw" == alphanumeric_id){
                info.numeric_id = 308;
                info.name = "Hojo's Right Arm";
                info.name_normal = "hojos_right_arm";
                info.is_enemy = true;
            }
            else if ("lx" == alphanumeric_id){
                info.numeric_id = 309;
                info.name = "LIFEFORM HOJO";
                info.name_normal = "lifeform_hojo";
                info.is_enemy = true;
            }
            else if ("ly" == alphanumeric_id){
                info.numeric_id = 310;
                info.name = "Magic Pot";
                info.name_normal = "magic_pot";
                info.is_enemy = true;
            }
            else if ("lz" == alphanumeric_id){
                info.numeric_id = 311;
                info.name = "Christopher";
                info.name_normal = "christopher";
                info.is_enemy = true;
            }
            else if ("ma" == alphanumeric_id){
                info.numeric_id = 312;
                info.name = "Gighee";
                info.name_normal = "gighee";
                info.is_enemy = true;
            }
            else if ("mb" == alphanumeric_id){
                info.numeric_id = 313;
                info.name = "King Behemoth";
                info.name_normal = "king_behemoth";
                info.is_enemy = true;
            }
            else if ("mc" == alphanumeric_id){
                info.numeric_id = 314;
                info.name = "Allemagne";
                info.name_normal = "allemagne";
                info.is_enemy = true;
            }
            else if ("md" == alphanumeric_id){
                info.numeric_id = 315;
                info.name = "Dragon Zombie";
                info.name_normal = "dragon_zombie";
                info.is_enemy = true;
            }
            else if ("me" == alphanumeric_id){
                info.numeric_id = 316;
                info.name = "Armored Golem";
                info.name_normal = "armored_golem";
                info.is_enemy = true;
            }
            else if ("mf" == alphanumeric_id){
                info.numeric_id = 317;
                info.name = "Master Tonberry";
                info.name_normal = "master_tonberry";
                info.is_enemy = true;
            }
            else if ("mg" == alphanumeric_id){
                info.numeric_id = 318;
                info.name = "Pollensalta";
                info.name_normal = "pollensalta";
                info.is_enemy = true;
            }
            else if ("mh" == alphanumeric_id){
                info.numeric_id = 319;
                info.name = "Mover";
                info.name_normal = "mover";
                info.is_enemy = true;
            }
            else if ("mi" == alphanumeric_id){
                info.numeric_id = 320;
                info.name = "Iron Man";
                info.name_normal = "iron_man";
                info.is_enemy = true;
            }
            else if ("mj" == alphanumeric_id){
                info.numeric_id = 321;
                info.name = "Parasite";
                info.name_normal = "parasite";
                info.is_enemy = true;
            }
            else if ("mk" == alphanumeric_id){
                info.numeric_id = 322;
                info.name = "Dark Dragon";
                info.name_normal = "dark_dragon";
                info.is_enemy = true;
            }
            else if ("ml" == alphanumeric_id){
                info.numeric_id = 323;
                info.name = "Death Dealer";
                info.name_normal = "death_dealer";
                info.is_enemy = true;
            }
            else if ("mm" == alphanumeric_id){
                info.numeric_id = 324;
                info.name = "JENOVA SYNTHESIS";
                info.name_normal = "jenova_synthesis";
                info.is_enemy = true;
            }
            else if ("mn" == alphanumeric_id){
                info.numeric_id = 325;
                info.name = "Bizarro Sephiroth";
                info.name_normal = "bizarro_sephiroth";
                info.is_enemy = true;
            }
            else if ("mo" == alphanumeric_id){
                info.numeric_id = 326;
                info.name = "Bizarro Sephiroth";
                info.name_normal = "bizarro_sephiroth";
                info.is_enemy = true;
            }
            else if ("mp" == alphanumeric_id){
                info.numeric_id = 327;
                info.name = "Bizarro Sephiroth";
                info.name_normal = "bizarro_sephiroth";
                info.is_enemy = true;
            }
            else if ("mq" == alphanumeric_id){
                info.numeric_id = 328;
                info.name = "Bizarro Sephiroth";
                info.name_normal = "bizarro_sephiroth";
                info.is_enemy = true;
            }
            else if ("mr" == alphanumeric_id){
                info.numeric_id = 329;
                info.name = "Bizarro Sephiroth";
                info.name_normal = "bizarro_sephiroth";
                info.is_enemy = true;
            }
            else if ("ms" == alphanumeric_id){
                info.numeric_id = 330;
                info.name = "Bizarro Sephiroth";
                info.name_normal = "bizarro_sephiroth";
                info.is_enemy = true;
            }
            else if ("mt" == alphanumeric_id){
                info.numeric_id = 331;
                info.name = "Safer Sephiroth";
                info.name_normal = "safer_sephiroth";
                info.is_enemy = true;
            }
            else if ("mu" == alphanumeric_id){
                info.numeric_id = 332;
                info.name = "Sephiroth";
                info.name_normal = "sephiroth";
                info.is_enemy = true;
            }
            else if ("mv" == alphanumeric_id){
                info.numeric_id = 333;
                info.name = "ULTIMA WEAPON";
                info.name_normal = "ultima_weapon";
                info.is_enemy = true;
            }
            else if ("mw" == alphanumeric_id){
                info.numeric_id = 334;
                info.name = "Ultima Weapon";
                info.name_normal = "ultima_weapon";
                info.is_enemy = true;
            }
            else if ("mx" == alphanumeric_id){
                info.numeric_id = 335;
                info.name = "Ultima Weapon (aerial fights)";
                info.name_normal = "ultima_weapon_aerial_fights";
                info.is_enemy = true;
            }
            else if ("my" == alphanumeric_id){
                info.numeric_id = 336;
                info.name = "Ultima Weapon (aerial fights)";
                info.name_normal = "ultima_weapon_aerial_fights";
                info.is_enemy = true;
            }
            else if ("mz" == alphanumeric_id){
                info.numeric_id = 337;
                info.name = "Cactuar";
                info.name_normal = "cactuar";
                info.is_enemy = true;
            }
            else if ("na" == alphanumeric_id){
                info.numeric_id = 338;
                info.name = "Goblin";
                info.name_normal = "goblin";
                info.is_enemy = true;
            }
            else if ("nb" == alphanumeric_id){
                info.numeric_id = 339;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("nc" == alphanumeric_id){
                info.numeric_id = 340;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("nd" == alphanumeric_id){
                info.numeric_id = 341;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("ne" == alphanumeric_id){
                info.numeric_id = 342;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("nf" == alphanumeric_id){
                info.numeric_id = 343;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("ng" == alphanumeric_id){
                info.numeric_id = 344;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("nh" == alphanumeric_id){
                info.numeric_id = 345;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("ni" == alphanumeric_id){
                info.numeric_id = 346;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("nj" == alphanumeric_id){
                info.numeric_id = 347;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("nk" == alphanumeric_id){
                info.numeric_id = 348;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("nl" == alphanumeric_id){
                info.numeric_id = 349;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("nm" == alphanumeric_id){
                info.numeric_id = 350;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("nn" == alphanumeric_id){
                info.numeric_id = 351;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("no" == alphanumeric_id){
                info.numeric_id = 352;
                info.name = "Chocobo";
                info.name_normal = "chocobo";
                info.is_enemy = true;
            }
            else if ("np" == alphanumeric_id){
                info.numeric_id = 353;
                info.name = "Mystery Ninja(yuf)";
                info.name_normal = "mystery_ninjayuf";
                info.is_enemy = true;
            }
            else if ("nq" == alphanumeric_id){
                info.numeric_id = 354;
                info.name = "Yuffie (as Enemy)";
                info.name_normal = "yuffie_as_enemy";
                info.is_enemy = true;
            }
            else if ("nr" == alphanumeric_id){
                info.numeric_id = 355;
                info.name = "Yuffie (as Enemy)";
                info.name_normal = "yuffie_as_enemy";
                info.is_enemy = true;
            }
            else if ("ns" == alphanumeric_id){
                info.numeric_id = 356;
                info.name = "Yuffie (as Enemy)";
                info.name_normal = "yuffie_as_enemy";
                info.is_enemy = true;
            }
            else if ("nt" == alphanumeric_id){
                info.numeric_id = 357;
                info.name = "Yuffie (as Enemy)";
                info.name_normal = "yuffie_as_enemy";
                info.is_enemy = true;
            }
            else if ("nu" == alphanumeric_id){
                info.numeric_id = 358;
                info.name = "Yuffie (as Enemy)";
                info.name_normal = "yuffie_as_enemy";
                info.is_enemy = true;
            }
            else if ("nv" == alphanumeric_id){
                info.numeric_id = 359;
                info.name = "Corneo's Lacky";
                info.name_normal = "corneos_lacky";
                info.is_enemy = true;
            }
            else if ("nw" == alphanumeric_id){
                info.numeric_id = 360;
                info.name = "Corneo's Lacky";
                info.name_normal = "corneos_lacky";
                info.is_enemy = true;
            }
            else if ("nx" == alphanumeric_id){
                info.numeric_id = 361;
                info.name = "Corneo's Lacky";
                info.name_normal = "corneos_lacky";
                info.is_enemy = true;
            }
            else if ("ny" == alphanumeric_id){
                info.numeric_id = 362;
                info.name = "Bad Rap Sample";
                info.name_normal = "bad_rap_sample";
                info.is_enemy = true;
            }
            else if ("nz" == alphanumeric_id){
                info.numeric_id = 363;
                info.name = "Poodler Sample";
                info.name_normal = "poodler_sample";
                info.is_enemy = true;
            }
            else if ("oa" == alphanumeric_id){
                info.numeric_id = 364;
                info.name = "Cactuer";
                info.name_normal = "cactuer";
                info.is_enemy = true;
            }
            else if ("ob" == alphanumeric_id){
                info.numeric_id = 365;
                info.name = "Shinra Trooper";
                info.name_normal = "shinra_trooper";
                info.is_enemy = true;
            }
            else if ("oc" == alphanumeric_id){
                info.numeric_id = 366;
                info.name = "Shinra Trooper";
                info.name_normal = "shinra_trooper";
                info.is_enemy = true;
            }
            else if ("od" == alphanumeric_id){
                info.numeric_id = 367;
                info.name = "Shinra Trooper";
                info.name_normal = "shinra_trooper";
                info.is_enemy = true;
            }
            else if ("oe" == alphanumeric_id){
                info.numeric_id = 368;
                info.name = "Shinra Trooper";
                info.name_normal = "shinra_trooper";
                info.is_enemy = true;
            }
            else if ("of" == alphanumeric_id){
                info.numeric_id = 369;
                info.name = "Shinra Trooper";
                info.name_normal = "shinra_trooper";
                info.is_enemy = true;
            }
            else if("og" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Blank";
                info.name_normal = "blank";
                info.is_scene = true;
            }
            else if("oh" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Bizarro Sephiroth Battle - Center";
                info.name_normal = "bizarro sephiroth battle center";
                info.is_scene = true;
            }
            else if("oi" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Grassland";
                info.name_normal = "grassland";
                info.is_scene = true;
            }
            else if("oj" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Mt. Nibel";
                info.name_normal = "mt nibel";
                info.is_scene = true;
            }
            else if("ok" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Forest";
                info.name_normal = "forest";
                info.is_scene = true;
            }
            else if("ol" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Beach";
                info.name_normal = "beach";
                info.is_scene = true;
            }
            else if("om" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Desert";
                info.name_normal = "desert";
                info.is_scene = true;
            }
            else if("on" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Snow";
                info.name_normal = "snow";
                info.is_scene = true;
            }
            else if("oo" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Swamp";
                info.name_normal = "swamp";
                info.is_scene = true;
            }
            else if("op" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Sector 1 Train Station";
                info.name_normal = "sector 1 train station";
                info.is_scene = true;
            }
            else if("oq" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Reactor 1";
                info.name_normal = "reactor 1";
                info.is_scene = true;
            }
            else if("or" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Reactor 1 Core";
                info.name_normal = "reactor 1 core";
                info.is_scene = true;
            }
            else if("os" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Reactor 1 Entrance";
                info.name_normal = "reactor 1 entrance";
                info.is_scene = true;
            }
            else if("ot" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Sector 4 Subway";
                info.name_normal = "sector 4 subway";
                info.is_scene = true;
            }
            else if("ou" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Mythril Caves or Nibel Caves or AForest Caves";
                info.name_normal = "mythril caves or nibel caves or aforest caves";
                info.is_scene = true;
            }
            else if("ov" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Shinra HQ";
                info.name_normal = "shinra hq";
                info.is_scene = true;
            }
            else if("ow" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Midgar Raid Subway";
                info.name_normal = "midgar raid subway";
                info.is_scene = true;
            }
            else if("ox" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Hojo's Lab Battle Field";
                info.name_normal = "hojos lab battle field";
                info.is_scene = true;
            }
            else if("oy" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Shinra Elevators";
                info.name_normal = "shinra elevators";
                info.is_scene = true;
            }
            else if("oz" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Shinra Roof";
                info.name_normal = "shinra roof";
                info.is_scene = true;
            }
            else if("pa" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Midgar Highway";
                info.name_normal = "midgar highway";
                info.is_scene = true;
            }
            else if("pb" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Wutai Pagoda";
                info.name_normal = "wutai pagoda";
                info.is_scene = true;
            }
            else if("pc" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Church Battle Field";
                info.name_normal = "church battle field";
                info.is_scene = true;
            }
            else if("pd" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Coral Valley";
                info.name_normal = "coral valley";
                info.is_scene = true;
            }
            else if("pe" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Midgar Slums";
                info.name_normal = "midgar slums";
                info.is_scene = true;
            }
            else if("pf" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Sector 4 Corridors or Junon Path";
                info.name_normal = "sector 4 corridors or junon path";
                info.is_scene = true;
            }
            else if("pg" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Sector 4 Gantries or Midgar Underground";
                info.name_normal = "sector 4 gantries or midgar underground";
                info.is_scene = true;
            }
            else if("ph" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Sector 7 Support Pillar Stairway";
                info.name_normal = "sector 7 support pillar stairway";
                info.is_scene = true;
            }
            else if("pi" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Sector 7 Support Pillar Top";
                info.name_normal = "sector 7 support pillar top";
                info.is_scene = true;
            }
            else if("pj" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Sector 8";
                info.name_normal = "sector 8";
                info.is_scene = true;
            }
            else if("pk" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Sewers";
                info.name_normal = "sewers";
                info.is_scene = true;
            }
            else if("pl" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Mythril Mines";
                info.name_normal = "mythril mines";
                info.is_scene = true;
            }
            else if("pm" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Northern Crater - Floating Platforms";
                info.name_normal = "northern crater floating platforms";
                info.is_scene = true;
            }
            else if("pn" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Corel Mountain Path";
                info.name_normal = "corel mountain path";
                info.is_scene = true;
            }
            else if("po" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Junon Beach";
                info.name_normal = "junon beach";
                info.is_scene = true;
            }
            else if("pp" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Junon Cargo Ship";
                info.name_normal = "junon cargo ship";
                info.is_scene = true;
            }
            else if("pq" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Corel Prison";
                info.name_normal = "corel prison";
                info.is_scene = true;
            }
            else if("pr" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Battle Square";
                info.name_normal = "battle square";
                info.is_scene = true;
            }
            else if("ps" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Da Chao - Rapps Battle";
                info.name_normal = "da chao rapps battle";
                info.is_scene = true;
            }
            else if("pt" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Cid's Backyard";
                info.name_normal = "cids backyard";
                info.is_scene = true;
            }
            else if("pu" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Final Descent to Sephiroth";
                info.name_normal = "final descent to sephiroth";
                info.is_scene = true;
            }
            else if("pv" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Reactor 5 Entrance";
                info.name_normal = "reactor 5 entrance";
                info.is_scene = true;
            }
            else if("pw" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Temple of the Ancients - Escher Room";
                info.name_normal = "temple of the ancients escher room";
                info.is_scene = true;
            }
            else if("px" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Shinra Mansion";
                info.name_normal = "shinra mansion";
                info.is_scene = true;
            }
            else if("py" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Junon Airship Dock";
                info.name_normal = "junon airship dock";
                info.is_scene = true;
            }
            else if("pz" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Whirlwind Maze";
                info.name_normal = "whirlwind maze";
                info.is_scene = true;
            }
            else if("qa" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Junon Underwater Reactor";
                info.name_normal = "junon underwater reactor";
                info.is_scene = true;
            }
            else if("qb" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Gongaga Reactor";
                info.name_normal = "gongaga reactor";
                info.is_scene = true;
            }
            else if("qc" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Gelnika";
                info.name_normal = "gelnika";
                info.is_scene = true;
            }
            else if("qd" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Train Graveyard";
                info.name_normal = "train graveyard";
                info.is_scene = true;
            }
            else if("qe" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Great Glacier Ice Caves & Gaea Cliffs - Inside";
                info.name_normal = "great glacier ice caves and gaea cliffs inside";
                info.is_scene = true;
            }
            else if("qf" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Sister Ray";
                info.name_normal = "sister ray";
                info.is_scene = true;
            }
            else if("qg" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Sister Ray Base";
                info.name_normal = "sister ray base";
                info.is_scene = true;
            }
            else if("qh" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Forgotten City Altar";
                info.name_normal = "forgotten city altar";
                info.is_scene = true;
            }
            else if("qi" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Northern Crater - Initial Descent";
                info.name_normal = "northern crater initial descent";
                info.is_scene = true;
            }
            else if("qj" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Northern Crater - Hatchery";
                info.name_normal = "northern crater hatchery";
                info.is_scene = true;
            }
            else if("qk" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Northern Crater - Water Area";
                info.name_normal = "northern crater water area";
                info.is_scene = true;
            }
            else if("ql" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Safer Battle";
                info.name_normal = "safer battle";
                info.is_scene = true;
            }
            else if("qm" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Kalm Flashback - Dragon Battle";
                info.name_normal = "kalm flashback dragon battle";
                info.is_scene = true;
            }
            else if("qn" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Junon Underwater Pipe";
                info.name_normal = "junon underwater pipe";
                info.is_scene = true;
            }
            else if("qo" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Blank";
                info.name_normal = "blank";
                info.is_scene = true;
            }
            else if("qp" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "qp";
                info.name_normal = "qp";
                info.is_scene = true;
            }
            else if("qq" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Corel Railway - Canyon";
                info.name_normal = "corel railway canyon";
                info.is_scene = true;
            }
            else if("qr" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Whirlwind Maze - Crater";
                info.name_normal = "whirlwind maze crater";
                info.is_scene = true;
            }
            else if("qs" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Corel Railway - Rollercoaster";
                info.name_normal = "corel railway rollercoaster";
                info.is_scene = true;
            }
            else if("qt" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Wooden Bridge";
                info.name_normal = "wooden bridge";
                info.is_scene = true;
            }
            else if("qu" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Da Chao";
                info.name_normal = "da chao";
                info.is_scene = true;
            }
            else if("qv" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Fort Condor";
                info.name_normal = "fort condor";
                info.is_scene = true;
            }
            else if("qw" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Bizarro Battle - Right Side";
                info.name_normal = "bizarro battle right side";
                info.is_scene = true;
            }
            else if("qx" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Bizarro Battle - Left Side";
                info.name_normal = "bizarro battle left side";
                info.is_scene = true;
            }
            else if("qy" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Jenova*SYNTHESIS Battle";
                info.name_normal = "jenova synthesis battle";
                info.is_scene = true;
            }
            else if("qz" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Corel Train Battle";
                info.name_normal = "corel train battle";
                info.is_scene = true;
            }
            else if("ra" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Cosmo Canyon";
                info.name_normal = "cosmo canyon";
                info.is_scene = true;
            }
            else if("rb" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Caverns of the Gi";
                info.name_normal = "caverns of the gi";
                info.is_scene = true;
            }
            else if("rc" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Nibelheim Mansion Basement";
                info.name_normal = "nibelheim mansion basement";
                info.is_scene = true;
            }
            else if("rd" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Temple of the Ancients - Demons Gate";
                info.name_normal = "temple of the ancients demons gate";
                info.is_scene = true;
            }
            else if("re" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Temple of the Ancients - Mural Room";
                info.name_normal = "temple of the ancients mural room";
                info.is_scene = true;
            }
            else if("rf" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Temple of the Ancients - Clock Passage";
                info.name_normal = "temple of the ancients clock passage";
                info.is_scene = true;
            }
            else if("rg" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Final Battle - Sephiroth";
                info.name_normal = "final battle sephiroth";
                info.is_scene = true;
            }
            else if("rh" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Jungle";
                info.name_normal = "jungle";
                info.is_scene = true;
            }
            else if("ri" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Ultimate Weapon - Battle on Highwind";
                info.name_normal = "ultimate weapon highwind";
                info.is_scene = true;
            }
            else if("rj" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Corel Reactor";
                info.name_normal = "corel reactor";
                info.is_scene = true;
            }
            else if("rk" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Blank";
                info.name_normal = "blank";
                info.is_scene = true;
            }
            else if("rl" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Don Corneo's Mansion";
                info.name_normal = "don corneos mansion";
                info.is_scene = true;
            }
            else if("rm" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Emerald Weapon Battle";
                info.name_normal = "emerald weapon battle";
                info.is_scene = true;
            }
            else if("rn" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Reactor 5";
                info.name_normal = "reactor 5";
                info.is_scene = true;
            }
            else if("ro" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Shinra HQ - Escape";
                info.name_normal = "shinra hq escape";
                info.is_scene = true;
            }
            else if("rp" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Ultimate Weapon - Gongaga Reactor";
                info.name_normal = "ultimate weapon gongaga reactor";
                info.is_scene = true;
            }
            else if("rq" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Corel Prison - Dyne Battle";
                info.name_normal = "corel prison dyne battle";
                info.is_scene = true;
            }
            else if("rr" == alphanumeric_id){
                info.numeric_id = 4;
                info.name = "Ultimate Weapon - Forest";
                info.name_normal = "ultimate weapon forest";
                info.is_scene = true;
            }
            else if ("rs" == alphanumeric_id){
                info.numeric_id = -1;
                info.name = "Playable frog";
                info.name_normal = "playable_frog";
                info.is_player = true;
            }
            else if ("rt" == alphanumeric_id){
                info.numeric_id = CHAR_ID::CLOUD;
                info.name = "Cloud";
                info.name_normal = "cloud";
                info.is_player = true;
            }
            else if ("si" == alphanumeric_id){
                info.numeric_id = CHAR_ID::CLOUD;
                info.name = "Cloud (hi-res)";
                info.name_normal = "cloud_hi_res";
                info.is_player = true;
            }
            else if (
              "sb" == alphanumeric_id
              || "sc" == alphanumeric_id
              || "sd" == alphanumeric_id
              || "se" == alphanumeric_id
            ){
                info.numeric_id = CHAR_ID::BARRET;
                info.name = "Barret";
                info.name_normal = "barret";
                info.is_player = true;
            }
            else if ("ru" == alphanumeric_id){
                info.numeric_id = CHAR_ID::TIFA;
                info.name = "Tifa";
                info.name_normal = "tifa";
                info.is_player = true;
            }
            else if ("rv" == alphanumeric_id){
                info.numeric_id = CHAR_ID::AERIS;
                info.name = "Aeris";
                info.name_normal = "aeris";
                info.is_player = true;
            }
            else if ("rw" == alphanumeric_id){
                info.numeric_id = CHAR_ID::RED_XIII;
                info.name = "Red XIII";
                info.name_normal = "red_xiii";
                info.is_player = true;
            }
            else if ("rx" == alphanumeric_id){
                info.numeric_id = CHAR_ID::YUFFIE;
                info.name = "Yuffie";
                info.name_normal = "yuffie";
                info.is_player = true;
            }
            else if ("ry" == alphanumeric_id){
                info.numeric_id = CHAR_ID::CAIT_SITH;
                info.name = "Cait Sith";
                info.name_normal = "cait_sith";
                info.is_player = true;
            }
            else if (
              "sf" == alphanumeric_id
              || "sg" == alphanumeric_id
              || "sh" == alphanumeric_id
            ){
                info.numeric_id = CHAR_ID::VINCENT;
                info.name = "Vincent";
                info.name_normal = "vincent";
                info.is_player = true;
            }
            else if ("sj" == alphanumeric_id){
                info.numeric_id = CHAR_ID::VINCENT;
                info.name = "Vincent - Gallian Beast";
                info.name_normal = "vincent_gallian_beast";
                info.is_player = true;
            }
            else if ("sk" == alphanumeric_id){
                info.numeric_id = CHAR_ID::VINCENT;
                info.name = "Vincent - Death Gigas";
                info.name_normal = "vincent_death_gigas";
                info.is_player = true;
            }
            else if ("sl" == alphanumeric_id){
                info.numeric_id = CHAR_ID::VINCENT;
                info.name = "Vincent - Hellmasker";
                info.name_normal = "vincent_hellmasker";
                info.is_player = true;
            }
            else if ("sm" == alphanumeric_id){
                info.numeric_id = CHAR_ID::VINCENT;
                info.name = "Vincent - Chaos";
                info.name_normal = "vincent_chaos";
                info.is_player = true;
            }
            else if ("sa" == alphanumeric_id){
                info.numeric_id = CHAR_ID::SEPHIROTH;
                info.name = "Sephiroth";
                info.name_normal = "sephiroth";
                info.is_player = true;
            }
            return info;
        };
};
