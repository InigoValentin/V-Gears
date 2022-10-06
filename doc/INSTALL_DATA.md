# <img src="../v-gears.png" alt="V-Gears" width="50"/> V-Gears Data Installation Guide

## Extract game data.

To install Final Fantasy VII game data, you'll nedd a PC version copy of if. Extract the contents of the installation dist to a folder anywehre you like. For this guide, we'll be calling that folder `FFVIIPC/`. Once extracted, verify that the directory structure contains the following subdirectories:
```
FFVIIPC/
├── battle
├── cd
├── field
├── kernel
├── menu
├── midi
├── minigame
├── music
├── sound
└── wm
```

## Launch the Installer

If you have followed the [build](BUILD.md "Building guide") and [installaton](INSTALL.md "Installation guide") guides, you'll have the game data copied to `~/.v-gears/`, and the binaries compilled somewhere (we'll assum they are in `~/V-Gears/build/bin`, change the directories according to your system).


On a terminal, do into the game data folder, and from there, run the installer

```Bash
cd ~/.v-gears/;
~/V-Gears/build/bin/v-gears-launcher;

```

## Run the Installer

In the 'Data Installer' tab, in the 'Import data source' field, select the folder where you extracted the game data (in this guide `FFVIIPC/`), and in the 'VGears Data Path', select the folder whith the game data (in this guide `~/.v-gears/`).

Then just click 'Install Data'.

The process shouldn't take too long. Dont' be discouraged by the huge amount of errors you might see, if at the end you see 'Installation finished', the data is succesfully installed.


## Next steps

- [Run V-Gears](RUN.md "Run V-Gears")