# Spectrum v%1

## Quick start
- Click `Open` or drag and drop a Spectrum snapshot into the window to load.
- Click `Save` to save a snapshot of the current Spectrum state.
- Press `Shift` + `F1` to `F5` to save a snapshot to one of five quick-access slots.
- Press `F1` to `F5` to load a snapshot from one of five quick-access slots.
- Press `Esc` or `CTRL` + `P` to pause the emulator; press either key again to resume.
- Hold `Tab` to temporarily make the emulation run as fast as it can.
- Press `F12` to open and close the debugger.

## Models
All the main Spectrum models are emulated, from the 16K through to the +3. Choose the desired model from the **Spectrum** / **Model** menu. When loading a
snapshot, if the snapshot needs a different model than the one you have selected, you will be prompted to change to the required model.

## Spectrum Keyboard
All the alphanumeric keys on the Spectrum keyboard, plus `Space` and `Enter`, are mapped directly to the same keys on your keyboard. In addition, several common
Spectrum key combinations are mapped to the dedicated keys on your keyboard so you can type almost naturally and get the expected outcome in the emulated
Spectrum. For example, typing `"` on your keyboard will simulate `Symbol Shift` + `P` in the spectrum, producing a `"`.

Owing to the way the Qt framework handles keyboard events, non-UK/US keyboards may not produce the expected outcome when pressing `Caps Shift` +
`[number key]`.

The special Spectrum keys are:
- `Caps Shift` press `Shift` (left or right)
- `Symbol Shift` press `Left Alt`

## Joysticks
All of the popular Spectrum joystick interfaces are supported, choose the one your game needs from the **Spectrum** / **Joystick Interface** menu.

If you have any joysticks or game controllers attached to your PC they should be automatically detected, and you can choose which one to use from the
**Spectrum** / **Game Controller** menu. The controller you choose will appear to the Spectrum to be connected via the joystick interface you choose.

There is also a **Keyboard** game controller, which maps the cursor keys and the `CTRL` key to the joystick directions and fire button respectively.

## Sound
Sound is not yet emulated.

## Cassette Tapes
Support for loading from .tap cassette images is a work in progress that is not present in this release. Support for
saving to .tap images, and support for .tzx images, are not currently planned.

## Microdrives
Emulation of ZX Inteface 1, and therefore support for microdrives, is being considered.
