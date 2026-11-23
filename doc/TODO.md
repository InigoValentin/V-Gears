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
