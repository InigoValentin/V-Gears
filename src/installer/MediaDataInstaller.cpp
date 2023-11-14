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

#include <string>
#include <iostream>
#include <cstdio>
#include <OgreDataStream.h>
#include <OgreResourceGroupManager.h>
#include <OgreImage.h>
#include <OgreColourValue.h>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/predef/os.h>
#include <boost/filesystem.hpp>
#include <tinyxml.h>
#include "MediaDataInstaller.h"
#include "data/VGearsLGPArchive.h"
#include "data/VGearsTexFile.h"
#include "TexFile.h"
#if (BOOST_OS_WINDOWS)
#include <stdlib>
#elif (BOOST_OS_SOLARIS)
#include <stdlib>
#include <limits>
#elif (BOOST_OS_LINUX)
#include <unistd.h>
#include <limits.h>
#elif (BOOST_OS_MACOS)
#include <mach-o/dyld.h>
#elif (BOOST_OS_BSD_FREE)
#include <sys/types>
#include <sys/sysctl>
#endif

int MediaDataInstaller::TOTAL_SOUNDS = 750;

MediaDataInstaller::MediaDataInstaller(
  const std::string input_dir, const std::string output_dir, const bool keep_originals,
  const bool no_ffmpeg, const bool no_timidity
):
  input_dir_(input_dir), output_dir_(output_dir), keep_originals_(keep_originals),
  no_ffmpeg_(no_ffmpeg), no_timidity_(no_timidity),
  menu_(input_dir + "data/menu/menu_us.lgp", "LGP"), window_(input_dir + "data/kernel/WINDOW.BIN"),
  midi_(input_dir + "data/midi/midi.lgp", "LGP")
{PopulateMaps();}


std::string MediaDataInstaller::GetExecutablePath(){
    #if (BOOST_OS_WINDOWS)
    char *exe_path;
    if (_get_pgmptr(&exe_path) != 0) exe_path = "";
    #elif (BOOST_OS_SOLARIS)
    char exe_path[PATH_MAX];
    if (realpath(getexecname(), exe_path) == NULL) exe_path[0] = '\0';
    #elif (BOOST_OS_LINUX)
    char exe_path[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", exe_path, sizeof(exe_path));
    if (len == -1 || len == sizeof(exe_path)) len = 0;
    exe_path[len] = '\0';
    #elif (BOOST_OS_MACOS)
    char exe_path[PATH_MAX];
    uint32_t len = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &len) != 0){
        exe_path[0] = '\0'; // buffer too small (!)
    }
    else {
        // resolve symlinks, ., .. if possible
        char *canonical_path = realpath(exe_path, NULL);
        if (canonical_path != NULL){
            strncpy(exe_path,canonical_path,len);
            free(canonical_path);
        }
    }
    #elif (BOOST_OS_BSD_FREE)
    char exe_path[2048];
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;
    size_t len = sizeof(exe_path);
    if (sysctl(mib, 4, exe_path, &len, NULL, 0) != 0) exe_path[0] = '\0';
    #endif
    return
      strlen(exe_path) > 0
      ? boost::filesystem::path(exe_path).remove_filename().make_preferred().string()
      : std::string();
}


