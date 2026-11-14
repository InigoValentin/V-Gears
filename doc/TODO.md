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
