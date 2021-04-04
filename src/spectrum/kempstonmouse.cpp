//
// Created by darren on 04/04/2021.
//

#include "kempstonmouse.h"
#include "../z80/types.h"

using namespace Spectrum;
using ::Z80::UnsignedWord;
using ::Z80::UnsignedByte;

namespace
{
    constexpr const UnsignedWord ButtonsReadPort = 0xfadf;
    constexpr const UnsignedWord XReadPort = 0xfbdf;
    constexpr const UnsignedWord YReadPort = 0xffdf;

    constexpr const std::uint8_t LeftButtonBit = 0;
    constexpr const std::uint8_t RightButtonBit = 1;

    // does it have a middle button?
//    constexpr const std::uint8_t MiddleButtonBit = 2;
}

bool KempstonMouse::checkReadPort(UnsignedWord port) const
{
    return ButtonsReadPort == port || XReadPort == port || YReadPort == port;
}

UnsignedByte KempstonMouse::readByte(UnsignedWord port)
{
    UnsignedByte ret = 0xff;

    switch (port) {
        case ButtonsReadPort:
            if (button1Pressed()) {
                ret ^= (1 << LeftButtonBit);
            }

            if (button2Pressed()) {
                ret ^= (1 << RightButtonBit);
            }
//
//            if (button3Pressed()) {
//                ret ^= (1 << MiddleButtonBit);
//            }
            break;

        case XReadPort:
            // TODO bound to 0-256?
            ret = static_cast<std::uint8_t>(x());
            break;

        case YReadPort:
            // TODO bound to 0-192?
            ret = static_cast<std::uint8_t>(y());
            break;

        default:
            // nothing to do
            break;
    }

    return ret;
}
