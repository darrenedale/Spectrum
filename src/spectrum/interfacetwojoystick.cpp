//
// Created by darren on 13/03/2021.
//

#include "interfacetwojoystick.h"

using namespace Spectrum;

int InterfaceTwoJoystick::availableJoysticks() const
{
    // TODO properly implement this based on config
    return 2;
}

Z80::UnsignedByte InterfaceTwoJoystick::readByte(Z80::UnsignedWord port)
{
    Z80::UnsignedByte result = 0xff;

    if (0 == (port & 0x1000)) {
        // read first joystick port
        result &= joystick1IsLeft() ? 0b11101111 : 0xff;            // key 6
        result &= joystick1IsRight() ? 0b11110111 : 0xff;           // key 7
        result &= joystick1IsDown() ? 0b11111011 : 0xff;            // key 8
        result &= joystick1IsUp() ? 0b11111101 : 0xff;              // key 9
        result &= joystick1Button1IsPressed() ? 0b11111110 : 0xff;  // key 0
    }

    if (0 == (port & 0x0800)) {
        // read second joystick port
        result &= joystick2Button1IsPressed() ? 0b11101111 : 0xff;  // key 5
        result &= joystick2IsUp() ? 0b11110111 : 0xff;              // key 4
        result &= joystick2IsDown() ? 0b11111011 : 0xff;            // key 3
        result &= joystick2IsRight() ? 0b11111101 : 0xff;           // key 2
        result &= joystick2IsLeft() ? 0b11111110 : 0xff;            // key 1
    }

    return result;
}
