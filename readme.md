# Spectrum

A ZX Spectrum emulator written in C++20.

This is primarily a personal project to explore a few ideas and to see if I could create an emulator from scratch. The
emulator can emulate most of the popular models of ZX Spectrum:

- Spectrum 16k
- Spectrum 48k
- Spectrum 128k
- Spectrum +2
- Spectrum +2a
- Spectrum +3

and it emulates some external hardware:

- ZX Interface 2 joysticks
- Kempston joystick
- Kempston mouse
- Fuller joystick
- Cursor joystick

Game controllers are supported for the Qt5 UI as long as the QtGamepad module is available. Otherwise, using the cursor
keys and CTRL (for the fire button) are your the only option for joystick emulation.

As yet it has no sound and no tape emulation, so it can only load snapshots. It supports pok files so you can load pokes
for games and enable/disable them on-the-fly if you want to cheat!

It works with quite a large number of Spectrum programs, but there are definitely some that fail. The Z80 CPU core
almost certainly has some bugs in it, and the interrupt timings are almost certainly off a bit.

There is also a disassembler, an interpreter and a simple viewer for Spectrum screenshots (.scr files). An assembler
will be in the works shortly. Tape emulation is in the works, and Interface 1 (including microdrive) emulation is being
considered. Sound emulation is also being considered but is further outside my comfort zone than most other features.

The GUIs (emulator and screen viewer) use the [Qt](https://doc.qt.io/qt-6/) framework. They should adapt to your 
desktop theme. Qt5 and Qt6 are supported; Qt6 is the default.

Gamepads are supported only in Qt5 because the QtGamepad module was not ported to Qt6. There are various third-party 
ports available, but I'm reluctant to depend on anything that isn't readily available in a Linux distro's package
manager or which adds to the build burden placed on the user. You're obviously free to fork and add support :). I may
look into other ways of supporting gamepads.

For Windows and MacOS where there is no concept of a system-wide icon theme, the UI uses icons from the KDE Breeze icon
set. These icons are licensed under the [LGPL V3](https://www.gnu.org/licenses/lgpl-3.0.html) (see [COPYING-ICONS](https://github.com/KDE/breeze-icons/blob/master/COPYING-ICONS)) by the
[KDE Visual Design Group](https://community.kde.org/Get_Involved/design) and sources for all the icons used are in the `src/spectrum/icons/` directory. The
original sources are maintained at [https://github.com/KDE/breeze-icons](https://github.com/KDE/breeze-icons).

This repository is currently a little untidy and needs cleaning up a bit. Apologies ;)

## Building

All build and run instructions assume your terminal's working directory is the root of the project.

### Qt6
Requires the headers and libraries for Qt6 (for the emulator and screen viewer) and
readline (for the interpreter).


#### Install build dependencies

Debinan/Ubuntu and derivatives:

    sudo apt update && sudo apt install qt6-base-dev libreadline-dev

Arch/Manjaro and derivatives:

    sudo pacman -Syyu && sudo pacman -S qt6-base readline

RHEL/Fedora/SUSE and derivatives:

    TBD

Gentoo and derivatives:

    TBD

Slackware and derivatives:

    TBD

#### Configure and build

This will build all components.

    cmake -B cmake-build -DUSE_QT6=1        # the argument to -B can be whatever subdir you want
                                            # strictly speaking -DUSE_QT6 is optional, but will be required if/when Qt7
                                            # is released and becomes the default version
    cmake --build cmake-build -j$(nproc)    # you can omit the -j option if you don't have nproc "cmake-build" is 
                                            # whatever subdir you used in the above command

### Qt5
Requires the headers and libraries for Qt5 (for the emulator and screen viewer) and
readline (for the interpreter).

#### Debinan/Ubuntu and derivatives

Install build dependencies:
    
    sudo apt install qtbase-5-dev libreadline-dev

#### Configure and build

This will build all components.

    cmake -B cmake-build -DUSE_QT5=1        # the argument to -B can be whatever subdir you want
                                            # strictly speaking -DUSE_QT6 is optional, but will be required if/when Qt7
                                            # is released and becomes the default version
    cmake --build cmake-build -j$(nproc)    # you can omit the -j option if you don't have nproc "cmake-build" is 
                                            # whatever subdir you used in the above command

## Running

Install the ROMs:

    cp -r roms ~/.config/Equit/

This directory will be changed soon, and it will be possible to specify the location of the ROMs and/or config on the
command-line (and to specify the ROM location in the config file).

Run the emulator:

    ./cmake-build/src/spectrum/Spectrum

## Installing

Package build files for popular distros will follow shortly.
