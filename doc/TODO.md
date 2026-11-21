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
