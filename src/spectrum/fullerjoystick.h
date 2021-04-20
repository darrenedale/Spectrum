//
// Created by darren on 05/03/2021.
//

#ifndef SPECTRUM_FULLERJOYSTICK_H
#define SPECTRUM_FULLERJOYSTICK_H

#include "joystickinterface.h"

namespace Spectrum
{
    class FullerJoystick
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

        [[nodiscard]] bool checkReadPort(::Z80::UnsignedWord port) const override
        {
            return 0x7f == (port & 0x00ff);
        }

        ::Z80::UnsignedByte readByte(::Z80::UnsignedWord port) override;
    };
}

#endif //SPECTRUM_FULLERJOYSTICK_H
