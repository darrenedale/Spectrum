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

    Z80::UnsignedByte value = 0x00;

    if (joystick1IsLeft()) {
        value |= 0b00000010;
    } else if (joystick1IsRight()) {
        value |= 0b00000001;
    }

    if (joystick1IsUp()) {
        value |= 0b00001000;
    } else if (joystick1IsDown()) {
        value |= 0b00000100;
    }

    if (joystick1Button1IsPressed()) {
        value |= 0b00010000;
    }

    if (joystick1Button2IsPressed()) {
        value |= 0b01000000;
    }

    if (joystick1Button3IsPressed()) {
        value |= 0b10000000;
    }

    return value;
}
