//
// Created by darren on 04/04/2021.
//

#ifndef SPECTRUM_KEMPSTONMOUSE_H
#define SPECTRUM_KEMPSTONMOUSE_H

#include "mouseinterface.h"

namespace Spectrum
{
    /**
     * Button 1 is left, 2 is right, 3 is middle
     */
    class KempstonMouse
    : public MouseInterface
    {
    public:

        [[nodiscard]] bool checkReadPort(::Z80::UnsignedWord port) const override;

        ::Z80::UnsignedByte readByte(::Z80::UnsignedWord port) override;
    };
}

#endif //SPECTRUM_KEMPSTONMOUSE_H
