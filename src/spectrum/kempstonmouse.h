//
// Created by darren on 04/04/2021.
//

#ifndef SPECTRUM_KEMPSTONMOUSE_H
#define SPECTRUM_KEMPSTONMOUSE_H

#include "mouseinterface.h"

namespace Spectrum
{
    /**
     * Implementation of the Kempston mouse interface.
     *
     * The mouse has two buttons. Currently, button 1 is hard-coded as the left button, 2 as the right.
     */
    class KempstonMouse
    : public MouseInterface
    {
    public:
        /**
         * Check whether the mouse interface responds to a given port address.
         *
         * The kempston mouse interface responds to the following addresses:
         * - bit 0 set, bits 5 and 8 clear (reads the buttons)
         * - bits 0 and 8 set, bits 5 and 10 clear (reads the x-position)
         * - bits 0, 8 and 10 set, bit 5 clear (reads the y-position)
         *
         * @param port The port to check.
         * @return
         */
        [[nodiscard]] bool checkReadPort(::Z80::UnsignedWord port) const override;

        /**
         * Read a byte from the mouse interface.
         *
         * See checkReadPort() for the ports on which the interface responds.
         *
         * When reading the buttons, the read byte has all bits set, except:
         * - bit 0 is clear if button 1 is currently pressed
         * - bit 1 is clear if button 2 is currently pressed
         * (Note, both bits will be clear if both buttons are currently pressed.)
         *
         * When reading the x-position, the read byte contains the position from 0 (leftmost screen pixel, not including the border) to 255 (rightmost screen
         * pixel not including the border).
         *
         * When reading the y-position, the read byte contains the position from 0 (bottom screen pixel, not including the border) to 192 (top screen pixel not
         * including the border).
         *
         * @param port The port to read.
         *
         * @return
         */
        ::Z80::UnsignedByte readByte(::Z80::UnsignedWord port) override;
    };
}

#endif //SPECTRUM_KEMPSTONMOUSE_H
