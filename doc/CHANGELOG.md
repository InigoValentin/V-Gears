# Changelog

## Unreleased

Changes since 0.4.0. Most of the work is in the data installer and in reorganizing the code, with the bugs found along the way.

**The game data has to be installed again.**

### Installer

**Command line**
- The installer also works from the command line (`v-gears-installer --iso <file>`), with presets and parts to choose what to install, `--verify` to check an installation, and exit statuses for scripts.
- Installing doesn't need a display, a window or a graphics driver.
- In a terminal it shows a single progress line, with the warnings and errors above it. Ctrl+C says that the installation didn't finish.

**Installing**
- Clean install works: it deletes the data of a previous installation and keeps your saves, settings and videos.
- The installer keeps a record of the installation (`install.info`). The verifier and the game report unfinished installations and outdated data.
- The extracted disk and the original files are deleted at the end, unless Preserve original data is selected, which is now off by default.
- The progress covers the whole installation, including the extraction of the disk, and the window can install again without being closed.
- Two installations of the same disk produce identical files.
- A failed extraction of a file of the disk now fails the installation. Damaged disk images are handled safely.
- The verifier looks for the Ogre plugins where the game looks for them.
- Skipping the kernel data no longer skips the images and the fonts, and skipping the field models no longer says that the field maps were skipped.
- Fixed crashes when no field had been converted or there were no field models, warnings about the language of the French releases, and the escaped characters in the log of the window.

