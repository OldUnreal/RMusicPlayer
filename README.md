# Introduction

This is OldUnreal's fork of RMusicPlayer. The goal of this fork was to fix some
bugs, update RMusicPlayer's build system, and port RMusicPlayer to a more recent
version of FMOD.

# Setup

You should clone this repository into your Unreal/UT root folder:

```
cd UnrealTournament
git clone https://github.com/OldUnreal/RMusicPlayer.git
```

# Building RMusicPlayer

Our fork uses CMake as the build system on all platforms. The CMake scripts in
this repository are based on the scripts for UT 469. Our build system can target
multiple operating systems (Windows, Linux, macOS) and architectures (x86,
amd64, arm, arm64). We have only tested this fork for Windows/x86.

## Dependencies

RMusicPlayer uses FMOD Engine as its Audio backend. We included a copy of FMOD
Engine in the External folder.

## Main Project

Building the RMusicPlayer is quite straightforward. You should navigate to your
RMusicPlayer folder, create a build folder, run CMake to generate project files,
build the binaries, and install them into the UnrealTournament/Unreal System
folder.

## Windows/x86

On Windows, you should use the Visual Studio command prompt and run the
following commands. Don't forget to update the path to your System folder!

```
cd \path\to\UnrealTournament\RMusicPlayer\
mkdir cmake-build
cd cmake-build
cmake -A Win32 -DCMAKE_INSTALL_PREFIX=\path\to\UnrealTournament\System -DCMAKE_TOOLCHAIN_FILE=cmake\MSVC.cmake ..
cmake --build . --config Release --toolchain cmake\MSVC.cmake -j 8 --target install
```

## Linux/x86

Although we have not tested RMusicPlayer on Linux, you should be able to build
it as follows. Once again, don't forget to update the paths in the commands
below!

```
cd /path/to/UnrealTournament/RMusicPlayer/
mkdir cmake-build
cd cmake-build
cmake -DCMAKE_INSTALL_PREFIX=/path/to/UnrealTournament/System ..
cmake --build . --config Release -j 8 --target install
```

# Original README

RMusicPlayer's original readme can be found at: http://wiki.beyondunreal.com/UE1:RMusicPlayer
