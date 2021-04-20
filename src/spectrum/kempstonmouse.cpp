//
// Created by darren on 04/04/2021.
//

#include <iostream>
#include <iomanip>
#include "kempstonmouse.h"
#include "../z80/types.h"
#include "../util/debug.h"

using namespace Spectrum;
using ::Z80::UnsignedWord;
using ::Z80::UnsignedByte;

namespace
{
    // to read buttons, bit 0 must be set high, bits 5 and 8 set low, anything else is irrelevant
    constexpr const UnsignedWord ButtonsReadPortMask = 0b00000001'00100001;
     constexpr const UnsignedWord ButtonsReadPort = 0b00000000'00000001;

    // to read X pos, bits 0 and 8 must be set high, bits 5 and 10 set low, anything else is irrelevant    
    constexpr const UnsignedWord XReadPortMask = 0b00000101'00100001;
    constexpr const UnsignedWord XReadPort = 0b00000001'00000001;

    // to read Y pos, bits 0, 8 and 10 must be set high, bit 5 set low, anything else is irrelevant    
    constexpr const UnsignedWord YReadPortMask = 0b00000101'00100001;
    constexpr const UnsignedWord YReadPort = 0b00000101'00000001;

    // which bits are set low in the returned byte if the button is pressed
    constexpr const std::uint8_t LeftButtonBit = 0;
    constexpr const std::uint8_t RightButtonBit = 1;

    // does it have a middle button?
//    constexpr const std::uint8_t MiddleButtonBit = 2;
}

bool KempstonMouse::checkReadPort(UnsignedWord port) const
{
     return ButtonsReadPort == (port & ButtonsReadPortMask)
         || XReadPort == (port & XReadPortMask)
         || YReadPort == (port & YReadPortMask);
}

UnsignedByte KempstonMouse::readByte(UnsignedWord port)
{
    UnsignedByte ret = 0xff;

    if (ButtonsReadPort == (port & ButtonsReadPortMask)) {
        if (button1Pressed()) {
            ret ^= (1 << LeftButtonBit);
        }

        if (button2Pressed()) {
            ret ^= (1 << RightButtonBit);
        }

//        if (button3Pressed()) {
//            ret ^= (1 << MiddleButtonBit);
//        }
    } else if (XReadPort == (port & XReadPortMask)) {
        ret = static_cast<std::uint8_t>(x());
    } else if (YReadPort == (port & YReadPortMask)) {
        ret = static_cast<std::uint8_t>(y());
    }

    return ret;
}
