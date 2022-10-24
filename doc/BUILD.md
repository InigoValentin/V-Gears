# <img src="../v-gears.png" alt="V-Gears" width="50"/> V-Gears Building Guide

## Install dependencies
Install required dependencies by runing the script `CIBuild/linux/dependencies.sh`. Alternativelly, install them manually by running as root

```bash
apt install g++ cmake libqt5widgets5 qtbase5-dev zlib1g-dev libogre-1.12-dev libois-dev libvorbis-dev libboost-dev libboost-test-dev libboost-filesystem-dev lua5.2 liblua5.2-dev libluabind-dev luajit libopenal-dev libtinyxml-dev
```

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