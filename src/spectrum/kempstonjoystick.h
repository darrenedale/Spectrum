//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_KEMPSTONJOYSTICK_H
#define SPECTRUM_KEMPSTONJOYSTICK_H

#include "joystickinterface.h"

namespace Spectrum
{
    class KempstonJoystick
    : public JoystickInterface
    {
    public:
        static constexpr int supportedJoysticks()
        {
            return 1;
        }

        [[nodiscard]] int availableJoysticks() const override
        {
            return 1;
        }

        [[nodiscard]] bool checkReadPort(Z80::UnsignedWord port) const override
        {
            return 31 == (port & 0x00ff);
        }

        Z80::UnsignedByte readByte(Z80::UnsignedWord port) override;
    };
}

#endif //SPECTRUM_KEMPSTONJOYSTICK_H
