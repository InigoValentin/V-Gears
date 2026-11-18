# Changelog

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
