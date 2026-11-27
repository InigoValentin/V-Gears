# TODO

Notes of things found while playing through the fields that still have to be reviewed.

## Long pauses between messages in chrin_1b

Some scenes in chrin_1b (the opening, and the ones after it) wait a long time between messages, so
it looks as if the game had frozen. Not investigated yet.

Suspects:

- The script scheduling in `ScriptManager::Update`: a script that yields with `wait(0)` now lets the
  next script of its entity run in the same cycle.
- The priority of the talk script, `INTERACT_SCRIPT_PRIORITY` in `EntityManager.cpp`.
- The durations of `WAIT` and `script:wait` that the installer converts (frames to seconds).

First step: play the scene with log level 3 and look at the time between the `dialog_open` and
`Start script` lines.

## Background parallax

`BGPDH` and `BGSCR` are converted, but the meaning of their values (ratio = distance / 4096, speed in
1/256 of a pixel per frame, sign of the speed) has not been checked against the original game. The far
layer of mds5_4 has no such opcodes and still scrolls with the rest, and it is not known why it should
not.

## Battle: recruits that remain untested

Yuffie, Vincent and Cait Sith remain untested: the level they join with (`Characters.join` in `system.lua`)
and, for Vincent and Cait Sith, the data they get when they are named (`Characters.load_recruit_record`). The
other characters are working fine.

- Yuffie's level comes from the variable at bank 1, address 2. Only the field blackbg4 writes it, so it is not
  known what sets it in a normal game. While it is 0 the average level of the party is used, which is not from
  the original.

## Battle: special limits

Not done yet (kernel data in ff7.exe, ids 170-176, see `get_special_limit` in `battle_ui.lua`):

- Cait Sith's Slots and Dice (additional effects 8 and 15, formula 6 with power 255): they need a small
  interactive piece and the random effects of the reels.
- Vincent's transformations (additional effect 3 with modifiers 0 to 3): they change the model, the
  stats and the commands during the battle.

## World map: culling of the terrain geometry

The terrain of WM0 is built as one `ManualObject` per texture for the whole map (`CreateWorldMap`), so
its bounding box is the whole map and Ogre never culls it: every frame draws all of its ~142,000
triangles in ~320 batches, however little of it the camera sees. With the entity debug off it runs at
about 110-140 fps in the tests. The search of the triangle under a position was the bigger problem and
is solved (spatial grid, `BuildTerrainIndex`).

Not planned for now. If it is needed: build the geometry per block or mesh (one object per mesh and
texture), so the frustum culling skips the ones that are out of view, and measure the number of batches
afterwards, because it can grow. Loading and unloading blocks around the player is the next step after
that, but the whole map fits in memory, so it should only be for the load time. The 6 alternate blocks of
WM0 (story shapes) are not loaded now and would have to be handled.

## Field: black corners of diagonal backgrounds (mds5_3)

In mds5_3 the top-right (and bottom-left) corner of the screen can show black when the player walks
near the edge. The data is the same as in the original: layer 0 has 777 sprites in a diagonal band of
816x432 pixels, layer 1 has 531 sprites, layers 2 and 3 are disabled, and the installed `bg.xml` keeps all
of them. The camera range (from the trigger data) is the full rectangle around the art, so the camera can
reach the unpainted corners.

Not known: whether the original game shows the same black corner. It was left as it is. If it needs a fix,
shrink the camera range of such fields so the screen never shows the empty tiles, but that would differ
from the original camera. Other fields with diagonal art may be affected too.

## Field: palette animations that are not implemented

The palette opcodes that fields use for waterfalls, lights and fades are implemented (`STPAL`, `LDPAL`,
`CPPAL`, `ADPAL`, `MPPAL`, `MPPAL2`): the installer writes `palette.bin` for the fields whose script loads
colors into a palette (`LDPAL`), and the engine paints the tiles of those palettes again (see
`BackgroundPalette`). These are still comments in the generated scripts: `STPLS` and `LDPLS` (partial
palettes, 5 uses), `CPPAL2` (8), `ADPAL2` (22), `RTPAL` (1) and `RTPAL2` (0). Their handlers are in
ff7.exe next to the others (0x61a9b8, 0x61ab52, 0x61ad32, 0x61b3e9, 0x61ae43, 0x61af9c).

Not checked in the game for every field: the opcode arithmetic comes from the exe, but only the
waterfall of eals_1 was used as a test. The first palette page may not be the "VRAM palette id" in
every field (it is assumed that the id is the index of the page in the field palette file).

## Field: opcodes that are still not complete

- `KAWAI` (eyes, transparency, light, shine...) and `KAWIW`: none of the `KAWAI` operations is implemented (for example `EYETX`, which closes and opens the eyes of the models in ealin_2), so `KAWIW` has nothing to wait for. The converted models may not even keep the eye textures.
- `CMUSC` changes the music at once. The original fades the current music out and the new one in (the fade parameters are in the call, see `change_map_music`). A change without fades is played once so the `CHMST` loops that wait for it end, which is what the inns need, but a change that should stay would be played once too.
- `SLIP` is implemented for the player only, as "do not slide along the wall when the move is blocked".
- Scripts that read the Cancel button (key 64) also see it pressed while the "run" key is held (see `EntityManager::IsKeyOn`).

