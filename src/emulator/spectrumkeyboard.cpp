//
// Created by darren on 05/03/2021.
//

#include "spectrumkeyboard.h"

using namespace Spectrum;

namespace
{
    // a mask to mask out where the 0 bit should be in the high byte of the port for each keyboard half-row
    constexpr const Z80::UnsignedByte HalfRowCsV = 0b00000001;
    constexpr const Z80::UnsignedByte HalfRowAG = 0b00000010;
    constexpr const Z80::UnsignedByte HalfRowQT = 0b00000100;
    constexpr const Z80::UnsignedByte HalfRow15 = 0b00001000;
    constexpr const Z80::UnsignedByte HalfRow60 = 0b00010000;
    constexpr const Z80::UnsignedByte HalfRowYP = 0b00100000;
    constexpr const Z80::UnsignedByte HalfRowHEnter = 0b01000000;
    constexpr const Z80::UnsignedByte HalfRowBSpace = 0b10000000;
}

SpectrumKeyboard::SpectrumKeyboard()
: Z80::Z80IODevice(),
  m_state(0)
{
}

bool SpectrumKeyboard::checkReadPort(Z80::UnsignedWord port) const
{
    // keyboard responds to reads on any even port
    return !(port & 0x0001);
}

// NOTE we receive the LAST key (the one that goes in bit 4) so that we can loop easily, exiting when the mask is 0
Z80::UnsignedByte SpectrumKeyboard::readHalfRow(Key lastKey) const
{
    Z80::UnsignedByte result = 0b00011111;
    Z80::UnsignedByte mask = 0b00010000;

    for (auto key = static_cast<std::uint8_t>(lastKey); mask; --key, mask >>= 1) {
        if (keyState(static_cast<Key>(key))) {
            result &= ~mask;
        }
    }

    return result;
}

// NOTE we receive the LAST key (the one that goes in bit 4) so that we can loop easily, exiting when the mask is 0
Z80::UnsignedByte SpectrumKeyboard::readHalfRowReverse(Key lastKey) const
{
    Z80::UnsignedByte result = 0b00011111;
    Z80::UnsignedByte mask = 0b00010000;

    for (auto key = static_cast<int>(lastKey); mask; ++key, mask >>= 1) {
        if (keyState(static_cast<Key>(key))) {
            result &= ~mask;
        }
    }

    return result;
}

Z80::UnsignedByte SpectrumKeyboard::readByte(Z80::UnsignedWord port)
{
    Z80::UnsignedByte result = 0b00011111;
    Z80::UnsignedByte halfRows = (port & 0xff00) >> 8;

    if (0xff == halfRows) {
        // no half-rows selected
        return result;
    }

    // for each half-row the keys are represented by bits 0 to 4 in the returned byte. on the right side of the
    // keyboard, the keys are represented from right-to-left (e.g. P is bit 0, O is bit 1, I is bit 2, ...); on the left
    // side the keys are from left-to-right (e.g. Q is in bit 0, W is in bit 1, E is in bit 2, ...).
    Z80::UnsignedByte halfRowMask = 0b00000001;

    while (halfRowMask) {
        // the bit in the port is 0 if it is being queried
        if (!(halfRows & halfRowMask)) {
            switch (halfRowMask) { // NOLINT(hicpp-multiway-paths-covered)
                case HalfRowCsV:
                    result &= readHalfRow(Key::V);
                    break;

                case HalfRowAG:
                    result &= readHalfRow(Key::G);
                    break;

                case HalfRowQT:
                    result &= readHalfRow(Key::T);
                    break;

                case HalfRow15:
                    result &= readHalfRow(Key::Num5);
                    break;

                case HalfRow60:
                    result &= readHalfRowReverse(Key::Num6);
                    break;

                case HalfRowYP:
                    result &= readHalfRowReverse(Key::Y);
                    break;

                case HalfRowHEnter:
                    result &= readHalfRowReverse(Key::H);
                    break;

                case HalfRowBSpace:
                    result &= readHalfRowReverse(Key::B);
                    break;
            }
        }

        halfRowMask <<= 1;
    }

    return result;
}
