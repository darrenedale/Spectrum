//
// Created by darren on 13/03/2021.
//

#ifndef SPECTRUM_CURSORJOYSTICK_H
#define SPECTRUM_CURSORJOYSTICK_H

#include "joystickinterface.h"
#include "../z80/types.h"

namespace Spectrum
{
    /**
     * Spectrum cursor joystick device.
     *
     * Cursor joystick interfaces simulate presses on the key combinations for the cursor keys in response to joystick
     * movements. The fire button simulates a press on the 0 key.
     */
    class CursorJoystick
    : public JoystickInterface
    {
    public:
        /**
         * There is only one set of cursor keys, so only a single joystick can be attached to a cursor interface.
         *
         * @return 1.
         */
        static constexpr int supportedJoysticks()
        {
            return 1;
        }

        /**
         * The number of attached joysticks.
         *
         * There is currently always a single attached joystick - attaching and detaching joysticks is not currently
         * supported, but this functionality is reserved for future use.
         *
         * @return 1.
         */
        [[nodiscard]] int availableJoysticks() const override
        {
            return 1;
        }

        /**
         * Interfaces respond to ports that correspond to reads of the simulated key combinations.
         *
         * @param port The port the Z80 is preparing to read.
         *
         * @return true if the port corresponds to one of the keyboard half-rows of interest, false otherwise.
         */
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

        /**
         * Read from the device.
         *
         * The device responds the same way as the Spectrum keyboard for the simulated keys, depending upon the state of
         * the joystick.
         *
         * @param port The port to read.
         * @return
         */
        Z80::UnsignedByte readByte(::Z80::UnsignedWord port) override;
    };
}

#endif //SPECTRUM_CURSORJOYSTICK_H
