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

Game controllers are supported.

As yet it has no sound and no tape emulation, so it can only load snapshots. It supports pok files so you can load pokes
for games and enable/disable them on-the-fly if you want to cheat!

It works with quite a large number of Spectrum programs, but there are definitely quite a few that fail. The Z80 CPU
core definitely has some bugs in it, there's at least one instruction that is not emulated correctly, albeit I've yet to
track down the actual culprit(s).

There is also a disassembler, an interpreter and a simple viewer for Spectrum screenshots (.scr files). An assembler
will be in the works shortly. Tape emulation is in the works, and Interface 1 (including microdrive) emulation is being
considered. Sound emulation is also being considered but is further outside my comfort zone than most other features.

The graphical UIs use the [Qt](https://doc.qt.io/qt-5/) framework. They should adapt to your desktop theme. For Windows
and MacOS where there is no concept of a system-wide icon theme, the UI uses icons from the KDE Breeze icon set. These
icons are licensed under the [LGPL V3](https://www.gnu.org/licenses/lgpl-3.0.html) (see
[COPYING-ICONS](https://github.com/KDE/breeze-icons/blob/master/COPYING-ICONS)) by the
[KDE Visual Design Group](https://community.kde.org/Get_Involved/design) and sources for all the icons used are in the
`src/spectrum/icons/`. The original sources are maintained at
[https://github.com/KDE/breeze-icons](https://github.com/KDE/breeze-icons).

This repository is currently a little untidy and needs cleaning up a bit. Apologies ;)