**Converted data**
- Enemy attacks are no longer duplicated with broken animations.
- Enemies drop and have the right items to steal: armors and accessories became other items.
- Attacks that target all rows are read correctly, as are the formation flags of the enemies (the game doesn't apply them yet).
- Each character is listed once in the character list, with its different models.
- Fields are no longer skipped on purpose, and fields without texts are converted.
- Fixed the textures of effect models, empty battle models, truncated animations and the garbage in some field tile images.
- Fixed the third gateway and ladder arrows and the images of the "level" and "status" texts.

### Installation directory
- New layout: `engine/` (the files of V-Gears), `defaults/`, `game/` (converted game data, by area), `user/` (settings, controls, saves and screenshots) and `logs/`. Saves, settings and videos of older installations are moved to their new places.
- The shipped files are replaced on every installation. Your settings are kept.
- The Japanese and Russian languages are removed, as the font of the game can't show them.

### Saving
- Saving no longer keeps items, materia and data that were gone since the last save.
- Saving to one slot no longer changes the others.
- The key items, the learned enemy skills, the control key and the slot are read correctly.

### Field and engine
- Texts set by scripts are no longer cut after a tag.
- Invalid texts no longer crash the game.
- Fixed the scroll limits at the start of a field.
- A map jump to a walkmesh triangle that doesn't exist uses the triangle under the position.

### Code, build and documentation
- The source code is organized by layer and subsystem, large classes and functions are split, and unused code is removed.
- Each source folder has its own `CMakeLists.txt`.
- Removed the data of the original game that was in the repository. The installer builds the fonts from the files of the game.
- New tests, and a rewritten data installation guide.

## 0.4.0

Changes since 0.3.5. Most of the work is in the battle and menu modules (rules, cameras, shops and materia), and in fixes from playing through the fields and the world map.

### Menus

**Shops**
- New shop menu to buy and sell items, weapons, armor, accessories and materia, opened by the field scripts. The installer reads the inventory of each shop from the game executable.
- New console command `/shop <shop_id>`.
- Fixed the ids and prices of the accessories.

**Materia**
- The Check, Order and Exchange options of the materia menu work.

**Party, items and saving**
- The Order option of the main menu swaps party slots and rows. The character controlled in the field is set by the scripts, saved with the game, and doesn't change with the party order.
- Locked characters can change slots, but can't leave or join the party.
- Item menu: Order > Custom swaps items by hand.
- The save menu asks before saving over a slot and confirms when the game is saved.
- New console command `/phs <character_id|all> [on|off]`.
- The console is drawn over the menus.

### Battle

**Rules**
- Enemy HP and MP are read from the game data. Enemies pay the MP of their attacks, and fail the ones they can't pay.
- Source bonuses count in the stats (capped at 255). Equipped materia and accessories modify the stats.
- Accuracy and evasion fixes, including magic hit chance, Darkness, evade modifiers and several commands.
- Per character divisors for the limit bar.
- Fixed MP usage and spell strength with support materia.
- Only felled enemies give rewards.

**Characters**
- Characters level up with the growth curves of the original game.
- Recruits get their level from the party average, and reused characters get their own data first.
- Fields can enable every character in the PHS, not only the first two. Hiding a character removes it from the party.

**Statuses**
- Silence, Frog and Small restrict the actions they should.
- Only physical hits cure Sleep, Confusion and Manipulate. Haste and Slow replace each other, and Petrify removes Sleep, Poison, Confusion and Regen. Actions of combatants that can't act wait until they can.
- Confused combatants attack or cast a random spell at the opposite side. Berserk party members only use the plain Attack.

**Cameras and animation**
- Fixed the camera axes, and the depth, aim and scale of the scripted attack cameras.
- Items and limits use their own cameras. New camera instructions (orbit, target group, single target and model view).
- Added the camera at the start of a battle, the victory camera, and the shots and view changes of the enemy animations.
- Fixed the scale of party members, enemies and backgrounds.
- Unknown animation sequences are skipped.
- Fixed consecutive battles.

### Field

**Installer and decompiler**
- New opcodes: `SMTRA` (give materia), `GTPYE` and `SPTYE` (store and restore the party), `MENU` 17 (unequip materia), `MINIGAME` (skips to the return field), `MVCAM` (movie camera), `MUSVT` (stop music), `WNUMB` (number windows), `SLINE` (move a line trigger) and party requests by party slot.
- Fixed the menu lock, byte arithmetic wrapping, `KAWAI` sub-opcodes (party members could wait forever), and scripts falling through after `RETTO`.
- Fields no longer fail to convert because of a request to a missing script. frcyo and del3 are no longer skipped.
- Fixed the intensity of additive tiles. Music tracks 14, 64, 87 and 96 are converted.

**Engine**
- Touching an entity runs its contact script. Fixes to script priorities, movement and turn syncs, scripted walks off the walkmesh, player entity changes and party followers.
- Fixed the camera limits of off-center ranges and the height of characters in some scripted walks.
- Backgrounds hold up to 4096 tiles of each kind, and tile intensity scales their colour.
- Movies can be drawn behind the models, and fades don't cover them.
- Dialogs handle long texts and number windows, and text pauses are counted in frames.
- The party and save menus can be opened by scripts. The pause menu doesn't open when a script reads Start.
- `/map` uses the field list, and `/screenshot` saves PNG files.

### World map

- Entering a field from the world no longer makes the player walk on their own.
- Flags set by fields are seen by the world scripts.
- The Warship scenes after jundoc1a and shpin_3 play.
- Placed models face the right way.
- Exits back to the world map put the player where they entered.

### Build and documentation

- The tests build again.
- Updated the README and the build, run and authors documentation. `INSTALL.md` was removed.

## 0.3.5

Changes since 0.3.4. Most of the work is in the world map module, which now runs the original world scripts, and in the field module and its installer.

### World map

**Story and scripts**
- The world script decompiler handles more opcodes (0x11d, 0x31b, 0x33c, 0x355, 0x01b), and fixes `Write(Read(N), v)` stores and address collisions between local, byte, word and bit banks. The scripts of the animated world models now decompile correctly.
- The world memory shares the field memory banks, so the story counter and the vehicle flags set by the field scripts are visible to the world scripts.
- The world situation (Midgar era, cannon destroyed, Ultimate Weapon killed...) is chosen from the story and flags instead of a debug value.
- Vehicles the story has not given yet are hidden; the positions in the scripts are used as spawns.
- Scenery models are placed from the world scripts. New bindings `get_animation_name`, `play_animation_index` and `set_default_animation_index`.
- Entering from a field: the world script selects the map and the player spawn; the player appears at the exit point set by the field script. `vehicles.lua` is reloaded on entering the world and on returning from a field.
- The installer writes the world map exit locations.

**Settings**
- New `world_settings.lua`: speed and camera (distance and pitch) per vehicle, walking speed, music tracks and overrides, and town entry rules.
- Music depends on the vehicle and on the story progress.
- Towns can only be entered under the conditions of each vehicle.

**Visuals and mechanics**
- Animated water textures in WM0.
- Fog in WM2, and WM2/WM3 texture mapping fixed with the texture table.
- WM3: snow overlay, camera behaviour and poles.
- Cliff terrain can't be walked on foot.
- Faster search of the triangle under a position (spatial grid).

### Field

**Decompiler and installer**
- Entities without a model (Director, AD, MES) are handled, so the model opcodes are harmless on them; names that aren't valid Lua identifiers are skipped.
- `LSTMP` fixed: the previous field id is stored from each script header, and a destination in bank 0 is guarded.
- Scripts that fall through into the next one now call it explicitly.
- New opcodes `PREQ`, `PRQSW`, `PRQEW` (`party_request`) and `CKITM` (key item check).
- The Director uses the entity named "cloud" for `place_player_at_map_entry` and autoscroll.
- Text colour tags left open at the end of a dialog are closed.

**Engine**
- Background palettes (`BackgroundPalette`): the palette opcodes repaint the tiles of the animated palettes (for example waterfalls).
- Background scrolling: scripted scrolls are not clamped, scrolls that follow an entity are clamped to the range.
- Changing the player entity moves the autoscroll to the new player and copies the position of the previous one if the new one has none.
- Single-frame animations are held, and a running offset is cancelled by `AT_NONE`.
- Additive screen fade (type 6) fixed.
- Timer dialog windows: rendering and padding.
- Movies without a sound file are skipped quietly. `play_sounds` takes optional arguments.
- `field.lua`: `party_request` helper; fixed the map change race that erased a request made in the first frame of a field.
- `system.lua`: `NullEntity`; `Timer` updates the dialog.

## 0.3.4

Changes since 0.3.2. Most of the work is in the battle module, which now uses data extracted from the original game for cameras, effects and rules.

### Battle

**Rules**
- Damage formulas checked against ff7.exe: physical, magical, cure, critical, row, Berserk/Frog/Defend/Sadness and level attacks.
- Physical and magical hit/evade formulas fixed, including luck, auto-hit statuses and Fury. Armor defense, magic defense and evasion now count for party members.
- Statuses with timers, Fury/Sadness (limit bar speed and colour) and Near Death.
- ATB speed and turn order, counterattacks and action priority.
- Change and Defend side commands.

**Cameras**
- Original battle camera scripts are extracted by the installer (`gamedata/battle_cameras.lua`) and interpreted by `battle_camera.lua`: camera chosen per attack or enemy attack, file chosen by battle layout, random variants, smooth return to the formation camera.
- `effect_camera.lua` runs the summon camera scripts.
- Upright battle camera, with the UI compensation scaled with distance.

**Effects**
- Summons Shiva, Ifrit and Ramuh built from the original data: morphing ground/mountain meshes, sprite animations, ice spikes that shatter with the exe's physics, storm, snow, crystals, debris and screen shake.
- Effect sprites are anchored to the combatant so they follow the moving camera.
- Caster auras: stacked, expanding light tubes with the original textures and sounds, for spells, limits, enemy skills and summons.

**Materia**
- Linked support materia (adjacent slot pairs): All, MP Turbo, HP/MP Absorb, Quadra Magic, Counter, Magic Counter, Final Attack, Sneak Attack, Added Cut, Steal as well, Elemental, Added Effect.
- Independent materia: Cover, HP<->MP swap, encounter rate, EXP/Gil Plus.
- Magic list with one row per materia and modifier tags (ALL, MPT...).
- HP and MP are capped when the equipment changes.

**Commands**
- Steal, Mug (formulas fixed), Sense (now shows elements and statuses), Throw, Coin (gil window), Morph, D.blow, Manipulate and Mime.
- Double commands W-Magic, W-Summon and W-Item.
- Attack commands 2x-Cut, 4x-Cut, Slash-All and Flash.
- E.Skill lists and casts the skills learned by the equipped Enemy Skill materia.

**Limits**
- Limit bar filling, flashing and reset on death; Limit command and menu; pink limit windows and messages; Aerith's and Red XIII's limits.
- Limit attack data found in ff7.exe and written by the kernel installer; non-physical limits read from the kernel data.
- Limit animations extracted from magic.lgp and added to the party model; limit aura.
- Multi-hit limits (Omnislash...) hit as many times as the data says, spread over the animation, at random enemies when flagged.
- Limit progression: kills are credited to characters and each level's first technique is learned from the kernel kill thresholds.

**Engine and scripting**
- `get_camera()` script binding for the formation camera; effect entities hidden from `/debug_entity`.
- Enemy bindings `get_morph`, `get_manip_count`, `get_manip_attack`; the enemy XML reads the manipulate attack list.
- `EntityModel` supports effect models.
- Test helpers: debug enemy HP and debug party setups.

### Field

- Fixed a crash (eals_1) when a dialog started with a character-name tag: `TextHandler::GetDialogText` now builds the text from all parts of the node.
- Decompiler: the check that stops the events of a line while it is off now also covers `on_near_once`.
- Debug text no longer scrolls wrongly: `DebugDraw` resets the world, view and projection matrices before drawing screen-space text and quads.

### World map

- No changes.

### Menu

- Minor change in the materia menu script to support the new battle materia.

### Installer

- Extracts battle camera scripts, effect camera scripts, morphing meshes and ice shard meshes from ff7.exe.
- Effect sprite frames written with the exact `.s` file layout.
- Item, armor and accessory throwable flags, attack additional effects, limit animation conversion and item status decoding fixes.
- Fixed fuzzy match problems.
- Fixed a data race in `InstallLog` that crashed the installer.
- Removed leftover movie-frame, audio player and scroll diagnostics from logging.

### Tests

- Added `TextHandler` text extraction tests (plain, character tag first, other tags) and a decompiler case for the events of a line that is off.
