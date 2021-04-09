//
// Created by darren on 04/04/2021.
//

#include <iostream>
#include <iomanip>
#include "kempstonmouse.h"
#include "../z80/types.h"

using namespace Spectrum;
using ::Z80::UnsignedWord;
using ::Z80::UnsignedByte;

namespace
{
    // to read butons, bit 0 must be set high, bits 5 and 8 set low, anything else is irrelevant
    constexpr const UnsignedWord ButtonsReadPortMask = 0b00000001'00100001;
//     constexpr const UnsignedWord ButtonsReadPort = 0b00000000'00000001;
    constexpr const UnsignedWord ButtonsReadPort = 64223;

    // to read X pos, bits 0 and 8 must be set high, bits 5 and 10 set low, anything else is irrelevant    
    constexpr const UnsignedWord XReadPortMask = 0b00000101'00100001;
//    constexpr const UnsignedWord XReadPort = 0b00000001'00000001;
    constexpr const UnsignedWord XReadPort = 64479;

    // to read Y pos, bits 0, 8 and 10 must be set high, bit 5 set low, anything else is irrelevant    
    constexpr const UnsignedWord YReadPortMask = 0b00000101'00100001;
//    constexpr const UnsignedWord YReadPort = 0b00000101'00000001;
    constexpr const UnsignedWord YReadPort = 65503;

    // which bits are set low in the returned byte if the button is pressed
    constexpr const std::uint8_t LeftButtonBit = 0;
    constexpr const std::uint8_t RightButtonBit = 1;

    // does it have a middle button?
//    constexpr const std::uint8_t MiddleButtonBit = 2;
}

bool KempstonMouse::checkReadPort(UnsignedWord port) const
{
#if (!defined(NDEBUG))
//     if (ButtonsReadPort == (port & ButtonsReadPortMask)
//         || XReadPort == (port & XReadPortMask)
//         || YReadPort == (port & YReadPortMask)) {
    if (ButtonsReadPort == port
        || XReadPort == port
        || YReadPort == port) {
        std::cout << "kempston mouse responds to port " << port << '\n';
    }
#endif
    return ButtonsReadPort == port
        || XReadPort == port
        || YReadPort == port;
//     return ButtonsReadPort == (port & ButtonsReadPortMask)
//         || XReadPort == (port & XReadPortMask)
//         || YReadPort == (port & YReadPortMask);
}

UnsignedByte KempstonMouse::readByte(UnsignedWord port)
{
    UnsignedByte ret = 0xff;

    if (ButtonsReadPort == (port /*& ButtonsReadPortMask*/)) {
        if (button1Pressed()) {
#if (!defined(NDEBUG))
            std::cout << "reporting mouse button 1 pressed\n";
#endif
            ret ^= (1 << LeftButtonBit);
        }

        if (button2Pressed()) {
#if (!defined(NDEBUG))
            std::cout << "reporting mouse button 2 pressed\n";
#endif
            ret ^= (1 << RightButtonBit);
        }

#if (!defined(NDEBUG))
            std::cout << "buttons: " << std::hex << std::setfill('0') << std::setw(2) << static_cast<std::uint16_t>(ret) << std::dec << std::setfill(' ') << '\n';
#endif

//        if (button3Pressed()) {
//            ret ^= (1 << MiddleButtonBit);
//        }
    } else if (XReadPort == (port /*& XReadPortMask*/)) {
        ret = static_cast<std::uint8_t>(x());
#if (!defined(NDEBUG))
            std::cout << "x: " << static_cast<std::uint16_t>(ret) << '\n';
#endif
    } else if (YReadPort == (port /*& YReadPortMask*/)) {
        // TODO bound to 0-192?
        ret = static_cast<std::uint8_t>(y());
#if (!defined(NDEBUG))
            std::cout << "y: " << static_cast<std::uint16_t>(ret) << '\n';
#endif
    }

    return ret;
}
