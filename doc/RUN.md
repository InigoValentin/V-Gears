# <img src="../v-gears.png" alt="V-Gears" width="50"/> V-Gears Run Guide

## Run the engine

If you have followed the [build](BUILD.md "Building guide") and [installation](INSTALL.md "Installation guide") guides, you'll have the game data installed to `~/.v-gears/`, and the binaries compiled somewhere (we'll assume they are in `~/V-Gears/build/bin`, change the directories according to your system).

Run the engine from any directory:

```Bash
~/V-Gears/build/bin/v-gears
```

If you installed V-Gears from a package, run `v-gears`, or start it from the applications menu.

The engine looks for its data directory in this order, and changes into it:

1. The directory given with `--data-dir`, for example `v-gears --data-dir ~/games/v-gears`.
2. The directory in the `VGEARS_DATA_DIR` environment variable.
3. The current directory, if it has the V-Gears files (`plugins.cfg` and a `data` folder).
4. `~/.v-gears/` (`%USERPROFILE%\v-gears\` in Windows), the same default as the data installer.

If the chosen directory doesn't have the V-Gears files yet, the engine copies them there from the `data/` folder of the repository (or from `share/v-gears` in an installed package; `VGEARS_TEMPLATE_DIR` overrides it). Files that are already there are never overwritten. The data of the original game still has to be added with the data installer.

The data installer also has a **Launch game** button, which starts the engine with the installation directory.

## Controls

The default bindings are in `config/config.cfg` of the installation directory, in the `bind_game_event` lines. The game reads them as events, so the menus, the field and the battles share them. They can be changed there.

|Key|Event|Action|
|--|--|--|
|Arrow keys|Up, Down, Left, Right|Move, move through menus|
|Enter, Space|CONFIRM|Confirm, talk, interact|
|Esc|CANCEL|Cancel, go back|
|M|MENU|Open the main menu|
|Backspace|EXTRA|Extra action in some menus|
|P|START|Start|
|V|SELECT|Select|
|Page Up / Page Down|L1 / R1|Switch tab or character in menus|
|Left Ctrl|run|Run in the field|

The settings made in the in-game config menu (controls, sound, etc.) are saved with the game settings and loaded on start.

<!-- TODO(review): verify the table against the in-game config menu (control remapping), and check the gamepad status. -->

## Debug keys

These are also defined in `config.cfg`:

- **F11**: pause / resume the game timer. **Num +** / **Num -**: change the game speed.
- **F12**: take a screenshot.
- **Left Ctrl + Left Alt** plus **S, F, E, T, W, G, A, B, U, M**: toggle the debug displays (script, FPS, entities, triggers, walkmesh, grid, axis, background, UI, messages).
- **F8** / **F9**: show / hide the begin menu.
- **Alt + F4**: quit.