void MediaDataInstaller::PopulateMaps(){
    // Most data here comes from https://forums.qhimm.com/index.php?topic=15786.0
    sound_map_[0] = "Cursor";
    sound_description_[0] = "Cursor movement in windows and menus.";
    sound_map_[1] = "Window|SaveReady";
    sound_description_[1] = "Opening some windows, memory cards available on the save menu.";
    sound_map_[2] = "Error|InvalidChoice";
    sound_description_[2] = "Failure or error, selecting a greyed out option in a choice list.";
    sound_map_[3] = "Back|Cancel";
    sound_description_[3] = "Cancel button on choices, closing menus.";
    sound_map_[4] = "Miss";
    sound_description_[4] = "Hit miss during battle.";
    sound_map_[5] = "Cure|Potion";
    sound_description_[5] = "Cure spell or Potion during battle.";
    sound_map_[6] = "HighPotion";
    sound_description_[6] = "Hi-Potion during battle.";
    sound_map_[7] = "Cure2|XPotion";
    sound_description_[7] = "Cure2 or X-Potion during battle.";
    sound_map_[8] = "Fire";
    sound_description_[8] = "Fire spell.";
    sound_map_[9] = "Fire2";
    sound_description_[9] = "Fire 2 spell.";
    sound_map_[10] = "Bolt";
    sound_description_[10] = "Bolt spell.";
    sound_map_[11] = "Bolt2|Thunder";
    sound_description_[11] = "Bolt2 spell, sometimes thunder on fields (eg HoneyBee inn).";
    sound_map_[12] = "MagicPrep";
    sound_description_[12] = "Light that envolves a character when casting magic, summon...";
    sound_map_[13] = "LeviathanWave";
    sound_description_[13] = "Part of Leviathan summoning.";
    sound_map_[14] = "GunHit";
    sound_description_[14] = "Gun sound, heard on fields and battles.";
    sound_map_[15] = "BarretShoot";
    sound_description_[15] = "Barrets chain gun.";
    sound_map_[16] = "LeviathanScream";
    sound_description_[16] = "Part of Leviathan summoning.";
    sound_map_[17] = "CloudHit";
    sound_description_[17] = "Cloud sword hit.";
    sound_map_[18] = "";
    sound_description_[18] = "";
    sound_map_[19] = "Grenade";
    sound_description_[19] = "Grenade item in battle.";
    sound_map_[20] = "Ladder|BattleItem";
    sound_description_[20]
      = "Character grabbing a ladder on the fields, character reaching for an item during battle";
    sound_map_[21] = "EnemyDeath";
    sound_description_[21] = "When most enemies die.";
    sound_map_[22] = "AeriCriticalHit";
    sound_description_[22] = "Aeris staff critical hit.";
    sound_map_[23] = "Ice";
    sound_description_[23] = "Ice spell";
    sound_map_[24] = "Bite";
    sound_description_[24] = "enemy bite-type attack, dogs or behemoths. Similar to 399.";
    sound_map_[25] = "Flee";
    sound_description_[25] = "Footsteps escaping from battle.";
    sound_map_[26] = "CloudCriticalHit";
    sound_description_[26] = "Cloud sword critical hit";
    sound_map_[27] = "FieldPunch";
    sound_description_[27] = "Punch sound on fields.";
    sound_map_[28] = "Ice3";
    sound_description_[28] = "Ice3 spell.";
    sound_map_[29] = "Demi";
    sound_description_[29] = "Demi sound after dark matter launched.";
    sound_map_[30] = "";
    sound_description_[30] = "";
    sound_map_[31] = "GateDecipher";
    sound_description_[31] = "Part of Jessie opening the deciphered gate (second half).";
    sound_map_[32] = "MeteoStrike";
    sound_description_[32] = "Tifas Meteo Strike limit.";
    sound_map_[33] = "HealingWind";
    sound_description_[33] = "Aeris Healing Wind limit.";
    sound_map_[34] = "Punch";
    sound_description_[34] = "Enemy punch-type attack";
    sound_map_[35] = "LimitAura";
    sound_description_[35] = "Before any limit, the orange aura sound.";
    sound_map_[36] = "ReactorCore";
    sound_description_[36] = "Background ambient sound in deeper reactor areas.";
    sound_map_[37] = "ReactorBackground";
    sound_description_[37] = "Background ambient sound in less deep reactor areas.";
    sound_map_[38] = "UnderwaterReactor";
    sound_description_[38] = "Background ambient sound in the underwater reactor.";
    sound_map_[39] = "BigShot";
    sound_description_[39] = "Barrets Big Shot limit.";
    sound_map_[40] = "SummonVanish";
    sound_description_[40] = "When about to summon the party disappears.";
    sound_map_[41] = "Elevator";
    sound_description_[41] = "Elevator moving.";
    sound_map_[42] = "BattleSwirl";
    sound_description_[42] = "Sound during battle swirling before battles.";
    sound_map_[43] = "ReelStop";
    sound_description_[43] = "When a reel slot stops during battle (limits, arena...)";
    sound_map_[44] = "Load";
    sound_description_[44] = "Loading game?";
    sound_map_[45] = "BarretCriticalHit";
    sound_description_[45] = "Barrets critical gun hit.";
    sound_map_[46] = "BarretMiss|StealMiss";
    sound_description_[46] = "Barret missing hit, unsuccessful steal.";
    sound_map_[47] = "BoostedJump";
    sound_description_[47] = "Some kind of boosted jump, battles and/or fields?";
    sound_map_[48] = "SmokeShot";
    sound_description_[48] = "Enemy Smoke shot (and similar) attacks.";
    sound_map_[49] = "AerisHit";
    sound_description_[49] = "Aeris staff hit.";
    sound_map_[50] = "On";
    sound_description_[50] = "Switching something on";
    sound_map_[51] = "GateUnlocked";
    sound_description_[51] = "Part of Jessie opening the deciphered gate (second half).";
    sound_map_[52] = "TrainDoor";
    sound_description_[52] = "Train door sound.";
    sound_map_[53] = "ReinforcedDoor.";
    sound_description_[53] = "Some reinforced doors. In reactors?";
    sound_map_[54] = "Jump";
    sound_description_[54] = "Character jump on fields.";
    sound_map_[55] = "Land";
    sound_description_[55] = "Character landing after a jump on fields.";
    sound_map_[56] = "Charge";
    sound_description_[56] = "Enemy charge-type attack.";
    sound_map_[57] = "Laser";
    sound_description_[57] = "enemy laser-type attack.";
    sound_map_[58] = "Beep";
    sound_description_[58] = "Computer or machine beep sound.";
    sound_map_[59] = "Alert";
    sound_description_[59] = "Alarm sound.";
    sound_map_[60] = "Computer|SetBomb";
    sound_description_[60] = "operating some computers, setting Jessies bomb.";
    sound_map_[61] = "Dodge";
    sound_description_[61] = "Dodging an attack.";
    sound_map_[62] = "Bio";
    sound_description_[62] = "Bio spell.";
    sound_map_[63] = "UNKNOWN63";
    sound_description_[63] = "";
    sound_map_[64] = "Slow";
    sound_description_[64] = "Slow spell.";
    sound_map_[65] = "Berserk";
    sound_description_[65] = "Berserk spell.";
    sound_map_[66] = "Shell";
    sound_description_[66] = "Shell spell.";
    sound_map_[67] = "Wall";
    sound_description_[67] = "Wall spell.";
    sound_map_[68] = "Cure3|Elixir";
    sound_description_[68] = "Cure3 or Elixir during battle.";
    sound_map_[69] = "Bol3";
    sound_description_[69] = "Bolt3 spell.";
    sound_map_[70] = "Quake";
    sound_description_[70] = "Quake spell.";
    sound_map_[71] = "Reflect";
    sound_description_[71] = "Reflect spell.";
    sound_map_[72] = "Sleepel";
    sound_description_[72] = "Sleepel spell.";
    sound_map_[73] = "Demi3";
    sound_description_[73] = "Demi 3 spell, after the dark matter is launched.";
    sound_map_[74] = "Demi2";
    sound_description_[74] = "Demi 2 spell, after the dark matter is launched.";
    sound_map_[75] = "Comet";
    sound_description_[75] = "Comet spell.";
    sound_map_[76] = "";
    sound_description_[76] = "";
    sound_map_[77] = "Tornado";
    sound_description_[77] = "tornado spell.";
    sound_map_[78] = "";
    sound_description_[78] = "";
    sound_map_[79] = "DemiShot";
    sound_description_[79] = "Demi/2/3, before 29, 74 or 73.";
    sound_map_[80] = "";
    sound_description_[80] = "";
    sound_map_[81] = "";
    sound_description_[81] = "";
    sound_map_[82] = "Haste";
    sound_description_[82] = "Haste spell.";
    sound_map_[83] = "Silence";
    sound_description_[83] = "Silence spell.";
    sound_map_[84] = "Break";
    sound_description_[84] = "Break spell.";
    sound_map_[85] = "Bolt3";
    sound_description_[85] = "Bolt3 spell. All?";
    sound_map_[86] = "Stop";
    sound_description_[86] = "Stop spell.";
    sound_map_[87] = "Quake3";
    sound_description_[87] = "Quake3 spell.";
    sound_map_[88] = "";
    sound_description_[88] = "";
    sound_map_[89] = "Bio2|Bio3";
    sound_description_[89] = "Bio2 and Bio3 spells.";
    sound_map_[90] = "Regen";
    sound_description_[90] = "Regen spell.";
    sound_map_[91] = "PoisonNull";
    sound_description_[91] = "PoisonNull spell.";
    sound_map_[92] = "";
    sound_description_[92] = "";
    sound_map_[93] = "Protect";
    sound_description_[93] = "Protect spell.";
    sound_map_[94] = "Wind";
    sound_description_[94] = "Wind sound as background for some maps.";
    sound_map_[95] = "";
    sound_description_[95] = "";
    sound_map_[96] = "Aparition";
    sound_description_[96] = "Aparition of someone in fields?";
    sound_map_[97] = "SearchScope";
    sound_description_[97] = "Guard Scorpion Search Scope attack.";
    sound_map_[98] = "100Needles";
    sound_description_[98] = "100 Needles enemy skill.";
    sound_map_[99] = "Seed";
    sound_description_[99] = "Enemy seed-type attack.";
    sound_map_[100] = "BeamGun";
    sound_description_[100] = "Enemy beam-gun-type attack.";
    sound_map_[101] = "";
    sound_description_[101] = "";
    sound_map_[102] = "";
    sound_description_[102] = "";
    sound_map_[103] = "";
    sound_description_[103] = "Rage";
    sound_map_[104] = "Enemy rage-type attack.";
    sound_description_[104] = "";
    sound_map_[105] = "";
    sound_description_[105] = "";
    sound_map_[106] = "RedLight|EnemyFlash";
    sound_description_[106] = "Jenovas Red Light attack, enemy Flash attack.";
    sound_map_[107] = "TetraLaser";
    sound_description_[107] = "Enemy TetraLaser (and similar) attacks.";
    sound_map_[108] = "RedXIIIHit";
    sound_description_[108] = "RedXIII attack hit.";
    sound_map_[109] = "";
    sound_description_[109] = "Unknown ambient sound.";
    sound_map_[110] = "";
    sound_description_[110] = "Unknown ambient sound.";
    sound_map_[111] = "";
    sound_description_[111] = "Unknown ambient sound.";
    sound_map_[112] = "";
    sound_description_[112] = "Unknown ambient sound.";
    sound_map_[113] = "";
    sound_description_[113] = "Unknown ambient sound.";
    sound_map_[114] = "";
    sound_description_[114] = "Unknown ambient sound.";
    sound_map_[115] = "";
    sound_description_[115] = "Unknown ambient sound.";
    sound_map_[116] = "";
    sound_description_[116] = "Unknown ambient sound.";
    sound_map_[117] = "Slap";
    sound_description_[117] = "Slapping sound. Tifa vs Scarlet?";
    sound_map_[118] = "Flush";
    sound_description_[118] = "Enemy flush-type attack.";
    sound_map_[119] = "DiamondFlash";
    sound_description_[119] = "Enemy DiamondFlash attack. Diamond Weapon?";
    sound_map_[120] = "";
    sound_description_[120] = "";
    sound_map_[121] = "DoorOpen";
    sound_description_[121] = "Door opening";
    sound_map_[122] = "VincentHit";
    sound_description_[122] = "Vincent gun hit.";
    sound_map_[123] = "";
    sound_description_[123] = "";
    sound_map_[124] = "Fascination";
    sound_description_[124] = "Enemy Fascination attack.";
    sound_map_[125] = "Drain|VampireTooth";
    sound_description_[125] = "Drain spell, probably Vampire Tooth?";
    sound_map_[126] = "DeBarrier";
    sound_description_[126] = "DeBarrier spell.";
    sound_map_[127] = "Resist";
    sound_description_[127] = "Resist spell.";
    sound_map_[128] = "WhiteWind";
    sound_description_[128] = "White Wind enemy skill.";
    sound_map_[129] = "MatraMagic";
    sound_description_[129] = "Matra magic enemy skill.";
    sound_map_[130] = "FlyingSickle";
    sound_description_[130] = "Enemy Flying Sickle (and similar) attacks.";
    sound_map_[131] = "Drums";
    sound_description_[131] = "Sound of drums, when chocobo race prices are about to be revealed.";
    sound_map_[132] = "Fall";
    sound_description_[132] = "Falling sound.";
    sound_map_[133] = "Despell";
    sound_description_[133] = "Despell spell.";
    sound_map_[134] = "Faint";
    sound_description_[134] = "Fainting sound.";
    sound_map_[135] = "NibelBurning";
    sound_description_[135] = "Nibelheim burning sound.";
    sound_map_[136] = "";
    sound_description_[136] = "";
    sound_map_[137] = "InkAttack";
    sound_description_[137] = "Enemy Ink (and similar) attacks.";
    sound_map_[138] = "Esuna";
    sound_description_[138] = "Esuna spell.";
    sound_map_[139] = "Beta";
    sound_description_[139] = "Beta enemy skill.";
    sound_map_[140] = "";
    sound_description_[140] = "";
    sound_map_[141] = "Fire3";
    sound_description_[141] = "Fire3 spell.";
    sound_map_[142] = "JunonAlarm";
    sound_description_[142] = "Junon alarm.";
    sound_map_[143] = "Electricity.";
    sound_description_[143] = "Electricity effect, for example, in Sector 8 after bombing.";
    sound_map_[144] = "BarrelLand";
    sound_description_[144] = "Barrel landing effect in Sector 5 church.";
    sound_map_[145] = "BarrelRoll";
    sound_description_[145] = "Barrel rolling effect in Sector 5 church.";
    sound_map_[146] = "";
    sound_description_[146] = "";
    sound_map_[147] = "Comet2";
    sound_description_[147] = "Comet2 spell.";
    sound_map_[148] = "Ailment";
    sound_description_[148] = "Status ailment inflicted.";
    sound_map_[149] = "Sensor";
    sound_description_[149]
      = "Sensors or lasers on some fields, like the red ones blocking the party on the plate.";
    sound_map_[150] = "TV";
    sound_description_[150] = "Television transmitting.";
    sound_map_[151] = "AribusterExplode";
    sound_description_[151] = "Airbuster explosion on the field?";
    sound_map_[152] = "CorneoSurprise";
    sound_description_[152] = "Don Corneo surprised?";
    sound_map_[153] = "Train";
    sound_description_[153] = "Train running.";
    sound_map_[154] = "FrogSong";
    sound_description_[154] = "Frog Song enemy skill.";
    sound_map_[155] = "MagicBreath";
    sound_description_[155] = "MagicBreath enemy skill.";
    sound_map_[156] = "AquaLung";
    sound_description_[156] = "AquaLung enemy skill, and some similar enemy attacks.";
    sound_map_[157] = "GoblinPunch";
    sound_description_[157] = "Goblin Punch enemy skill.";
    sound_map_[158] = "PresidentSteps";
    sound_description_[158] = "President ShinRa footsteps on Reactor 5.";
    sound_map_[159] = "Trine";
    sound_description_[159] = "Trine enemy skill.";
    sound_map_[160] = "PresidentFinger";
    sound_description_[160] = "President ShinRa fingersnap.";
    sound_map_[161] = "BadBreath1";
    sound_description_[161] = "First part of Bad Breath enemy skill.";
    sound_map_[162] = "BadBreath2";
    sound_description_[162] = "Second part of Bad Breath enemy skill.";
    sound_map_[163] = "MagicHammer";
    sound_description_[163] = "MagicHammer enemy skill.";
    sound_map_[164] = "Elevator2";
    sound_description_[164] = "Other type of elevator: flipper, prison.";
    sound_map_[165] = "";
    sound_description_[165] = "";
    sound_map_[166] = "";
    sound_description_[166] = "";
    sound_map_[167] = "RunLoud";
    sound_description_[167] = "Loud footsteps running, like Barrets.";
    sound_map_[168] = "Fire3All";
    sound_description_[168] = "Fire3 spell, all targets.";
    sound_map_[169] = "DrainingTentacles";
    sound_description_[169] = "Enemy Draining Tentacles (and similar) attacks.";
    sound_map_[170] = "Coin";
    sound_description_[170] = "Coin sound. In fields, when somebody is paid, a coin inserted...";
    sound_map_[171] = "Impact";
    sound_description_[171] = "Some kind of impact?";
    sound_map_[172] = "";
    sound_description_[172] = "";
    sound_map_[173] = "Monitor";
    sound_description_[173] = "Some kind of monitor?";
    sound_map_[174] = "";
    sound_description_[174] = "";
    sound_map_[175] = "ColdBreath";
    sound_description_[175] = "Enemy Cold Breath attack.";
    sound_map_[176] = "Mini";
    sound_description_[176] = "Mini spell.";
    sound_map_[177] = "PoisonBreath";
    sound_description_[177] = "Enemy poison breath attack.";
    sound_map_[178] = "";
    sound_description_[178] = "";
    sound_map_[179] = "WardDoor";
    sound_description_[179] = "WardDoor?";
    sound_map_[180] = "ElevatorRing";
    sound_description_[180] = "Elevator button ring.";
    sound_map_[181] = "RedCaravan";
    sound_description_[181] = "Red caravan?";
    sound_map_[182] = "MansionDoor";
    sound_description_[182] = "Door in mansion?";
    sound_map_[183] = "TrainWhistle";
    sound_description_[183] = "Train whistle sound.";
    sound_map_[184] = "IceCubes";
    sound_description_[184] = "Ice cubes?";
    sound_map_[185] = "Refuse";
    sound_description_[185] = "Enemy Refuse attack.";
    sound_map_[186] = "Shake";
    sound_description_[186] = "Shake effect, like Tifa shaking a drink.";
    sound_map_[187] = "";
    sound_description_[187] = "";
    sound_map_[188] = "";
    sound_description_[188] = "";
    sound_map_[189] = "Curtain";
    sound_description_[189] = "Curtain slipping, as in the dress shop in Wall Market.";
    sound_map_[190] = "MateriaGlow";
    sound_description_[190] = "Materia glow effect.";
    sound_map_[191] = "Helicopter";
    sound_description_[191] = "Helicopters on fields";
    sound_map_[192] = "Unlock";
    sound_description_[192] = "Some king of unlock sound.";
    sound_map_[193] = "Birdies";
    sound_description_[193] = "Birdies of Mt. Corel, up close";
    sound_map_[194] = "Boulder";
    sound_description_[194] = "Boulder rolling.";
    sound_map_[195] = "";
    sound_description_[195] = "";
    sound_map_[196] = "Chirping";
    sound_description_[196] = "Birds chirping in Mt. Corel heard from the tracks.";
    sound_map_[197] = "Appear";
    sound_description_[197] = "Someone is appearing?";
    sound_map_[198] = "BreakOpen";
    sound_description_[198] = "Break open sound?";
    sound_map_[199] = "AirBuster";
    sound_description_[199] = "AirBuster humming sound?";
    sound_map_[200] = "";
    sound_description_[200] = "";
    sound_map_[201] = "";
    sound_description_[201] = "";
    sound_map_[202] = "Collapse";
    sound_description_[202] = "Collapsing, as in the Great glacier.";
    sound_map_[203] = "";
    sound_description_[203] = "";
    sound_map_[204] = "BiggsThrown";
    sound_description_[204] = "Biggs thrown away by Barret in the hideout.";
    sound_map_[205] = "";
    sound_description_[205] = "";
    sound_map_[206] = "SaveLaunched";
    sound_description_[206] = "Game loaded?";
    sound_map_[207] = "";
    sound_description_[207] = "";
    sound_map_[208] = "TrainIgnite";
    sound_description_[208] = "Train ignition sound.";
    sound_map_[209] = "";
    sound_description_[209] = "";
    sound_map_[210] = "";
    sound_description_[210] = "";
    sound_map_[211] = "";
    sound_description_[211] = "";
    sound_map_[212] = "";
    sound_description_[212] = "";
    sound_map_[213] = "";
    sound_description_[213] = "";
    sound_map_[214] = "";
    sound_description_[214] = "";
    sound_map_[215] = "JunonElevator";
    sound_description_[215] = "Elevator in Junon. Which one?";
    sound_map_[216] = "Leap";
    sound_description_[216] = "A leap?";
    sound_map_[217] = "Snoring";
    sound_description_[217] = "People snoring in fields.";
    sound_map_[218] = "TentacleHit";
    sound_description_[218] = "enemy tentacle hit, ie: The dogs with a tentacle.";
    sound_map_[219] = "";
    sound_description_[219] = "";
    sound_map_[220] = "";
    sound_description_[220] = "";
    sound_map_[221] = "";
    sound_description_[221] = "";
    sound_map_[222] = "PunchCritical";
    sound_description_[222] = "Enemy punch-type critical attack.";
    sound_map_[223] = "Claw";
    sound_description_[223] = "Enemy claw-type attack.";
    sound_map_[224] = "Horn";
    sound_description_[224] = "Enemy horn-type attack.";
    sound_map_[225] = "Sting";
    sound_description_[225] = "enemy sting-type attack.";
    sound_map_[226] = "";
    sound_description_[226] = "";
    sound_map_[227] = "";
    sound_description_[227] = "";
    sound_map_[228] = "";
    sound_description_[228] = "";
    sound_map_[229] = "";
    sound_description_[229] = "";
    sound_map_[230] = "";
    sound_description_[230] = "RapidMove";
    sound_map_[231] = "Like Cloud dodging Barret in 7th Heaven.";
    sound_description_[231] = "";
    sound_map_[232] = "WaterGun";
    sound_description_[232] = "Enemy water gun and similar attacks.";
    sound_map_[233] = "Crazy";
    sound_description_[233] = "When Cloud loses his mind.";
    sound_map_[234] = "";
    sound_description_[234] = "";
    sound_map_[235] = "";
    sound_description_[235] = "";
    sound_map_[236] = "";
    sound_description_[236] = "";
    sound_map_[237] = "";
    sound_description_[237] = "";
    sound_map_[238] = "";
    sound_description_[238] = "";
    sound_map_[239] = "Crowd";
    sound_description_[239] = "Crowd yelling, as in the Junon Parade.";
    sound_map_[240] = "";
    sound_description_[240] = "";
    sound_map_[241] = "UnderwaterElevator";
    sound_description_[241] = "Junon underwater reactor elevator.";
    sound_map_[242] = "HiTechDoor";
    sound_description_[242] = "Some hi-tech door. Hojos lab?";
    sound_map_[243] = "";
    sound_description_[243] = "Moogle";
    sound_map_[244] = "Moogle Kupo sound";
    sound_description_[244] = "Bark";
    sound_map_[245] = "Dog barking on the fields.";
    sound_description_[245] = "";
    sound_map_[246] = "";
    sound_description_[246] = "";
    sound_map_[247] = "BigBomber";
    sound_description_[247] = "Enemy Big Bomber and similar attacks.";
    sound_map_[248] = "DoorClose";
    sound_description_[248] = "Some door closing?";
    sound_map_[249] = "Shower";
    sound_description_[249] = "Shower sound on the fields.";
    sound_map_[250] = "Price";
    sound_description_[250] = "Winning a price, like in Gold Saucer.";
    sound_map_[251] = "";
    sound_description_[251] = "";
    sound_map_[252] = "";
    sound_description_[252] = "";
    sound_map_[253] = "Chest";
    sound_description_[253] = "Treasure chest opening";
    sound_map_[254] = "Clothes";
    sound_description_[254] = "Changing clothes.";
    sound_map_[255] = "";
    sound_description_[255] = "";
    sound_map_[256] = "";
    sound_description_[256] = "";
    sound_map_[257] = "";
    sound_description_[257] = "";
    sound_map_[258] = "";
    sound_description_[258] = "";
    sound_map_[259] = "";
    sound_description_[259] = "";
    sound_map_[260] = "MenuCure";
    sound_description_[260] = "Curing while on the menu, magic or items.";
    sound_map_[261] = "Buy";
    sound_description_[261] = "Buying something in shops.";
    sound_map_[262] = "PlanetCry";
    sound_description_[262] = "Cry of the planet.";
    sound_map_[263] = "LifeStreamExit";
    sound_description_[263] = "When Cloud exits the LifeStream?";
    sound_map_[264] = "Springs";
    sound_description_[264] = "Springs in battle? in the Great Glacier?";
    sound_map_[265] = "";
    sound_description_[265] = "";
    sound_map_[266] = "MachineReact";
    sound_description_[266] = "Some machine reacting.";
    sound_map_[267] = "";
    sound_description_[267] = "";
    sound_map_[268] = "";
    sound_description_[268] = "";
    sound_map_[269] = "LongGunning";
    sound_description_[269] = "When Barrett aims at the ruins of Sector 7. Identical to 304.";
    sound_map_[270] = "";
    sound_description_[270] = "";
    sound_map_[271] = "VincentCriticalHit";
    sound_description_[271] = "Vincent gun critical hit.";
    sound_map_[272] = "ChocoboSad";
    sound_description_[272] = "Chocobo battle sad sound.";
    sound_map_[273] = "chocoboHappy";
    sound_description_[273] = "Chocobo battle happy sound.";
    sound_map_[274] = "ChocoboAngry";
    sound_description_[274] = "Chocobo battle angry sound.";
    sound_map_[275] = "";
    sound_description_[275] = "";
    sound_map_[276] = "RubyFlameThrower";
    sound_description_[276] = "Ruby Weapon Flame Thrower attack.";
    sound_map_[277] = "";
    sound_description_[277] = "";
    sound_map_[278] = "BeamGun2";
    sound_description_[278] = "Part of an enemy beam gun attack?";
    sound_map_[279] = "";
    sound_description_[279] = "";
    sound_map_[280] = "JunonElevatorAlt";
    sound_description_[280] = "Another elevator at Junon?";
    sound_map_[281] = "";
    sound_description_[281] = "";
    sound_map_[282] = "";
    sound_description_[282] = "";
    sound_map_[283] = "";
    sound_description_[283] = "";
    sound_map_[284] = "";
    sound_description_[284] = "";
    sound_map_[285] = "";
    sound_description_[285] = "";
    sound_map_[286] = "";
    sound_description_[286] = "";
    sound_map_[287] = "";
    sound_description_[287] = "";
    sound_map_[288] = "SouthernCross";
    sound_description_[288] = "Enemy Southern Cross attack.";
    sound_map_[289] = "TifaHit";
    sound_description_[289] = "Tifa hit sound.";
    sound_map_[290] = "Somersault";
    sound_description_[290] = "Tifas Somersault limit";
    sound_map_[291] = "";
    sound_description_[291] = "";
    sound_map_[292] = "";
    sound_description_[292] = "";
    sound_map_[293] = "";
    sound_description_[293] = "";
    sound_map_[294] = "";
    sound_description_[294] = "";
    sound_map_[295] = "";
    sound_description_[295] = "";
    sound_map_[296] = "";
    sound_description_[296] = "";
    sound_map_[297] = "";
    sound_description_[297] = "";
    sound_map_[298] = "";
    sound_description_[298] = "";
    sound_map_[299] = "";
    sound_description_[299] = "";
    sound_map_[300] = "";
    sound_description_[300] = "";
    sound_map_[301] = "DragonForce";
    sound_description_[301] = "Dragon Force enemy skill.";
    sound_map_[302] = "";
    sound_description_[302] = "";
    sound_map_[303] = "Tsunami";
    sound_description_[303] = "Enemy Sewer Tsunami (and similar) attacks.";
    sound_map_[304] = "LongGunningAlt";
    sound_description_[304] = "When Barrett aims at the ruins of Sector 7. Identical to 269.";
    sound_map_[305] = "";
    sound_description_[305] = "";
    sound_map_[306] = "";
    sound_description_[306] = "";
    sound_map_[307] = "Ultima";
    sound_description_[307] = "Ultima spell.";
    sound_map_[308] = "";
    sound_description_[308] = "";
    sound_map_[309] = "";
    sound_description_[309] = "";
    sound_map_[310] = "";
    sound_description_[310] = "";
    sound_map_[311] = "";
    sound_description_[311] = "";
    sound_map_[312] = "";
    sound_description_[312] = "";
    sound_map_[313] = "";
    sound_description_[313] = "";
    sound_map_[314] = "";
    sound_description_[314] = "";
    sound_map_[315] = "";
    sound_description_[315] = "";
    sound_map_[316] = "";
    sound_description_[316] = "";
    sound_map_[317] = "";
    sound_description_[317] = "";
    sound_map_[318] = "";
    sound_description_[318] = "";
    sound_map_[319] = "";
    sound_description_[319] = "";
    sound_map_[320] = "";
    sound_description_[320] = "";
    sound_map_[321] = "";
    sound_description_[321] = "";
    sound_map_[322] = "";
    sound_description_[322] = "";
    sound_map_[323] = "";
    sound_description_[323] = "";
    sound_map_[324] = "";
    sound_description_[324] = "";
    sound_map_[325] = "";
    sound_description_[325] = "";
    sound_map_[326] = "";
    sound_description_[326] = "";
    sound_map_[327] = "";
    sound_description_[327] = "";
    sound_map_[328] = "";
    sound_description_[328] = "";
    sound_map_[329] = "";
    sound_description_[329] = "";
    sound_map_[330] = "";
    sound_description_[330] = "";
    sound_map_[331] = "";
    sound_description_[331] = "";
    sound_map_[332] = "";
    sound_description_[332] = "";
    sound_map_[333] = "";
    sound_description_[333] = "";
    sound_map_[334] = "";
    sound_description_[334] = "";
    sound_map_[335] = "";
    sound_description_[335] = "";
    sound_map_[336] = "";
    sound_description_[336] = "";
    sound_map_[337] = "";
    sound_description_[337] = "";
    sound_map_[338] = "";
    sound_description_[338] = "Masamune";
    sound_map_[339] = "Masamune sound?";
    sound_description_[339] = "";
    sound_map_[340] = "";
    sound_description_[340] = "";
    sound_map_[341] = "";
    sound_description_[341] = "";
    sound_map_[342] = "YuffieHit";
    sound_description_[342] = "Yuffie shuriken hit.";
    sound_map_[343] = "";
    sound_description_[343] = "";
    sound_map_[344] = "";
    sound_description_[344] = "";
    sound_map_[345] = "LenoHit";
    sound_description_[345] = "Leno rod hit.";
    sound_map_[346] = "";
    sound_description_[346] = "";
    sound_map_[347] = "TifaCriticalHit";
    sound_description_[347] = "Tifa critical hit.";
    sound_map_[348] = "";
    sound_description_[348] = "";
    sound_map_[349] = "";
    sound_description_[349] = "";
    sound_map_[350] = "";
    sound_description_[350] = "";
    sound_map_[351] = "";
    sound_description_[351] = "";
    sound_map_[352] = "";
    sound_description_[352] = "";
    sound_map_[353] = "";
    sound_description_[353] = "";
    sound_map_[354] = "";
    sound_description_[354] = "";
    sound_map_[355] = "";
    sound_description_[355] = "";
    sound_map_[356] = "SavePoint";
    sound_description_[356] = "Save point sound.";
    sound_map_[357] = "Obtain";
    sound_description_[357] = "Obtained something.";
    sound_map_[358] = "DeathForce";
    sound_description_[358] = "Death Force enemy skill.";
    sound_map_[359] = "";
    sound_description_[359] = "";
    sound_map_[360] = "";
    sound_description_[360] = "";
    sound_map_[361] = "";
    sound_description_[361] = "";
    sound_map_[362] = "";
    sound_description_[362] = "Shield";
    sound_map_[363] = "Shield spell.";
    sound_description_[363] = "";
    sound_map_[364] = "";
    sound_description_[364] = "";
    sound_map_[365] = "BigGuard";
    sound_description_[365] = "Big Guard enemy skill.";
    sound_map_[366] = "Heartbeat";
    sound_description_[366]
      = "ShinRa mansion heartbeat background effects. Heard in other places too.";
    sound_map_[367] = "";
    sound_description_[367] = "";
    sound_map_[368] = "BladeBeam";
    sound_description_[368] = "Clouds Blade Beam limit.";
    sound_map_[369] = "FlareFirst";
    sound_description_[369] = "First part of the Flare spell.";
    sound_map_[370] = "FlareSecond";
    sound_description_[370] = "Second part of the flare spell.";
    sound_map_[371] = "";
    sound_description_[371] = "";
    sound_map_[372] = "";
    sound_description_[372] = "";
    sound_map_[373] = "MasamuneField";
    sound_description_[373] = "The masamune used on fields.";
    sound_map_[374] = "DemonLaugh";
    sound_description_[374] = "Demon laugh heard on Gold Saucer.";
    sound_map_[375] = "";
    sound_description_[375] = "";
    sound_map_[376] = "InnkeeperScream";
    sound_description_[376] = "Gold Saucer innkeeper scream.";
    sound_map_[377] = "";
    sound_description_[377] = "";
    sound_map_[378] = "Ocean";
    sound_description_[378] = "Ocean sound?";
    sound_map_[379] = "PhoenixTail";
    sound_description_[379] = "Phoenix Tail used in battle.";
    sound_map_[380] = "";
    sound_description_[380] = "";
    sound_map_[381] = "LimitLearned";
    sound_description_[381] = "4th limit level learned.";
    sound_map_[382] = "";
    sound_description_[382] = "";
    sound_map_[383] = "";
    sound_description_[383] = "";
    sound_map_[384] = "";
    sound_description_[384] = "";
    sound_map_[385] = "";
    sound_description_[385] = "";
    sound_map_[386] = "";
    sound_description_[386] = "";
    sound_map_[387] = "";
    sound_description_[387] = "";
    sound_map_[388] = "";
    sound_description_[388] = "";
    sound_map_[389] = "AirBusterBreak";
    sound_description_[389] = "AirBuster malfunction sound.";
    sound_map_[390] = "";
    sound_description_[390] = "";
    sound_map_[391] = "";
    sound_description_[391] = "";
    sound_map_[392] = "";
    sound_description_[392] = "";
    sound_map_[393] = "";
    sound_description_[393] = "";
    sound_map_[394] = "";
    sound_description_[394] = "";
    sound_map_[395] = "";
    sound_description_[395] = "";
    sound_map_[396] = "";
    sound_description_[396] = "";
    sound_map_[397] = "";
    sound_description_[397] = "";
    sound_map_[398] = "";
    sound_description_[398] = "";
    sound_map_[399] = "";
    sound_description_[399] = "";
    sound_map_[400] = "Whistle";
    sound_description_[400] = "Heard when practicing for Junon parade.";
    sound_map_[401] = "";
    sound_description_[401] = "";
    sound_map_[402] = "Sonar";
    sound_description_[402] = "Submarine sonar.";
    sound_map_[403] = "";
    sound_description_[403] = "";
    sound_map_[404] = "";
    sound_description_[404] = "";
    sound_map_[405] = "";
    sound_description_[405] = "";
    sound_map_[406] = "";
    sound_description_[406] = "";
    sound_map_[407] = "";
    sound_description_[407] = "";
    sound_map_[408] = "";
    sound_description_[408] = "";
    sound_map_[409] = "";
    sound_description_[409] = "";
    sound_map_[410] = "";
    sound_description_[410] = "";
    sound_map_[411] = "";
    sound_description_[411] = "";
    sound_map_[412] = "";
    sound_description_[412] = "";
    sound_map_[413] = "";
    sound_description_[413] = "";
    sound_map_[414] = "";
    sound_description_[414] = "";
    sound_map_[415] = "";
    sound_description_[415] = "";
    sound_map_[416] = "";
    sound_description_[416] = "";
    sound_map_[417] = "";
    sound_description_[417] = "";
    sound_map_[418] = "";
    sound_description_[418] = "";
    sound_map_[419] = "";
    sound_description_[419] = "";
    sound_map_[420] = "Life2";
    sound_description_[420] = "Life2 spell.";
    sound_map_[421] = "";
    sound_description_[421] = "";
    sound_map_[422] = "";
    sound_description_[422] = "";
    sound_map_[423] = "";
    sound_description_[423] = "";
    sound_map_[424] = "";
    sound_description_[424] = "";
    sound_map_[425] = "";
    sound_description_[425] = "";
    sound_map_[426] = "";
    sound_description_[426] = "";
    sound_map_[427] = "";
    sound_description_[427] = "";
    sound_map_[428] = "";
    sound_description_[428] = "";
    sound_map_[429] = "";
    sound_description_[429] = "";
    sound_map_[430] = "";
    sound_description_[430] = "";
    sound_map_[431] = "";
    sound_description_[431] = "";
    sound_map_[432] = "";
    sound_description_[432] = "";
    sound_map_[433] = "";
    sound_description_[433] = "";
    sound_map_[434] = "";
    sound_description_[434] = "";
    sound_map_[435] = "";
    sound_description_[435] = "";
    sound_map_[436] = "";
    sound_description_[436] = "";
    sound_map_[437] = "";
    sound_description_[437] = "";
    sound_map_[438] = "Applauses";
    sound_description_[438] = "Applauses effect.";
    sound_map_[439] = "";
    sound_description_[439] = "";
    sound_map_[440] = "";
    sound_description_[440] = "";
    sound_map_[441] = "";
    sound_description_[441] = "";
    sound_map_[442] = "";
    sound_description_[442] = "";
    sound_map_[443] = "";
    sound_description_[443] = "";
    sound_map_[444] = "MenuEquip";
    sound_description_[444] = "Equipping something in menus.";
    sound_map_[445] = "BossDeath";
    sound_description_[445] = "A boss dying.";
    sound_map_[446] = "";
    sound_description_[446] = "";
    sound_map_[447] = "Announcement.";
    sound_description_[447] = "Anouncement sound, ie Gold Saucer PA calling jockeys.";
    sound_map_[448] = "PHS";
    sound_description_[448] = "PHS ringing.";
    sound_map_[449] = "Freeze";
    sound_description_[449] = "Freeze spell.";
    sound_map_[450] = "";
    sound_description_[450] = "";
    sound_map_[451] = "";
    sound_description_[451] = "";
    sound_map_[452] = "Meow";
    sound_description_[452] = "Meow spell.";
    sound_map_[453] = "";
    sound_description_[453] = "";
    sound_map_[454] = "";
    sound_description_[454] = "";
    sound_map_[455] = "SadDog";
    sound_description_[455] = "Sad dog sound.";
    sound_map_[456] = "";
    sound_description_[456] = "";
    sound_map_[457] = "";
    sound_description_[457] = "";
    sound_map_[458] = "";
    sound_description_[458] = "";
    sound_map_[459] = "";
    sound_description_[459] = "";
    sound_map_[460] = "";
    sound_description_[460] = "";
    sound_map_[461] = "";
    sound_description_[461] = "";
    sound_map_[462] = "";
    sound_description_[462] = "";
    sound_map_[463] = "";
    sound_description_[463] = "";
    sound_map_[464] = "Slots";
    sound_description_[464] = "Slots rolling.";
    sound_map_[465] = "Truck";
    sound_description_[465] = "A truck like the one Cloud and Zack ride.";
    sound_map_[466] = "";
    sound_description_[466] = "";
    sound_map_[467] = "";
    sound_description_[467] = "";
    sound_map_[468] = "";
    sound_description_[468] = "";
    sound_map_[469] = "";
    sound_description_[469] = "";
    sound_map_[470] = "";
    sound_description_[470] = "";
    sound_map_[471] = "";
    sound_description_[471] = "";
    sound_map_[472] = "";
    sound_description_[472] = "";
    sound_map_[473] = "";
    sound_description_[473] = "";
    sound_map_[474] = "";
    sound_description_[474] = "";
    sound_map_[475] = "";
    sound_description_[475] = "";
    sound_map_[476] = "";
    sound_description_[476] = "";
    sound_map_[477] = "";
    sound_description_[477] = "";
    sound_map_[478] = "";
    sound_description_[478] = "";
    sound_map_[479] = "";
    sound_description_[479] = "";
    sound_map_[480] = "";
    sound_description_[480] = "";
    sound_map_[481] = "";
    sound_description_[481] = "";
    sound_map_[482] = "";
    sound_description_[482] = "";
    sound_map_[483] = "";
    sound_description_[483] = "";
    sound_map_[484] = "";
    sound_description_[484] = "";
    sound_map_[485] = "Point";
    sound_description_[485] = "Earning points/GP.";
    sound_map_[486] = "";
    sound_description_[486] = "";
    sound_map_[487] = "";
    sound_description_[487] = "";
    sound_map_[488] = "";
    sound_description_[488] = "";
    sound_map_[489] = "";
    sound_description_[489] = "";
    sound_map_[490] = "";
    sound_description_[490] = "";
    sound_map_[491] = "";
    sound_description_[491] = "";
    sound_map_[492] = "";
    sound_description_[492] = "";
    sound_map_[493] = "";
    sound_description_[493] = "";
    sound_map_[494] = "";
    sound_description_[494] = "";
    sound_map_[495] = "";
    sound_description_[495] = "";
    sound_map_[496] = "";
    sound_description_[496] = "";
    sound_map_[497] = "";
    sound_description_[497] = "";
    sound_map_[498] = "";
    sound_description_[498] = "";
    sound_map_[499] = "";
    sound_description_[499] = "";
    sound_map_[506] = "Do";
    sound_description_[506] = "Piano Do note.";
    sound_map_[507] = "Re";
    sound_description_[507] = "Piano Re note.";
    sound_map_[508] = "Mi";
    sound_description_[508] = "Piano Mi note.";
    sound_map_[509] = "Fa";
    sound_description_[509] = "Piano Fa note.";
    sound_map_[510] = "Sol";
    sound_description_[510] = "Piano Sol note.";
    sound_map_[511] = "La";
    sound_description_[511] = "Piano La note.";
    sound_map_[512] = "Si";
    sound_description_[512] = "Piano Si note.";
    sound_map_[513] = "DoP";
    sound_description_[513] = "Piano Do up note.";
    sound_map_[514] = "SolP";
    sound_description_[514] = "Piano Sol up note.";
    sound_map_[515] = "LaP";
    sound_description_[515] = "Piano La up note.";
    sound_map_[516] = "SiP";
    sound_description_[516] = "Piano Si up note.";
    // TODO: Fill this list with somewhat descriptive names.
}

