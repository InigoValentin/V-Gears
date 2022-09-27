# V-Gears

## What is V-Gears? 

V-Gears is an RPG engine for games like Squaresoft's Final Fantasy 7, Final Fantasy IX or Xenogears. It is designed to be cross-platfrom and run on modern operating systems such as Win32 and Linux.

See here for builds http://qgears.itscovarr.com here you will find nightly builds and Linux builds for every version. Since the Windows version only builds daily not every change will have a matching Windows build.

## How to build V-Gears?

### GNU/Linux

**Install required packages:**
```
* apt install git
* apt install build-essential
* apt install libvorbis-dev
* apt install libalut-dev
* apt install cmake
* apt install zlib1g-dev
* apt install libboost-all-dev
* apt install libogre-1.12-dev
* apt install libois-dev
* apt install libqt5widgets5
* apt install libtinyxml-dev
```

**Build the project:**
```
mkdir build
cd build
cmake ..
make
```

#### Building DEB package for Debian GNU/Linux and derivates

To do this use "cpack -C Debug" or "cpack -C Release" after running the make command.

### Windows

**Download and install the community version of Visual Studio.**
```
http://www.visualstudio.com/en-us/products/free-developer-offers-vs
```

***Download and install WIX toolset to generate MSI installers***
```
https://wixtoolset.org/releases/
```

**Download and install the Ogre SDK 1.12:**
```
https://www.ogre3d.org/download/sdk/sdk-ogre
```
Install/extract to "C:\OgreSDK\"

**Download and install cmake for windows:**
```
https://cmake.org/download/
```

Select "Add CMake to the system PATH for all users" - this makes things easier later on.

**Download boost 55:**
```
http://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.zip/download
```
Extract to C:\boost_1_55_0. 

Now we need to compile boost.

**Build boost 55:**

Open the Visual Studio 2013 command prompt. This can be found at "Start menu -> All programs -> Visual studio 2013 -> Visual studio tools" then you'll see "Developer Command Prompt for VS2013". Which is simply a shortcut for:

%comspec% /k ""C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\VsDevCmd.bat""

In the command prompt cd to the boost dir by entering:

```
"C:\boost_1_55_0" 
Then run
bootstrap.bat
Finally run (this may take a while to complete, you may pass -j8 here, where 8 is the number of cpu cores/threads to use):
b2 --toolset=msvc-12.0 --build-type=complete stage 
```

**Download the zlib dll version:**
```
http://zlib.net/zlib128-dll.zip
```
Extract to the root of C drive.


**Download Qt 4.8 source code:**
```
https://download.qt.io/archive/qt/4.8/4.8.6/qt-everywhere-opensource-src-4.8.6.zip
```
Again Extract to the root of C drive.

**Build Qt:**

Open a Visual Studio 2013 command prompt:

cd to extracted source location and enter:

```
configure -platform win32-msvc2013 -no-webkit -debug-and-release -no-phonon -no-qt3support -mp -no-scripttools -nomake demos -nomake examples -no-openssl 
```

Key in "o" to use the open source license and then "y" to agree. Once this is done enter "nmake" to finish compling Qt.

Now sync down the VGears source code, first create a directory some where you want to sync the code to using Windows explorer. Then right in the newly created directory and select "GIT Bash here".

In the git bash enter:
```
git clone --recursive https://github.com/v-gears/v-gears.git
```

**Building the VGears code:**
First launch a "Visual studio developer command prompt" (Found under "tools" in the start menu in the Visual studio directory" (you've already done this if you've got this far).

Now cd to where you synced the qgears source code to. For me this was "cd C:\Users\paul\Desktop\qgears\src"
```
mkdir build
cd build
set QTDIR=C:/qt-everywhere-opensource-src-4.8.6
set OGRE_HOME=C:/OgreSDK/OgreSDK_vc12_v1-9-0
cmake .. -DZLIB_ROOT=C:/zlib128-dll -DBOOST_ROOT=C:/boost_1_55_0
```

(Note how we use / and not \ in paths here).

Now you can open the newly created VGears.sln in Visual Studio and build Win32/Release or Debug. If files are added/deleted/or the depends change then repeat the set and cmake commands in the Visual Studio command prompt in order to update VGears.sln.

You can also build the sln file from the command line by entering "msbuild VGears.sln /m" in the Visual Studio command prompt.

To build the installer build the "PACKAGE" project from VisualStudio, this will generate an MSI, alternatively you can run it from the Visual Studio command prompt by entering "cpack -C Debug" (replace Debug with Release if building release).

# What next?

Now that you've got everything built see the wiki for how to run VGears, coding style, how to start hacking etc.

https://github.com/v-gears/v-gears/wiki
