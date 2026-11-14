# <img src="../v-gears.png" alt="V-Gears" width="50"/> V-Gears Installation Guide

## Manual installation

The data installer can do this for you: enable "Copy V-Gears data files" in its advanced options and it copies the `data/` folder of the repository to the installation directory. To do it by hand:

1. Create a folder for V-Gears data, outside the project. In Linux-based systems, it's recommended to use `~/.v-gears/`.
2. Copy all the contents of the `data/` folder of the repository to the newly created directory. It should end up like this:
```
~/.v-gears/
  ├── data
  ├── field_models_and_animation_metadata.xml
  ├── plugins.cfg
  ├── plugins_d.cfg
  ├── resources.cfg
  └── resources_d.cfg
```
3. If you used a directory other than `~/.v-gears/`, start V-Gears from it, or pass it with `--data-dir` (see [Run V-Gears](RUN.md "Run V-Gears")).

If you skip these steps, V-Gears copies the files to `~/.v-gears/` itself the first time it runs.

## Next steps

- [Install data](INSTALL_DATA.md "Install Data")
- [Run V-Gears](RUN.md "Run V-Gears")