//
// Created by darren on 13/03/2021.
//

#ifndef SPECTRUM_CURSORJOYSTICK_H
#define SPECTRUM_CURSORJOYSTICK_H

#include "joystickinterface.h"
#include "../z80/types.h"

namespace Spectrum
{
    class CursorJoystick
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
            // cursor joystick reports arrow keys (CS+5-8) as being pressed when the joystick is moved. it therefore
            // responds to reads on any even port when the keyboard half-row is one of the halves of the top row
            // (keys 5-8) or when it's the left half of the bottom row (caps-shift)
            // port high byte:
            // - half-row 6-0, bit 4 is clear (= ~0x10)
            // - half-row 1-5, bit 3 is clear (= ~0x08)
            // - half-row CS-V, bit 0 is clear (= ~0x01)
            return !(port & 0x0001) && (0 == (port & 0x0800) || 0 == (port & 0x1000) || 0 == (port & 0x0100));
        }

        Z80::UnsignedByte readByte(::Z80::UnsignedWord port) override;
    };
}

#endif //SPECTRUM_CURSORJOYSTICK_H
