//
// Created by darren on 31/03/2021.
//

#include <cassert>
#include "types.h"

using ::Z80::InterruptMode;

std::string std::to_string(const Z80::Register16 & reg)
{
    switch (reg) {
        case Z80::Register16::AF:
            return "AF"s;

        case Z80::Register16::BC:
            return "BC"s;

        case Z80::Register16::DE:
            return "DE"s;

        case Z80::Register16::HL:
            return "HL"s;

        case Z80::Register16::IX:
            return "IX"s;

        case Z80::Register16::IY:
            return "IY"s;

        case Z80::Register16::SP:
            return "SP"s;

        case Z80::Register16::PC:
            return "PC"s;

        case Z80::Register16::AFShadow:
            return "AF'"s;

        case Z80::Register16::BCShadow:
            return "BC'"s;

        case Z80::Register16::DEShadow:
            return "DE'"s;

        case Z80::Register16::HLShadow:
            return "HL'"s;
    }

    // unreachable code - someone's added a 16-bit register and not updated the function or type-punned an invalid value
    // to a register
    assert(false);
    return {};
}

std::string std::to_string(const Z80::Register8 & reg)
{
    switch (reg) {
        case Z80::Register8::A:
            return "A"s;

        case Z80::Register8::F:
            return "F"s;

        case Z80::Register8::B:
            return "B"s;

        case Z80::Register8::C:
            return "C"s;

        case Z80::Register8::D:
            return "D"s;

        case Z80::Register8::E:
            return "E"s;

        case Z80::Register8::H:
            return "H"s;

        case Z80::Register8::L:
            return "L"s;

        case Z80::Register8::IXH:
            return "IXH"s;

        case Z80::Register8::IXL:
            return "IXL"s;

        case Z80::Register8::IYH:
            return "IYH"s;

        case Z80::Register8::IYL:
            return "IYL"s;

        case Z80::Register8::I:
            return "I"s;

        case Z80::Register8::R:
            return "R"s;

        case Z80::Register8::AShadow:
            return "A'"s;

        case Z80::Register8::FShadow:
            return "F'"s;

        case Z80::Register8::BShadow:
            return "B'"s;

        case Z80::Register8::CShadow:
            return "C'"s;

        case Z80::Register8::DShadow:
            return "D'"s;

        case Z80::Register8::EShadow:
            return "E'"s;

        case Z80::Register8::HShadow:
            return "H'"s;

        case Z80::Register8::LShadow:
            return "L'"s;
    }

    // unreachable code - someone's added an 8-bit register and not updated the function or type-punned an invalid value
    // to a register
    assert(false);
    return {};
}

std::string std::to_string(const InterruptMode & im)
{
    switch (im) {
        case InterruptMode::IM0:
            return "IM0"s;

        case InterruptMode::IM1:
            return "IM1"s;

        case InterruptMode::IM2:
            return "IM2"s;
    }
    
    // unreachable code - someone's added a mode and not updated the function or type-punned an invalid value to a mode
    assert(false);
    return {};
}
