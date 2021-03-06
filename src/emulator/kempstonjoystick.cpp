//
// Created by darren on 05/03/2021.
//

#include "kempstonjoystick.h"

Z80::UnsignedByte Spectrum::KempstonJoystick::readByte(Z80::UnsignedWord port)
{
    // bit 0 is right
    // bit 1 is left
    // bit 2 is down
    // bit 3 is up
    // bit 4 is button
    // bit 6 is button 2
    // bit 7 is button 3

    // TODO configurable mappings
    Z80::UnsignedByte value = 0x00;

    if (joystickIsLeft()) {
        value |= 0b00000010;
    } else if (joystickIsRight()) {
        value |= 0b00000001;
    }

    if (joystickIsUp()) {
        value |= 0b00001000;
    } else if (joystickIsDown()) {
        value |= 0b00000100;
    }

    if (button1IsPressed()) {
        value |= 0b00010000;
    }

    if (button2IsPressed()) {
        value |= 0b01000000;
    }

    if (button3IsPressed()) {
        value |= 0b10000000;
    }

    return value;
}
