//
// Created by darren on 13/03/2021.
//

#include "cursorjoystick.h"

using namespace Spectrum;

Z80::UnsignedByte CursorJoystick::readByte(Z80::UnsignedWord port)
{
    Z80::UnsignedByte result = 0xff;

    if (0 == (port & 0x1000)) {
        result &= joystick1IsDown() ? 0b11101111 : 0xff;            // key 6
        result &= joystick1IsUp() ? 0b11110111 : 0xff;              // key 7
        result &= joystick1IsRight() ? 0b11111011 : 0xff;           // key 8
        result &= joystick1Button1IsPressed() ? 0b11111110 : 0xff;  // key 0
    }

    if (0 == (port & 0x0800)) {
        result &= joystick1IsLeft() ? 0b11101111 : 0xff;            // key 5
    }

    if (0 == (port & 0x0100)) {
        // indicate caps-shift is pressed if the joystick is not in the neutral position
        result &= (joystick1IsLeft() | joystick1IsRight() | joystick1IsUp() | joystick1IsDown()) ? 0b11111110 : 0xff;
    }

    return result;
}
