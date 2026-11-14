# <img src="v-gears.png" alt="V-Gears" width="50"/> V-Gears

## What is V-Gears? 

V-Gears is a free implementation of the game Final Fantasy 7 engine, designed to work with the original game data.

It's designed to run the original game data on modern operating systems, making the game playable without the need of emulators.

It's free software, and it's designed to be cross-platform. It, of course, doesn't include any copyrighted data from the original game, but it provides an installer to get the required data from your own PC copy of the game.

It's a continuation of [q-gears](https://github.com/q-gears/q-gears "Q-Gears on Github"), which is an enormous project started by some great people many years ago, with the same purpose as this, but which unfortunately became stalled.

## Current status

Note that, while these are actual screenshots of V-Gears, none of the images, sprites, textures, backgrounds, etc are part of the project. They have been extracted locally from an original Final Fantasy VII game, and are in no way distributed here.
### Field maps:

Most of the field maps are working. Some are buggy, some have their own quircks, and some can even crash the whole game, but overall, almost oll of them are playable. Most models, animations and opcodes are extracted and implemented. Background music work, and FMVs can be played
|**Cloud model**|**Dialogs**|**Animations**|
|--|--|--|
|[<img src="https://v-gears.inigovalentin.com/img/screenshot/cloud.png" alt="Cloud model" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/cloud.png "Cloud model")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/dialog.png" alt="dialog" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/dialog.png "Dialogs")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/animations.gif" alt="animation" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/animations.gif "Animations")|
|**Transparencies**|**Timers**|**Choices**|
|[<img src="https://v-gears.inigovalentin.com/img/screenshot/transparent_dialogs.png" alt="Transparency" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/transparent_dialogs.png "Transparency")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/timers.png" alt="Timers" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/timers.png "Timers")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/choices.png" alt="Choices" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/choices.png "Choices")|

### Battle:
Battles are... playable. Battle models and animations are implemented. The battle commands show according to the equiped materia. Basic attacks and magic are working, and some spell effects are shown. But there is atill a lot to fix: Most formulas, a lot of commands, summons...
**Battle Flow**|
|--|
[<img src="https://v-gears.inigovalentin.com/img/screenshot/battle.gif" alt="Battle Flow" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/battle.gif "Battle Flow")|

### Menus:

All menus are implemented and working. They behave exactly like the originals. 

|**Begin**|**Main**|**Name**|
|--|--|--|
|[<img src="https://v-gears.inigovalentin.com/img/screenshot/begin_menu.png" alt="Begin Menu" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/begin_menu.png "Begin Menu")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/main_menu.png" alt="Main Menu" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/main_menu.png "Main Menu")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/name_menu.png" alt="Name Menu" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/name_menu.png "Name Menu")|
|**Items**|**Key Items**|**Equip**|
|[<img src="https://v-gears.inigovalentin.com/img/screenshot/item_menu.png" alt="Item Menu" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/item_menu.png "Item Menu")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/item_menu_key.png" alt="Key items menuy" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/item_menu_key.png "Key items menu")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/equip_menu.png" alt="Equip Menu" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/equip_menu.png "Equip Menu")|
|**Materia**|||
|[<img src="https://v-gears.inigovalentin.com/img/screenshot/materia_menu.png" alt="Materia Menu" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/materia_menu.png "Materia Menu")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/save_menu.png" alt="Save Menu" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/save_menu.png "Save Menu")||

### World map:

The world map is bareli playable. The world map models are extracted and can be displayed. The world map is navigable, and most cities can be entered. Most vehicles are implemented and can be used, but there are still a number of bugs

|**Worldmap**|**Dialogs**|**Animations**|
|--|--|--|
|[<img src="https://v-gears.inigovalentin.com/img/screenshot/world_map.png" alt="The worldmap and the vehicles" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/world_map.png "The worldmap and the vehicles")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/highwind.png" alt="The Highwind, mid-flight" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/highwind.png "The Highwind, mid-flight")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/worldmap.gif" alt="World map working" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/worldmap.gif "World map working")| 

### Installer:
The installer is the tool that extract the data from the original game
|**Installer**|**Advanced**|
|--|--|
|[<img src="https://v-gears.inigovalentin.com/img/screenshot/installer.png" alt="Installer" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/installer.png "Installer")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/installer_advanced.png" alt="Advanced options" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/installer_advanced.png "Advanced options")|

### Debug Tools
|**Field maps**|**Walkmesh**|**Console**|
|--|--|--|
|[<img src="https://v-gears.inigovalentin.com/img/screenshot/debug_dialog.png" alt="Debug fields" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/debug_dialog.png "Debug Fields")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/walkmesh.png" alt="Walkmesh" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/walkmesh.png "Walkmesh")|[<img src="https://v-gears.inigovalentin.com/img/screenshot/console.png" alt="Console" width="250"/>](https://v-gears.inigovalentin.com/img/screenshot/console.png "Console")|

## In the meantime...

If it picked your curiosity, you can check out the [history](doc/HISTORY.md "V-Gears history") behind the project. Or, if you want to try it for yourself, maybe the [build](doc/BUILD.md "Building guide") and [installaton](doc/INSTALL.md "Installation guide") guides are a good place to start.

Or even better, if you want to see this done and want to contribute, clone the repo, read the [style guide](doc/STYLE.md "V-Gears C++ Style Guide") and have a go at it!

