//
// Created by darren on 31/03/2021.
//

#include <cassert>
#include <string>

#include "types.h"

using namespace std::string_literals;

using Z80::InterruptMode;

std::string Z80::to_string(const Register16 & reg)
{
    switch (reg) {
        case Register16::AF:
            return "AF"s;

        case Register16::BC:
            return "BC"s;

        case Register16::DE:
            return "DE"s;

        case Register16::HL:
            return "HL"s;

        case Register16::IX:
            return "IX"s;

        case Register16::IY:
            return "IY"s;

        case Register16::SP:
            return "SP"s;

        case Register16::PC:
            return "PC"s;

        case Register16::AFShadow:
            return "AF'"s;

        case Register16::BCShadow:
            return "BC'"s;

        case Register16::DEShadow:
            return "DE'"s;

        case Register16::HLShadow:
            return "HL'"s;
    }

    // unreachable code - someone's added a 16-bit register and not updated the function or type-punned an invalid value
    // to a register
    [[unlikely]]
    assert(false);
    return {};
}

std::string Z80::to_string(const Register8 & reg)
{
    switch (reg) {
        case Register8::A:
            return "A"s;

        case Register8::F:
            return "F"s;

        case Register8::B:
            return "B"s;

        case Register8::C:
            return "C"s;

        case Register8::D:
            return "D"s;

        case Register8::E:
            return "E"s;

        case Register8::H:
            return "H"s;

        case Register8::L:
            return "L"s;

        case Register8::IXH:
            return "IXH"s;

        case Register8::IXL:
            return "IXL"s;

        case Register8::IYH:
            return "IYH"s;

        case Register8::IYL:
            return "IYL"s;

        case Register8::I:
            return "I"s;

        case Register8::R:
            return "R"s;

        case Register8::AShadow:
            return "A'"s;

        case Register8::FShadow:
            return "F'"s;

        case Register8::BShadow:
            return "B'"s;

        case Register8::CShadow:
            return "C'"s;

        case Register8::DShadow:
            return "D'"s;

        case Register8::EShadow:
            return "E'"s;

        case Register8::HShadow:
            return "H'"s;

        case Register8::LShadow:
            return "L'"s;
    }

    // unreachable code - someone's added an 8-bit register and not updated the function or type-punned an invalid value
    // to a register
    [[unlikely]]
    assert(false);
    return {};
}

std::string Z80::to_string(const InterruptMode & im)
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
    [[unlikely]]
    assert(false);
    return {};
}
