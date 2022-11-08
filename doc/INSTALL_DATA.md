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

## Install FFmpeg

The installer uses [FFmpeg](https://ffmpeg.org "FFmpeg") to convert sound effect files. To do this, it uses the ffmpeg executable in the system, and the sounds files will not be properly installed if there is such executable. In most GNU/Linux distros, ffmpeg is available as a package, so you can install it with your package manager. For the rest of cases, see [the downloads page](https://ffmpeg.org/download.html "FFmpeg download page").

It's important that the ffmpeg executable is located in a globally accessed location. To verify this, try to run this command from a console: `ffmpeg -version`. If FFmpeg version is show, you are all set. If not, you must include the path to the ffmpeg executable to your system PATH environment variable.

If you can't (or don't want to) install FFmpeg in your system, a workaround is explained later.

## Install TiMidity

The installer uses [TiMidity](http://timidity.sourceforge.net/ "TiMidity") to convert the original music tracks. To do this, it uses the timidity executable in the system, and the tracks files will not be properly installed if there is such executable. In most GNU/Linux distros, timidity is available as a package, so you can install it with your package manager. For the rest of cases, see [the downloads page](https://timidity.sourceforge.net/#download "TiMidity download page").

It's important that the timidity executable is located in a globally accessed location. To verify this, try to run this command from a console: `timidity -v`. If TiMidity version is show, you are all set. If not, you must include the path to the timidity executable to your system PATH environment variable.

If you can't (or don't want to) install TiMidity in your system, a workaround is explained later.

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

### Workaround: FFmpeg is installed, but TiMidity is not.

If your system has FFmpeg installed, but not Timidity, you will stil get the sound effects, but V-Gears will not be able to play background music unless you find a way to convert mid files to ogg. If you know of one, here is wht you can do:

- In the installer, open the advanced options.
- Check the options 'Prevent TiMidity calls' and 'Keep originals'.
- Proceed with the installation normally.
- Once is done, use your own method to convert the `.mid` files in `~/.v-gears/data/audio/music/` to `.ogg` files, while keeping the same name.
- (Optional) When done, mid files can be safely deleted.

### Workaround: FFmpeg is not installed.

If your system doesn't have ffmpeg installed, the instaler will not be able to convert sound effects or background music. If you have a way to convert `.mid` and `.wav` files to `.ogg`, there is still something you can do:

- In the installer, open the advanced options.
- Check the options 'Prevent FFmpeg calls' and 'Keep originals'.
- Proceed with the installation normally.
- Once is done, use your own method to convert the `.mid` files in `~/.v-gears/data/audio/music/` to `.ogg` files, while keeping the same name.
- Use your own method to convert the `.wav` files in `~/.v-gears/data/audio/sound/` to `.ogg` files, while keeping the same name.
- The installation disk has 4 `.wav` files that you will also need to convert to `.ogg`. They are located in `data/music/`. For each one, you can find the corespondant name in the file `~/.v-gears/data/musics.xml` (it wil be one or two digits). Convert the tracks to `ogg`, with that name, and place them in `~/.v-gears/data/audio/music/`.
- (Optional) When done, mid and wav files can be safely deleted.

## Advanced options

The installer has a list of advanced options. They have tooltips with short explanations of what they do. For a fresh installation, there is ususlly no nned to check any advanced options (unless you are using one of the workarounds described below).

For subsequent installations, it's generally usefull ti check the options 'Don't extract sounds' and 'Don't extract music'. By doing so, music and sounds will not be reinstalled, and the installation wil shorten by afew minutes.


## Next steps

- [Run V-Gears](RUN.md "Run V-Gears")