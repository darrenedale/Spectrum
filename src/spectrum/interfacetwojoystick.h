//
// Created by darren on 13/03/2021.
//

#ifndef SPECTRUM_INTERFACETWOJOYSTICK_H
#define SPECTRUM_INTERFACETWOJOYSTICK_H

#include "joystickinterface.h"
#include "../z80/types.h"

namespace Spectrum
{
    class InterfaceTwoJoystick
    : public JoystickInterface
    {
    public:
        static constexpr int supportedJoysticks()
        {
            return 2;
        }

        [[nodiscard]] int availableJoysticks() const override;

        [[nodiscard]] bool checkReadPort(::Z80::UnsignedWord port) const override
        {
            // IF2 responds to reads on any even port when the keyboard half-row is one of the halves of the top row
            // (because it's mapped to the keys 1-5 and 6-0)
            // row is in high byte:
            // - half-row 6-0, bit 4 is clear (= ~0x10)
            // - half-row 1-5, bit 3 is clear (= ~0x08)
            return !(port & 0x0001) && (0 == (port & 0x0800) || 0 == (port & 0x1000));
        }

        Z80::UnsignedByte readByte(::Z80::UnsignedWord port) override;
    };
}

#endif //SPECTRUM_INTERFACETWOJOYSTICK_H
