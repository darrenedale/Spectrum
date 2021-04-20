//
// Created by darren on 05/03/2021.
//

#include "fullerjoystick.h"

Z80::UnsignedByte Spectrum::FullerJoystick::readByte(Z80::UnsignedWord port)
{
    // bit 0 is up
    // bit 1 is down
    // bit 2 is left
    // bit 3 is right
    // bit 7 is button 1

    Z80::UnsignedByte value = 0xff;

    if (joystick1IsUp()) {
        value ^= 0b00000001;
    } else if (joystick1IsDown()) {
        value ^= 0b00000010;
    }

    if (joystick1IsLeft()) {
        value ^= 0b00000100;
    } else if (joystick1IsRight()) {
        value ^= 0b00001000;
    }

    if (joystick1Button1IsPressed()) {
        value |= 0b10000000;
    }

    return value;
}
