# <img src="../v-gears.png" alt="V-Gears" width="50"/> V-Gears Building Guide

## Install dependencies
Install required dependencies by runing the script `CIBuild/linux/dependencies.sh`. Alternativelly, install them manually by running as root

```bash
apt install libogre-1.12-dev libtinyxml-dev boost1.55 libvorbis-dev libalut-devlibfreetype6-dev libfreeimage-dev zlib1g-dev libzzip-dev libois-dev libglu-dev libqt5core5a libqt5widgets5 libpng
```
## TODO:
I will either explain tis, or, preferably, fix it so it isn't nedded.

## Build using CMake
```bash
mkdir build;
cd build;
cmake ../;
make;
```

TODO: Explain what is generated and where.

## Next steps

- [Install V-Gears](INSTALL.md "Install V-Gears")
- [Install data](INSTALL_DATA.md "Install Data")
- [Run V-Gears](RUN.md "Run V-Gears")