## Field: dialog text codes that are not converted

The variable code (`FE DE`, a number given by `MPARA` or `MPRA2`) is implemented. These codes still show as
`[MISSING 0xFE xx]` in the converted dialogs: `E1` (about 30 uses, it looks like a tab or spacing, as in
"Current GP[E1] GP"), `E2` (7 uses, button icons, followed by characters such as `@#&`) and `E9` (32 uses,
it looks like a spacer, as in "1BET [E9]100[E9]gil"). Their meaning has not been checked. No documentation
for them was found.

## Field: colne_3, the guards can trap the player

After the cutscene the guards (ZAKOB, ZAKOc, ZAKOD, ZAKOE) chase the player with the original loop (move
to the player, repeat) until the scene ends, which is intended: the way out is to talk to three of them
and then to SOTCH. Their speed was measured (about 0.22 units/s against 0.7 of the player walking), so
they should not catch a moving player, but the player has been trapped a couple of times in the game
(probably surrounded by solid models). Left as is. If it is looked at again: check how the player
collides with several solid NPCs that stop around it (`CheckSolidCollisions`).

## Field: pillar_3, Ren not visible (to test)

The model of Ren was invisible. The probable cause (an instant `OFST` did not cancel a running smooth
offset, so Ren stayed 4 units away) was fixed in `Entity::ScriptOffsetToPosition`, but it has not been
tested in the game. Check the helicopter intro of pillar_3 and that Ren appears and jumps down.

## World map: the exit of sichi

When the player leaves sichi he appears where he entered (the exit script puts him at the corner of the mesh, which is a mountain, so `WorldVM.prepare` uses the exact position saved by `EnterFieldScene`). The original game position for this exit is not known. The direction that is saved (special 4) is not used for the player.

## World map: the scenes of the Warship (what is missing)

The scenes after jundoc1a (record 36) and shpin_3 (record 37) work: `WorldVM.start_scene` in `world_vm.lua` runs the
function of the Warship (type 8) as a script of the entity, with `Wait`, speed and direction (the speed is
`speed / 8192` of a mesh each frame, from ff7.exe; the scripts run at 15 frames per second, which is what the user
remembers of the original). Not done:

- `PlaySoundEffect(381)` at the start is a stub (the sound id is not mapped to an audio file).
- `kernel_unknown_0x319`, `0x31C`, `0x31F`, `0x329`, `0x32A` and `0x32F` are stubs: they may be camera moves, fades or
  sound state, and the original may use them to frame the ship or fade out at the end. They can be decoded from the
  jump table at 0x75bfb9 of ff7.exe (kernel dispatcher 0x75b14c).
- The records 38 and 39 also have a Warship scene in `world_function_64`; it is not known which fields lead to them and they
  were not tested. The last move of record 37 (direction 176, facing 48) is a plain move without turning.
- The vehicles may run at 15 frames per second as well (their speeds were tuned by eye).

## Fonts: possible improvements

The fonts are built by the installer from WINDOW.BIN (see `WindowFonts`) and drawn as a texture atlas: the PNG is loaded once, each letter is a quad with its cell as texture coordinates, and the geometry is only rebuilt when the text or the position changes. Converting the fonts to TTF was considered and discarded: an engine that loads TTF files rasterizes them into the same kind of atlas, the FF7 font is a two colour pixel font (glyph and shadow) that an outline format can't keep, and the installer would need a font writing library. Possible improvements, not planned yet:

- `UiFont::GetCharData` searches the list of characters for every letter each time the geometry is rebuilt. An array indexed by the character code would make it a direct lookup.
- The PC version has a font twice the resolution (24x24 cells) in `usfont_a_h.tex` and `usfont_b_h.tex` of menu_us.lgp, split in two textures. The installer could merge them in one image and write the XML for it, for sharper text at high resolutions.
- The installer could write `ffvii_en.xml` from the width table in WINDOW.BIN (third file, one byte per character: 3 bits of lead, 5 bits of width) instead of keeping it by hand. The XML in the repository matches the table except for `! " , . : ?`, which were made tighter by hand.
- Loading TTF files makes sense only to support fonts made by users, not for the fonts of the game.

## Changing the resolution breaks the display

The console command `/resolution <width> <height> [full screen]` leaves menus, fields, the world map and battles broken. Screenshots (`/screenshot`) work. It doesn't seem new: the resize functions (`CmdResolution`, `GameFrameListener::windowResized` and the `OnResize` of the managers and widgets) are the same as before the code reorganization, but it hasn't been compared in the game with an older build.

To check, not verified: whether `windowResized` is called at all when the size is changed with `Ogre::RenderWindow::resize` or `setFullscreen`, and not by the window system. Several `OnResize` functions are also empty (battle, world map, dialogs).