MediaDataInstaller::~MediaDataInstaller(){}

void MediaDataInstaller::InstallSprites(){
    // Actually open the lgp as a file for reading
    File menu(input_dir_ + "data/menu/menu_us.lgp");

    // Also, open it as a LGP archive.
    menu_.open(input_dir_ + "data/menu/menu_us.lgp", true);
    menu_.load();


    Ogre::StringVectorPtr file_list = menu_.list(true, true);
    //std::cout << " " << file_list << " files \n";
    VGears::LGPArchive::FileList files = menu_.GetFiles();

    for (int i = 0; i < file_list->size(); i ++){
        VGears::LGPArchive::FileEntry f = files.at(i);
        if (
          f.file_name != "barre.tex"
          && f.file_name != "bins.tex"
          && f.file_name != "cido.tex"
          && f.file_name != "cloud.tex"
          && f.file_name != "earith.tex"
          && f.file_name != "ketc.tex"
          && f.file_name != "pcefi.tex"
          && f.file_name != "pcloud.tex"
          && f.file_name != "red.tex"
          && f.file_name != "tifa.tex"
          && f.file_name != "yufi.tex"
          && f.file_name != "buster.tex"
          && f.file_name != "choco.tex"
          && f.file_name != "btl_win_c_l.tex"
          && f.file_name != "btl_win_a_h.tex"
          && f.file_name != "btl_win_b_h.tex"
          && f.file_name != "btl_win_b_l.tex"
        ){
            continue;
        }

        // Save the TEX File
        std::fstream out;
        out.open(output_dir_ + "images/" + f.file_name, std::ios::out);
        menu.SetOffset(f.data_offset);
        for (int j = 0; j < f.data_size; j ++) out << menu.readU8();
        out.close();

        // Open the TEX file and save PNGs.
        std::string img_dir = output_dir_ + "images/";
        TexFile tex(output_dir_ + "images/" + f.file_name);
        if (f.file_name == "barre.tex")
            tex.SavePng(img_dir + "characters/1.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "bins.tex")
            tex.SavePng(img_dir + "characters/7.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "cido.tex")
            tex.SavePng(img_dir + "characters/8.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "cloud.tex")
            tex.SavePng(img_dir + "characters/0.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "earith.tex")
            tex.SavePng(img_dir + "characters/3.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "ketc.tex")
            tex.SavePng(img_dir + "characters/6.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "pcefi.tex")
            tex.SavePng(img_dir + "characters/10.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "pcloud.tex")
            tex.SavePng(img_dir + "characters/9.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "red.tex")
            tex.SavePng(img_dir + "characters/4.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "tifa.tex")
            tex.SavePng(img_dir + "characters/2.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "yufi.tex")
            tex.SavePng(img_dir + "characters/5.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "buster.tex")
            tex.SavePng(img_dir + "other/begin_menu.png", 0, 0, 150, 150, 0);
        else if (f.file_name == "choco.tex")
            tex.SavePng(img_dir + "other/choco.png", 0, 0, 83, 95, 0);
        else if (f.file_name == "btl_win_c_l.tex"){
            // Weapon (no index 0), armor, accessory icons, palette 1.
            tex.SavePng(img_dir + "icons/item_weapon_1.png", 192, 0, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_2.png", 224, 0, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_3.png", 192, 32, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_4.png", 224, 32, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_5.png", 192, 64, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_6.png", 224, 64, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_7.png", 192, 96, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_8.png", 224, 96, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_armor.png", 192, 128, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_accessory.png", 224, 128, 32, 32, 1);
            // Status effects battle animations, palette 1.
            tex.SavePng(img_dir + "icons/status_2_0.png", 128, 188, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_2_1.png", 160, 188, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_2_2.png", 192, 188, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_2_3.png", 224, 188, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_7_0.png", 128, 219, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_7_1.png", 160, 219, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_7_2.png", 192, 219, 32, 32, 1);
            tex.SavePng(img_dir + "icons/status_7_3.png", 224, 219, 32, 32, 1);
            // Door arrows, palette 9.
            tex.SavePng(img_dir + "icons/pointer_gateway_0.png", 96, 160, 18, 12, 9);
            tex.SavePng(img_dir + "icons/pointer_gateway_1.png", 128, 160, 18, 12, 9);
            tex.SavePng(img_dir + "icons/pointer_gateway_1.png", 160, 160, 18, 12, 9);
            tex.SavePng(img_dir + "icons/pointer_gateway_3.png", 192, 160, 18, 12, 9);
            tex.SavePng(img_dir + "icons/pointer_gateway_4.png", 224, 160, 18, 12, 9);
            // Ladder arrows, palette 4.
            tex.SavePng(img_dir + "icons/pointer_ladder_0.png", 96, 160, 18, 12, 4);
            tex.SavePng(img_dir + "icons/pointer_ladder_1.png", 128, 160, 18, 12, 4);
            tex.SavePng(img_dir + "icons/pointer_ladder_1.png", 160, 160, 18, 12, 4);
            tex.SavePng(img_dir + "icons/pointer_ladder_3.png", 192, 160, 18, 12, 4);
            tex.SavePng(img_dir + "icons/pointer_ladder_4.png", 224, 160, 18, 12, 4);
            // Recovery word, palette 4.
            tex.SavePng(img_dir + "icons/battle_recovery.png", 64, 64, 212, 232, 64, 36, 20, 4);
            // Death word, palette 2.
            tex.SavePng(img_dir + "icons/battle_death.png", 64, 192, 64, 20, 2);
            // Window borders and corners, palette 0
            tex.SavePng(img_dir + "window/b_t.png", 0, 192, 32, 16, 0);
            tex.SavePng(img_dir + "window/b_b.png", 32, 208, 32, 16, 0);
            tex.SavePng(img_dir + "window/b_l.png", 0, 224, 16, 32, 0);
            tex.SavePng(img_dir + "window/b_r.png", 48, 224, 16, 32, 0);
            tex.SavePng(img_dir + "window/c_tl.png", 0, 208, 16, 16, 0);
            tex.SavePng(img_dir + "window/c_tr.png", 16, 208, 16, 16, 0);
            tex.SavePng(img_dir + "window/c_bl.png", 32, 192, 16, 16, 0);
            tex.SavePng(img_dir + "window/c_br.png", 48, 192, 16, 16, 0);
        }
        else if (f.file_name == "btl_win_a_h.tex"){
            // Items and weapon 0, palette 1.
            tex.SavePng(img_dir + "icons/item_item.png", 192, 223, 32, 32, 1);
            tex.SavePng(img_dir + "icons/item_weapon_0.png", 220, 223, 32, 32, 1);
            // Battle bar and texts, palette 1.
            tex.SavePng(img_dir + "icons/battle_bar.png", 32, 0, 76, 18, 1);
            tex.SavePng(img_dir + "icons/battle_limit.png", 112, 0, 48, 10, 1);
            tex.SavePng(img_dir + "icons/battle_hp.png", 160, 0, 28, 10, 1);
            tex.SavePng(img_dir + "icons/battle_mp.png", 192, 0, 30, 10, 1);
            tex.SavePng(img_dir + "icons/battle_time.png", 224, 162, 0, 16, 32, 14, 10, 1);
            tex.SavePng(img_dir + "icons/battle_barrier.png", 112, 16, 76, 10, 1);
            tex.SavePng(img_dir + "icons/battle_wait.png", 192, 16, 44, 10, 1);
            tex.SavePng(img_dir + "icons/battle_1.png", 130, 34, 10, 12, 1);
            tex.SavePng(img_dir + "icons/battle_2.png", 146, 34, 10, 12, 1);
            tex.SavePng(img_dir + "icons/battle_3.png", 162, 34, 10, 12, 1);
            tex.SavePng(img_dir + "icons/battle_4.png", 178, 34, 10, 12, 1);
            tex.SavePng(img_dir + "icons/battle_5.png", 194, 34, 10, 12, 1);
            tex.SavePng(img_dir + "icons/battle_mp_needed.png", 0, 40, 80, 24, 1);
            // Some other battle text, palette 7.
            tex.SavePng(img_dir + "icons/battle_all.png", 240, 32, 16, 16, 7);
            tex.SavePng(img_dir + "icons/battle_x.png", 240, 16, 16, 16, 7);
            // No growth materia slot and linker, palette 1.
            tex.SavePng(img_dir + "icons/materia_slot_no_growth.png", 224, 63, 24, 24, 1);
            tex.SavePng(img_dir + "icons/materia_slot_link.png", 227, 95, 14, 24, 1);
            // Buttons, palette 7.
            tex.SavePng(img_dir + "icons/bt_up.png", 160, 70, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_down.png", 192, 70, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_right.png", 160, 102, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_left.png", 192, 102, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_3.png", 192, 134, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_2.png", 224, 134, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_6.png", 192, 166, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_8.png", 224, 166, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_9.png", 192, 198, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_10.png", 224, 198, 26, 20, 7);
            // Crossed out cursor, palette 7.
            tex.SavePng(img_dir + "icons/cursor_crossed.png", 80, 32, 48, 32, 7);
            // Sound settings, palette 1.
            tex.SavePng(img_dir + "icons/settings_music.png", 101, 131, 30, 30, 1);
            tex.SavePng(img_dir + "icons/settings_fx.png", 133, 131, 30, 30, 1);
        }
        else if (f.file_name == "btl_win_b_h.tex"){
            // Don't take cursors fom here, they don't have shadows and are too narrow.
            // Take them from btl_win_b_l.tex

            // Menu and battle text, palette 1.
            tex.SavePng(img_dir + "icons/slash.png", 176, 0, 8, 16, 1);
            tex.SavePng(img_dir + "icons/battle_name.png", 0, 32, 52, 10, 1);
            tex.SavePng(img_dir + "icons/battle_status.png", 0, 48, 64, 10, 1);
            tex.SavePng(img_dir + "icons/battle_status.png", 144, 48, 52, 10, 1);
            // Up/Down pointers, palette 1.
            tex.SavePng(img_dir + "icons/pointer_up.png", 198, 0, 22, 16, 1);
            tex.SavePng(img_dir + "icons/pointer_down.png", 230, 0, 22, 16, 1);
            // Materia slot, palette 1.
            tex.SavePng(img_dir + "icons/materia_slot.png", 0, 95, 24, 24, 1);
            // Materia stuff, several plettes.
            tex.SavePng(img_dir + "icons/materia_1.png", 3, 67, 18, 18, 4);
            tex.SavePng(img_dir + "icons/materia_2.png", 3, 67, 18, 18, 5);
            tex.SavePng(img_dir + "icons/materia_3.png", 3, 67, 18, 18, 8);
            tex.SavePng(img_dir + "icons/materia_4.png", 3, 67, 18, 18, 9);
            tex.SavePng(img_dir + "icons/materia_5.png", 3, 67, 18, 18, 10);
            tex.SavePng(img_dir + "icons/materia_star_1.png", 32, 64, 23, 26, 4);
            tex.SavePng(img_dir + "icons/materia_star_2.png", 32, 64, 23, 26, 5);
            tex.SavePng(img_dir + "icons/materia_star_3.png", 32, 64, 23, 26, 8);
            tex.SavePng(img_dir + "icons/materia_star_4.png", 32, 64, 23, 26, 9);
            tex.SavePng(img_dir + "icons/materia_star_5.png", 32, 64, 23, 26, 10);
            tex.SavePng(img_dir + "icons/materia_star_empty_1.png", 32, 96, 23, 26, 4);
            tex.SavePng(img_dir + "icons/materia_star_empty_2.png", 32, 96, 23, 26, 5);
            tex.SavePng(img_dir + "icons/materia_star_empty_3.png", 32, 96, 23, 26, 8);
            tex.SavePng(img_dir + "icons/materia_star_empty_4.png", 32, 96, 23, 26, 9);
            tex.SavePng(img_dir + "icons/materia_star_empty_5.png", 32, 96, 23, 26, 10);
            // Buttons, palette 7.
            tex.SavePng(img_dir + "icons/bt_1.png", 0, 134, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_4.png", 32, 134, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_5.png", 0, 166, 26, 20, 7);
            tex.SavePng(img_dir + "icons/bt_7.png", 32, 16, 26, 20, 7);
            // Some other battle text, palette 7.
            tex.SavePng(img_dir + "icons/battle_inf.png", 36, 194, 26, 14, 7);
            // Right pointer, palette 7.
            tex.SavePng(img_dir + "icons/pointer_right.png", 4, 16, 10, 16, 7);
            // Battle and menu bars, palette 1.
            tex.SavePng(img_dir + "icons/battle_bar_barriers.png", 64, 32, 76, 24, 1);
            tex.SavePng(img_dir + "icons/menu_bar.png", 16, 16, 126, 16, 1);
            // Fonts, save all palettes, make one default.
            for (int p = 0; p < 16; p ++){
                // Digits font. Default palette 7.
                tex.SavePng(
                  img_dir + "fonts/digits_" + std::to_string(p) + ".png", 16, 0, 156, 16, p
                );
                if (p == 7) tex.SavePng(img_dir + "fonts/digits.png", 16, 0, 156, 16, p);
            }
            for (int p = 0; p < 16; p ++){
                // Time font. Default palette 12.
                tex.SavePng(
                  img_dir + "fonts/timer_" + std::to_string(p) + ".png", 32, 160, 224, 90, p
                );
                if (p == 12) tex.SavePng(img_dir + "fonts/timer.png", 32, 160, 224, 90, p);
            }
            // Reels, palete 11.
            tex.SavePng(img_dir + "reels/yeah.png", 64, 64, 64, 30, 11);
            tex.SavePng(img_dir + "reels/luck.png", 64, 98, 64, 30, 11);
            tex.SavePng(img_dir + "reels/hit.png", 128, 64, 64, 30, 11);
            tex.SavePng(img_dir + "reels/bar.png", 128, 97, 64, 30, 11);
            tex.SavePng(img_dir + "reels/miss.png", 192, 64, 64, 30, 11);
            tex.SavePng(img_dir + "reels/choso.png", 192, 98, 64, 30, 11);

        }
        else if (f.file_name == "btl_win_b_l.tex"){
            // Cursors, palette 1.
            tex.SavePng(img_dir + "icons/cursor.png", 192, 16, 48, 32, 1);
            tex.SavePng(img_dir + "icons/cursor_left.png", 144, 16, 48, 32, 1);
            tex.SavePng(img_dir + "icons/pointer_position.png", 0, 192, 32, 26, 1);
        }

        // TODO: Extract arena reels from coloa.tex, colob.tex and coloc.tex

        // Delete Tex file.
        if (!keep_originals_) std::remove((output_dir_ + "images/" + f.file_name).c_str());
    }
}

int MediaDataInstaller::InstallSoundsInit(){
    // SFXDump handles the wav conversion
    std::string command = (boost::format(
      "%1%/sfxdump %2%data/sound/audio.fmt %2%data/sound/audio.dat %3%audio/sounds/"
    ) % GetExecutablePath() % input_dir_ % output_dir_).str();    
    std::system(command.c_str());
    processed_sounds_ = 0;
    return TOTAL_SOUNDS;
}

bool MediaDataInstaller::InstallSounds(){
    if (!no_ffmpeg_){
        std::string f_path
          = output_dir_ + "audio/sounds/" + std::to_string(processed_sounds_) + ".wav";
        std::ifstream file(f_path);
        if (file.is_open()){
            file.close();
             std::string command = (boost::format(
              "ffmpeg -hide_banner -loglevel panic -y "
              "-i %1%audio/sounds/%2%.wav %1%audio/sounds/%2%.ogg"
            ) % output_dir_ % processed_sounds_).str();
            std::system(command.c_str());
            std::cout << "    ADD SOUND " << processed_sounds_ << ".ogg" << std::endl;
            sounds_.push_back("audio/sounds/" + std::to_string(processed_sounds_) + ".ogg");
            // Remove the wav file.
            if (!keep_originals_)
                std::remove((output_dir_ + "audio/sounds/" + std::to_string(processed_sounds_) + ".wav").c_str());
        }
        else{
            sounds_.push_back("audio/sounds/INVALID.ogg");
            std::cout << "    -- ADD SOUND INVALID.ogg" << std::endl;
        }
    }
    processed_sounds_ ++;
    if (processed_sounds_ >= TOTAL_SOUNDS) return true;
    else return false;
}

void MediaDataInstaller::WriteSoundIndex(){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("sounds"));
    //for (string item : items_){
    for (int id = 0; id < sounds_.size(); id ++){
        std::string path = sounds_[id];
        std::unique_ptr<TiXmlElement> xml_sound(new TiXmlElement("sound"));
        xml_sound->SetAttribute("file_name", path);
        xml_sound->SetAttribute("name", id);
        container->LinkEndChild(xml_sound.release());
        // If there is a friendly name for this sound, add another entry
        if (sound_map_.count(id) != 0 && sound_map_[id] != ""){
            std::unique_ptr<TiXmlElement> xml_sound_name(new TiXmlElement("sound"));
            xml_sound_name->SetAttribute("file_name", path);
            std::string name = sound_map_[id];
            xml_sound_name->SetAttribute("name", name);
            xml_sound_name->SetAttribute("description", sound_description_[id]);
            container->LinkEndChild(xml_sound_name.release());
        }
    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(output_dir_ + "audio/sounds/_sounds.xml");
}

int MediaDataInstaller::InstallMusicsInit(){
    // Read the music index file.
    std::ifstream music_idx(input_dir_ + "data/music/music.idx");
    int i = 0;
    std::string name;
    if (music_idx.is_open()){
        while (std::getline(music_idx, name)){
            // Newlines are very likely WINDOWS/DOS format, but check them all.
            boost::replace_all(name, "\r\n", "");
            boost::replace_all(name, "\n", "");
            boost::replace_all(name, "\r", "");
            musics_map_[i] = name;
            i ++;
        }
        music_idx.close();
    }

    // Load midi_
    midi_.load();

    processed_musics_ = 0;
    return midi_.list(true, true)->size();
}

bool MediaDataInstaller::InstallMusics(){
    VGears::LGPArchive::FileEntry f = midi_.GetFiles().at(processed_musics_);

    // Find the index in the map.
    int index = 1000 + processed_musics_;
    for (auto const& m : musics_map_){
        if (m.second + ".mid" == f.file_name){
            index = m.first;
            break;
        }
    }

    File midi(input_dir_ + "data/midi/midi.lgp");

    std::fstream out;
    out.open(output_dir_ + "audio/musics/" + std::to_string(index) + ".mid", std::ios::out);
    midi.SetOffset(f.data_offset);
    for (int j = 0; j < f.data_size; j ++) out << midi.readU8();
    out.close();

    // Convert to ogg (TiMidity + FFMpeg)
    std::string command = (boost::format(
      "timidity --quiet=3 %1%audio/musics/%2%.mid -Ow -o - "
      "| ffmpeg -hide_banner -loglevel panic -y -i - %1%audio/musics/%2%.ogg"
    ) % output_dir_ % index).str();
    if (!no_ffmpeg_ && !no_timidity_) std::system(command.c_str());

    // Remove the midi file.
    if (!keep_originals_)
        std::remove((output_dir_ + "audio/musics/" + std::to_string(index) + ".mid").c_str());

    musics_.push_back("audio/musics/" + std::to_string(processed_musics_) + ".ogg");
    processed_musics_ ++;
    if (processed_musics_ == midi_.list(true, true)->size()) return true;
    else return false;
}

void MediaDataInstaller::InstallHQMusics(){
    std::vector<std::string> hq_musics = {"hearth", "sato", "sensui", "wind"};
    for (std::string hq_music : hq_musics){

        int index = 2000;
        for (auto const& m : musics_map_){
            if (m.second == hq_music){
                index = m.first;
                break;
            }
        }

        std::string command = (boost::format(
          "ffmpeg -hide_banner -loglevel panic -y -i %1%musics/%2%.wav %3%audio/sounds/%4%.ogg"
        ) % input_dir_ % hq_music % output_dir_ % index).str();
        if (!no_ffmpeg_)  std::system(command.c_str());
    }
}

void MediaDataInstaller::WriteMusicsIndex(){
    TiXmlDocument xml;
    std::unique_ptr<TiXmlElement> container(new TiXmlElement("musics"));
    //for (string item : items_){
    for (int id = 0; id < musics_.size(); id ++){
        std::string path = musics_[id];
        std::unique_ptr<TiXmlElement> xml_music(new TiXmlElement("music"));
        xml_music->SetAttribute("file_name", path);
        xml_music->SetAttribute("name", id);
        xml_music->SetAttribute("loop", "0");
        switch (id){
            case 1:
                xml_music->SetAttribute("default", "initial");
                break;
            case 6:
                xml_music->SetAttribute("default", "battle");
                break;
            case 9:
                xml_music->SetAttribute("default", "boss");
                break;
            case 13:
                xml_music->SetAttribute("default", "world_map");
                break;
            case 43:
                xml_music->SetAttribute("default", "chocobo_battle");
                break;
            case 45:
                xml_music->SetAttribute("default", "victory");
                break;
            case 46:
                xml_music->SetAttribute("default", "airship");
                break;
            case 50:
                xml_music->SetAttribute("default", "chocobo_ride");
                break;
            case 57:
                xml_music->SetAttribute("default", "game_over");
                break;
            case 70:
                xml_music->SetAttribute("default", "world_map_danger");
                break;
            case 84:
                xml_music->SetAttribute("default", "submarine");
                break;

        }
        container->LinkEndChild(xml_music.release());
        // If there is a friendly name for this sound, add another entry
        if (musics_map_.count(id) != 0){
            std::unique_ptr<TiXmlElement> xml_music_name(new TiXmlElement("music"));
            xml_music_name->SetAttribute("file_name", path);
            xml_music_name->SetAttribute("name", musics_map_[id]);
            xml_music_name->SetAttribute("loop", "0");
            container->LinkEndChild(xml_music_name.release());
        }
    }
    xml.LinkEndChild(container.release());
    xml.SaveFile(output_dir_ + "audio/musics/_musics.xml");
}

